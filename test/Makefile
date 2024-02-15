APP=tests

# MEM=-fsanitize=address -fsanitize=leak

CXX=/usr/local/opt/llvm/bin/clang++
CFLAGS=-std=c++23 -fmodules $(MEM) -O0 -g \
	-mmacosx-version-min=12.0 \
	-Wall -Wextra -Wno-deprecated-anon-enum-enum-conversion \
	-Werror\=return-type \
	-Wno-unused-parameter \
	-I/usr/local/opt/llvm/include/c++ \
	-I../upstream/kaffeeklatsch/src

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
# LIB=-lprotobuf -lgmod -lwslay -lnettle

SRC = coroutine.spec.cc cdr_decoder.spec.cc cdr_encoder.spec.cc giop.spec.cc corba.spec.cc \
	main.cc util.cc ../upstream/kaffeeklatsch/src/kaffeeklatsch.cc \
 	../src/corba/orb.cc ../src/corba/skeleton.cc ../src/corba/url.cc \
	../src/corba/giop.cc ../src/corba/cdr.cc  ../src/corba/hexdump.cc

OBJ = $(SRC:.cc=.o)

.SUFFIXES: .cc .o

all: $(APP)

depend:
	makedepend -I. -Y $(SRC)

run:
	DYLD_LIBRARY_PATH=$(MEDIAPIPE_CPP_DIR)/library ./$(APP) --reporter=info --report-timing --only=foo

valgrind:
	/usr/local/Cellar/valgrind/HEAD-93191c1/bin/valgrind ./$(APP)

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

cdr_decoder.o: ../src/corba/cdr.hh
cdr_encoder.o: ../src/corba/cdr.hh
giop.o: ../src/corba/giop.hh ../src/corba/cdr.hh util.hh
corba.spec.o: ../src/corba/corba.hh ../src/corba/giop.hh ../src/corba/cdr.hh
corba.spec.o: ../src/corba/coroutine.hh ../src/corba/orb.hh
corba.spec.o: ../src/corba/url.hh
util.o: util.hh
../upstream/kaffeeklatsch/src/kaffeeklatsch.o: ../upstream/kaffeeklatsch/src/kaffeeklatsch.hh
../src/corba/orb.o: ../src/corba/orb.hh ../src/corba/coroutine.hh
../src/corba/orb.o: ../src/corba/giop.hh ../src/corba/cdr.hh
../src/corba/orb.o: ../src/corba/corba.hh ../src/corba/hexdump.hh
../src/corba/orb.o: ../src/corba/url.hh ../src/corba/ws/EventHandler.hh
../src/corba/skeleton.o: ../src/corba/corba.hh ../src/corba/giop.hh
../src/corba/skeleton.o: ../src/corba/cdr.hh ../src/corba/coroutine.hh
../src/corba/skeleton.o: ../src/corba/orb.hh
../src/corba/url.o: ../src/corba/url.hh ../src/corba/cdr.hh
../src/corba/url.o: ../src/corba/giop.hh
../src/corba/giop.o: ../src/corba/giop.hh ../src/corba/cdr.hh
../src/corba/giop.o: ../src/corba/corba.hh ../src/corba/coroutine.hh
../src/corba/cdr.o: ../src/corba/cdr.hh
