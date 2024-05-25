APP=backend

APP_SRC=main.cc makehuman_impl.cc makehuman.cc util.cc mesh/wavefront.cc \
	    livelink/livelink.cc livelink/livelinkframe.cc livelink/udpserver.cc
IDL_GEN=makehuman.hh makehuman.cc makehuman_skel.hh makehuman_stub.hh

# how to get core dumps on macos
# chmod 1777 /cores
# ulimit -c unlimited
# lldb /cores/core.15593 backend 
# bt all

HAVE_MEDIAPIPE=1
HAVE_METAL=1
MEMCHECK=-fsanitize=address -fsanitize=leak

CORBA_PATH=../upstream/corba.cc/src
CORBA_SRC=orb.cc ior.cc skeleton.cc stub.cc giop.cc cdr.cc url.cc \
	naming.cc hexdump.cc \
	net/ws.cc net/ws/socket.cc net/ws/createAcceptKey.cc
CXX=/usr/local/opt/llvm/bin/clang++
LDFLAGS=-L/usr/local/opt/llvm/lib/c++ -Wl,-rpath,/usr/local/opt/llvm/lib/c++ -L/usr/local/lib

OPENCV_FLAGS=-I/usr/local/opt/opencv@3/include
WSLAG_FLAGS=-I/usr/local/include

ifeq ($(HAVE_MEDIAPIPE), 1)
MEDIAPIPE_CC_LIB=../upstream/mediapipe_cc_lib
MEDIAPIPE_CC_LIB_DIR=$(MEDIAPIPE_CC_LIB)/bazel-bin/cc_lib
MEDIAPIPE_CFLAGS=-I$(MEDIAPIPE_CC_LIB) -DHAVE_MEDIAPIPE=1
MEDIAPIPE_LFLAGS=-L$(MEDIAPIPE_CC_LIB_DIR) -lmediapipe
endif

ifeq ($(HAVE_METAL), 1)
METAL_SRC=metal/metal.cc metal/target.cc metal/renderer.cc metal/renderapp.cc metal/algorithms.cc
METAL_CFLAGS=-ObjC++ -fno-objc-arc -I../upstream/metal-cpp -I../upstream/metal-cpp-extensions -DHAVE_METAL=1
METAL_LFLAGS=-framework Metal -framework Foundation -framework Cocoa -framework CoreGraphics -framework MetalKit -framework AVFoundation -framework CoreMedia
endif

# CFLAGS=-std=c++23 -fmodules
CFLAGS=-std=c++23 $(MEMCHECK) \
	-Wall -Wextra -Werror=return-type -Werror=shadow \
	-Wno-deprecated-anon-enum-enum-conversion -Wno-module-import-in-extern-c -Wno-unused-parameter \
	-O0 -g -I/usr/local/opt/llvm/include/c++ \
	-I$(CORBA_PATH) $(MEDIAPIPE_CFLAGS) $(METAL_CFLAGS)
LIB=$(MEMCHECK) $(MEDIAPIPE_LFLAGS) $(METAL_LFLAGS) -fno-objc-arc \
	-L/usr/local/opt/opencv@3/lib -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs \
	-lev -lwslay -lnettle

# SRC=metal/metal.cc metal/renderer.cc metal/renderapp.cc metal/algorithms.cc util.cc mesh/wavefront.cc
SRC=$(APP_SRC) $(patsubst %.cc,$(CORBA_PATH)/corba/%.cc,$(CORBA_SRC)) $(METAL_SRC)
OBJ = $(SRC:.cc=.o)

all: $(APP)

depend:
	makedepend -I$(CORBA_PATH) -Y $(SRC)

run:
	cd .. && DYLD_LIBRARY_PATH=upstream/mediapipe_cc_lib/bazel-bin/cc_lib ./src/$(APP)

valgrind:
	/usr/local/Cellar/valgrind/HEAD-93191c1/bin/valgrind --track-origins=yes ./$(APP)

clean:
	rm -f $(OBJ) $(IDL_GEN)

.SUFFIXES: .idl .cc .c .h .hh .o

$(IDL_GEN): ../upstream/makehuman.js/src/net/makehuman.idl
	bun ../upstream/corba.js/src/idl/idl.ts --cc-all -o . ../upstream/makehuman.js/src/net/makehuman.idl

$(APP): $(OBJ)
	@echo "linking..."
	$(CXX) $(LDFLAGS) $(LIB) $(OBJ) -o $(APP)

.cc.o:
	@echo compiling $*.cc ...
	$(CXX) $(CFLAGS) $(WSLAG_FLAGS) $(OPENCV_FLAGS) \
	-c -o $*.o $*.cc

# DO NOT DELETE

