//-------------------------------------------------------------------
// KeyDefaultAggregate.h
//
// This provides the prototype for the CKeyDefaultAggregate class
// which allows for overriding of key defaults for different control
// styles. This uses the template design pattern.
//
// Author: John O'Rorke
// Created: 4/6/01
// Modification History:
//
//--------------------------------------------------------------------
#ifndef __KEYDEFAULTAGGREGATE_H__
#define __KEYDEFAULTAGGREGATE_H__

class CHotKey;

class CKeyDefaultAggregate
{
public:

	CKeyDefaultAggregate();
	virtual ~CKeyDefaultAggregate();

	//gets the aggregate for this aggregate
	virtual CKeyDefaultAggregate*	GetAggregate();

	//sets the aggregate, orphans the object, assumes new allocation
	virtual void					SetAggregate(CKeyDefaultAggregate* pAggregate);

	//allows this aggregate to have a pass at defining a key, if the key
	//is defined, it should return TRUE, otherwise false
	bool							DefineKey(CHotKey& HotKey);


private:

	//this is the function that derived classes override, so that they don't have
	//to handle aggregates
	virtual bool					InternalDefineKey(CHotKey& HotKey);

	//the aggregate
	CKeyDefaultAggregate*			m_pAggregate;
	
};

#endif
