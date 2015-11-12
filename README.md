# EACSMB
A 3D city building game. Currently under construction.


It's written in pure C using no frameworks. Dependencies are Xlib, FreeType, fontconfig, libPNG, GLEW, and OpenGL 4.5. You should probably have a lot of ram as well as a real CPU.

# Installing (ubuntu)

* `https://github.com/yzziizzy/c3dlas` linked as `src/c3dlas`

* `mkdir m4`

* `sudo apt-get install libx11-dev libglew-dev libfreetype6-dev <whatever dev-lib is needed for fontconfig>`

* `./autogen.sh`
* `make && ./src/eacsmb`

# Controls (for now)

a/s to rotate.
Mouse scroll or z/x to zoom.
Right click or the arrow keys to move.


Licensed under Affero GPL v3.0 until I get a stronger copyleft license for games.

