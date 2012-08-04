//-------------------------------------------------------------------
// IWorldPacker.h
//
// Provides the definition for an interface that all world packers
// must derive from so that preprocessor can communicate with them.
// 
// Author: John O'Rorke
// Created: 3/14/01
// Modification History:
//
//--------------------------------------------------------------------

#ifndef __IWORLDPACKER_H__
#define __IWORLDPACKER_H__

#ifndef __BDEFS_H__
	#include "bdefs.h"
#endif

//forward declarations
class CPreMainWorld;

//--------------------
class IWorldPacker
{
public:

	// enumeration of valid platform types
	enum PackerPlatformType
	{
		ePackerPlatform_PC				= 0,
		ePackerPlatform_PS2				= 1,
		ePackerPlatform_XBOX			= 2,
		ePackerPlatform_MAX_PLATFORMS
	};


	IWorldPacker()					{}
	virtual ~IWorldPacker()			{}


	//handles the saving of the file to disk in the appropriate format.
	//the parameters are 
	//
	//pszFilename - the path, and filename, but not the extension of the 
	//				world that is being processed. It is the packer's
	//				job to append the extension.
	//
	//pMainWorld -	a pointer to the CPreMainWorld structure that holds
	//				all the data that needs to be saved to the file.
	//
	//pfnDisplay -	a pointer to a function that allows for messages to
	//				be printed out
	//
	//bObjectsOnly - determines if only objects need to be updated
	//
	//Return Val:	The return value should be true if successful, otherwise
	//				it should be false

	virtual bool		PackWorld(	const char* pszFilename, 
									CPreMainWorld* pMainWorld, 
									bool bObjectsOnly)	= 0;

	//gets the name of the platform that this packer is intended for
	virtual const char*	GetPlatform() const								= 0;

	//gets the id of the platform that this packer is intended for
	virtual const uint32 GetPlatformId() const							= 0;

private:

};

#endif
