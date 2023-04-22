#!/bin/sh -ex

APP=demo

CXX=c++
CXXFLAGS=-std=c++17 -g -O0
PROTOBUF_FLAGS=-I/Users/mark/lib/include
OPENCV_FLAGS=-I/usr/local/opt/opencv@3/include
WSLAG_FLAGS=-I/usr/local/opt/wslay/include
PROTOBUF_LDFLAGS=-L$(HOME)/lib/lib
MEDIAPIPE_CPP_DIR=$(HOME)/Sites/mediapipe_cpp_lib
MEDIAPIPE_CPP_LDFLAGS=-L$(MEDIAPIPE_CPP_DIR)/library
# LIBS="-lprotobuf -lgmod -lwslay -lnettle"
LIB=-lwslay -lnettle

SRC = ws.cc EchoWebSocketHandler.cc HttpHandshakeSendHandler.cc HttpHandshakeRecvHandler.cc \
	ListenEventHandler.cc EventHandler.cc createAcceptKey.cc socket.cc
OBJ = $(SRC:.cc=.o)

.SUFFIXES: .cc .o

all: $(APP)

depend:
	makedepend -I. -Y $(SRC)

$(APP): $(OBJ)
	@echo "linking..."
	$(CXX) $(LIB) $(OBJ) -o $(APP)

.cc.o:
	@echo compiling $*.cc ...
	$(CXX) $(CXXFLAGS) $(PROTOBUF_FLAGS) $(OPENCV_FLAGS) $(PROTOBUF_FLAGS) \
	$(PROTBUF_LDFLAGS) $(MEDIAPIPE_CPP_LDFLAGS) \
	-c -o $*.o $*.cc

# DO NOT DELETE

ws.o: EventHandler.hh ListenEventHandler.hh socket.hh
EchoWebSocketHandler.o: EchoWebSocketHandler.hh EventHandler.hh
HttpHandshakeSendHandler.o: HttpHandshakeSendHandler.hh EventHandler.hh
HttpHandshakeSendHandler.o: EchoWebSocketHandler.hh
HttpHandshakeRecvHandler.o: HttpHandshakeRecvHandler.hh EventHandler.hh
HttpHandshakeRecvHandler.o: HttpHandshakeSendHandler.hh createAcceptKey.hh
ListenEventHandler.o: ListenEventHandler.hh EventHandler.hh
ListenEventHandler.o: HttpHandshakeRecvHandler.hh socket.hh
EventHandler.o: EventHandler.hh
createAcceptKey.o: createAcceptKey.hh
