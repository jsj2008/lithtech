
#ifndef __PARSE_WORLD_INFO_H__
#define __PARSE_WORLD_INFO_H__


	#define DEFAULT_LIGHT_TABLE_RES	250.0f

	// Get the ambient light.  Initializes pAmbient to (0,0,0) no matter what is returned.
	LTBOOL ParseAmbientLight(char *pStr, LTVector *pAmbient);

	// If LightTableRes can't be found, it returns DEFAULT_LIGHT_TABLE_RES.	
	float ParseLightTableRes(char *pStr);


#endif  // __PARSEAMBIENTLIGHT_H__