main.o: ../upstream/corba.cc/src/corba/corba.hh
main.o: ../upstream/corba.cc/src/corba/orb.hh
main.o: ../upstream/corba.cc/src/corba/coroutine.hh
main.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
main.o: ../upstream/corba.cc/src/corba/giop.hh
main.o: ../upstream/corba.cc/src/corba/cdr.hh
main.o: ../upstream/corba.cc/src/corba/blob.hh
main.o: ../upstream/corba.cc/src/corba/skeleton.hh
main.o: ../upstream/corba.cc/src/corba/object.hh
main.o: ../upstream/corba.cc/src/corba/stub.hh
main.o: ../upstream/corba.cc/src/corba/ior.hh
main.o: ../upstream/corba.cc/src/corba/exception.hh
main.o: ../upstream/corba.cc/src/corba/net/ws.hh
main.o: ../upstream/corba.cc/src/corba/protocol.hh livelink/livelink.hh
main.o: livelink/udpserver.hh livelink/livelinkframe.hh makehuman_impl.hh
main.o: makehuman_skel.hh ../upstream/corba.cc/src/corba/orb.hh
main.o: ../upstream/corba.cc/src/corba/giop.hh
main.o: ../upstream/corba.cc/src/corba/coroutine.hh makehuman.hh
makehuman_impl.o: makehuman_impl.hh makehuman_skel.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/corba.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/orb.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
makehuman_impl.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/giop.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/cdr.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/blob.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/skeleton.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/object.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/stub.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/ior.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/exception.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/orb.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/giop.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh makehuman.hh
makehuman.o: ../upstream/corba.cc/src/corba/corba.hh
makehuman.o: ../upstream/corba.cc/src/corba/orb.hh
makehuman.o: ../upstream/corba.cc/src/corba/coroutine.hh
makehuman.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
makehuman.o: ../upstream/corba.cc/src/corba/giop.hh
makehuman.o: ../upstream/corba.cc/src/corba/cdr.hh
makehuman.o: ../upstream/corba.cc/src/corba/blob.hh
makehuman.o: ../upstream/corba.cc/src/corba/skeleton.hh
makehuman.o: ../upstream/corba.cc/src/corba/object.hh
makehuman.o: ../upstream/corba.cc/src/corba/stub.hh
makehuman.o: ../upstream/corba.cc/src/corba/ior.hh
makehuman.o: ../upstream/corba.cc/src/corba/exception.hh
makehuman.o: ../upstream/corba.cc/src/corba/orb.hh
makehuman.o: ../upstream/corba.cc/src/corba/giop.hh
makehuman.o: ../upstream/corba.cc/src/corba/coroutine.hh makehuman.hh
makehuman.o: makehuman_skel.hh makehuman_stub.hh
util.o: util.hh
mesh/wavefront.o: mesh/wavefront.hh ../upstream/fast_float/fast_float.h
mesh/wavefront.o: util.hh
livelink/livelink.o: livelink/livelink.hh livelink/udpserver.hh
livelink/livelink.o: livelink/livelinkframe.hh
livelink/livelink.o: ../upstream/corba.cc/src/corba/hexdump.hh
livelink/livelink.o: ../upstream/corba.cc/src/corba/blob.hh
livelink/livelinkframe.o: livelink/livelinkframe.hh
livelink/udpserver.o: livelink/udpserver.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/corba.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/skeleton.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/stub.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/ior.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/hexdump.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/naming.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/protocol.hh
../upstream/corba.cc/src/corba/orb.o: ../upstream/corba.cc/src/corba/url.hh
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
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/stub.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/stub.o: ../upstream/corba.cc/src/corba/protocol.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/corba.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/skeleton.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/object.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/stub.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/ior.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/hexdump.hh
../upstream/corba.cc/src/corba/giop.o: ../upstream/corba.cc/src/corba/protocol.hh
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
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/hexdump.hh
../upstream/corba.cc/src/corba/naming.o: ../upstream/corba.cc/src/corba/ior.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/src/corba/net/ws.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/src/corba/protocol.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/src/corba/net/ws/createAcceptKey.hh
../upstream/corba.cc/src/corba/net/ws.o: ../upstream/corba.cc/src/corba/net/ws/socket.hh
../upstream/corba.cc/src/corba/net/ws/createAcceptKey.o: ../upstream/corba.cc/src/corba/net/ws/createAcceptKey.hh
metal/metal.o: metal/metal.hh metal/renderer.hh mesh/wavefront.hh
metal/metal.o: metal/algorithms.hh metal/renderapp.hh metal/target.hh
metal/metal.o: metal/shadertypes.hh
metal/target.o: metal/target.hh metal/shadertypes.hh metal/algorithms.hh
metal/renderer.o: metal/renderer.hh
metal/renderapp.o: metal/renderapp.hh metal/renderer.hh
metal/algorithms.o: metal/algorithms.hh
