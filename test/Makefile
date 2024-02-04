APP=tests

#MEM=-fsanitize=address -fsanitize=leak

CXX=/usr/local/opt/llvm/bin/clang++
CFLAGS=-std=c++23 -fmodules $(MEM) -O0 -g \
	-Wall -Wextra -Wno-deprecated-anon-enum-enum-conversion \
	-I/usr/local/opt/llvm/include/c++ \
	-I../upstream/bandit

LDFLAGS=-L/usr/local/opt/llvm/lib/c++ -Wl,-rpath,/usr/local/opt/llvm/lib/c++ \
	-L/usr/local/lib $(MEM) -g

PROTOBUF_FLAGS=-I/Users/mark/lib/include
OPENCV_FLAGS=-I/usr/local/opt/opencv@3/include
WSLAG_FLAGS=-I/usr/local/include
PROTOBUF_LDFLAGS=-L$(HOME)/lib/lib
MEDIAPIPE_CPP_DIR=$(HOME)/Sites/mediapipe_cpp_lib
MEDIAPIPE_CPP_LDFLAGS=-L$(MEDIAPIPE_CPP_DIR)/library
# LIB=-lprotobuf -lgmod -lwslay -lnettle

SRC = coroutine.cc cdr_decoder.cc cdr_encoder.cc giop.cc \
	main.cc util.cc \
	../src/corba/orb.cc ../src/corba/giop.cc ../src/corba/cdr.cc
OBJ = $(SRC:.cc=.o)

.SUFFIXES: .cc .o

all: $(APP)

depend:
	makedepend -I. -Y $(SRC)

run:
	DYLD_LIBRARY_PATH=$(MEDIAPIPE_CPP_DIR)/library ./$(APP) --reporter=info --report-timing

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

coroutine.o: ../src/corba/coroutine.hh
cdr_decoder.o: ../src/corba/cdr.hh
cdr_encoder.o: ../src/corba/cdr.hh
giop.o: ../src/corba/giop.hh ../src/corba/cdr.hh
util.o: util.hh
../src/corba/orb.o: ../src/corba/cdr.hh ../src/corba/corba.hh
../src/corba/orb.o: ../src/corba/giop.hh ../src/corba/ws/EventHandler.hh
../src/corba/orb.o: ../src/corba/hexdump.hh
../src/corba/giop.o: ../src/corba/giop.hh ../src/corba/cdr.hh
../src/corba/giop.o: ../src/corba/corba.hh
../src/corba/cdr.o: ../src/corba/cdr.hh
