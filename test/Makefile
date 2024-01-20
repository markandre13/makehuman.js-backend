#!/bin/sh -ex

APP=tests

CXX=/usr/local/opt/llvm/bin/clang++
CFLAGS=-std=c++23 -fmodules \
	-Wall -Wextra -Wno-deprecated-anon-enum-enum-conversion \
	-O0 -g -I/usr/local/opt/llvm/include/c++ \
	-DGTEST_HAS_PTHREAD=1
LDFLAGS=-L/usr/local/Cellar/googletest/1.14.0/lib  -lgtest -lgtest_main \
	-L/usr/local/opt/llvm/lib/c++ -Wl,-rpath,/usr/local/opt/llvm/lib/c++ \
	-L/usr/local/lib

PROTOBUF_FLAGS=-I/Users/mark/lib/include
OPENCV_FLAGS=-I/usr/local/opt/opencv@3/include
WSLAG_FLAGS=-I/usr/local/include
PROTOBUF_LDFLAGS=-L$(HOME)/lib/lib
MEDIAPIPE_CPP_DIR=$(HOME)/Sites/mediapipe_cpp_lib
MEDIAPIPE_CPP_LDFLAGS=-L$(MEDIAPIPE_CPP_DIR)/library
LIB=-lprotobuf -lgmod -lwslay -lnettle

SRC = giop.cc dataview.cc util.cc ../src/corba/giop.cc ../src/corba/dataview.cc

OBJ = $(SRC:.cc=.o)

.SUFFIXES: .cc .o

all: $(APP)

depend:
	makedepend -I. -Y $(SRC)

run:
	DYLD_LIBRARY_PATH=$(MEDIAPIPE_CPP_DIR)/library ./$(APP) 
	# --gtest_filter=GIOPDecoder.OmniOrbReply

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

giop.o: ../src/corba/giop.hh ../src/corba/dataview.hh util.hh
util.o: util.hh
../src/corba/giop.o: ../src/corba/giop.hh ../src/corba/dataview.hh
../src/corba/dataview.o: ../src/corba/dataview.hh
