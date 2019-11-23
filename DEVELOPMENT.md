


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
	* MyNewDrawable_init() function, called from World_init() containing:
		* NOTHING to do with OpenGL. It will crash the game.
		* Anything not dealing with OpenGL
		* Data structure allocation and initialization
		* MDI allocation
	* MyNewDrawable_initGL() function, called from World_initGL() containing:
		* Anything dealing with OpenGL
		* Shader loading
		* Uniform location caching
		* MDI_initGL()
		* Any non-GL operations that must happen after GL initialization. 
	* _init() is called first **from a seperate thread**, then _initGL() is called from the main thread with an active OpenGL context.
3. Methods
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
* VECMP, a typesafe vector with stable memory addresses using Linux's lazy allocation of physical memory. `VECMP([type])`, `mempool.h`
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
PNG and JPEG files can be loaded. All textures will be converted to RGBA8. *They must be a **square power of two**.*
### Procedural Generation
Working but few operations. Prefixing the config texture path with "$" indicates a procedural 
config. See `texgen.[ch]`


# TODO

## Bugs
* Terrain culling in the tessellation shader sometimes has false positives (blank areas on the screen).
* segfault when custom decal start/end points are the same.
* Texture splatting on terrain is slow. Seems to be the indirect texture fetches.
* mCopy and the memcpy inside it have src/dst backwards but all usage needs to be fixed too...
* Distance culling on CPU side needs to handle shadow passes properly
* Bias calculation on shadows is terrible. Trees do not shadow themselves.
* On an NVidia GT 730, first run after modifying terrain.glsl or wiping shader cache results in random terrain corruption.
* Exclude non-printing characters from sdf generation in FontManager
* CustomDecals are not vertically positioned properly. Show boxes to see it.

### Needs attention in the future
* MDI max meshes limit fixed at 16. Might be able to choose the right value in `initGL` if all meshes are loaded first.

## Graphics
* Cache low-resolution pre-baked terrain textures for blocks in the distance.
* Animated meshes.
* Normal Mapping on meshes and decals.
* Improve shadowing.
* Screen Space Reflections
* SSAO (Screen Space Ambient Occlusion)
* Emitters need animated 3D textures.
* Texture scaling algorithm is broken (TextureManager)
* Lighting:
	* Point lights using geom-shader billboards for small/distance rendering.
	* HDR and bloom
* Ray-marched volumetric clouds and fog.
* Custom mouse pointer support. 
* Falling rain and snow.
	* Make a special version of Emitter that only spawns in rage of the camera, but consistently.
* Mipmapping in decals. 
* 3D billboards/text/gui
* Dynamic LOD
* Easy, cheap ephemeral meshes, decal and effects that can automatically spawn on terrain at certain zoom levels. Grass, dust, flowers, decorative decals, etc. They should not be in the CES as they are not real game items. 
* Texture compression
* Calculate proper derivatives in decal fragment shader for anisotropic filtering
* Texture atlassing for meshes, including coordinate transformation in/near shaders.
* Polygonal/triangular decals
* NormalMap values are not transformed properly in mesh vertex shaders

## Sound
* Finish API.
* Integrate with main itemLoader config.
* Integer WAV support.
* Adjustable channel count
* Finish/fix the SoundClip rate resampler.
* Implement the mixer, with auto-ranging volume on overflow.
* Make sound/ALSA support optional in makefile.
* Make OGG/Vorbis support optional.
* libav/ffmpeg support. (optional)
* PulseAudio support. (optional)
* JACK support. (optional)
* Put sound in its own thread.

## Core
* Better loading screen
* Water and fluid dynamics are a broken mess right now.
* B+ tree leaves become heavily lopsided when filled with an increasing integer sequence. (half full leaves) 
* B+ tree does not have a delete operation.
* CES system lacks a delete operation.
* Figure out the proper API for adding and spawning CustomDecals in the CES system.
* CLI options parsing for custom config files.
* Prevent zooming through the terrain.
* Tree and bush generation algorithm.
* JSON saving.
* Proper logging utility.
* More features in building generator.
* Track GPU memory usage as much as possible.
* Destructors and resource cleanup on just about everything.
* Fix quad tree to bin items by size
* Function to programmatically resize window.

## UI
* Config system
* Finish wiring clipping boxes through everything
* Arbitrary lines (via rotation?)
* Components:
	* Text boxes with wrapping
	* Sliders
	* Menu boxes
	* Scroll bars and scrollable windows (depends on clipping)
	* Input Button:
		* Radio-like selection behavior
	* Tab Control
	* Edit Control:
		* Clip text properly
		* Horizontal scroll
		* Borders, active/inactive colors
* SDF calculation on the GPU. (current multithreaded version is not *too* bad.)
* 3D gui (positioned in the world rather than overlayed in 2D.)
* Consistent z offsets for child windows to prevent flickering from inconsistent sorting
* Animations
* Debug float* adjusters
* Make naming conventions consistent
* Macro Magic for creating new controls


## Petty Basic Optimization
* TextureAtlas_addFolder()
* Many of the texgen algorithms
* `const` and `restrict` in function arguments where needed

## Gameplay
* Road networks
* Pipe Lines (sequences of meshes in a line, like sewer pipes - needs better name)
* Code to find an area to place something beside roads


# Reference
## Texture unit allocation
Texture units are (mostly) used for only one texture, reducing rebinding costs. EACSMB assumes there will only be 32 units, the minimum required by modern GL versions.

The first five are the main GBuffer. It is not the most optimized GBuffer and could use 
some work later on when the engine is more mature.


0. diffuse buffer
1. normal buffer
2. depth buffer
3. material buffer
4. lighting buffer
5. shadow map depth buffer (temp?)
6.
7. mesh texture
8. 
9. decal textures
10. diffuse alt buffer
11. normal alt buffer
12. lighting alt buffer
13. depth alt buffer
14.
15. mesh RGBA textures (diffuse)
16. mesh RGB textures (normal)
17. mesh R textures (materials)
18.
19. terrain integer data (tex indices)
20. terrain block position lookup
21. terrain heightmap
22. terrain diffuse tex array
23. terrain Material tex array
24. 
25. 
26.
27.
28. gui atlas texture
29. sdf array texture 
30. guiImage (render target)
31. gui icon array (obsolete?)
