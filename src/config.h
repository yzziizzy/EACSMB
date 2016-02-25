#ifndef __config_h__
#define __config_h__






typedef struct UserConfig {
	
	float scrollSpeed;
	
	
	
} UserConfig;



UserConfig* loadConfigFile(char* path);


















#endif // __config_h__