#pragma once

#include "../../makehuman.hh"

std::vector<std::shared_ptr<VideoCamera2>> getVideoCameras(std::shared_ptr<CORBA::ORB> orb);
