//------------------------------------------------------------------
//
//   MODULE  : PHASE.H
//
//   PURPOSE : Defines class CPhase
//
//   CREATED : On 11/9/98 At 4:57:50 PM
//
//------------------------------------------------------------------

#ifndef __PHASE__H_
	#define __PHASE__H_

	// Includes....

	#include "Track.h"

	class CPhase
	{
		public :

			// Constuctor

											CPhase();

			// Destructor

										   ~CPhase();

			// Member Functions

			BOOL							Init();
			void							Term();

			CTrack*							AddTrack();
			CKey*							GetKeyByID(DWORD dwID);

			void							SetupUniqueID();

			DWORD							GetUniqueID() { return m_dwUniqueID ++; }

			int								GetNumFX();

			// Accessors
			
			CLinkList<CTrack *>*			GetTracks() { return &m_collTracks; }
			int								GetPhaseLength() { return m_nPhaseLength; }

			void							SetPhaseLength(int nPhaseLength) { m_nPhaseLength = nPhaseLength; }

		protected :

			// Member Variables

			CLinkList<CTrack *>				m_collTracks;
			int								m_nPhaseLength;
			DWORD							m_dwUniqueID;
	};

#endif