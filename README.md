# makehuman.js backend

A daemon for [makehuman.js](https://github.com/markandre13/makehuman.js) providing access to [mediapipe_cpp_lib](https://github.com/markandre13/mediapipe_cpp_lib) and [Chordata Motion](https://chordata.cc).

### Build

Experts only for now.

### Technical

* mediapipe will open a window, which on macOS means that the mediapipe code
  needs to run in the main thread
* frontend and backend use websocket to communicate with the intention to
  move to webtransport later. the protocol on top of that is corba.

### Code

i'm still in the proof of concept stage hence the code is still messy.

<!--
once mediapipe_cpp_lib has been build:

# we need this version, the one in protobuf won't do
cd protobuf-3.19.1
./configure --prefix=/Users/mark/lib
make -j6
make install

cp /Users/mark/upstream/mediapipe_cpp_lib/src/gmod_api.h .
ln -s /Users/mark/upstream/mediapipe_cpp_lib/import_files mediapipe
ln -s /Users/mark/upstream/mediapipe_cpp_lib/mediapipe_graphs .
ln -s /Users/mark/upstream/mediapipe_cpp_lib/mediapipe_models .

c++ -std=c++17 -I. -I/Users/mark/lib/include -I/usr/local/Cellar/opencv@3/3.4.16_4/include  -L/Users/mark/lib/lib -L /Users/mark/upstream/mediapipe_cpp_lib/library -lprotobuf -lgmod main.cc
DYLD_LIBRARY_PATH=/Users/mark/upstream/mediapipe_cpp_lib/library ./a.out

-->
