#ifndef __EACSMB_CONFIG_H__
#define __EACSMB_CONFIG_H__






typedef struct UserConfig {
	
	float keyRotateSensitivity;
	float keyScrollSensitivity;
	float keyZoomSensitivity;
	
	float mouseRotateSensitivity;
	float mouseScrollSensitivity;
	float mouseZoomSensitivity;
	
} UserConfig;



UserConfig* loadConfigFile(char* path);

int updateConfigFromFile(UserConfig* config, char* path);

void zeroConfig(UserConfig* config);













#endif // __EACSMB_CONFIG_H__