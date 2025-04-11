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

// kaffeeklatsch_spec([] {
//     fdescribe("class FileSystem", [] {
//          it("XX", [] {
//             auto directory = make_shared<FileSystem_impl>();

//             directory->list().then([](std::vector<DirectoryEntry> list) {
//                 for(auto &a: list) {
//                     println("{} {}\t{}", a.directory ? 'D' : ' ', a.name, a.size);
//                 }
//             });
//         });
//     });
// });
