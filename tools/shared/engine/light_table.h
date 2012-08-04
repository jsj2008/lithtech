
#ifndef __LIGHT_TABLE_H__
#define __LIGHT_TABLE_H__

    #ifndef __DE_WORLD_H__
	#include "de_world.h"
    #endif


	// Used to determine how to shade things throughout the level.
	class LightTable
	{
	public:

					LightTable();
					~LightTable();

		// Sets up all the size variables (m_FullLookupSize, m_LookupSize, etc).
		// This does NOT allocate anything.
		void		InitLightTableSize(
			LTVector *pMin, 
			LTVector *pMax, 
			float lightTableRes);

		void		Clear();
		void		Term();

	
	public:

		LTRGBColor	*m_Lookup;
		uint32		m_FullLookupSize;
		uint32		m_LookupSize[3];		// Indexed by [z*(xSize*ySize)+y*xSize+x]
		int32		m_LookupSizeMinus1[3];	// m_LookupSize[x]-1
		uint32		m_XSizeTimesYSize;	// Precalculated..
		
		LTVector	m_BlockSize;		// Size of a block in the table.
		LTVector	m_InvBlockSize;		// 1/m_BlockSize.
		LTVector	m_LookupStart;		// Base position of the table.
	};


#endif




