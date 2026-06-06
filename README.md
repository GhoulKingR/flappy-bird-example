# Flappy bird

A flappy bird game made with the [GEV0 engine](https://github.com/GhoulKingR/game-engine-v0).
![screenshot](screenshot.png)

## Requirements
This project depends on the following:
* OpenGL
* glm
* SDL3
* CMake
* Make

Platform-wise, this engine is verified to run on macOS and Linux.

## How to run

Clone the example and cd into it:
```
git clone https://github.com/GhoulKingR/flappy-bird-example.git
cd flappy-bird-example
```

Clone the engine and assets files using this command:
```
git clone https://github.com/GhoulKingR/game-engine-v0.git gev0_engine
git clone https://github.com/samuelcust/flappy-bird-assets assets
```

Create a build directory and cd into it:
```
mkdir build
cd build
```

Build the game:
```
cmake ..
make
```

After this, you can now run the game using by running `./flappybird`.
