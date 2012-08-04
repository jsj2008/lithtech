#ifndef __DEDITKEYAGGREGATE_H__
#define __DEDITKEYAGGREGATE_H__

#ifndef __KEYDEFAULTAGGREGATE_H__
#	include "KeyDefaultAggregate.h"
#endif

class CDEditKeyAggregate : 
	public CKeyDefaultAggregate
{
private:

	bool InternalDefineKey(CHotKey& HotKey)
	{
		return false;
	}
};

#endif
