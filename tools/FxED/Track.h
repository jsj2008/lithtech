//------------------------------------------------------------------
//
//   MODULE  : TRACK.H
//
//   PURPOSE : Defines class CTrack
//
//   CREATED : On 11/9/98 At 4:52:30 PM
//
//------------------------------------------------------------------

#ifndef __TRACK__H_
	#define __TRACK__H_

	// Includes....

	#include "linklist.h"
	#include "Key.h"

	class CTrack
	{
		public :

			// Constuctor

											CTrack();

			// Destructor

										   ~CTrack();

			// Member Functions

			BOOL							Init();
			void							Term();

			void							ArrangeKeys(CKey *pSquishKey);
			void							Select(BOOL bSelect = TRUE) { m_bSelected = TRUE; }

			// Accessors

			CLinkList<CKey *>*				GetKeys() { return &m_collKeys; }
			BOOL							IsSelected() { return m_bSelected; }

		protected :

			// Member Variables
			
			CLinkList<CKey *>				m_collKeys;
			BOOL							m_bSelected;
	};

#endif