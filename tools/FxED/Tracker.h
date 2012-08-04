//------------------------------------------------------------------
//
//   MODULE  : TRACKER.H
//
//   PURPOSE : Defines class CTracker
//
//   CREATED : On 11/9/98 At 6:20:11 PM
//
//------------------------------------------------------------------

#ifndef __TRACKER__H_
	#define __TRACKER__H_

	class CTracker
	{
		public :

			// Constuctor

											CTracker();

			// Destructor

										   ~CTracker();

			// Member Functions

			BOOL							Init(CWnd *pWnd, CRect *prcTrack = NULL);
			void							Term();

			void							Track(CPoint ptAnchor);
			virtual void					TrackUpdate() { }

			// Accessors

		protected :

			// Member Variables

			CWnd							m_wndFocus;
			CRect							m_rcTrack;
			CPoint							m_ptAnchor;
			CPoint							m_ptLast;
	};

#endif