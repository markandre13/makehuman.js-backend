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

SRC = ws.cc
OBJ = $(SRC:.cc=.o)

.SUFFIXES: .cc .o

all: $(APP)

$(APP): $(OBJ)
	@echo "linking..."
	$(CXX) $(LIB) $(OBJ) -o $(APP)

.cc.o:
	@echo compiling $*.cc ...
	$(CXX) $(CXXFLAGS) $(PROTOBUF_FLAGS) $(OPENCV_FLAGS) $(PROTOBUF_FLAGS) \
	$(PROTBUF_LDFLAGS) $(MEDIAPIPE_CPP_LDFLAGS) \
	-c -o $*.o $*.cc
