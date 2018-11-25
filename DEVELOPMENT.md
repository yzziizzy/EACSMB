


# Overview

Each kind of drawable has been developed incrementally over time. Some are newer in technology
and technique than others. Don't use an old one as a template to make a new one without good reason.

## Newer
* DynamicMeshManager
* Emitter
* MultiDrawIndirect
* RenderPipeline / RenderPass / PassDrawable
* Marker

## Older
* Decals
* Map
* Roads

## Obsolete, Do Not Use
* MeshManager / `staticMesh.[ch]`


# General Approach

EACSMB is exclusively targeted at modern systems. Use the most performant techniques only. Do not use
old OpenGL API's for compatibility reasons. The target GL version is 4.5 right now. In the
future it may be moved to 4.6+ or Vulkan after drivers stabilize and support proliferates. GCC is the
only target compiler. Linux is the only target kernel for now. 

Minimum external dependency is a primary design goal. Do not pull in more 3rd-party libraries, 
especially bloated ones with dependencies themselves. 

## Quick guide to making a new renderable
Use DynamicMeshManager as the example.

1. Basic Blocks
  * struct with:
    * An MDI instance
    * A VEC or list or something to handle each geometry type
  * A struct for holding geometry data and instance lists containing:
    * A field for how many instances to draw this frame (numToDraw)
    * A list of instance data
  * A struct for holding instance information, and potentially
    * A struct packed and aligned to std140 rules for copying instance data into video memory 
2. Static Variables in the .c file:
  * A ShaderProgram* 
  * GLuint's for uniform locations
2. Initialization
  * initMyNewDrawable() function, called from World_init() containing:
    * Shader loading
    * Uniform location caching
3. Methods
  * MyNewDrawable_init/alloc() function with:
    * MDI initialization
  * PassDrawable* MyNewDrawable_CreateDrawable(MyNewDrawable* o)
    * usually just calls MDI_CreateDrawable
  * RenderPass* MyNewDrawable_CreateRenderPass(MyNewDrawable* o)
    * creates a RenderPass and calls MyNewDrawable_CreateDrawable to add the drawable
4. Important Static Methods 
  * instanceSetup: called each frame to load data for each instance into video memory. This
    is where you calculate matrices or perform culling. The specific implementation in DynamicMeshManager
    is more complex than most use cases will need to be.
  * uniformSetup: called immediately before drawing to allow custom uniforms to be set. Thi is
    where you set texture id's. Lots of bad, lazy code can be seen in these functions throughout 
    the game.
5. Drawing: Determine the appropriate pass or location for a new pass within World_draw*() or 
  in `drawFrame()` in `renderLoop.c`. Call the correct RenderPass_* functions, or add the drawable to 
  an existing pass. If adding a new pass, put it in the World struct and initialiaze it appropriately.
6. Extras:
  * TextureManager. Try to share an existing instance with the same or similar format. Don't make a 
    new one unless an existing one will not work.
  * Potentially a new ItemType in `world.h` and complimentary loading code in `itemLoader.c` and `world.c`. 


# Common Data Structures

* Vectors (dynamic array): `VEC([type])`, `ds.h`
	Built with type-safe macros.
* Linked lists: various helper macros at the bottom of `ds.h`. 
	See `building.c` for usage example. Very new, may have bugs.
* Hash Tables: `HashTable([type])`, `hash.h`
	Linear probing hash table. Default growth threshold at 75%, configurable. Stores a void*.
* Ordered Hash Tables: `OHashTable([type])`, `ordered_hash.h`
	Ordered version of HashTable. Mostly compete but missing a few vital operations. Needs Love.
* B+ Tree: BPlusTree, `btree.h`
	Currently only used for the CES backend. Very immature. Many missing operations.


# 1st-Party Libraries

* `c3dlas`: a linear algebra lib. No known algorithmic bugs. If you get "Matrix has no inverse" errors
	then you fed in a bad matrix; the inversion function is correct.
* `c_json`: a json loader. No write support currently. Probably leaks memory some places. Uses a 
	bundled version of HashTable.
		* `json_gl.h` with useful helper functions.
* `sexp`: A simple S-expression parser. Currently used by the procedural texture generation. 


# Useful References
* Macro loop magic in `ds.h` for VEC_LOOP et al.
* Macro magic in `texgen.h`
* `utilities.h`


# Art
Art is loaded in `assets/config/combined_config.json`
## Models
OBJ files **need** to have the following to be parsed and rendered correctly:
* 2D Texture Coordinates
* 3D Normals
OBJ files do not break out "objects" right now. All geometry goes into one mesh. Material and
texture file information is ignored completely. 

Triangulation is not required. The loader will automatically triangulate (as fans) any n>3 polys.

## Textures
### From File
PNG files can be loaded. *They must be full **RGBA** and a **square power of two**.*
### Procedural Generation
Working but few operations. Prefixing the config texture path with "$" indicates a procedural 
config. See `texgen.[ch]`


# TODO

## Bugs
* Terrain culling in the tessellation shader sometimes has false positives (blank areas on the screen).
* Specular lighting algorithm is broken. (shading.glsl)
* segfault when custom decal start/end points are the same.
* Calculate correct shader error message line numbers.
* Texture splatting on terrain is bad.

## Graphics
* Cache low-resolution pre-baked terrain textures for blocks in the distance.
* Animated meshes.
* Normal Mapping on meshes and decals.
* Shadow Maps.
* Screen Space Reflections
* SSAO (Screen Space Ambient Occlusion)
* Emitters need animated 3D textures.
* Texture scaling algorithm is broken (TextureManager)
* Point lights using geom-shader billboards for small/distance rendering.
* Ray-marched volumetric clouds and fog.
* Custom mouse pointer support. 
* Configurable decal render order (road markings on top of roads on top of leaves).
* HDR lighting and bloom.
* Falling rain and snow.
* Mipmapping in decals.


## Sound
* Finish API.
* Integrate with main itemLoader config.
* Integer WAV support.
* Finish/fix the SoundClip rate resampler.
* Implement the mixer, with auto-ranging volume on overflow.
* Make sound/ALSA support optional in makefile.
* OGG/Vorbis support. (optional)
* libav/ffmpeg support. (optional)
* PulseAudio support. (optional)
* JACK support. (optional)
* Put sound in its own thread.

## Core
* Water and fluid dynamics are a broken mess right now.
* B+ tree leaves become heavily lopsided when filled with an increasing integer sequence. (half full leaves) 
* B+ tree does not have a delete operation.
* CES system lacks a delete operation.
* Figure out the proper API for adding and spawning CustomDecals in the CES system.
* CLI options parsing for custom config files.
* #include support in shaders.
* Prevent zooming through the terrain.
* Tree and bush generation algorithm.
* JSON saving.
* Proper logging utility.
* More features in building generator.
* Track GPU memory usage as much as possible.
