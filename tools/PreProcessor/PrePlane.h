//------------------------------------------------------------------
//
//	FILE	  : PrePlane.h
//
//	PURPOSE	  : Defines the CPrePlane class, which does most of
//              the polygon operations.
//
//	CREATED	  : July 28 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __PREPLANE_H__
	#define __PREPLANE_H__


	// Includes....
	#include "bdefs.h"

	// High bit of plane index is reserved.
	#define PLANEINDEX_HIGH_BIT		((uint32)(1<<31))
	#define PLANEINDEX_INDEXMASK	((uint32)(~PLANEINDEX_HIGH_BIT))


	class CPrePlane : public LTPlane, public CGLLNode
	{
	public:

		LTPlane& operator=(const LTPlane& thePlane) 
		{
			m_Normal	= thePlane.m_Normal;
			m_Dist		= thePlane.m_Dist;

			return *this;
		}

		// Access the index.
		uint32	GetIndex() const		{return m_Index & PLANEINDEX_INDEXMASK;}
		void	SetIndex(uint32 index)	{m_Index &= ~PLANEINDEX_INDEXMASK; m_Index |= (index & PLANEINDEX_INDEXMASK);}

		// Access the high bit.
		uint32	GetHighBit() const		{return m_Index & PLANEINDEX_HIGH_BIT;}
		void	SetHighBit()			{m_Index |= PLANEINDEX_HIGH_BIT;}
		void	ClearHighBit()			{m_Index &= ~PLANEINDEX_HIGH_BIT;}


	private:

		// This plane's index.. used while saving the DAT file.
		uint32	m_Index;
	};


#endif



