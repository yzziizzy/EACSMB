# EACSMB
A 3D city building game. Currently under construction.


It's written in pure C using no frameworks. Dependencies are Xlib, FreeType, libPNG, GLEW, and OpenGL 4.5. You should probably have a lot of ram as well as a real CPU.

# Installing (ubuntu)

* `https://github.com/yzziizzy/c3dlas` linked as `src/c3dlas`
* `https://github.com/yzziizzy/opengl_text` linked as `src/text`

* `mkdir m4`

* `sudo apt-get install libx11-dev libglew-dev libfreetype6-dev ttf-mscorefonts-installer`

* `sudo mkdir /usr/share/fonts/corefonts`
* `sudo ln -s /usr/share/fonts/truetype/msttcorefonts /usr/share/fonts/corefonts`

* `./autogen.sh`
* `make && ./src/eacsmb`

# Controls (for now)

a/s to rotate.
Mouse scroll or z/x to zoom.
Right click or the arrow keys to move.


Licensed under Affero GPL v3.0 until I get a stronger copyleft license for games.

