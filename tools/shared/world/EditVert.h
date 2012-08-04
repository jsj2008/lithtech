//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditVert.h
//
//	PURPOSE	  : Defines the CEditVert class.
//
//	CREATED	  : October 5 1996
//
//
//------------------------------------------------------------------

#ifndef __EDITVERT_H__
#define __EDITVERT_H__


	// Includes....


	class CEditVert : public LTVector
	{
	public:

			//different flags that can be used by the vertex
			enum	{	VERTFLAG_INFRUSTUM		= (1 << 0)	};

						CEditVert() :
							m_nFlags(0),
							m_nR(255),
							m_nG(255),
							m_nB(255),
							m_nA(255)
						{
						}

						CEditVert( const CVector &vec ) :
							LTVector(vec),
							m_nFlags(0),
							m_nR(255),
							m_nG(255),
							m_nB(255),
							m_nA(255)
						{
						}



			void		operator = ( const CEditVert& other )	
			{
				m_nFlags = other.m_nFlags;
				m_nR = other.m_nR;
				m_nG = other.m_nG;
				m_nB = other.m_nB;
				m_nA = other.m_nA;
				x=other.x; y=other.y; z=other.z; 
			}

			void		operator = ( const LTVector& other )	{ x=other.x; y=other.y; z=other.z; }


			bool		IsInFrustum() const						{ return IsFlagSet(VERTFLAG_INFRUSTUM); }
			void		SetInFrustum(bool bVal)					{ SetFlag(VERTFLAG_INFRUSTUM, bVal); }

			bool		IsFlagSet(uint8 nFlag) const			{ return (m_nFlags & nFlag) ? true : false; }
			void		EnableFlag(uint8 nFlag)					{ m_nFlags |= nFlag; }
			void		DisableFlag(uint8 nFlag)				{ m_nFlags &= ~nFlag; }

			void		SetFlag(uint8 nFlag, bool bVal)
			{
				if(bVal)
					EnableFlag(nFlag);
				else
					DisableFlag(nFlag);
			}

	public:

			// Transformed and projected positions..
			LTVector	m_Transformed;
			LTVector	m_Projected;

			//determines if the last time this vertex was rendered if it was
			//inside the view frustum
			uint8		m_nFlags;

			//a bit list of which planes culled this vertex out if any
			uint8		m_nClipPlanes;

			//the actual color of this vertex
			uint8		m_nR;
			uint8		m_nG;
			uint8		m_nB;
			uint8		m_nA;
	};


	typedef CMoArray<CEditVert>		CEditVertArray;


#endif  // __EDITVERT_H__


