#include "fs_impl.hh"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <pwd.h>
#include <fnmatch.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <print>
#include <stdexcept>
#include <string>

using namespace std;

// NOTE: for fileio with async.cc, there's the posix aio api
// NOTE: there's libeio: http://blog.schmorp.de/2015-07-12-how-to-scan-directories-fast-the-tricks-of-aio_scandir.html
// NOTE: CORBA FTAM-FTP Interworking: CosFileTransfer Module

static string getHomeDirectory() {
    string s;

    char buffer[PATH_MAX];
    struct passwd pwd, *result = NULL;
    if (getpwuid_r(getuid(), &pwd, buffer, PATH_MAX, &result) != 0 || !result) {
        s = "/";
    } else {
        s = pwd.pw_dir;
    }
    return s;
}

static string getCurrentWorkingDirectory() {
    char buffer[PATH_MAX];
    getcwd(buffer, PATH_MAX);
    return string(buffer);
}

FileSystem_impl::FileSystem_impl() { _path = getCurrentWorkingDirectory(); }

CORBA::async<std::string> FileSystem_impl::path() { co_return _path; }

CORBA::async<> FileSystem_impl::path(const std::string_view &path) {
    auto directory = ::opendir(_path.c_str());
    if (!directory) {
        throw runtime_error(format("Directory_impl::path(\"{}\"): {}: {}", path, _path, strerror(errno)));
    }
    closedir(directory);
    _path = path;
    co_return;
}

CORBA::async<std::vector<DirectoryEntry>> FileSystem_impl::list(const std::string_view &filter) {
    vector<DirectoryEntry> directoryEntries;
    println("Directory_impl::list(\"{}\")", filter);

    auto directory = ::opendir(_path.c_str());
    if (!directory) {
        throw runtime_error(format("Directory_impl::list(): {}: {}", _path, strerror(errno)));
    }

    vector<string> filenames;
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        filenames.push_back(string(entry->d_name, entry->d_namlen));        
    }
    closedir(directory);

    sort(filenames.begin(), filenames.end(), [](const std::string &a, const std::string &b) -> bool {
        return strcasecmp(a.c_str(), b.c_str()) < 0;
    });

    for (auto &filename : filenames) {
        struct stat s;
        if (stat(format("{}/{}", _path, filename).c_str(), &s) != 0) {
            println("WARN: stat(\"{}/{}\") failed: {}", _path, filename, strerror(errno));
            continue;
        }
        if (!S_ISDIR(s.st_mode) && !S_ISREG(s.st_mode)) {
            continue;
        }
        if (!S_ISDIR(s.st_mode) && fnmatch(filter.data(), filename.c_str(), FNM_CASEFOLD) != 0) {
            continue;
        }
        directoryEntries.push_back({.name = filename,
                                    .directory = S_ISDIR(s.st_mode),
                                    .size = (unsigned long long)s.st_size,
                                    .created = s.st_ctimespec.tv_sec * 1000ULL + s.st_ctimespec.tv_nsec,
                                    .modified = s.st_mtimespec.tv_sec * 1000ULL + s.st_mtimespec.tv_nsec});
    }

    co_return directoryEntries;
}

CORBA::async<> FileSystem_impl::up() {
    char buffer[PATH_MAX];
    _path = dirname_r(_path.c_str(), buffer);
    co_return;
}
CORBA::async<> FileSystem_impl::down(const std::string_view &name) {
    string path = format("{}/{}", _path, name);
    char buffer[PATH_MAX];
    if (realpath(path.c_str(), buffer) == nullptr) {
        throw runtime_error(format("Directory::down(\"{}\"): {}", name, strerror(errno)));
    }
    auto directory = ::opendir(buffer);
    if (!directory) {
        throw runtime_error(format("Directory::down(\"{}\"): {}", name, strerror(errno)));
    }
    closedir(directory);
    _path = buffer;
    co_return;
}
CORBA::async<> FileSystem_impl::rootdir() {
    _path = "";
    co_return;
}
CORBA::async<> FileSystem_impl::homedir() {
    _path = getHomeDirectory();
    co_return;
}
CORBA::async<> FileSystem_impl::currentdir() {
    _path = getCurrentWorkingDirectory();
    co_return;
}
CORBA::async<> FileSystem_impl::mkdir(const std::string_view &name) {
    string path = format("{}/{}", _path, name);
    if (::mkdir(path.c_str(), 0) != 0) {
        throw runtime_error(format("failed to create directory {}: {}", path, strerror(errno)));
    }
    co_return;
}
CORBA::async<> FileSystem_impl::rmdir(const std::string_view &name) {
    string path = format("{}/{}", _path, name);
    if (::rmdir(path.c_str()) != 0) {
        throw runtime_error(format("failed to remove directory {}: {}", path, strerror(errno)));
    }
    co_return;
}
// this would be the place for POSIX AIO
CORBA::async<CORBA::blob> FileSystem_impl::read(const std::string_view &name) {
    throw runtime_error(format("{}:{}: not implemented yet", __FILE__, __LINE__));
    CORBA::blob data;
    co_return data;
}
CORBA::async<> FileSystem_impl::write(const std::string_view &name, const CORBA::blob_view &data) {
    throw runtime_error(format("{}:{}: not implemented yet", __FILE__, __LINE__));
    co_return;
}
CORBA::async<> FileSystem_impl::rm(const std::string_view &name) {
    throw runtime_error(format("{}:{}: not implemented yet", __FILE__, __LINE__));
    co_return;
}

//  static void checkFilename(const std::string_view &filename) {
//     if (filename.size() == 0) {
//         throw runtime_error("Backend_impl::save(): filename must not be empty");
//     }
//     if (filename[0] == '.') {
//         throw runtime_error("Backend_impl::save(): filename must not start with a dot");
//     }
//     if (filename.find('/') != filename.npos) {
//         throw runtime_error("Backend_impl::save(): filename must not contain a '/'");
//     }
// }

// CORBA::async<> FileSystem_impl::save(const std::string_view &filename, const std::string_view &data) {
//     println("Backend_impl::save(\"{}\", ...)", filename);
//     checkFilename(filename);
//     std::ofstream file(string(filename).c_str());
//     if (!file) {
//         println("failed to write");
//     }
//     file.write(data.data(), data.size());
//     // file << string(data);
//     println("ok");
//     co_return;
// }
// CORBA::async<std::string> FileSystem_impl::load(const std::string_view &filename) {
//     println("load {}", filename);

//     checkFilename(filename);

//     int fd = open(filename.data(), O_RDONLY);
//     if (fd < 0) {
//         throw runtime_error(format("failed to open file '{}': {}", filename, strerror(errno)));
//     }
//     off_t len = lseek(fd, 0, SEEK_END);
//     if (len < 0) {
//         throw runtime_error(format("failed to get size of file '{}': {}", filename, strerror(errno)));
//     }
//     const char *data = (const char *)mmap(nullptr, len, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
//     if (data == MAP_FAILED) {
//         throw runtime_error(format("failed to mmap file '{}': {}", filename, strerror(errno)));
//     }

//     string result(data, len);

//     munmap((void *)data, len);
//     close(fd);

//     println("ok");

//     co_return result;
// }
