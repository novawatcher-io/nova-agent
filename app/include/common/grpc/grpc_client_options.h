#pragma once

#include <algorithm>        // for lexicographical_compare
#include <bits/chrono.h>    // for system_clock
#include <cstddef>          // for size_t
#include <cctype>          // for tolower
#include <grpcpp/channel.h> // for Channel
#include <map>              // for multimap
#include <memory>           // for shared_ptr
#include <string>           // for basic_string, string

namespace App::Common::Grpc {
struct cmp_ic {
    bool operator()(const std::string& s1, const std::string& s2) const {
        return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(),
                                            [](char c1, char c2) { return ::tolower(c1) < ::tolower(c2); });
    }
};
using Headers = std::multimap<std::string, std::string, cmp_ic>;

struct ClientOptions {
    /** The endpoint to export to. */
    std::string endpoint;

    /** Use SSL. */
    bool use_ssl_credentials = false;

    /** CA CERT, path to a file. */
    std::string ssl_credentials_cacert_path;

    /** CA CERT, as a string. */
    std::string ssl_credentials_cacert_as_string;

#ifdef ENABLE_OTLP_GRPC_SSL_MTLS_PREVIEW
    /** CLIENT KEY, path to a file. */
    std::string ssl_client_key_path;

    /** CLIENT KEY, as a string. */
    std::string ssl_client_key_string;

    /** CLIENT CERT, path to a file. */
    std::string ssl_client_cert_path;

    /** CLIENT CERT, as a string. */
    std::string ssl_client_cert_string;
#endif

    /** Export timeout. */
    std::chrono::system_clock::duration timeout;

    /** Additional HTTP headers. */
    Headers metadata;

    /** User agent. */
    std::string user_agent;

    /** max number of threads that can be allocated from this */
    std::size_t max_threads;

    // Concurrent requests
    std::size_t max_concurrent_requests;
};

/**
 * Create gRPC channel from the exporter options.
 */
std::shared_ptr<grpc::Channel> MakeChannel(const ClientOptions& options);
} // namespace App::Common::Grpc
