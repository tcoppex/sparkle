
# Spärkle
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

![screenshot](https://i.imgur.com/TPVEcoe.png)

Spärkle is a particle engine running entirely on the GPU, inspired by the work of SquareEnix Advance Technology Division on *Agni's Philosophy*.

It is written in **C++ 14** and **OpenGL 4.4**.

## Features

- Complete Compute Shader based GPU pipeline,
- Bitonic Sorting for alpha-blending,
- Curl Noise,
- 3D Vector Field,
- *Structure of Arrays* and *Array of Structures* data layout patterns.

For more images, check the [gallery](https://imgur.com/a/uMMGV).

## Quickstart

We will be using the command-line on Unix and [Git Bash](https://git-for-windows.github.io/) on Windows.

### Dependencies

The following dependencies are pulled in as submodules.

- [GLFW 3.2.1](https://github.com/glfw/glfw) as window manager,
- [GLM 9.8](https://github.com/g-truc/glm/releases/tag/0.9.8.1) as mathematics library.
- [imgui 1.6](https://github.com/ocornut/imgui) as user interface.

To retrieve them, type the following command line in the project directory :
```bash
git submodule init
git submodule update
```

### Build

We will first create a build directory then generate the CMake cache depending on your system.

```bash
mkdir BUILDs && cd BUILDs
```

On **Unix**, using Makefile (*replace `$NUM_CPU` by the number of core you want to use*) :
```bash
cmake .. -G Makefile -DCMAKE_BUILD_TYPE=Release
make -j$NUM_CPU
```

On **Windows**, using MSVC 15 for x64:
```bash
cmake .. -G "Visual Studio 15 2017 Win64"
cmake --build . --target ALL_BUILD --config Release
```

*Notes:*

 1. *Using CMake, the build configuration type (ie. Debug, Release) is set at Build Time with MSVC and at Cache Generation Time with Makefile.*

 2. *OpenGL extensions are generated automatically by a custom [Python](https://www.python.org/downloads/) script.  Alternatively [GLEW](http://glew.sourceforge.net/) can be used by specifying the option `-DUSE_GLEW=ON` to CMake.*

### Run

The binary can be found in the project `./bin/` directory:
```bash
../bin/sparkle_demo
```

[//]: # (## Directory structure)
[//]: # (## Known bugs)

---

## References

- *Practical Applications of Compute for Simulation in Agni's Philosophy*, Napaporn Metaaphanon, [GPU Compute for Graphics, ACM SIGGRAPH ASIA 2014 Courses](http://www.jp.square-enix.com/tech/library/pdf/SiggraphAsia2014_simulation.pdf),
- *Curl noise for procedural fluid flow*, R. Bridson, J. Hourihan, and M. Nordenstam, [Proc. ACM SIGGRAPH 2007](https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph2007-curlnoise.pdf),
- *Noise-Based Particles*, Philip Rideout, [The Little Grasshoper](http://prideout.net/blog/?p=63),
- *Implementing Improved Perlin Noise*, Simon Green, [GPU Gems 2](https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter26.html)

## License

*Spärkle* is released under the *MIT* license.
