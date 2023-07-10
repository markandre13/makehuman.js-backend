# mediapipe_daemon

A daemon for [makehuman.js](https://github.com/markandre13/makehuman.js) providing access to [mediapipe_cpp_lib](https://github.com/markandre13/mediapipe_cpp_lib) and [Chordata Motion](https://chordata.cc).

### Technical

* the networking code is organized around the WebSocket library we are using
* mediapipe will open a window, which on macOS means that the mediapipe code
  needs to run in the main thread
* since there is still no WebTransport in Safari, the implementation currently
  uses WebSockets and the client needs to pull events from the daemon.

### Code

I'm still in a prototyping stage hence the code layout is still messy.
Here's an overview:

    main.cc
    EventHandler.cc
        wsInit()        creates a websocket on port 9001
                        for the webapp to connect to and
                        adds a ListenEventHandler()
        wsHandle()      handle incoming WS events
    ListenEventHandler.cc
                        waits for a client to connect,
                        creates a HttpHandshakeRecvHandler
    HttpHandshakeRecvHandler.cc
                        handles transition from HTTP to WS
                        creates a HttpHandshakeSendHandler
    HttpHandshakeSendHandler.cc
                        handles transition from HTTP to WS
                        creates a EchoWebSocketHandler
    EchoWebSocketHandler.cc
        isFaceRequested()
                        returns 'true' when sendFace() needs to be called
        void sendFace(float* float_array, int size)
        isChordataRequested()
                        returns 'true' when sendChordata() needs to be called
        void sendChordata(...)
        on_msg_recv_callback(...)
                        handles the incoming WS messages
    socket.cc
        int create_listen_socket()
        void ignore_sig_pipe()
        int make_non_block()    0: true, -1: false

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

c++ -std=c++17 -I. -I/Users/mark/lib/include -I/usr/local/Cellar/opencv@3/3.4.16_4/include  -L/Users/mark/lib/lib -L /Users/mark/upstream/mediapipe_cpp_lib/library -lprotobuf -lgmod main.cc
DYLD_LIBRARY_PATH=/Users/mark/upstream/mediapipe_cpp_lib/library ./a.out


-->
