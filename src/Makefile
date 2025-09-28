# mediapipe doesn't always build
HAVE_MEDIAPIPE=0
HAVE_METAL=1

APP=backend

APP_SRC=main.cc \
		backend_impl.cc \
		recorder_impl.cc \
		mesh/wavefront.cc \
		mediapipe/mediapipetask_impl.cc \
		opencv/loop.cc opencv/videoreader.cc \
		livelink/livelink.cc livelink/livelinkframe.cc \
		freemocap/freemocap.cc freemocap/player.cc \
		chordata/chordata.cc \
		ev/timer.cc ev/udpserver.cc \
		fs_impl.cc util.cc

# IDL modules
IDL_DEF=makehuman fs
# IDL generated source files
IDL_SRC=$(patsubst %,generated/%.cc,$(IDL_DEF))
# IDL generated source and header files
IDL_GEN=$(patsubst %,generated/%.cc,$(IDL_DEF)) \
		$(patsubst %,generated/%.hh,$(IDL_DEF)) \
		$(patsubst %,generated/%_skel.hh,$(IDL_DEF)) \
		$(patsubst %,generated/%_stub.hh,$(IDL_DEF))
IDL_IDL=$(patsubst %,../upstream/makehuman.js/src/net/%.idl,$(IDL_DEF))

# how to get core dumps on macos
# chmod 1777 /cores
# ulimit -c unlimited
# lldb /cores/core.15593 backend 
# bt all
MEMCHECK=-fsanitize=address -fsanitize=leak

CORBA_PATH=../upstream/corba.cc/src
CORBA_SRC=orb.cc ior.cc skeleton.cc stub.cc giop.cc cdr.cc \
    url.cc naming.cc \
	util/hexdump.cc util/logger.cc \
	net/connection.cc net/stream2packet.cc \
	net/tcp/protocol.cc net/tcp/connection.cc \
	net/ws/protocol.cc net/ws/connection.cc \
	net/util/socket.cc net/util/createAcceptKey.cc

GLM_FLAGS=-I../upstream/glm

HOMEBREW := $(shell brew config | grep HOMEBREW_PREFIX | sed 's/.*: //')

CXX=$(HOMEBREW)/opt/llvm/bin/clang++
LDFLAGS=-L$(HOMEBREW)/opt/llvm/lib/c++ -Wl,-rpath,$(HOMEBREW)/opt/llvm/lib/c++ -L$(HOMEBREW)/lib

OPENCV_FLAGS=-I$(HOMEBREW)/include/opencv4
OPENCV_LFLAGS=-L$(HOMEBREW)/lib/opencv4
WSLAY_FLAGS=-I$(HOMEBREW)/include

ifeq ($(HAVE_MEDIAPIPE), 1)
MEDIAPIPE_SRC=mediapipe/face.cc mediapipe/pose.cc
MEDIAPIPE_CC_LIB=../upstream/mediapipe_cc_lib
MEDIAPIPE_CC_LIB_DIR=$(MEDIAPIPE_CC_LIB)/bazel-bin/cc_lib
MEDIAPIPE_CFLAGS=-I$(MEDIAPIPE_CC_LIB) -DHAVE_MEDIAPIPE=1
MEDIAPIPE_LFLAGS=-L$(MEDIAPIPE_CC_LIB_DIR) -lmediapipe
endif

ifeq ($(HAVE_METAL), 1)
METAL_SRC=metal.cc target.cc renderer.cc renderapp.cc algorithms.cc
VIDEO_SRC=videocamera_impl.cc
# prefix METAL_SRC and VIDEO_SRC with full path
MACOS_SRC=$(patsubst %.cc,macos/metal/%.cc,$(METAL_SRC)) $(patsubst %.cc,macos/video/%.cc,$(VIDEO_SRC))
MACOS_CFLAGS=-ObjC++ -fno-objc-arc -DHAVE_METAL=1
MACOS_LFLAGS=-framework Metal \
			 -framework Foundation \
			 -framework Cocoa \
			 -framework CoreGraphics \
			 -framework MetalKit \
			 -framework AVFoundation \
			 -framework CoreMedia
endif

CFLAGS=-std=c++23 $(MEMCHECK) \
	-Wall -Wextra \
	-Werror=return-type \
	-Werror=shadow \
	-Wno-deprecated-anon-enum-enum-conversion \
	-Wno-module-import-in-extern-c \
	-Wno-unused-parameter \
	-Wno-vla-cxx-extension \
	-Werror=suggest-override \
	-O0 -g -I$(HOMEBREW)/opt/llvm/include/c++ \
	-I$(CORBA_PATH) $(GLM_FLAGS) $(MEDIAPIPE_CFLAGS) $(MACOS_CFLAGS)
