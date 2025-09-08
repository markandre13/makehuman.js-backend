# makehuman.js backend

A daemon for [makehuman.js](https://github.com/markandre13/makehuman.js) providing access to [mediapipe_cpp_lib](https://github.com/markandre13/mediapipe_cpp_lib) and [Chordata Motion](https://chordata.cc).

### Build

The software is work in progress

#### macOS

* Xcode 16.2 (bazel 6.5.0 fails to build zlib with Xcode 16.3)
  [Apple Downloads](https://developer.apple.com/download/all/?q=xcode)
* brew
* brew install llvm makedepend opencv libev nettle wslay python@3.12 numpy
* bazel 6.5.0
* bun

### Technical

* mediapipe will open a window, which on macOS means that the mediapipe code
  needs to run in the main thread
* frontend and backend use websocket to communicate with the intention to
  move to webtransport later. the protocol on top of that is corba.

#### how implement the recorder

```
modes:
  camcorder (face, body, hand)
    camera + videofile
  freemocap (face, body, hand)
    file
  chordata (body)
    remote + capture file
  livelink (face)
    remote + capture file

for now we just start with the camcorder
* on start: show first frame, enter pause mode
* on drag: pause in playing, seek, play if playing
* ...

we put most of the logic into the frontend

```

#### how to extend the collada exporter

function exportCollada(humanMesh: HumanMesh)
    colladaHead() +
    colladaEffects(materials) +
    colladaMaterials(materials) +
    colladaGeometries(s, geometry, materials) + // mesh
    colladaControllers(s, geometry, materials) + // weights
    colladaAnimations() +
    colladaVisualScenes(s, materials) + // skeleton
    colladaScene() +
    colladaTail()
