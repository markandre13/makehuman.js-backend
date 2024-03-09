#!/bin/sh -ex

# brew install opencv@3 wslay ossp-uuid

# once mediapipe_cpp_lib has been build:

# we need this version, the one in protobuf won't do
# cd protobuf-3.19.1
# ./configure --prefix=/Users/mark/lib
# make -j6
# make install

# compiles 'main.cpp' into 'demo'
# please adjust variables as needed

make

MEDIAPIPE_CPP_DIR=${HOME}/Sites/mediapipe_cpp_lib
DYLD_LIBRARY_PATH=${MEDIAPIPE_CPP_DIR}/library ./demo 9001

#    main.cc socket.cc \
