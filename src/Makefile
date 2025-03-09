APP=backend

APP_SRC=main.cc \
		makehuman_impl.cc \
		fs_impl.cc \
		util.cc mesh/wavefront.cc \
		opencv/videocamera.cc opencv/videosource.cc opencv/videoreader.cc \
		mediapipe/mediapipetask_impl.cc mediapipe/face.cc mediapipe/pose.cc \
		livelink/livelink.cc livelink/livelinkframe.cc \
		chordata/chordata.cc \
		freemocap/freemocap.cc \
		ev/timer.cc ev/udpserver.cc

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

HAVE_MEDIAPIPE=1
HAVE_METAL=1
MEMCHECK=-fsanitize=address -fsanitize=leak

CORBA_PATH=../upstream/corba.cc/src
CORBA_SRC=orb.cc ior.cc skeleton.cc stub.cc giop.cc cdr.cc url.cc \
	naming.cc \
	util/hexdump.cc util/logger.cc \
	net/connection.cc net/stream2packet.cc \
	net/tcp/protocol.cc net/tcp/connection.cc \
	net/ws/protocol.cc net/ws/connection.cc \
	net/util/socket.cc net/util/createAcceptKey.cc

GLM_FLAGS=-I../upstream/glm

CXX=/usr/local/opt/llvm/bin/clang++
LDFLAGS=-L/usr/local/opt/llvm/lib/c++ -Wl,-rpath,/usr/local/opt/llvm/lib/c++ -L/usr/local/lib

OPENCV_FLAGS=-I/usr/local/include/opencv4
OPENCV_LFLAGS=-L/usr/local/lib/opencv4
WSLAY_FLAGS=-I/usr/local/include

ifeq ($(HAVE_MEDIAPIPE), 1)
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
MACOS_LFLAGS=-framework Metal -framework Foundation -framework Cocoa -framework CoreGraphics -framework MetalKit -framework AVFoundation -framework CoreMedia
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
	-O0 -g -I/usr/local/opt/llvm/include/c++ \
	-I$(CORBA_PATH) $(GLM_FLAGS) $(MEDIAPIPE_CFLAGS) $(MACOS_CFLAGS)
LIB=$(MEMCHECK) $(MEDIAPIPE_LFLAGS) $(OPENCV_LFLAGS) $(MACOS_LFLAGS) -fno-objc-arc \
	 -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs \
	-lev -lwslay -lnettle

SRC=$(APP_SRC) $(IDL_SRC) $(MACOS_SRC) \
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

