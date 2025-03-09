#pragma once

#include "generated/fs_skel.hh"

class FileSystem_impl: public FileSystem_skel {
    std::string _path;
public:
    FileSystem_impl();
    CORBA::async<std::string> path() override;
    CORBA::async<> path(const std::string_view &) override;
    CORBA::async<std::vector<DirectoryEntry>> list(const std::string_view &) override;
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
