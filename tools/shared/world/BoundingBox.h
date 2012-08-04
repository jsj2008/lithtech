//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : BoundingBox.h
//
//	PURPOSE	  : Defines the CBoundingBox class.  These are used
//              on Brushes to speed up intersection detection.
//
//	CREATED	  : November 12 1996
//
//
//------------------------------------------------------------------

#ifndef __BOUNDINGBOX_H__
	#define __BOUNDINGBOX_H__

	
	// Includes....


	class CBoundingBox
	{
		public:

							CBoundingBox();
			
			void			Init( LTVector &start );

		
		public:

			void			Extend( LTVector &extendBy );
			LTVector		Dims() const;
			BOOL			Intersects( const CBoundingBox &other ) const;
			LTVector		Center() const;

		
		public:
	
			LTVector		m_Min, m_Max;

	};


	inline CBoundingBox::CBoundingBox()
	{
		m_Min.Init();
		m_Max.Init();
	}

	inline LTVector CBoundingBox::Center() const
	{
		return (m_Min + m_Max) * 0.5f;
	}


	inline void CBoundingBox::Init( CVector &start )
	{
		m_Min = m_Max = start;
	}


	inline void CBoundingBox::Extend( LTVector &extendBy )
	{
		if( extendBy.x < m_Min.x )	m_Min.x = extendBy.x;
		if( extendBy.y < m_Min.y )	m_Min.y = extendBy.y;
		if( extendBy.z < m_Min.z )	m_Min.z = extendBy.z;

		if( extendBy.x > m_Max.x )	m_Max.x = extendBy.x;
		if( extendBy.y > m_Max.y )	m_Max.y = extendBy.y;
		if( extendBy.z > m_Max.z )	m_Max.z = extendBy.z;
	}


	inline LTVector CBoundingBox::Dims() const
	{
		return m_Max - m_Min;
	}


	inline BOOL CBoundingBox::Intersects(const CBoundingBox &other ) const
	{
		if( (m_Min.x > other.m_Max.x) ||
			(m_Min.y > other.m_Max.y) ||
			(m_Min.z > other.m_Max.z) ||
			
			(m_Max.x < other.m_Min.x) ||
			(m_Max.y < other.m_Min.y) ||
			(m_Max.z < other.m_Min.z) )
			return FALSE;
			
		return TRUE;
	}


#endif  // __BOUNDINGBOX_H__



