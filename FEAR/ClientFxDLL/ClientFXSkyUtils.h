//-----------------------------------------------------------------------
// ClientFXSkyUtils.h
// 
// Contains a collection of data types and utility functions for handling
// managing sky information on client fx objects.
//
//-----------------------------------------------------------------------

#ifndef __CLIENTFXSKYUTILS_H__
#define __CLIENTFXSKYUTILS_H__

// FX sky settings
enum EFXSkySetting
{
	eFXSkySetting_None,
	eFXSkySetting_Sky,
	eFXSkySetting_SkyOverlay
};

//Sky utility functions

//given a sky setting, this will determine if it is in the sky or not
inline bool IsInSky(EFXSkySetting eSetting)			
{ 
	return eSetting != eFXSkySetting_None; 
}

//given a sky setting, this will return the FLAG2_ flag that should be used
inline uint32 GetSkyFlags(EFXSkySetting eSetting)
{
	if(eSetting == eFXSkySetting_Sky)
		return FLAG2_SKYOBJECT;
	if(eSetting == eFXSkySetting_SkyOverlay)
		return FLAG2_SKYOVERLAYOBJECT;
	return 0;
}

//strings for the sky properties
#define SKY_PROP_DEFAULT		"No"
#define SKY_PROP_ENUM			"No,Yes,Overlay"
#define SKY_PROP_DESCRIPTION	"Determines whether or not this effect should be treated as if it were in the sky. Yes indicates the object should be in the sky. Overlay means it should be in the sky but rendered over the scene."


#endif
