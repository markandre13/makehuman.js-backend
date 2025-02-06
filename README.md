# makehuman.js backend

A daemon for [makehuman.js](https://github.com/markandre13/makehuman.js) providing access to [mediapipe_cpp_lib](https://github.com/markandre13/mediapipe_cpp_lib) and [Chordata Motion](https://chordata.cc).

### Build

Experts only for now.

### Technical

* mediapipe will open a window, which on macOS means that the mediapipe code
  needs to run in the main thread
* frontend and backend use websocket to communicate with the intention to
  move to webtransport later. the protocol on top of that is corba.

### Code

i'm still in the proof of concept stage hence the code is still messy.

ISSUE: ...

SOLUTION: check if ld is /usr/bin/ld or /blah/anaconda3/bin/ld 

    which -a ld

if so do

    conda deactivate

(e.g. conda is used when building FreeMoCap)

ISSUE

    ERROR: /private/var/tmp/_bazel_mark/c52e233fcbac3692d587d7f5bf1da4db/external/XNNPACK/BUILD.bazel:1297:31: Compiling src/amalgam/gen/avx512fp16.c failed: (Exit 1): wrapped_clang failed: error executing command (from target @XNNPACK//:avx512fp16_prod_microkernels) external/local_config_cc/wrapped_clang '-D_FORTIFY_SOURCE=1' -fstack-protector -fcolor-diagnostics -Wall -Wt

    Use --sandbox_debug to see verbose messages from the sandbox and retain the sandbox build root for debugging
    clang: error: unknown argument: '-mavx512fp16'
    Error in child process '/usr/bin/xcrun'. 1
    Target //cc_lib:mediapipe failed to build
    Use --verbose_failures to see the command lines of failed build steps.
    INFO: Elapsed time: 656.551s, Critical Path: 480.07s
    INFO: 2062 processes: 58 internal, 2004 darwin-sandbox.
    FAILED: Build did NOT complete successfully
    make[1]: *** [all] Error 1
    make: *** [build] Error 2

https://github.com/google-ai-edge/mediapipe/issues/5752
https://github.com/tensorflow/tensorflow/issues/70199

ATTEMPTS TO FIX IT:
* switching between Xcode Command Line Tools 14 and 13 => did not help
* conda activate => this worked
* re-install llvm

ld: warning: dylib (../upstream/mediapipe_cc_lib/bazel-bin/cc_lib/libmediapipe.dylib) was built for newer macOS version (13.1) than being linked (12.0)
ld: warning: dylib (/usr/local/Cellar/llvm/19.1.7/lib/clang/19/lib/darwin/libclang_rt.asan_osx_dynamic.dylib) was built for newer macOS version (12.7) than being linked (12.0)
ld: warning: dylib (/usr/local/opt/llvm/lib/c++/libc++.dylib) was built for newer macOS version (12.7) than being linked (12.0)

