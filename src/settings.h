#ifndef __EACSMB_settings_h__
#define __EACSMB_settings_h__




#define SETTING_LIST \
	SETTING(string, coreConfigPath, "assets/config/core.json") \
	SETTING(string, assetsPath, "assets") \
	\
	SETTING(string, configDir, "config") \
	SETTING(string, shadersDir, "shaders") \
	SETTING(string, texturesDir, "textures") \
	SETTING(string, modelsDir, "models") \
	SETTING(string, soundsDir, "sounds") \
	SETTING(string, uiDir, "ui") \
	\
	SETTING(string, configDirPath, NULL) \
	SETTING(string, shadersDirPath, "src/shaders") \
	SETTING(string, texturesDirPath, NULL) \
	SETTING(string, modelsDirPath, NULL) \
	SETTING(string, soundsDirPath, NULL) \
	SETTING(string, uiDirPath, NULL) \
	\
	SETTING(string, worldConfigPath, NULL) \
	\
	SETTING(int, DynamicMeshManager_maxInstances, 8192) \
	SETTING(int, RiggedMeshManager_maxInstances, 4096) \
	SETTING(int, DecalManager_maxInstances, 8192) \
	SETTING(int, BushManager_maxInstances, 8192) \
	SETTING(int, CustomDecalManager_maxInstances, 8192) \
	SETTING(int, EmitterManager_maxInstances, 8192) \
	SETTING(int, MarkerManager_maxInstances, 8192) \
	SETTING(int, GUIManager_maxInstances, 8192) \
	\
	SETTING(int, SunShadow_size, 1024) \
	\
	SETTING(float, keyRotateSensitivity, 0.0f) \
	SETTING(float, keyScrollSensitivity, 0.0f) \
	SETTING(float, keyZoomSensitivity  , 0.0f) \
	\
	SETTING(float, mouseRotateSensitivity, 0.0f) \
	SETTING(float, mouseScrollSensitivity, 0.0f) \
	SETTING(float, mouseZoomSensitivity  , 0.0f) \
	\
	SETTING(int, regenTerrain, 0) \
	




typedef struct GlobalSettings {
#define string char*
#define SETTING(type, name, val) type name;
	SETTING_LIST
#undef SETTING
#undef string
} GlobalSettings;


void GlobalSettings_loadDefaults(GlobalSettings* s);
void GlobalSettings_loadFromFile(GlobalSettings* s, char* path);
void GlobalSettings_finalize(GlobalSettings* s);


#endif // __EACSMB_settings_h__
