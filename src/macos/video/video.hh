#pragma once

#include "../../opencv/videocamera.hh"

std::vector<std::shared_ptr<VideoCamera2>> getVideoCameras(std::shared_ptr<CORBA::ORB> orb);
