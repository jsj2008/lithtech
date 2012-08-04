#ifndef __PCWORLDPACKER_H__
#define __PCWORLDPACKER_H__

#ifndef __IWORLDPACKER_H__
#	include "IWorldPacker.h"
#endif

class CPCWorldPacker : public IWorldPacker
{
public:
	
	CPCWorldPacker();
	~CPCWorldPacker();

	//handles the saving of the file to disk in the appropriate format.
	virtual bool	PackWorld(	const char* pszFilename, 
								CPreMainWorld* pMainWorld, 
								bool bObjectsOnly);

	//gets the name of the platform that this packer is intended for
	virtual const char*	GetPlatform() const		{return CPCWorldPacker::GetClassPlatform();}

	//gets the id of the platform that this packer is intended for
	virtual const uint32 GetPlatformId() const	{return CPCWorldPacker::GetClassPlatformId();}

	//static version so this class can be queried for its platform name
	static const char*	GetClassPlatform()		{return "PC";}

	//static version so this class can be queried for its platform id
	static const uint32 GetClassPlatformId()	{return IWorldPacker::ePackerPlatform_PC;}

private:

};

#endif

