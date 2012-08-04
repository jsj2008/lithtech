//-------------------------------------------------------------------
// LTAParseUtils.h
//
// Provides numerous functions used for loading an LTA file into
// datastructures.
//
// Created: 1/25/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTAPARSEUTILS_H__
#define __LTAPARSEUTILS_H__

#ifndef __LTANODE_H__
#	include "ltanode.h"
#endif

//determines if the node is a list and if it has 2 elements
inline bool			IsPair( const CLTANode* pNode );

//gets the first element of a pair's value
inline const char*	PairCar(const CLTANode* pNode );

//gets the second element of a pair's value
inline const char*	PairCdr(const CLTANode* pNode );

//gets the second element of a pair as a node
inline CLTANode*	PairCdrNode( CLTANode* pNode );

//gets the value of a node as a uint8
inline uint8		GetUint8(const CLTANode* pNode);

//gets the value as a boolean
inline bool			GetBool(const CLTANode* pNode);

//gets the value of a node as a uint32
inline uint32		GetUint32(const CLTANode* pNode);

//gets the value of a node as a int32
inline int32		GetInt32(const CLTANode* pNode);

//gets the value of a node as a float
inline float		GetFloat(const CLTANode* pNode);

//gets the value of a node
inline const char*	GetString(const CLTANode* pNode);




//---------------------------------------
// Inlines
//---------------------------------------

#ifndef __LTACONVERTER_H__
#	include "ltaconverter.h"
#endif

//determines if the node is a list and if it has 2 elements
bool IsPair(const CLTANode* pNode )
{
	bool retval = false;
	if( pNode )
	{
		ASSERT(pNode->IsList());

		//we have a list, make sure it has 2 elements
		if( pNode->GetNumElements() == 2 )
		{
			retval = true;
		}
	}
	return retval;
}

//gets the first element of a pair's value
const char * PairCar(const CLTANode* pNode )
{
	if( IsPair(pNode) )	
	{
		return pNode->GetElement(0)->GetValue();
	}
	else
	{
		return "";
	}	
}

//gets the second element of a pair's value
const char * PairCdr(const CLTANode* pNode )
{
	if( IsPair(pNode) )	
	{
		return pNode->GetElement(1)->GetValue();
	}
	else
	{
		return "";
	}
	
}

//gets the second element of a pair as a node
CLTANode* PairCdrNode(CLTANode* pNode )
{
	if( pNode )
	{
		if( IsPair(pNode) )	
		{
			return pNode->GetElement(1);			
		}
	}
	return 0;
}

//gets the value of a node as a uint8
uint8 GetUint8(const CLTANode* pNode)
{
	ASSERT(pNode);
	ASSERT(pNode->IsAtom());

	return uint8(CLTAConverter::StrToInt(pNode->GetValue()));
}

//gets the value as a boolean
bool GetBool(const CLTANode* pNode)
{
	ASSERT(pNode);
	ASSERT(pNode->IsAtom());

	return (CLTAConverter::StrToInt(pNode->GetValue())) ? true : false;
}

//gets the value of a node as a uint32
uint32 GetUint32(const CLTANode* pNode)
{
	ASSERT(pNode);
	ASSERT(pNode->IsAtom());

	return uint32(CLTAConverter::StrToInt(pNode->GetValue()));
}

//gets the value of a node as an int32
int32 GetInt32(const CLTANode* pNode)
{
	ASSERT(pNode);
	ASSERT(pNode->IsAtom());

	return CLTAConverter::StrToInt(pNode->GetValue());
}

//gets the value of a node as a float
float GetFloat(const CLTANode* pNode)
{
	ASSERT(pNode);
	ASSERT(pNode->IsAtom());

	return (float)(CLTAConverter::StrToReal(pNode->GetValue()));
}

//gets the value of a node
const char* GetString(const CLTANode* pNode)
{
	ASSERT(pNode);
	ASSERT(pNode->IsAtom());

	return pNode->GetValue();
}

#endif

