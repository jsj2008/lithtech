
#include "bdefs.h"
#include "light_table.h"



// ----------------------------------------------------------------------------- //
// LightTable.
// ----------------------------------------------------------------------------- //

LightTable::LightTable()
{
	Clear();
}


LightTable::~LightTable()
{
	Term();
}


void LightTable::InitLightTableSize(
	LTVector *pMin, 
	LTVector *pMax, 
	float lightTableRes)
{
	m_BlockSize.Init(lightTableRes, lightTableRes, lightTableRes);
	m_InvBlockSize.x = 1.0f / m_BlockSize.x;
	m_InvBlockSize.y = 1.0f / m_BlockSize.y;
	m_InvBlockSize.z = 1.0f / m_BlockSize.z;

	m_LookupSize[0] = (uint32)((pMax->x - pMin->x) * m_InvBlockSize.x) + 1;
	m_LookupSize[1] = (uint32)((pMax->y - pMin->y) * m_InvBlockSize.y) + 1;
	m_LookupSize[2] = (uint32)((pMax->z - pMin->z) * m_InvBlockSize.z) + 1;
	if(m_LookupSize[0] == 0) m_LookupSize[0] = 1;
	if(m_LookupSize[1] == 0) m_LookupSize[1] = 1;
	if(m_LookupSize[2] == 0) m_LookupSize[2] = 1;
	
	m_LookupSizeMinus1[0] = m_LookupSize[0] - 1;
	m_LookupSizeMinus1[1] = m_LookupSize[1] - 1;
	m_LookupSizeMinus1[2] = m_LookupSize[2] - 1;

	m_XSizeTimesYSize = m_LookupSize[0] * m_LookupSize[1];
	m_FullLookupSize = m_LookupSize[0]*m_LookupSize[1]*m_LookupSize[2];

	m_LookupStart = *pMin;
}


void LightTable::Clear()
{
	m_Lookup = LTNULL;
	m_FullLookupSize = 0;
	m_LookupSize[0] = m_LookupSize[1] = m_LookupSize[2] = 0;
	m_LookupSizeMinus1[0] = m_LookupSizeMinus1[1] = m_LookupSizeMinus1[2] = 0;
	m_BlockSize.Init();
	m_InvBlockSize.Init();
	m_LookupStart.Init();
}


void LightTable::Term()
{
	dfree(m_Lookup);
	Clear();
}


