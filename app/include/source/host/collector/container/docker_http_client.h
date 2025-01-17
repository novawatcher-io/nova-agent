#pragma once
#include <array>
#include <atomic>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>
#include "source/host/collector/container/container_info.h"

namespace App::Source::Host::Collector::Container {

class HTTPRequest {
public:
    struct CallbackWrapper {
        std::function<void(int, std::string)> callback;
    };
    enum Method : uint8_t {
        GET = 1,
        POST,
    };

    HTTPRequest(Method method, std::string url) : method_(method), url_(std::move(url)) {
        header_["Host"] = "localhost";
        header_["User-Agent"] = "Trace-Agent/0.1";
    }

    void AddHeader(std::string key, std::string value) {
        header_[key] = value;
    }

    void AddBody(std::string body) {
        body_ = std::move(body);
    }

    struct evhttp_request* CreateRequest(std::function<void(int, std::string)> callback) const {
        auto private_data = new CallbackWrapper{std::move(callback)};

        auto request_ = evhttp_request_new(ResponseCallback, private_data);
        if (!request_) {
            SPDLOG_ERROR("Could not create HTTP request!");
            return nullptr;
        }
        auto header = evhttp_request_get_output_headers(request_);
        for (auto [key, value] : header_) {
            evhttp_add_header(header, key.c_str(), value.c_str());
        }

        if (!body_.empty()) {
            auto output_buffer = evhttp_request_get_output_buffer(request_);
            evbuffer_add(output_buffer, body_.data(), body_.size());
        }
        return request_;
    }

    static void ResponseCallback(struct evhttp_request* req, void* arg) {
        auto* private_data = static_cast<CallbackWrapper*>(arg);
        if (!private_data) {
            SPDLOG_ERROR("Invalid request object");
            return;
        }
        std::unique_ptr<CallbackWrapper> wrapper(private_data);
        assert(wrapper->callback);
        if (!req) {
            SPDLOG_ERROR("Request failed");
            if (private_data->callback) {
                private_data->callback(-1, "");
            }
            return;
        }

        struct evbuffer* evb = evhttp_request_get_input_buffer(req);
        if (!evb) {
            SPDLOG_ERROR("No input buffer");
            return;
        }

        size_t len = evbuffer_get_length(evb);
        if (len > 0  && private_data->callback) {
            std::string response_data((const char*)evbuffer_pullup(evb, len), len);
            private_data->callback(0, std::move(response_data));
        }
    }

    evhttp_cmd_type GetLibeventMethod() const {
        switch (method_) {
        case GET:
            return EVHTTP_REQ_GET;
        case POST:
            return EVHTTP_REQ_POST;
        default:
            assert(!"Invalid method");
        }
        return EVHTTP_REQ_GET;
    }

    std::string GetURI() const {
        struct evhttp_uri* uri = evhttp_uri_parse(url_.c_str());
        if (uri == nullptr) {
            SPDLOG_ERROR("Failed to parse URL: {}", url_);
            return "/";
        }

        const char* path = evhttp_uri_get_path(uri);
        if (path == nullptr || strlen(path) == 0) {
            path = "/";
        }

        const char* query = evhttp_uri_get_query(uri);
        std::string full_path = path;
        if (query != nullptr) {
            full_path += "?";
            full_path += query;
        }

        evhttp_uri_free(uri);
        return full_path;
    }

private:
    Method method_;
    std::string url_;
    std::map<std::string, std::string> header_;
    std::string body_;
};

class HttpClient {
public:
    explicit HttpClient(struct event_base* base) : base_(base) {
    }

    void SetUnixDomainSocket(std::string socket) {
        unix_socket_ = std::move(socket);
        if (unix_socket_.empty()) {
            return;
        }

        // release conn_ if it is not null
        if (conn_) {
            evhttp_connection_free(conn_);
            conn_ = nullptr;
        } else {
            conn_ = evhttp_connection_base_bufferevent_unix_new(base_, nullptr, unix_socket_.c_str());
        }
    }

    // Request will take ownership of the request object
    int Request(const HTTPRequest& request, std::function<void(int, std::string)> callback) {
        if (!conn_) {
            SPDLOG_ERROR("Connection is not initialized");
            return -1;
        }
        auto ev_request = request.CreateRequest(std::move(callback));

        auto ret = evhttp_make_request(conn_, ev_request, request.GetLibeventMethod(), request.GetURI().c_str());
        if (ret != 0) {
            SPDLOG_ERROR("evhttp_make_request failed: {}", ret);
            return -1;
        }

        return 0;
    }

private:
    std::string unix_socket_;
    struct evhttp_connection* conn_ = nullptr;
    struct event_base* base_ = nullptr;
};



class DockerHttpClient {
public:
    DockerHttpClient(struct event_base* base, const std::string& unix_domain_socket,
                     std::function<void(std::vector<ContainerInfo>)> callback)
        : http_client_(base),
          callback_(std::move(callback)),
          pending_tasks_(0) {
        if (!IsUnixDomainSocket(unix_domain_socket.c_str())) {
            SPDLOG_ERROR("{} is not a valid unix domain socket.");
            return;
        }
        unix_domain_socket_ = unix_domain_socket;
        SPDLOG_INFO("Docker unix domain socket: {}", unix_domain_socket_);
        http_client_.SetUnixDomainSocket(unix_domain_socket_);
    }

