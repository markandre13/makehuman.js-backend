#!/bin/sh -ex

APP=daemon

CXX=c++
CXXFLAGS=-std=c++17 -g -O0
PROTOBUF_FLAGS=-I/Users/mark/lib/include
OPENCV_FLAGS=-I/usr/local/opt/opencv@3/include
WSLAG_FLAGS=-I/usr/local/opt/wslay/include
PROTOBUF_LDFLAGS=-L$(HOME)/lib/lib
MEDIAPIPE_CPP_DIR=$(HOME)/Sites/mediapipe_cpp_lib
MEDIAPIPE_CPP_LDFLAGS=-L$(MEDIAPIPE_CPP_DIR)/library
LIB=-lprotobuf -lgmod -lwslay -lnettle

SRC = main.cc \
	MakeHumanHandler.cc HttpHandshakeSendHandler.cc HttpHandshakeRecvHandler.cc \
	ListenEventHandler.cc EventHandler.cc createAcceptKey.cc socket.cc chordata.cc
OBJ = $(SRC:.cc=.o)

.SUFFIXES: .cc .o

all: $(APP)

depend:
	makedepend -I. -Y $(SRC)

run:
	DYLD_LIBRARY_PATH=$(MEDIAPIPE_CPP_DIR)/library ./$(APP)

clean:
	rm $(OBJ)

$(APP): $(OBJ)
	@echo "linking..."
	$(CXX) $(PROTBUF_LDFLAGS) $(MEDIAPIPE_CPP_LDFLAGS) $(LIB) $(OBJ) -o $(APP)

.cc.o:
	@echo compiling $*.cc ...
	$(CXX) $(CXXFLAGS) $(PROTOBUF_FLAGS) $(OPENCV_FLAGS) $(PROTOBUF_FLAGS) \
	-c -o $*.o $*.cc

# DO NOT DELETE

main.o: EventHandler.hh gmod_api.h mediapipe/framework/formats/landmark.pb.h
MakeHumanHandler.o: MakeHumanHandler.hh EventHandler.hh wslay_event.h
HttpHandshakeSendHandler.o: HttpHandshakeSendHandler.hh EventHandler.hh
HttpHandshakeSendHandler.o: MakeHumanHandler.hh
HttpHandshakeRecvHandler.o: HttpHandshakeRecvHandler.hh EventHandler.hh
HttpHandshakeRecvHandler.o: HttpHandshakeSendHandler.hh createAcceptKey.hh
ListenEventHandler.o: ListenEventHandler.hh EventHandler.hh
ListenEventHandler.o: HttpHandshakeRecvHandler.hh socket.hh
EventHandler.o: EventHandler.hh ListenEventHandler.hh socket.hh
createAcceptKey.o: createAcceptKey.hh
chordata.o: MakeHumanHandler.hh EventHandler.hh
