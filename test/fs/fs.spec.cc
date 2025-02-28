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

// TODO: why not get rid of FileSystem and do everything in Directory? this way the UI could get away
//       with a single instance
//       FileSystem could serve as a factory. Directory could get a remove() method similar to the CosLifeCycle spec
//       FileSystem is available by name
class FileSystem_impl: public FileSystem_skel {
    public:
        CORBA::async<std::shared_ptr<Directory>> opendir(const std::string_view & path) override;
        CORBA::async<std::shared_ptr<Directory>> rootdir() override;
        CORBA::async<std::shared_ptr<Directory>> homedir() override;
        CORBA::async<std::shared_ptr<Directory>> currentdir() override;
};

class Directory_impl: public Directory_skel {
        std::string _path;
    public:
        Directory_impl(std::string path);
        CORBA::async<std::string> path() override;
        CORBA::async<std::vector<DirectoryEntry>> list() override;
        CORBA::async<std::shared_ptr<Directory>> parent() override;
        CORBA::async<std::shared_ptr<Directory>> opendir(const std::string_view & name) override;
        CORBA::async<std::shared_ptr<Directory>> mkdir(const std::string_view & name) override;
};

CORBA::async<std::shared_ptr<Directory>> FileSystem_impl::opendir(const std::string_view & path) {
    co_return make_shared<Directory_impl>(string(path));
}
CORBA::async<std::shared_ptr<Directory>> FileSystem_impl::rootdir() {
    co_return make_shared<Directory_impl>("/");
}
CORBA::async<std::shared_ptr<Directory>> FileSystem_impl::homedir() {
    co_return make_shared<Directory_impl>(getHomeDirectory());
}
CORBA::async<std::shared_ptr<Directory>> FileSystem_impl::currentdir() {
    co_return make_shared<Directory_impl>(getCurrentWorkingDirectory());
}

Directory_impl::Directory_impl(std::string path) {
    char buffer[PATH_MAX];
    if (realpath(path.c_str(), buffer) == nullptr) {
        throw runtime_error(format("Directory::Directory_impl(\"{}\"): {}", _path, strerror(errno)));
    }
    _path = buffer;
    auto dirp = ::opendir(_path.c_str());
    if (!dirp) {
        throw runtime_error(format("Directory::Directory_impl(\"{}\"): {}", _path, strerror(errno)));
    }
    closedir(dirp);
}

CORBA::async<std::string> Directory_impl::path() {
    co_return _path;
}

CORBA::async<std::vector<DirectoryEntry>> Directory_impl::list() {
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
            .modified = s.st_mtimespec.tv_sec * 1000ULL + s.st_mtimespec.tv_nsec
        });
    }

    co_return directoryEntries;
}

CORBA::async<std::shared_ptr<Directory>> Directory_impl::parent() {
    char buffer[PATH_MAX];
    co_return make_shared<Directory_impl>(dirname_r(_path.c_str(), buffer));
}
CORBA::async<std::shared_ptr<Directory>> Directory_impl::opendir(const std::string_view & name) {
    co_return make_shared<Directory_impl>(format("{}/{}", _path, name));
}
CORBA::async<std::shared_ptr<Directory>> Directory_impl::mkdir(const std::string_view & name) {
    string path = format("{}/{}", _path, name);
    if (::mkdir(path.c_str(), 0) != 0) {
        throw runtime_error(format("failed to create directory {}: {}", path, strerror(errno)));
    }
    co_return make_shared<Directory_impl>(path);
}

// TODO: there's a need to activate the object
// TODO: the skeleton interface still has a variant with an ORB we could use to make the activation nicer again?
// TODO: lifecycle management for Directory
// TODO: reuse Directory
// TODO: sanitize ".." in path -> realpath()
// TODO: filter -> fnmatch
// CORBA::release() ? CosLifeCycle object->release()
// can the client release a stub so that the implementation get's also deleted?

// basename(), realpath(), readlink(), glob(), fnmatch()...

kaffeeklatsch_spec([] {
    fdescribe("class FileSystem", [] {
         it("XX", [] {
            auto directory = make_shared<Directory_impl>(getHomeDirectory());

            directory->list().then([](std::vector<DirectoryEntry> list) {
                for(auto &a: list) {
                    println("{} {}\t{}", a.directory ? 'D' : ' ', a.name, a.size);
                }
            });
        });
    });
});
