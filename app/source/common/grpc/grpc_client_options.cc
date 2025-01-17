#include "app/include/common/grpc/grpc_client_options.h"
#include <grpcpp/grpcpp.h>                             // for CreateCustomCh...
#include <grpcpp/resource_quota.h>                     // for ResourceQuota
#include <grpcpp/security/credentials.h>               // for SslCredentials...
#include <grpcpp/support/channel_arguments.h>          // for ChannelArguments
#include <opentelemetry/ext/http/common/url_parser.h>  // for UrlParser
#include <fstream>                                     // for basic_ifstream
#include <iterator>                                    // for istreambuf_ite...
#include <memory>                                      // for allocator, sha...

namespace App {
namespace Common {
namespace Grpc {

static std::string GetFileContents(const char* fpath) {
    std::ifstream finstream(fpath);
    std::string contents;
    contents.assign((std::istreambuf_iterator<char>(finstream)), std::istreambuf_iterator<char>());
    finstream.close();
    return contents;
}

// If the file path is non-empty, returns the contents of the file. Otherwise returns contents.
static std::string GetFileContentsOrInMemoryContents(const std::string& file_path, const std::string& contents) {
    if (!file_path.empty()) {
        return GetFileContents(file_path.c_str());
    }
    return contents;
}

std::shared_ptr<grpc::Channel> MakeChannel(const ClientOptions& options) {
    std::shared_ptr<grpc::Channel> channel;

    //
    // Scheme is allowed in OTLP endpoint definition, but is not allowed for creating gRPC
    // channel. Passing URI with scheme to grpc::CreateChannel could resolve the endpoint to some
    // unexpected address.
    //

    opentelemetry::ext::http::common::UrlParser url(options.endpoint);
    if (!url.success_) {
        //        OTEL_INTERNAL_LOG_ERROR("[OTLP GRPC Client] invalid endpoint: " << options.endpoint);

        return nullptr;
    }

    std::string grpc_target = url.host_ + ":" + std::to_string(static_cast<int>(url.port_));
    grpc::ChannelArguments grpc_arguments;
    grpc_arguments.SetUserAgentPrefix(options.user_agent);

    if (options.max_threads > 0) {
        grpc::ResourceQuota quota;
        quota.SetMaxThreads(static_cast<int>(options.max_threads));
        grpc_arguments.SetResourceQuota(quota);
    }

    if (options.use_ssl_credentials) {
        grpc::SslCredentialsOptions ssl_opts;
        ssl_opts.pem_root_certs = GetFileContentsOrInMemoryContents(options.ssl_credentials_cacert_path,
                                                                    options.ssl_credentials_cacert_as_string);
#if ENABLE_OTLP_GRPC_SSL_MTLS_PREVIEW
        ssl_opts.pem_private_key =
            GetFileContentsOrInMemoryContents(options.ssl_client_key_path, options.ssl_client_key_string);
        ssl_opts.pem_cert_chain =
            GetFileContentsOrInMemoryContents(options.ssl_client_cert_path, options.ssl_client_cert_string);

#endif
        channel = grpc::CreateCustomChannel(grpc_target, grpc::SslCredentials(ssl_opts), grpc_arguments);
    } else {
        channel = grpc::CreateCustomChannel(grpc_target, grpc::InsecureChannelCredentials(), grpc_arguments);
    }

    return channel;
}

}
}
}
