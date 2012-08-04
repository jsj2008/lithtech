// ----------------------------------------------------------------------- //
//
// MODULE  : VisionModeMgr.h
//
// PURPOSE : Manages the switching and updating of vision modes.
//
// CREATED : 8/13/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VISION_MODE_MGR_H__
#define __VISION_MODE_MGR_H__

enum eVisionMode
{
	eVM_OFF = 0,
	eVM_SPY,
	eVM_NUMVISIONMODES,
};

class CVisionModeMgr
{
	public: // Methods...

		CVisionModeMgr();
		~CVisionModeMgr();

		inline	eVisionMode GetMode() const { return m_eVisionMode; }
		
		void		NextMode();
		void		PrevMode();

		void		Update();

	private: // Members

		eVisionMode	m_eVisionMode;
		eVisionMode	m_eLastVisionMode;
};

#endif // __VISION_MODE_MGR_H__