//---------------------------------------------------------------------------------------------
// GameRenderLayers.h
//
// This class is a very simple listing of the rendering layers used by the game code. These
// rendering layers are used to perform Z biasing when rendering in order to handle rendering
// issues such as players clipping into walls or decals not showing up on geometry.
//
//---------------------------------------------------------------------------------------------
#ifndef __GAMERENDERLAYERS_H__
#define __GAMERENDERLAYERS_H__

//an enumeration that allows the game code to easily access the different render layers
enum ERenderLayer
{
	//--------------------
	// Layer identifiers

	//this indicates no rendering layer, which is the default for objects
	eRenderLayer_None		= 0,

	//this rendering layer is used for biasing the player
	eRenderLayer_Player,

	//these rendering layer are used for decals
	eRenderLayer_Decal0,
	eRenderLayer_Decal1,
	eRenderLayer_Decal2,

	//--------------------
	// Layer information

	//the number of render layers, this must come after all the other decal layers
	eRenderLayer_NumLayers,

	//the number of decal layers we provide
	eRenderLayer_NumDecalLayers = 3,
};

#endif
