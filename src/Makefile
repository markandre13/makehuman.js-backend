#!/bin/sh -ex

APP=backend

CXX=/usr/local/opt/llvm/bin/clang++
CFLAGS=-std=c++23 -fmodules \
	-Wall -Wextra -Wno-deprecated-anon-enum-enum-conversion \
	-O0 -g -I/usr/local/opt/llvm/include/c++
LDFLAGS=-L/usr/local/opt/llvm/lib/c++ -Wl,-rpath,/usr/local/opt/llvm/lib/c++ -L/usr/local/lib

PROTOBUF_FLAGS=-I/Users/mark/lib/include
OPENCV_FLAGS=-I/usr/local/opt/opencv@3/include
WSLAG_FLAGS=-I/usr/local/include
PROTOBUF_LDFLAGS=-L$(HOME)/lib/lib
MEDIAPIPE_CPP_DIR=$(HOME)/Sites/mediapipe_cpp_lib
MEDIAPIPE_CPP_LDFLAGS=-L$(MEDIAPIPE_CPP_DIR)/library
# LIB=-lprotobuf -lgmod -lwslay -lnettle
LIB=-lwslay -lnettle


SRC = main.cc \
	chordata.cc \
	corba/corba.cc corba/orb.cc corba/url.cc \
	corba/skeleton.cc corba/giop.cc corba/cdr.cc \
	corba/hexdump.cc \
	corba/ws/socket.cc \
	corba/ws/ListenEventHandler.cc corba/ws/EventHandler.cc corba/ws/createAcceptKey.cc \
	corba/ws/HttpHandshakeSendHandler.cc corba/ws/HttpHandshakeRecvHandler.cc \
	corba/ws/MakeHumanHandler.cc

OBJ = $(SRC:.cc=.o)

.SUFFIXES: .cc .o

all: $(APP)

depend:
	makedepend -I. -Y $(SRC)

run:
	DYLD_LIBRARY_PATH=$(MEDIAPIPE_CPP_DIR)/library ./$(APP)

clean:
	rm -f $(OBJ)

$(APP): $(OBJ)
	@echo "linking..."
	$(CXX) $(LDFLAGS) $(PROTOBUF_LDFLAGS) $(MEDIAPIPE_CPP_LDFLAGS) $(LIB) $(OBJ) -o $(APP)

.cc.o:
	@echo compiling $*.cc ...
	$(CXX) $(PROTOBUF_FLAGS) $(CFLAGS) $(WSLAG_FLAGS) $(OPENCV_FLAGS) \
	-c -o $*.o $*.cc

# DO NOT DELETE

main.o: ../mediapipe/framework/formats/landmark.pb.h gmod_api.h
main.o: corba/corba.hh corba/giop.hh corba/cdr.hh corba/coroutine.hh
main.o: corba/giop.hh
chordata.o: corba/ws/MakeHumanHandler.hh corba/ws/EventHandler.hh
corba/corba.o: corba/corba.hh corba/giop.hh corba/cdr.hh corba/coroutine.hh
corba/corba.o: corba/ws/EventHandler.hh
corba/orb.o: corba/cdr.hh corba/corba.hh corba/giop.hh corba/coroutine.hh
corba/orb.o: corba/ws/EventHandler.hh corba/hexdump.hh
corba/giop.o: corba/giop.hh corba/cdr.hh corba/corba.hh corba/coroutine.hh
corba/cdr.o: corba/cdr.hh
corba/ws/ListenEventHandler.o: corba/ws/ListenEventHandler.hh
corba/ws/ListenEventHandler.o: corba/ws/EventHandler.hh
corba/ws/ListenEventHandler.o: corba/ws/HttpHandshakeRecvHandler.hh
corba/ws/ListenEventHandler.o: corba/ws/socket.hh
corba/ws/EventHandler.o: corba/ws/EventHandler.hh
corba/ws/EventHandler.o: corba/ws/ListenEventHandler.hh corba/ws/socket.hh
corba/ws/createAcceptKey.o: corba/ws/createAcceptKey.hh
corba/ws/HttpHandshakeSendHandler.o: corba/ws/HttpHandshakeSendHandler.hh
corba/ws/HttpHandshakeSendHandler.o: corba/ws/EventHandler.hh
corba/ws/HttpHandshakeSendHandler.o: corba/ws/MakeHumanHandler.hh
corba/ws/HttpHandshakeRecvHandler.o: corba/ws/HttpHandshakeRecvHandler.hh
corba/ws/HttpHandshakeRecvHandler.o: corba/ws/EventHandler.hh
corba/ws/HttpHandshakeRecvHandler.o: corba/ws/HttpHandshakeSendHandler.hh
corba/ws/HttpHandshakeRecvHandler.o: corba/ws/createAcceptKey.hh
corba/ws/MakeHumanHandler.o: corba/ws/MakeHumanHandler.hh
corba/ws/MakeHumanHandler.o: corba/ws/EventHandler.hh corba/ws/wslay_event.h
corba/ws/MakeHumanHandler.o: corba/corba.hh corba/giop.hh corba/cdr.hh
corba/ws/MakeHumanHandler.o: corba/coroutine.hh corba/hexdump.hh
