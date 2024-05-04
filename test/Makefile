APP=tests

MEM=-fsanitize=address -fsanitize=leak

CXX=/usr/local/opt/llvm/bin/clang++
CFLAGS=-std=c++23 -fmodules $(MEM) -O0 -g \
	-mmacosx-version-min=12.0 \
	-Wall -Wextra -Wno-deprecated-anon-enum-enum-conversion \
	-Werror=return-type -Werror=shadow \
	-Wno-unused-parameter -Wno-module-import-in-extern-c \
	-I/usr/local/opt/llvm/include/c++ \
	-I../upstream/kaffeeklatsch/src \
	-I../src
# -D_COROUTINE_DEBUG=1

LDFLAGS=-L/usr/local/opt/llvm/lib/c++ \
	-mmacosx-version-min=12.0 \
	-Wl,-rpath,/usr/local/opt/llvm/lib/c++ \
	-L/usr/local/lib $(MEM)

PROTOBUF_FLAGS=-I/Users/mark/lib/include
OPENCV_FLAGS=-I/usr/local/opt/opencv@3/include
WSLAG_FLAGS=-I/usr/local/include
PROTOBUF_LDFLAGS=-L$(HOME)/lib/lib
MEDIAPIPE_CPP_DIR=$(HOME)/Sites/mediapipe_cpp_lib
MEDIAPIPE_CPP_LDFLAGS=-L$(MEDIAPIPE_CPP_DIR)/library
LIB=-lev -lwslay -lnettle
# LIB=-lprotobuf -lgmod -lwslay -lnettle

    #   corba/net/ws.spec.cc \

SRC = mesh/wavefront.spec.cc \
	  ../src/util.cc ../src/mesh/wavefront.cc \
	  main.cc ../upstream/kaffeeklatsch/src/kaffeeklatsch.cc

WSLAY=../upstream/wslay/lib/wslay_event.c \
  ../upstream/wslay/lib/wslay_frame.c \
  ../upstream/wslay/lib/wslay_net.c \
  ../upstream/wslay/lib/wslay_queue.c

OBJ = $(SRC:.cc=.o) 
# $(WSLAY:.c=.o)

.SUFFIXES: .idl .cc .c .h .hh .o

all: $(APP)

depend:
	makedepend -I. -Y $(SRC)

run:
	DYLD_LIBRARY_PATH=$(MEDIAPIPE_CPP_DIR)/library ./$(APP) --reporter=info --report-timing --only=foo

valgrind:
	/usr/local/Cellar/valgrind/HEAD-93191c1/bin/valgrind --track-origins=yes ./$(APP)

clean:
	rm -f $(OBJ)

$(APP): $(OBJ)
	@echo "linking..."
	$(CXX) $(LDFLAGS) $(PROTOBUF_LDFLAGS) $(MEDIAPIPE_CPP_LDFLAGS) $(LIB) $(OBJ) -o $(APP)

.cc.o:
	@echo compiling $*.cc ...
	$(CXX) $(PROTOBUF_FLAGS) $(CFLAGS) $(WSLAG_FLAGS) $(OPENCV_FLAGS) \
	-c -o $*.o $*.cc

.c.o:
	@echo compiling $*.c ...
	$(CC) $(MEM) -g -O2 -DHAVE_CONFIG_H -I../upstream/wslay/lib/ -I../upstream/wslay/lib/includes -c -o $*.o $*.c

corba/interface/interface.hh: corba/interface/interface.idl
	bun ../upstream/corba.js/src/idl/idl.ts --cc-all corba/interface/interface.idl

makehuman.hh: makehuman.idl
	bun ../upstream/corba.js/src/idl/idl.ts --cc-all makehuman.idl

# DO NOT DELETE

../src/util.o: ../src/util.hh
../src/mesh/wavefront.o: ../src/mesh/wavefront.hh ../src/util.hh
../upstream/kaffeeklatsch/src/kaffeeklatsch.o: ../upstream/kaffeeklatsch/src/kaffeeklatsch.hh
