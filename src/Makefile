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
LIB=-lprotobuf -lgmod -lwslay -lnettle

SRC = main.cc \
	MakeHumanHandler.cc HttpHandshakeSendHandler.cc HttpHandshakeRecvHandler.cc \
	ListenEventHandler.cc EventHandler.cc createAcceptKey.cc socket.cc chordata.cc \
	corba/corba.cc
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

main.o: EventHandler.hh gmod_api.h
main.o: ../mediapipe/framework/formats/landmark.pb.h corba/corba.hh
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
corba/corba.o: corba/corba.hh
