#ifndef __PACKERFACTORY_H__
#define __PACKERFACTORY_H__

#ifndef __IWORLDPACKER_H__
#	include "IWorldPacker.h"
#endif

class CPackerFactory
{
public:

	static IWorldPacker* Create(const char* pszPlatform);

private:

	//don't allow instantiation
	CPackerFactory()		{}

};

#endif

