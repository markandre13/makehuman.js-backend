APP=tests

MEM=-fsanitize=address -fsanitize=leak

HOMEBREW := $(shell brew config | grep HOMEBREW_PREFIX | sed 's/.*: //')

CXX=$(HOMEBREW)/opt/llvm/bin/clang++
CFLAGS=-std=c++23 -fmodules $(MEM) -O0 -g \
	-mmacosx-version-min=12.0 \
	-Wall -Wextra -Wno-deprecated-anon-enum-enum-conversion \
	-Werror=return-type -Werror=shadow \
	-Wno-unused-parameter -Wno-module-import-in-extern-c \
	-I$(HOMEBREW)/opt/llvm/include/c++ \
	-I../upstream/kaffeeklatsch/src \
	-I../upstream/libeio/src \
	-I../src
# -D_COROUTINE_DEBUG=1

CORBA_PATH=../upstream/corba.cc/src
CORBA_SRC=orb.cc ior.cc skeleton.cc stub.cc giop.cc cdr.cc url.cc \
	naming.cc \
	util/hexdump.cc util/logger.cc \
	net/connection.cc net/stream2packet.cc \
	net/tcp/protocol.cc net/tcp/connection.cc \
	net/ws/protocol.cc net/ws/connection.cc \
	net/util/socket.cc net/util/createAcceptKey.cc

LDFLAGS=-L$(HOMEBREW)/opt/llvm/lib/c++ \
	-mmacosx-version-min=12.0 \
	-Wl,-rpath,/usr/local/opt/llvm/lib/c++ \
	-L$(HOMEBREW)/lib $(MEM)

CORBA_FLAGS=-I$(CORBA_PATH)
OPENCV_FLAGS=-I$(HOMEBREW)/include/opencv4
OPENCV_LFLAGS=-L$(HOMEBREW)/lib/opencv4
WSLAY_FLAGS=-I$(HOMEBREW)/include

LIB=-lev -lwslay -lnettle
#  -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs

SRC = mesh/wavefront.spec.cc mesh/rendermesh.spec.cc ../src/mesh/wavefront.cc \
	  livelink/livelink.spec.cc ../src/livelink/livelinkframe.cc \
	  freemocap/freemocap.spec.cc ../src/freemocap/freemocap.cc \
	  ev/timer.spec.cc ../src/ev/timer.cc \
	  ev/async.spec.cc ../src/ev/async.cc \
	  opencv/opencv.spec.cc \
	  fs/fs.spec.cc ../src/fs_impl.cc ../src/generated/fs.cc \
	  main.cc ../src/util.cc ../upstream/kaffeeklatsch/src/kaffeeklatsch.cc \
	  $(patsubst %.cc,$(CORBA_PATH)/corba/%.cc,$(CORBA_SRC))

# ../src/opencv/loop.cc ../src/opencv/videoreader.cc \

OBJ = $(SRC:.cc=.o) 

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
	$(CXX) $(CORBA_FLAGS) $(PROTOBUF_FLAGS) $(CFLAGS) $(WSLAY_FLAGS) $(OPENCV_FLAGS) \
	-c -o $*.o $*.cc

.c.o:
	@echo compiling $*.c ...
	$(CC) $(MEM) -g -O2 -DHAVE_CONFIG_H -I../upstream/wslay/lib/ -I../upstream/wslay/lib/includes -c -o $*.o $*.c

fs/fs.hh: fs/fs.idl
	bun ../upstream/corba.js/src/idl/idl.ts --cc-all fs/fs.idl

corba/interface/interface.hh: corba/interface/interface.idl
	bun ../upstream/corba.js/src/idl/idl.ts --cc-all corba/interface/interface.idl

makehuman.hh: makehuman.idl
	bun ../upstream/corba.js/src/idl/idl.ts --cc-all makehuman.idl

# DO NOT DELETE

