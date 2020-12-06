# GTA rendering hook
GTA rendering hook

Implements DirectX11 and Vulkan rendering API to GTA 3, GTA VC and GTA SA.

Currently there are 2 versions, one of which is broken(can't really build it anymore):
* Old one which implemented only DirectX11 and had a few features(only GTA SA)
* New one which at the moment runs only on RTX hardware and uses vk_nv_ray_tracing extension(to be replaced with KHR
 one once it is implemented in release drivers)

We have [discord server](https://discord.gg/rsZEUNW), come and help to improve it!

## How it works?
TL;DR: Currently we have incomplete RW rendering backend implementation that communicates via shared memory 
 to incomplete rendering engine instance running rendering loop based on Vulkan API.

Long version:
 * Game rendering engine code is partially redirected to shared memory task queue
 * Task queue loads game assets to GPU memory, processes various requests like retrieving supported resolutions etc.
 * One of the tasks prepares a frame to framebuffer based on uploaded scene data,
  each frame draw calls is uploaded to 64bit process via task queue.
 * Separation between rendering and game code is done for several reasons:
   * It allows to use newer drivers, and support raytracing extensions
   * It allows to use more memory if needed
   * Surprisingly it works faster!
 * Rendering right now is basic, and very hacky:
   * BLAS is built for each model instance being loaded by the game. For GTA 3 we load some sectors around the player using original lod system.
   * Animations are prepared by compute shaders and rebuilt every frame  
   * TLAS is rebuilt every frame
   * Scene is loaded to GPU, contains all materials, textures, vertex/index buffers and transform data
   * Lights are prepared using very basic tiled culling implementation
   * Primary rays are traced to compute G-Buffers: Albedo, Normals, LinearDepth, Motion vectors and Material params
   * AO rays are traced, temporally accumulated and blurred via bilateral blur with varying kernel(based on temporal variance)
   * Shading for sunlight and each point light inside a tile is generated, then temporally accumulated and blurred via bilateral blur(based on temporal luminance variance)
   * Reflections are traced with one shadow ray per-pixel and blurred via same filter that is used for shading
   * Everything is combined based on material properties etc.
   * Immediate 3D objects are drawn on top(with depth from G-Buffer)
   * Immediate 2D objects are drawn on top

## Current requirements to build development source code
1) VCPkg
2) Cmake
3) Visual Studio/CLion
4) C++ compiler with c++20

## Build instructions
1. Setup libraries via vcpkg(all dependencies are listed in response_file.txt)
2. Prepare environment:
   * GTA_3_FOLDER, GTA_VC_FOLDER and GTA_SA_FOLDER should be set to some places 
     where you want resulting executables to be. 
   * When building 32bit binaries use only BUILD_32BIT_LIBS
   * When building 64bit binaries use BUILD_32BIT_LIBS and ARCH_64BIT
   * Loading mod requires [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) installed.
   * For GTA 3 it'll work only on v1.1 and v1.0 at the moment.
3. Build x86 and x64 libraries/executables in release or debug configuration using configured CMake.
4. Run, test and report bugs and incompatibilities!

## Tips
1) If you encounter bug please report it, and attach .log files and crash dumps and build/game version if possible to bugreport
2) It's buggy and code is bad at the moment, sorry in advance :)
