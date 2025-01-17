#pragma once

#include <cerrno>
#include <fstream>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace App::Common {

struct FileReader {
    virtual bool ReadFile(const std::string& path, std::string* content) = 0;
    virtual bool ReadFileLink(const std::string& path, std::string* content) = 0;
    virtual ~FileReader() = default;
};

struct BasicFileReader : public FileReader {
    bool ReadFile(const std::string& path, std::string* content) override {
        if (content == nullptr) {
            return false;
        }
        content->clear();

        std::ifstream stat_file(path);
        if (!stat_file.is_open()) {
            SPDLOG_ERROR("Failed to open file: {}", path);
            return false;
        }
        std::stringstream buffer;
        buffer << stat_file.rdbuf();
        content->assign(buffer.str());
        stat_file.close();
        return true;
    }

    bool ReadFileLink(const std::string& path, std::string* content) override {
        if (content == nullptr) {
            return false;
        }
        content->clear();
        std::array<char, PATH_MAX> buffer;
        ssize_t const len = readlink(path.c_str(), buffer.data(), buffer.size());
        if (len == -1) {
            auto reason = errno;
            if (reason == EACCES) {
                SPDLOG_INFO("permission denied for: {}", path);
            } else if (reason == ENOENT) {
                SPDLOG_TRACE("empty link file: {}", path);
            } else {
                SPDLOG_WARN("readlink failed, file: {}, errno={}, message={}", path, reason, strerror(reason));
                return false;
            }
        } else if (len > 0) {
            content->append(buffer.data(), len);
        }
        return true;
    }
};

inline static std::string GetFileContent(const std::string& path) {
    std::string content;
    BasicFileReader reader;
    reader.ReadFile(path, &content);
    return content;
}

inline static bool IsFileExist(const std::string& path) {
    struct stat buf{};
    if (stat(path.c_str(), &buf) != 0) {
        if (errno == ENOENT) {
            SPDLOG_ERROR("File does not exist: {}", path);
        } else if (errno == EACCES) {
            SPDLOG_ERROR("Permission denied to access file: {}", path);
        } else {
            SPDLOG_ERROR("Error checking file existence: {} (errno: {})", path, errno);
        }
        return false;
    }
    return S_ISREG(buf.st_mode);
}
} // namespace App::Common
