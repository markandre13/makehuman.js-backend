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

SRC =  corba/net/ws.spec.cc ../src/corba/net/ws.cc \
	  corba/interface/interface.spec.cc corba/interface/interface.cc \
	  cdr_decoder.spec.cc cdr_encoder.spec.cc giop.spec.cc corba.spec.cc blob.spec.cc \
	  makehuman.cc makehuman_impl.cc \
	  main.cc util.cc fake.cc \
	  ../upstream/kaffeeklatsch/src/kaffeeklatsch.cc \
	  ../src/corba/orb.cc \
	  ../src/corba/ior.cc ../src/corba/skeleton.cc ../src/corba/stub.cc \
	  ../src/corba/giop.cc ../src/corba/cdr.cc \
	  ../src/corba/url.cc ../src/corba/hexdump.cc \
	  ../src/corba/net/ws/socket.cc ../src/corba/net/ws/createAcceptKey.cc

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

corba/net/ws.spec.o: ../src/corba/net/ws.hh ../src/corba/protocol.hh
corba/net/ws.spec.o: ../src/corba/blob.hh ../src/corba/coroutine.hh
corba/net/ws.spec.o: ../upstream/cpptask/src/async.hh makehuman_impl.hh
corba/net/ws.spec.o: makehuman_skel.hh makehuman.hh makehuman_stub.hh util.hh
corba/net/ws.spec.o: ../src/corba/hexdump.hh ../src/corba/coroutine.hh
corba/net/ws.spec.o: ../src/corba/corba.hh ../src/corba/orb.hh
corba/net/ws.spec.o: ../src/corba/giop.hh ../src/corba/cdr.hh
corba/net/ws.spec.o: ../src/corba/skeleton.hh ../src/corba/object.hh
corba/net/ws.spec.o: ../src/corba/stub.hh ../src/corba/ior.hh
corba/net/ws.spec.o: ../src/corba/exception.hh
../src/corba/net/ws.o: ../src/corba/net/ws.hh ../src/corba/protocol.hh
../src/corba/net/ws.o: ../src/corba/blob.hh ../src/corba/coroutine.hh
../src/corba/net/ws.o: ../upstream/cpptask/src/async.hh ../src/corba/orb.hh
../src/corba/net/ws.o: ../src/corba/giop.hh ../src/corba/cdr.hh
../src/corba/net/ws.o: ../src/corba/net/ws/createAcceptKey.hh
../src/corba/net/ws.o: ../src/corba/net/ws/socket.hh
../src/corba/net/ws.o: ../../../upstream/wslay/lib/wslay_event.h
../src/corba/net/ws.o: ../../../upstream/wslay/lib/wslay_queue.h
corba/interface/interface.spec.o: corba/interface/interface.hh util.hh
corba/interface/interface.spec.o: ../src/corba/hexdump.hh
corba/interface/interface.spec.o: ../src/corba/blob.hh
corba/interface/interface.spec.o: ../src/corba/coroutine.hh
corba/interface/interface.spec.o: ../upstream/cpptask/src/async.hh fake.hh
corba/interface/interface.spec.o: corba/interface/interface_skel.hh
corba/interface/interface.spec.o: corba/interface/interface_stub.hh
corba/interface/interface.o: corba/interface/interface.hh
corba/interface/interface.o: corba/interface/interface_skel.hh
corba/interface/interface.o: corba/interface/interface_stub.hh
cdr_decoder.spec.o: ../src/corba/cdr.hh ../src/corba/blob.hh
cdr_encoder.spec.o: ../src/corba/cdr.hh ../src/corba/blob.hh
giop.spec.o: ../src/corba/giop.hh ../src/corba/cdr.hh ../src/corba/blob.hh
giop.spec.o: util.hh ../src/corba/hexdump.hh ../src/corba/coroutine.hh
giop.spec.o: ../upstream/cpptask/src/async.hh
corba.spec.o: fake.hh ../src/corba/corba.hh ../src/corba/orb.hh
corba.spec.o: ../src/corba/coroutine.hh ../upstream/cpptask/src/async.hh
corba.spec.o: ../src/corba/giop.hh ../src/corba/cdr.hh ../src/corba/blob.hh
corba.spec.o: ../src/corba/skeleton.hh ../src/corba/object.hh
corba.spec.o: ../src/corba/stub.hh ../src/corba/ior.hh
corba.spec.o: ../src/corba/exception.hh ../src/corba/orb.hh
corba.spec.o: ../src/corba/protocol.hh ../src/corba/url.hh makehuman.hh
corba.spec.o: makehuman_skel.hh makehuman_stub.hh util.hh
corba.spec.o: ../src/corba/hexdump.hh ../src/corba/coroutine.hh
blob.spec.o: ../src/corba/blob.hh
makehuman.o: makehuman.hh makehuman_skel.hh makehuman_stub.hh
makehuman_impl.o: makehuman_impl.hh makehuman_skel.hh makehuman.hh
util.o: util.hh ../src/corba/hexdump.hh ../src/corba/blob.hh
util.o: ../src/corba/coroutine.hh ../upstream/cpptask/src/async.hh
fake.o: fake.hh
../upstream/kaffeeklatsch/src/kaffeeklatsch.o: ../upstream/kaffeeklatsch/src/kaffeeklatsch.hh
../src/corba/corba.o: ../src/corba/orb.hh ../src/corba/coroutine.hh
../src/corba/corba.o: ../upstream/cpptask/src/async.hh ../src/corba/giop.hh
../src/corba/corba.o: ../src/corba/cdr.hh ../src/corba/blob.hh
../src/corba/orb.o: ../src/corba/orb.hh ../src/corba/coroutine.hh
../src/corba/orb.o: ../upstream/cpptask/src/async.hh ../src/corba/giop.hh
../src/corba/orb.o: ../src/corba/cdr.hh ../src/corba/blob.hh
../src/corba/orb.o: ../src/corba/corba.hh ../src/corba/skeleton.hh
../src/corba/orb.o: ../src/corba/object.hh ../src/corba/stub.hh
../src/corba/orb.o: ../src/corba/ior.hh ../src/corba/exception.hh
../src/corba/orb.o: ../src/corba/hexdump.hh ../src/corba/protocol.hh
../src/corba/orb.o: ../src/corba/url.hh
../src/corba/ior.o: ../src/corba/ior.hh ../src/corba/object.hh
../src/corba/ior.o: ../src/corba/blob.hh ../src/corba/giop.hh
../src/corba/ior.o: ../src/corba/cdr.hh
../src/corba/skeleton.o: ../src/corba/skeleton.hh ../src/corba/object.hh
../src/corba/skeleton.o: ../src/corba/blob.hh ../src/corba/coroutine.hh
../src/corba/skeleton.o: ../upstream/cpptask/src/async.hh ../src/corba/orb.hh
../src/corba/skeleton.o: ../src/corba/giop.hh ../src/corba/cdr.hh
../src/corba/stub.o: ../src/corba/stub.hh ../src/corba/object.hh
../src/corba/stub.o: ../src/corba/blob.hh
../src/corba/giop.o: ../src/corba/giop.hh ../src/corba/cdr.hh
../src/corba/giop.o: ../src/corba/blob.hh ../src/corba/corba.hh
../src/corba/giop.o: ../src/corba/orb.hh ../src/corba/coroutine.hh
../src/corba/giop.o: ../upstream/cpptask/src/async.hh
../src/corba/giop.o: ../src/corba/skeleton.hh ../src/corba/object.hh
../src/corba/giop.o: ../src/corba/stub.hh ../src/corba/ior.hh
../src/corba/giop.o: ../src/corba/exception.hh ../src/corba/hexdump.hh
../src/corba/giop.o: ../src/corba/protocol.hh
../src/corba/cdr.o: ../src/corba/cdr.hh ../src/corba/blob.hh
../src/corba/url.o: ../src/corba/url.hh ../src/corba/cdr.hh
../src/corba/url.o: ../src/corba/blob.hh ../src/corba/giop.hh
../src/corba/url.o: ../src/corba/corba.hh ../src/corba/orb.hh
../src/corba/url.o: ../src/corba/coroutine.hh
../src/corba/url.o: ../upstream/cpptask/src/async.hh ../src/corba/skeleton.hh
../src/corba/url.o: ../src/corba/object.hh ../src/corba/stub.hh
../src/corba/url.o: ../src/corba/ior.hh ../src/corba/exception.hh
../src/corba/net/ws/createAcceptKey.o: ../src/corba/net/ws/createAcceptKey.hh
