# HalideExamples #

This is a repository of examples of Halide programming.

According to Halide's authors, "Halide is a new programming language designed to make it easier to write
high-performance image processing code on modern machines." The examples here serve two purposes:

* To help grow our experience and skill with Halide
* To push Halide in ways its authors may not have imagined

## Building HalideExamples ##

Prerequisites:

* CMake 3.0 (or later)
* g++ 4.8+ (the main developer uses 4.9)
* SDL2
* A recent distribution of Halide

Build script:

For the first build, you must specify the path to your Halide installation:

	$ ./build.sh -DHALIDE_ROOT=&lt;path to Halide&gt;

The example programs will be built and installed into the build-dir/bin directory.

Subsequent builds can be done by invoking the build script with no parameters:

	$ ./build.sh

## Examples ##

### Wave ###

The Wave example uses a simple method to simulate the 2D wave equation and renders the results in
a window using SDL.