    static bool IsUnixDomainSocket(const char* path) {
        struct stat statbuf{};
        if (stat(path, &statbuf) != 0) {
            return false;
        }
        return S_ISSOCK(statbuf.st_mode);
    }

    void ProbeUnixSocket() {
        std::array<const char*, 2> addr = {"/run/docker.sock", "/var/run/docker.sock"};
        for (const char* path : addr) {
            if (IsUnixDomainSocket(path)) {
                http_client_.SetUnixDomainSocket(path);
                SPDLOG_INFO("found unix domain socket: {}", path);
                return;
            }
        }
        SPDLOG_ERROR("No docker socket found");
    }

    void AsyncGetContainerList() {
        if (unix_domain_socket_.empty()) {
            return;
        }
        http_client_.SetUnixDomainSocket(unix_domain_socket_);
        HTTPRequest request(HTTPRequest::Method::GET, "http://localhost/containers/json");
        http_client_.Request(request, [this](int reason, std::string response) {
            if (reason == 0) {
                ParseContainerList(std::move(response));
            } else {
                SPDLOG_ERROR("request failed");
            }
        });
    }

    void GetContainerDetail(const std::string& container_id) {
        std::string const url = "http://localhost/containers/" + container_id + "/json";
        HTTPRequest const request(HTTPRequest::Method::GET, url);
        http_client_.Request(request, [this, container_id](int reason, std::string response) {
            if (reason != 0) {
                SPDLOG_ERROR("request failed");
                return;
            }
            ParseContainerDetail(std::move(response), container_info_[container_id]);
            {
                std::lock_guard<std::mutex> lock(mtx_);
                --pending_tasks_;
                if (pending_tasks_ == 0) {
                    std::vector<ContainerInfo> result;
                    for (const auto& [id, info] : container_info_) {
                        result.push_back(info);
                    }
                    callback_(std::move(result));
                }
            }
        });
    }

    template <typename... T> //
    bool ParseFromJson(const rapidjson::Value& body, std::string_view key, std::variant<T...> target,
                       bool required = false) {
        auto it = body.FindMember(key.data());
        if (it == body.MemberEnd()) {
            return !required;
        }
        const auto& value = it->value;
        return std::visit(
            [&value](auto arg) -> bool {
                using UnderlyingType = std::remove_pointer_t<decltype(arg)>;
                // std::string is a special case
                if constexpr (std::is_same_v<UnderlyingType, std::string>) {
                    if (!value.IsString()) {
                        return false;
                    }
                    *arg = value.GetString();
                } else {
                    if (!value.Is<UnderlyingType>()) {
                        return false;
                    }
                    *arg = value.Get<UnderlyingType>();
                }
                return true;
            },
            target);
    }

    void ParseContainerList(std::string response) {
        rapidjson::Document doc;
        if (doc.Parse(response.c_str()).HasParseError()) {
            SPDLOG_ERROR("response is not a valid json: {}", response);
            return;
        }

        if (!doc.IsArray()) {
            SPDLOG_ERROR("response is not an array: {}", response);
            return;
        }

        for (auto* it = doc.Begin(); it != doc.End(); ++it) {
            ContainerInfo info;

            std::vector<std::pair<std::string_view, std::variant<int*, std::string*, int64_t*>>> const parse_list{
                {"Id", &info.id},
                {"State", &info.state},
                {"Command", &info.command},
                {"Created", &info.created},
            };

            auto container = it->GetObj();
            for (auto [key, target] : parse_list) {
                if (!ParseFromJson(container, key, target, true)) {
                    SPDLOG_ERROR("Failed to parse key: {}", key);
                    continue;
                }
            }

            if (auto name = container.FindMember("Names"); name != container.MemberEnd() && name->value.IsArray()) {
                auto names = name->value.GetArray();
                if (!names.Empty()) {
                    info.name = names[0].GetString();
                }
            }
            container_info_[info.id] = info;
            if (info.id.empty()) {
                SPDLOG_ERROR("Empty container id");
                continue;
            }
            // SPDLOG_INFO("get details of {}", info.id);
            {
                std::lock_guard<std::mutex> lock(mtx_);
                ++pending_tasks_;
            }
            GetContainerDetail(info.id);
        }
    }

    bool ParseContainerDetail(std::string response, ContainerInfo& result) {
        if (response.empty()) {
            SPDLOG_ERROR("Empty response");
            return false;
        }
        rapidjson::Document doc;
        if (doc.Parse(response.c_str()).HasParseError()) {
            SPDLOG_ERROR("response is not a valid json: {}", response);
            return false;
        }

        if (!doc.IsObject()) {
            SPDLOG_ERROR("response is not an object: {}", response);
            return false;
        }

        const rapidjson::Value* pid = rapidjson::GetValueByPointer(doc, "/State/Pid");
        if (pid && pid->IsInt()) {
            result.pid = pid->GetInt();
        } else {
            SPDLOG_ERROR("Pid not found or not an integer in response: {}", response);
            return false;
        }
        SPDLOG_DEBUG("Container {} has pid {}", result.id, result.pid);
        return true;
    }

private:
    HttpClient http_client_;
    std::string unix_domain_socket_;
    std::function<void(std::vector<ContainerInfo>)> callback_;
    std::unordered_map<std::string, ContainerInfo> container_info_;

    std::atomic<int> pending_tasks_;
    std::mutex mtx_;
};
} // namespace App::Source::Host::Collector::Container
