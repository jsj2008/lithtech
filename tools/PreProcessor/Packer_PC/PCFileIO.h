//////////////////////////////////////////////////////////////////////////////
// PC-specific IO file functions

#ifndef __PCFILEIO_H__
#define __PCFILEIO_H__

#include "abstractio.h"

inline CAbstractIO &operator<<(CAbstractIO &file, const LTVector &vVec)
{
	file << vVec.x;
	file << vVec.y;
	file << vVec.z;

	return file;
}

#endif //__PCFILEIO_H__