mesh/wavefront.spec.o: ../src/mesh/wavefront.hh
mesh/rendermesh.spec.o: ../src/mesh/wavefront.hh
../src/mesh/wavefront.o: ../src/mesh/wavefront.hh ../src/util.hh
../src/mesh/wavefront.o: ../upstream/fast_float/fast_float.h
livelink/livelink.spec.o: ../src/livelink/livelinkframe.hh
../src/livelink/livelinkframe.o: ../src/livelink/livelinkframe.hh
freemocap/freemocap.spec.o: ../src/freemocap/freemocap.hh
freemocap/freemocap.spec.o: ../src/mediapipe/blazepose.hh
../src/freemocap/freemocap.o: ../src/freemocap/freemocap.hh
../src/freemocap/freemocap.o: ../src/mediapipe/blazepose.hh ../src/util.hh
../src/freemocap/freemocap.o: ../upstream/fast_float/fast_float.h
ev/timer.spec.o: ../src/ev/timer.hh
../src/ev/timer.o: ../src/ev/timer.hh
ev/async.spec.o: ../src/ev/async.hh ../src/ev/timer.hh
../src/ev/async.o: ../src/ev/async.hh
../src/fs_impl.o: ../src/fs_impl.hh ../src/generated/fs_skel.hh
../src/fs_impl.o: ../src/generated/fs.hh
../src/generated/fs.o: ../src/generated/fs.hh ../src/generated/fs_skel.hh
../src/generated/fs.o: ../src/generated/fs_stub.hh
../src/util.o: ../src/util.hh ../upstream/fast_float/fast_float.h
../upstream/kaffeeklatsch/src/kaffeeklatsch.o: ../upstream/kaffeeklatsch/src/kaffeeklatsch.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/corba.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/skeleton.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/stub.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/ior.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/naming.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/net/protocol.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/url.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/util/hexdump.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/util/logger.hh
../upstream/corba.cc/src/corba/ior.o: ../upstream/corba.cc/src/corba/ior.hh
../upstream/corba.cc/src/corba/ior.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/ior.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/ior.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/ior.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/skeleton.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/skeleton.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/stub.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/net/protocol.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/corba.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/skeleton.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/stub.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/ior.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/util/logger.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/util/hexdump.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/net/protocol.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/cdr.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/cdr.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/url.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/corba.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/skeleton.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/stub.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/ior.hh
../upstream/corba.cc/src/corba/url.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/naming.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/skeleton.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/stub.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/util/hexdump.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/ior.hh
../upstream/corba.cc/src/corba/util/logger.o: ../upstream/corba.cc/src/corba/util/logger.hh
../upstream/corba.cc/src/corba/net/connection.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/connection.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/connection.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/connection.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/net/connection.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/net/connection.o: ../upstream/corba.cc/src/corba/net/protocol.hh
../upstream/corba.cc/src/corba/net/connection.o: ../upstream/corba.cc/src/corba/stub.hh
../upstream/corba.cc/src/corba/net/connection.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/net/connection.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/connection.o: ../upstream/corba.cc/src/corba/util/logger.hh
../upstream/corba.cc/src/corba/net/stream2packet.o: ../upstream/corba.cc/src/corba/net/stream2packet.hh
../upstream/corba.cc/src/corba/net/stream2packet.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/net/stream2packet.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/net/stream2packet.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/tcp/protocol.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/protocol.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/tcp/connection.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/stream2packet.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/tcp/connection.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/tcp/protocol.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/protocol.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/stream2packet.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/util/logger.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/ws/protocol.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/tcp/protocol.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/protocol.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/ws/connection.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/stream2packet.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/ws/connection.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/ws/protocol.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/tcp/protocol.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/protocol.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/stream2packet.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/util/logger.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/util/createAcceptKey.hh
../upstream/corba.cc/src/corba/net/util/socket.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/net/util/createAcceptKey.o: ../upstream/corba.cc/src/corba/net/util/createAcceptKey.hh
