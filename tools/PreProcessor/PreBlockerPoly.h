//------------------------------------------------------------------
//
//	FILE	  : PreBlockerPoly.h
//
//	PURPOSE	  : Defines the CPreBlockerPoly class, which is used 
//              to hold information about polygons that block physics
//
//	CREATED	  : 2nd May 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __PREBLOCKERPOLY_H__
#define __PREBLOCKERPOLY_H__

	// Includes....
	#include "bdefs.h"
	#include "preplane.h"
	#include "prebasepoly.h"



	class CPreBlockerPoly : public CGLLNode
	{
		public:

			// Constructor
								CPreBlockerPoly()		{}
			virtual				~CPreBlockerPoly()		{}


		public:

			PVector&			Normal()				{ return GetPlane().m_Normal; }
			const PVector&		Normal() const			{ return GetPlane().m_Normal; }
			PReal&				Dist()					{ return GetPlane().m_Dist; }
			const PReal&		Dist() const			{ return GetPlane().m_Dist; }

			CPrePlane&			GetPlane()				{ return m_Plane; }
			const CPrePlane&	GetPlane() const		{ return m_Plane; }

		
		private:

			//the plane that this polygon lies in
			CPrePlane			m_Plane;

		public:

			//this must come last
			BASEPOLY_MEMBER()

	};


#endif