main.o: makehuman_impl.hh generated/makehuman_skel.hh
main.o: ../upstream/corba.cc/src/corba/corba.hh
main.o: ../upstream/corba.cc/src/corba/orb.hh
main.o: ../upstream/corba.cc/src/corba/coroutine.hh
main.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
main.o: ../upstream/corba.cc/src/corba/giop.hh
main.o: ../upstream/corba.cc/src/corba/cdr.hh
main.o: ../upstream/corba.cc/src/corba/blob.hh
main.o: ../upstream/corba.cc/src/corba/net/connection.hh
main.o: ../upstream/corba.cc/src/corba/blob.hh
main.o: ../upstream/corba.cc/src/corba/coroutine.hh
main.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
main.o: ../upstream/corba.cc/src/corba/skeleton.hh
main.o: ../upstream/corba.cc/src/corba/object.hh
main.o: ../upstream/corba.cc/src/corba/stub.hh
main.o: ../upstream/corba.cc/src/corba/ior.hh
main.o: ../upstream/corba.cc/src/corba/exception.hh
main.o: ../upstream/corba.cc/src/corba/orb.hh
main.o: ../upstream/corba.cc/src/corba/giop.hh
main.o: ../upstream/corba.cc/src/corba/coroutine.hh generated/makehuman.hh
main.o: opencv/videowriter.hh opencv/videoreader.hh opencv/videosource.hh
main.o: mediapipe/blazepose.hh mediapipe/mediapipetask_impl.hh
main.o: generated/makehuman_skel.hh fs_impl.hh generated/fs_skel.hh
main.o: generated/fs.hh ../upstream/corba.cc/src/corba/net/ws/protocol.hh
main.o: ../upstream/corba.cc/src/corba/net/tcp/protocol.hh
main.o: ../upstream/corba.cc/src/corba/net/protocol.hh
main.o: ../upstream/corba.cc/src/corba/net/connection.hh
main.o: ../upstream/corba.cc/src/corba/util/logger.hh chordata/chordata.hh
main.o: ev/udpserver.hh ev/timer.hh freemocap/freemocap.hh
main.o: mediapipe/blazepose.hh livelink/livelink.hh captureengine.hh
main.o: livelink/livelinkframe.hh mediapipe/face.hh
main.o: mediapipe/mediapipetask_impl.hh mediapipe/pose.hh
main.o: opencv/videocamera.hh util.hh ../upstream/fast_float/fast_float.h
main.o: macos/video/videocamera_impl.hh opencv/videocamera.hh
makehuman_impl.o: makehuman_impl.hh generated/makehuman_skel.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/corba.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/orb.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
makehuman_impl.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/giop.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/cdr.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/blob.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/net/connection.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/blob.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/skeleton.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/object.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/stub.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/ior.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/exception.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/orb.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/giop.hh
makehuman_impl.o: ../upstream/corba.cc/src/corba/coroutine.hh
makehuman_impl.o: generated/makehuman.hh opencv/videowriter.hh
makehuman_impl.o: opencv/videoreader.hh opencv/videosource.hh
makehuman_impl.o: mediapipe/blazepose.hh mediapipe/mediapipetask_impl.hh
makehuman_impl.o: generated/makehuman_skel.hh ev/timer.hh
makehuman_impl.o: freemocap/freemocap.hh mediapipe/blazepose.hh
makehuman_impl.o: livelink/livelink.hh ev/udpserver.hh captureengine.hh
makehuman_impl.o: livelink/livelinkframe.hh macos/video/videocamera_impl.hh
makehuman_impl.o: opencv/videocamera.hh opencv/videocamera.hh util.hh
makehuman_impl.o: ../upstream/fast_float/fast_float.h
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
mesh/wavefront.o: mesh/wavefront.hh util.hh
mesh/wavefront.o: ../upstream/fast_float/fast_float.h
opencv/videocamera.o: opencv/videocamera.hh generated/makehuman_skel.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/corba.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/orb.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/coroutine.hh
opencv/videocamera.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/giop.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/cdr.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/blob.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/net/connection.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/blob.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/coroutine.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/skeleton.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/object.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/stub.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/ior.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/exception.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/orb.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/giop.hh
opencv/videocamera.o: ../upstream/corba.cc/src/corba/coroutine.hh
opencv/videocamera.o: generated/makehuman.hh opencv/videosource.hh
opencv/videosource.o: opencv/videosource.hh
opencv/videoreader.o: opencv/videoreader.hh opencv/videosource.hh util.hh
opencv/videoreader.o: ../upstream/fast_float/fast_float.h
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
mediapipe/mediapipetask_impl.o: generated/makehuman.hh mediapipe/pose.hh
mediapipe/mediapipetask_impl.o: mediapipe/face.hh
mediapipe/face.o: mediapipe/face.hh mediapipe/mediapipetask_impl.hh
mediapipe/face.o: generated/makehuman_skel.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/corba.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/orb.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/coroutine.hh
mediapipe/face.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/giop.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/cdr.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/blob.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/net/connection.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/blob.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/coroutine.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/skeleton.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/object.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/stub.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/ior.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/exception.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/orb.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/giop.hh
mediapipe/face.o: ../upstream/corba.cc/src/corba/coroutine.hh
mediapipe/face.o: generated/makehuman.hh makehuman_impl.hh
mediapipe/face.o: generated/makehuman_skel.hh opencv/videowriter.hh
mediapipe/face.o: opencv/videoreader.hh opencv/videosource.hh
mediapipe/face.o: mediapipe/blazepose.hh mediapipe/mediapipetask_impl.hh
mediapipe/pose.o: mediapipe/pose.hh mediapipe/mediapipetask_impl.hh
mediapipe/pose.o: generated/makehuman_skel.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/corba.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/orb.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/coroutine.hh
mediapipe/pose.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/giop.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/cdr.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/blob.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/net/connection.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/blob.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/coroutine.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/skeleton.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/object.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/stub.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/ior.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/exception.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/orb.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/giop.hh
mediapipe/pose.o: ../upstream/corba.cc/src/corba/coroutine.hh
mediapipe/pose.o: generated/makehuman.hh makehuman_impl.hh
mediapipe/pose.o: generated/makehuman_skel.hh opencv/videowriter.hh
mediapipe/pose.o: opencv/videoreader.hh opencv/videosource.hh
mediapipe/pose.o: mediapipe/blazepose.hh mediapipe/mediapipetask_impl.hh
livelink/livelink.o: livelink/livelink.hh ev/udpserver.hh captureengine.hh
livelink/livelink.o: ../upstream/corba.cc/src/corba/util/hexdump.hh
livelink/livelink.o: ../upstream/corba.cc/src/corba/blob.hh
livelink/livelink.o: livelink/livelinkframe.hh
livelink/livelinkframe.o: livelink/livelinkframe.hh
chordata/chordata.o: chordata/chordata.hh ev/udpserver.hh
freemocap/freemocap.o: freemocap/freemocap.hh mediapipe/blazepose.hh util.hh
freemocap/freemocap.o: ../upstream/fast_float/fast_float.h
ev/timer.o: ev/timer.hh
ev/udpserver.o: ev/udpserver.hh
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
macos/video/videocamera_impl.o: opencv/videocamera.hh
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
macos/video/videocamera_impl.o: generated/makehuman.hh opencv/videosource.hh
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
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/protocol.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/tcp/protocol.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/exception.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/orb.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/giop.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/cdr.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/tcp/connection.o: ../upstream/corba.cc/src/corba/util/logger.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/protocol.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/net/ws/protocol.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/connection.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/blob.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/coroutine.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/upstream/async.cc/src/async.hh
../upstream/corba.cc/src/corba/net/ws/connection.o: ../upstream/corba.cc/src/corba/net/util/socket.hh
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
