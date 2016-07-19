# EACSMB
A 3D city building game. Currently under construction.


It's written in pure C using no frameworks. Dependencies are Xlib, FreeType, fontconfig, libPNG, GLEW, and OpenGL 4.5. You should probably have a lot of ram as well as a real CPU.

Current state:
![Current state](//i.imgur.com/KCIkUAs.png)


# Installing (ubuntu)

* `https://github.com/yzziizzy/c3dlas` linked as `src/c3dlas`

* `sudo apt-get install libx11-dev libglew-dev libfreetype6-dev libfontconfig1-dev libpng-dev`

* `./autogen.sh`
* `make && ./src/eacsmb`


# Settings

`defaults.ini` contains base settings, to override them add `settings.ini` with
the desired values.

## Input sensitivity:

* keyRotateSensitivity   : range 0.0 - 1.0
* keyScrollSensitivity   : range 0.0 - 1.0
* keyZoomSensitivity     : range 0.0 - 1.0
* mouseRotateSensitivity : range 0.0 - 1.0 (not currently used)
* mouseScrollSensitivity : range 0.0 - 1.0
* mouseZoomSensitivity   : range 0.0 - 1.0


# Controls (for now)

* a/s to rotate.
* Mouse scroll or z/x to zoom.
* Right click or the arrow keys to move.


Licensed under Affero GPL v3.0 until I get a stronger copyleft license for games.

