#include "fs.hh"
#include "fs_skel.hh"

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdexcept>
#include <sys/param.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>

#include <algorithm>
#include <print>
#include <string>

#include "kaffeeklatsch.hh"

using namespace kaffeeklatsch;
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

class FileSystem_impl: public FileSystem_skel {
        std::string _path;
    public:
        FileSystem_impl();
        CORBA::async<std::string> path() override;
        CORBA::async<std::vector<DirectoryEntry>> list() override;
        CORBA::async<> up() override;
        CORBA::async<> down(const std::string_view & name) override;
        CORBA::async<> rootdir() override;
        CORBA::async<> homedir() override;
        CORBA::async<> currentdir() override;
        CORBA::async<> mkdir(const std::string_view & name) override;
        CORBA::async<> rmdir(const std::string_view & name) override;
        CORBA::async<CORBA::blob> read(const std::string_view & name) override;
        CORBA::async<> write(const std::string_view & name, const CORBA::blob_view & data) override;
        CORBA::async<> rm(const std::string_view & name) override;
};

FileSystem_impl::FileSystem_impl() {
    _path = getCurrentWorkingDirectory();
}

CORBA::async<std::string> FileSystem_impl::path() {
    co_return _path;
}

CORBA::async<std::vector<DirectoryEntry>> FileSystem_impl::list() {
    vector<DirectoryEntry> directoryEntries;

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

    sort(filenames.begin(), filenames.end());

    for(auto &filename: filenames) {
        struct stat s;
        if (stat(format("{}/{}", _path, filename).c_str(), &s) != 0) {
            println("WARN: stat(\"{}/{}\") failed: {}", _path, filename, strerror(errno));
            continue;
        }
        if (!S_ISDIR(s.st_mode) && !S_ISREG(s.st_mode)) {
            continue;
        }
        directoryEntries.push_back({
            .name = filename,
            .directory = S_ISDIR(s.st_mode),
            .size = (unsigned long long)s.st_size,
            .created = s.st_ctimespec.tv_sec * 1000ULL + s.st_ctimespec.tv_nsec,
            .modified = s.st_mtimespec.tv_sec * 1000ULL + s.st_mtimespec.tv_nsec
        });
    }

    co_return directoryEntries;
}

CORBA::async<> FileSystem_impl::up() { 
    char buffer[PATH_MAX];
    _path = dirname_r(_path.c_str(), buffer);
    co_return;
}
CORBA::async<> FileSystem_impl::down(const std::string_view & name) { 
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
CORBA::async<> FileSystem_impl::mkdir(const std::string_view & name) { 
    string path = format("{}/{}", _path, name);
    if (::mkdir(path.c_str(), 0) != 0) {
        throw runtime_error(format("failed to create directory {}: {}", path, strerror(errno)));
    }
    co_return;
}
CORBA::async<> FileSystem_impl::rmdir(const std::string_view & name) { 
    string path = format("{}/{}", _path, name);
    if (::rmdir(path.c_str()) != 0) {
        throw runtime_error(format("failed to remove directory {}: {}", path, strerror(errno)));
    }
    co_return;
}
// this would be the place for POSIX AIO
CORBA::async<CORBA::blob> FileSystem_impl::read(const std::string_view & name) { 
    CORBA::blob data;
    co_return data;
}
CORBA::async<> FileSystem_impl::write(const std::string_view & name, const CORBA::blob_view & data) { 
    co_return;
}
CORBA::async<> FileSystem_impl::rm(const std::string_view & name) { co_return; }

kaffeeklatsch_spec([] {
    fdescribe("class FileSystem", [] {
         it("XX", [] {
            auto directory = make_shared<FileSystem_impl>();

            directory->list().then([](std::vector<DirectoryEntry> list) {
                for(auto &a: list) {
                    println("{} {}\t{}", a.directory ? 'D' : ' ', a.name, a.size);
                }
            });
        });
    });
});
