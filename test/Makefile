#!/bin/sh -ex

APP=tests

CXX=/usr/local/opt/llvm/bin/clang++
CFLAGS=-std=c++23 -fmodules -fsanitize=address \
	-Wall -Wextra -Wno-deprecated-anon-enum-enum-conversion \
	-O0 -g -I/usr/local/opt/llvm/include/c++ \
	-DGTEST_HAS_PTHREAD=1
LDFLAGS=-L/usr/local/Cellar/googletest/1.14.0/lib \
	-L/usr/local/opt/llvm/lib/c++ -Wl,-rpath,/usr/local/opt/llvm/lib/c++ \
	-L/usr/local/lib -fsanitize=address -fsanitize=leak -g
# -lgtest -lgtest_main

PROTOBUF_FLAGS=-I/Users/mark/lib/include
OPENCV_FLAGS=-I/usr/local/opt/opencv@3/include
WSLAG_FLAGS=-I/usr/local/include
PROTOBUF_LDFLAGS=-L$(HOME)/lib/lib
MEDIAPIPE_CPP_DIR=$(HOME)/Sites/mediapipe_cpp_lib
MEDIAPIPE_CPP_LDFLAGS=-L$(MEDIAPIPE_CPP_DIR)/library
# LIB=-lprotobuf -lgmod -lwslay -lnettle

# SRC = giop.cc cdr_decoder.cc cdr_encoder.cc coroutine.cc util.cc \
# 	../src/corba/orb.cc ../src/corba/giop.cc ../src/corba/cdr.cc
SRC = coroutine.cc

OBJ = $(SRC:.cc=.o)

.SUFFIXES: .cc .o

all: $(APP)

depend:
	makedepend -I. -Y $(SRC)

run:
	DYLD_LIBRARY_PATH=$(MEDIAPIPE_CPP_DIR)/library ./$(APP) --gtest_filter="Coroutine.Nested"

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

giop.o: ../src/corba/giop.hh ../src/corba/cdr.hh util.hh
cdr_decoder.o: ../src/corba/cdr.hh
cdr_encoder.o: ../src/corba/cdr.hh
util.o: util.hh
../src/corba/orb.o: ../src/corba/cdr.hh ../src/corba/corba.hh
../src/corba/orb.o: ../src/corba/giop.hh ../src/corba/ws/EventHandler.hh
../src/corba/orb.o: ../src/corba/hexdump.hh
../src/corba/giop.o: ../src/corba/giop.hh ../src/corba/cdr.hh
../src/corba/giop.o: ../src/corba/corba.hh
../src/corba/cdr.o: ../src/corba/cdr.hh
