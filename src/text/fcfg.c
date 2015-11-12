#include <stdio.h>
#include <stdlib.h>
#include <fontconfig/fontconfig.h>

#include "fcfg.h"


static FcConfig* config = NULL;


void initFontConfig() {
	config = FcInitLoadConfigAndFonts();
}


char* getFontFile(char* fontName) {
	FcPattern* pattern, *font;
	FcResult result;
	char* fileName = NULL;
	
	if(!config) initFontConfig();
	
	pattern = FcNameParse((const FcChar8*)fontName);
	FcConfigSubstitute(config, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);
	
	font = FcFontMatch(config, pattern, &result);
	
	if(!(font && FcPatternGetString(font, FC_FILE, 0, (FcChar8**)&fileName) == FcResultMatch)) {
		fprintf(stderr, "Could not find a font file for '%s'\n", fontName);
	}
	
	FcPatternDestroy(pattern);
	
	return fileName;
}
