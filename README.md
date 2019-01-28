# EACSMB
A 3D city building game. Currently under construction.


It's written in pure C using no frameworks. Hard dependencies are Xlib, FreeType, fontconfig, libPNG, GLEW, and OpenGL 4.5. You should probably have a lot of ram as well as a real CPU.
Optional dependencies are libJPEG, ALSA and libVorbis (OGG).

(Not so) Current state:
![Current state](http://i.imgur.com/KCIkUAs.png)


# Installing (ubuntu)

* `https://github.com/yzziizzy/c3dlas` symlinked as `src/c3dlas`
* `https://github.com/yzziizzy/c_json` symlinked as `src/c_json`

* `sudo apt-get install libx11-dev libglew-dev libfreetype6-dev libfontconfig1-dev libpng-dev`

* Optional Dependencies: `sudo apt-get install libjpeg-turbo8-dev libvorbis-dev alsa-source`

* `./autogen.sh`
* `make && ./src/eacsmb`


# Settings

Poke around in `assets/config/`

## Input sensitivity:

* keyRotateSensitivity   : range 0.0 - 1.0
* keyScrollSensitivity   : range 0.0 - 1.0
* keyZoomSensitivity     : range 0.0 - 1.0
* mouseRotateSensitivity : range 0.0 - 1.0 (not currently used)
* mouseScrollSensitivity : range 0.0 - 1.0
* mouseZoomSensitivity   : range 0.0 - 1.0


# Controls (for now)

* a/s to rotate.
* z/x to zoom.
* Arrow keys to move.
* q/w to change the sun angle
* click to spawn a model

Licensed under Affero GPL v3.0 until I get a stronger copyleft license for games.

