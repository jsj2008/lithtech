#include "keydefaultaggregate.h"
#include "hotkey.h"

CKeyDefaultAggregate::CKeyDefaultAggregate() :
	m_pAggregate(NULL)
{
}

CKeyDefaultAggregate::~CKeyDefaultAggregate()
{
	delete m_pAggregate;
}

//gets the aggregate for this aggregate
CKeyDefaultAggregate* CKeyDefaultAggregate::GetAggregate()
{
	return m_pAggregate;
}

//sets the aggregate, orphans the object, assumes new allocation
void CKeyDefaultAggregate::SetAggregate(CKeyDefaultAggregate* pAggregate)
{
	//clean up the old aggregate
	delete m_pAggregate;

	//setup the new one
	m_pAggregate = pAggregate;
}

//allows this aggregate to have a pass at defining a key, if the key
//is defined, it should return TRUE, otherwise false
bool CKeyDefaultAggregate::DefineKey(CHotKey& HotKey)
{
	//first see if our aggregate wants to handle this
	if(m_pAggregate)
	{
		if(m_pAggregate->DefineKey(HotKey))
		{
			//the aggregate handled it
			return true;
		}
	}

	//we need to handle this, allow the template implementor to handle it
	return InternalDefineKey(HotKey);
}

//this is the function that derived classes override, so that they don't have
//to handle aggregates
bool CKeyDefaultAggregate::InternalDefineKey(CHotKey& HotKey)
{
	//default implementation does not define any keys
	return false;
}

