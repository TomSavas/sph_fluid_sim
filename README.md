# SPH fluid sim
Simple Smoothed Particle Hydrodynamics fluid simulation using compute shaders.

Simulation based on [this whitepaper on SPH fluid sim](http://rlguy.com/sphfluidsim/).</br>
Visualization based on an [old GDC talk done by Simon Green on screen space fluids](https://developer.download.nvidia.com/presentations/2010/gdc/Direct3D_Effects.pdf).

This should not be taken as a learning source, as the code is rather messy, not to mention there are quite a few optimisations that are missing.

## Preview

https://user-images.githubusercontent.com/13164722/140618489-804a78e8-12eb-4c70-b65b-ad2286990a53.mp4

## Building
Make sure to get the submodules after cloning the repo
```
git submodule init
git submodule update
```
Then build as usual using cmake
```
mkdir build 
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j n # Here n should be something like <core count> + 1
```
This will produce a binary `sph_fluid_sim` that you should be able to run. _Make sure you run it from the build directory, as the shader source and cubemap images are hardcoded as relative paths_!

### Dependencies
Make sure you have GLEW on your system, as well as X server related libs, mainly: libxrandr, libxinerama, libxcursor.

## TODO
+ This could really use a nearest neighbour optimisation in the simulation code. [Some ideas on how to do that can be found here](https://wickedengine.net/2018/05/21/scalabe-gpu-fluid-simulation/).
+ Normal recalculation doesn't seem _quite_ right, needs looking at.
+ There are some weird grid-like artifacts that needs investigation.
+ Overall screen space fluid rendering needs optimisation.

## Licence
This project is licensed under MIT License - see [LICENCE](LICENCE) file for details.
