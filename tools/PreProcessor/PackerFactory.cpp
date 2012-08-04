#include "PackerFactory.h"
#include <stdlib.h>
#include <string.h>

//the different packers
#include "PCWorldPacker.h"


//this macro allows for the creation of a specific world packer. Note, that
//for this to work, the class must have a static function called GetClassPlatform
//that will return a string name of the platform, and the other string
//being compared must be pszPlatform
#define	DECLARE_PACKER(a)		if(stricmp(pszPlatform, a::GetClassPlatform()) == 0)	{return new a;}

IWorldPacker* CPackerFactory::Create(const char* pszPlatform)
{
	DECLARE_PACKER(CPCWorldPacker);


	//none of the other packers were valid, so none were found
	return NULL;
}
