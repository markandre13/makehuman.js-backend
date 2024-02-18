#!/bin/sh -ex

APP=backend

CXX=/usr/local/opt/llvm/bin/clang++
CFLAGS=-std=c++23 -fmodules \
	-Wall -Wextra -Werror=return-type -Werror=shadow \
	-Wno-deprecated-anon-enum-enum-conversion -Wno-module-import-in-extern-c -Wno-unused-parameter \
	-O0 -g -I/usr/local/opt/llvm/include/c++
LDFLAGS=-L/usr/local/opt/llvm/lib/c++ -Wl,-rpath,/usr/local/opt/llvm/lib/c++ -L/usr/local/lib

PROTOBUF_FLAGS=-I/Users/mark/lib/include
OPENCV_FLAGS=-I/usr/local/opt/opencv@3/include
WSLAG_FLAGS=-I/usr/local/include
PROTOBUF_LDFLAGS=-L$(HOME)/lib/lib
MEDIAPIPE_CPP_DIR=$(HOME)/Sites/mediapipe_cpp_lib
MEDIAPIPE_CPP_LDFLAGS=-L$(MEDIAPIPE_CPP_DIR)/library
# LIB=-lprotobuf -lgmod -lwslay -lnettle
LIB=-lev -lwslay -lnettle


SRC = main.cc \
	corba/corba.cc corba/orb.cc corba/url.cc \
	corba/skeleton.cc corba/giop.cc corba/cdr.cc \
	corba/hexdump.cc \
	corba/net/ws/socket.cc corba/net/ws/createAcceptKey.cc \

X =	chordata.cc \
	corba/corba.cc corba/orb.cc corba/url.cc \
	corba/skeleton.cc corba/giop.cc corba/cdr.cc \
	corba/hexdump.cc \
	corba/net/ws/ws.cc corba/net/ws/socket.cc \
	corba/net/ws/ListenEventHandler.cc corba/net/ws/EventHandler.cc corba/net/ws/createAcceptKey.cc \
	corba/net/ws/HttpHandshakeSendHandler.cc corba/net/ws/HttpHandshakeRecvHandler.cc \
	corba/net/ws/CorbaHandler.cc

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

main.o: corba/net/ws/createAcceptKey.hh corba/net/ws/socket.hh corba/corba.hh
main.o: corba/coroutine.hh corba/giop.hh corba/cdr.hh corba/orb.hh
corba/corba.o: corba/orb.hh corba/coroutine.hh corba/giop.hh corba/cdr.hh
corba/orb.o: corba/orb.hh corba/coroutine.hh corba/giop.hh corba/cdr.hh
corba/orb.o: corba/corba.hh corba/hexdump.hh corba/url.hh corba/protocol.hh
corba/url.o: corba/url.hh corba/cdr.hh corba/giop.hh corba/corba.hh
corba/url.o: corba/coroutine.hh
corba/skeleton.o: corba/corba.hh corba/coroutine.hh corba/giop.hh
corba/skeleton.o: corba/cdr.hh corba/orb.hh
corba/giop.o: corba/giop.hh corba/cdr.hh corba/corba.hh corba/coroutine.hh
corba/giop.o: corba/hexdump.hh corba/orb.hh corba/protocol.hh
corba/cdr.o: corba/cdr.hh
corba/net/ws/createAcceptKey.o: corba/net/ws/createAcceptKey.hh
