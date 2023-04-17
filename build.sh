#!/bin/sh -ex

# brew install opencv@3 wslay

# once mediapipe_cpp_lib has been build:

# we need this version, the one in protobuf won't do
# cd protobuf-3.19.1
# ./configure --prefix=/Users/mark/lib
# make -j6
# make install

# compiles 'main.cpp' into 'demo'
# please adjust variables as needed

MEDIAPIPE_CPP_DIR=${HOME}/Sites/mediapipe_cpp_lib

cp ${MEDIAPIPE_CPP_DIR}/src/gmod_api.h .
rm -f mediapipe mediapipe_graphs mediapipe_models
ln -s ${MEDIAPIPE_CPP_DIR}/import_files mediapipe
ln -s ${MEDIAPIPE_CPP_DIR}/mediapipe_graphs .
ln -s ${MEDIAPIPE_CPP_DIR}/mediapipe_models .

CXX="c++"
CXXFLAGS="-std=c++17"
PROTOBUF_FLAGS="-I/Users/mark/lib/include"
OPENCV_FLAGS="-I/usr/local/opt/opencv@3/include"
WSLAG_FLAGS="-I/usr/local/opt/wslay/include"
PROTOBUF_LDFLAGS="-L${HOME}/lib/lib"
MEDIAPIPE_CPP_LDFLAGS="-L${MEDIAPIPE_CPP_DIR}/library"
LIBS="-lprotobuf -lgmod -lwslay"

${CXX} ${CXXFLAGS} -I. ${PROTOBUF_FLAGS} ${OPENCV_FLAGS} ${PROTOBUF_FLAGS} \
   ${PROTBUF_LDFLAGS} ${MEDIAPIPE_CPP_LDFLAGS} \
   ${LIBS} main.cc socket.cc -o demo
DYLD_LIBRARY_PATH=${MEDIAPIPE_CPP_DIR}/library ./demo
