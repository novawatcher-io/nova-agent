#pragma once

extern "C" {
#include <sys/stat.h>
#include <unistd.h>
}

#include <cerrno>
#include <fstream>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <memory>

namespace App::Common {

struct FileReader {
    virtual bool ReadFile(const std::string& path, std::string* content) = 0;
    virtual bool ReadFileLink(const std::string& path, std::string* content) = 0;
    virtual ~FileReader() = default;
};

/* A mount table entry. */
struct mount_entry
{
    std::string me_devname;             /* Device node name, including "/dev/". */
    std::string me_mountdir;            /* Mount point directory name. */
    std::string me_mntroot;             /* Directory on filesystem of device used */
    /* as root for the (bind) mount. */
    std::string me_type;                /* "nfs", "4.2", etc. */
    dev_t me_dev;                 /* Device number of me_mountdir. */
    unsigned int me_dummy : 1;    /* Nonzero for dummy file systems. */
    unsigned int me_remote : 1;   /* Nonzero for remote file systems. */
    unsigned int me_type_malloced : 1; /* Nonzero if me_type was malloced. */
    // 大小
    uint64_t size = 0;
    // 已用
    uint64_t used = 0;
    // 可用
    uint64_t available = 0;
    std::unique_ptr<mount_entry> me_next;
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

    std::unique_ptr<mount_entry> ReadFileSystemList (bool need_fs_type);
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
