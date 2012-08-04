#ifndef __WON_CRC_H__
#define __WON_CRC_H__
#include "WONShared.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CRC16
{
private:
	unsigned short mRegister;
	unsigned short mInitValue;
	unsigned short mXOROutValue;

public:
	explicit CRC16(unsigned short theInitValue=0, unsigned short theXOROutValue=0);
	void Put(const void* theDataP, unsigned int theLen);
	
	unsigned short Get() const { return (mRegister ^ mXOROutValue); }
	void Reset() { mRegister = mInitValue; }
};

}; // namespace WONAPI

#endif