LIB=$(MEMCHECK) $(MEDIAPIPE_LFLAGS) $(OPENCV_LFLAGS) $(MACOS_LFLAGS) -fno-objc-arc \
	-lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs \
	-lev -lwslay -lnettle

SRC=$(APP_SRC) $(IDL_SRC) $(MACOS_SRC) $(MEDIAPIPE_SRC) \
	$(patsubst %.cc,$(CORBA_PATH)/corba/%.cc,$(CORBA_SRC))

OBJ = $(SRC:.cc=.o)

all: $(APP)

depend:
	makedepend -I$(CORBA_PATH) -Y $(SRC)

run:
	cd .. && DYLD_LIBRARY_PATH=upstream/mediapipe_cc_lib/bazel-bin/cc_lib ./src/$(APP)

lldb:
	@echo "cd .."
	@echo "lldb src/backend"
	@echo "b <file>:<line>"
	@echo "process launch --environment DYLD_LIBRARY_PATH=upstream/mediapipe_cc_lib/bazel-bin/cc_lib"
	# cd .. && DYLD_LIBRARY_PATH=upstream/mediapipe_cc_lib/bazel-bin/cc_lib lldb ./src/$(APP)

valgrind:
	/usr/local/Cellar/valgrind/HEAD-93191c1/bin/valgrind --track-origins=yes ./$(APP)

clean:
	rm -f $(OBJ) $(IDL_GEN)

.SUFFIXES: .idl .cc .c .h .hh .o

$(IDL_GEN): $(IDL_IDL)
	mkdir -p generated
	bun ../upstream/corba.js/src/idl/idl.ts --cc-all -o generated $(IDL_IDL)

$(APP): $(OBJ)
	@echo "linking..."
	$(CXX) $(LDFLAGS) $(LIB) $(OBJ) -o $(APP)

.cc.o:
	@echo compiling $*.cc ...
	$(CXX) $(CFLAGS) $(WSLAY_FLAGS) $(OPENCV_FLAGS) \
	-c -o $*.o $*.cc

# DO NOT DELETE

main.o: ../upstream/corba.cc/src/corba/net/ws/protocol.hh
main.o: ../upstream/corba.cc/src/corba/net/tcp/protocol.hh
main.o: ../upstream/corba.cc/src/corba/net/protocol.hh
main.o: ../upstream/corba.cc/src/corba/net/connection.hh
main.o: ../upstream/corba.cc/src/corba/blob.hh
main.o: ../upstream/corba.cc/src/corba/coroutine.hh
main.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
main.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
main.o: ../upstream/corba.cc/src/corba/util/logger.hh backend_impl.hh
main.o: generated/makehuman_skel.hh ../upstream/corba.cc/src/corba/corba.hh
main.o: ../upstream/corba.cc/src/corba/orb.hh
main.o: ../upstream/corba.cc/src/corba/coroutine.hh
main.o: ../upstream/corba.cc/src/corba/giop.hh
main.o: ../upstream/corba.cc/src/corba/cdr.hh
main.o: ../upstream/corba.cc/src/corba/blob.hh
main.o: ../upstream/corba.cc/src/corba/net/connection.hh
main.o: ../upstream/corba.cc/src/corba/skeleton.hh
main.o: ../upstream/corba.cc/src/corba/object.hh
main.o: ../upstream/corba.cc/src/corba/stub.hh
main.o: ../upstream/corba.cc/src/corba/ior.hh
main.o: ../upstream/corba.cc/src/corba/exception.hh
main.o: ../upstream/corba.cc/src/corba/orb.hh
main.o: ../upstream/corba.cc/src/corba/giop.hh
main.o: ../upstream/corba.cc/src/corba/coroutine.hh generated/makehuman.hh
main.o: opencv/videowriter.hh opencv/videoreader.hh mediapipe/blazepose.hh
main.o: mediapipe/mediapipetask_impl.hh generated/makehuman_skel.hh
main.o: fs_impl.hh generated/fs_skel.hh generated/fs.hh
main.o: macos/video/videocamera_impl.hh generated/makehuman_skel.hh
main.o: ev/timer.hh chordata/chordata.hh ev/udpserver.hh
main.o: freemocap/freemocap.hh mediapipe/blazepose.hh livelink/livelink.hh
main.o: captureengine.hh livelink/livelinkframe.hh opencv/loop.hh
main.o: lib/threadsync.hh util.hh ../upstream/fast_float/fast_float.h
backend_impl.o: backend_impl.hh generated/makehuman_skel.hh
backend_impl.o: ../upstream/corba.cc/src/corba/corba.hh
backend_impl.o: ../upstream/corba.cc/src/corba/orb.hh
backend_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
backend_impl.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
backend_impl.o: ../upstream/corba.cc/src/corba/giop.hh
backend_impl.o: ../upstream/corba.cc/src/corba/cdr.hh
backend_impl.o: ../upstream/corba.cc/src/corba/blob.hh
backend_impl.o: ../upstream/corba.cc/src/corba/net/connection.hh
backend_impl.o: ../upstream/corba.cc/src/corba/blob.hh
backend_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
backend_impl.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
backend_impl.o: ../upstream/corba.cc/src/corba/skeleton.hh
backend_impl.o: ../upstream/corba.cc/src/corba/object.hh
backend_impl.o: ../upstream/corba.cc/src/corba/stub.hh
backend_impl.o: ../upstream/corba.cc/src/corba/ior.hh
backend_impl.o: ../upstream/corba.cc/src/corba/exception.hh
backend_impl.o: ../upstream/corba.cc/src/corba/orb.hh
backend_impl.o: ../upstream/corba.cc/src/corba/giop.hh
backend_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
backend_impl.o: generated/makehuman.hh opencv/videowriter.hh
backend_impl.o: opencv/videoreader.hh mediapipe/blazepose.hh
backend_impl.o: mediapipe/mediapipetask_impl.hh generated/makehuman_skel.hh
backend_impl.o: livelink/livelinkframe.hh macos/video/videocamera_impl.hh
backend_impl.o: generated/makehuman_skel.hh opencv/loop.hh lib/threadsync.hh
backend_impl.o: recorder_impl.hh util.hh ../upstream/fast_float/fast_float.h
recorder_impl.o: recorder_impl.hh generated/makehuman_skel.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/corba.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/orb.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
recorder_impl.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/giop.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/cdr.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/blob.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/net/connection.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/blob.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/skeleton.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/object.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/stub.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/ior.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/exception.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/orb.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/giop.hh
recorder_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
recorder_impl.o: generated/makehuman.hh opencv/loop.hh lib/threadsync.hh
recorder_impl.o: opencv/videoreader.hh
mesh/wavefront.o: mesh/wavefront.hh util.hh
mesh/wavefront.o: ../upstream/fast_float/fast_float.h
mediapipe/mediapipetask_impl.o: mediapipe/mediapipetask_impl.hh
mediapipe/mediapipetask_impl.o: generated/makehuman_skel.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/corba.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/orb.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/giop.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/cdr.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/blob.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/net/connection.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/blob.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/skeleton.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/object.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/stub.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/ior.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/exception.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/orb.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/giop.hh
mediapipe/mediapipetask_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
mediapipe/mediapipetask_impl.o: generated/makehuman.hh
opencv/loop.o: opencv/loop.hh lib/threadsync.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/coroutine.hh
opencv/loop.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
opencv/loop.o: opencv/videoreader.hh util.hh
opencv/loop.o: ../upstream/fast_float/fast_float.h
opencv/loop.o: macos/video/videocamera_impl.hh generated/makehuman_skel.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/corba.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/orb.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/coroutine.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/giop.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/cdr.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/blob.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/net/connection.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/blob.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/coroutine.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/skeleton.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/object.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/stub.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/ior.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/exception.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/orb.hh
opencv/loop.o: ../upstream/corba.cc/src/corba/giop.hh generated/makehuman.hh
opencv/videoreader.o: opencv/videoreader.hh util.hh
opencv/videoreader.o: ../upstream/fast_float/fast_float.h
livelink/livelink.o: livelink/livelink.hh ev/udpserver.hh captureengine.hh
livelink/livelink.o: ../upstream/corba.cc/src/corba/util/hexdump.hh
livelink/livelink.o: ../upstream/corba.cc/src/corba/blob.hh
livelink/livelink.o: livelink/livelinkframe.hh
livelink/livelinkframe.o: livelink/livelinkframe.hh
freemocap/freemocap.o: freemocap/freemocap.hh mediapipe/blazepose.hh util.hh
freemocap/freemocap.o: ../upstream/fast_float/fast_float.h
freemocap/player.o: freemocap/player.hh ev/timer.hh freemocap/freemocap.hh
freemocap/player.o: mediapipe/blazepose.hh backend_impl.hh
freemocap/player.o: generated/makehuman_skel.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/corba.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/orb.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/coroutine.hh
freemocap/player.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/giop.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/cdr.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/blob.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/net/connection.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/blob.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/coroutine.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/skeleton.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/object.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/stub.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/ior.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/exception.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/orb.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/giop.hh
freemocap/player.o: ../upstream/corba.cc/src/corba/coroutine.hh
freemocap/player.o: generated/makehuman.hh opencv/videowriter.hh
freemocap/player.o: opencv/videoreader.hh mediapipe/blazepose.hh
freemocap/player.o: mediapipe/mediapipetask_impl.hh
freemocap/player.o: generated/makehuman_skel.hh
chordata/chordata.o: chordata/chordata.hh ev/udpserver.hh
ev/timer.o: ev/timer.hh
ev/udpserver.o: ev/udpserver.hh
fs_impl.o: fs_impl.hh generated/fs_skel.hh
fs_impl.o: ../upstream/corba.cc/src/corba/corba.hh
fs_impl.o: ../upstream/corba.cc/src/corba/orb.hh
fs_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
fs_impl.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
fs_impl.o: ../upstream/corba.cc/src/corba/giop.hh
fs_impl.o: ../upstream/corba.cc/src/corba/cdr.hh
fs_impl.o: ../upstream/corba.cc/src/corba/blob.hh
fs_impl.o: ../upstream/corba.cc/src/corba/net/connection.hh
fs_impl.o: ../upstream/corba.cc/src/corba/blob.hh
fs_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
fs_impl.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
fs_impl.o: ../upstream/corba.cc/src/corba/skeleton.hh
fs_impl.o: ../upstream/corba.cc/src/corba/object.hh
fs_impl.o: ../upstream/corba.cc/src/corba/stub.hh
fs_impl.o: ../upstream/corba.cc/src/corba/ior.hh
fs_impl.o: ../upstream/corba.cc/src/corba/exception.hh
fs_impl.o: ../upstream/corba.cc/src/corba/orb.hh
fs_impl.o: ../upstream/corba.cc/src/corba/giop.hh
fs_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh generated/fs.hh
util.o: util.hh ../upstream/fast_float/fast_float.h
generated/makehuman.o: ../upstream/corba.cc/src/corba/corba.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/orb.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/coroutine.hh
generated/makehuman.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/giop.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/cdr.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/blob.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/net/connection.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/blob.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/coroutine.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/skeleton.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/object.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/stub.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/ior.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/exception.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/orb.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/giop.hh
generated/makehuman.o: ../upstream/corba.cc/src/corba/coroutine.hh
generated/makehuman.o: generated/makehuman.hh generated/makehuman_skel.hh
generated/makehuman.o: generated/makehuman_stub.hh
generated/fs.o: ../upstream/corba.cc/src/corba/corba.hh
generated/fs.o: ../upstream/corba.cc/src/corba/orb.hh
generated/fs.o: ../upstream/corba.cc/src/corba/coroutine.hh
generated/fs.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
generated/fs.o: ../upstream/corba.cc/src/corba/giop.hh
generated/fs.o: ../upstream/corba.cc/src/corba/cdr.hh
generated/fs.o: ../upstream/corba.cc/src/corba/blob.hh
generated/fs.o: ../upstream/corba.cc/src/corba/net/connection.hh
generated/fs.o: ../upstream/corba.cc/src/corba/blob.hh
generated/fs.o: ../upstream/corba.cc/src/corba/coroutine.hh
generated/fs.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
generated/fs.o: ../upstream/corba.cc/src/corba/skeleton.hh
generated/fs.o: ../upstream/corba.cc/src/corba/object.hh
generated/fs.o: ../upstream/corba.cc/src/corba/stub.hh
generated/fs.o: ../upstream/corba.cc/src/corba/ior.hh
generated/fs.o: ../upstream/corba.cc/src/corba/exception.hh
generated/fs.o: ../upstream/corba.cc/src/corba/orb.hh
generated/fs.o: ../upstream/corba.cc/src/corba/giop.hh
generated/fs.o: ../upstream/corba.cc/src/corba/coroutine.hh generated/fs.hh
generated/fs.o: generated/fs_skel.hh generated/fs_stub.hh
macos/metal/metal.o: macos/metal/metal.hh macos/metal/renderer.hh
macos/metal/metal.o: livelink/livelinkframe.hh mesh/wavefront.hh
macos/metal/metal.o: macos/metal/algorithms.hh macos/metal/renderapp.hh
macos/metal/metal.o: macos/metal/target.hh macos/metal/shadertypes.hh
macos/metal/target.o: macos/metal/target.hh macos/metal/shadertypes.hh
macos/metal/target.o: macos/metal/algorithms.hh
macos/metal/renderer.o: macos/metal/renderer.hh
macos/metal/renderapp.o: macos/metal/renderapp.hh macos/metal/renderer.hh
macos/metal/algorithms.o: macos/metal/algorithms.hh
macos/video/videocamera_impl.o: macos/video/videocamera_impl.hh
macos/video/videocamera_impl.o: generated/makehuman_skel.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/corba.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/orb.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/giop.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/cdr.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/blob.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/net/connection.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/blob.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/skeleton.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/object.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/stub.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/ior.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/exception.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/orb.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/giop.hh
macos/video/videocamera_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
macos/video/videocamera_impl.o: generated/makehuman.hh
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
