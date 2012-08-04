
// ----------------------------------------------------------------------- //
//
// MODULE  : AnimeLineFX.h
//
// PURPOSE : Explosion special fx class - Definition
//
// CREATED : 5/27/98
//
// ----------------------------------------------------------------------- //

#ifndef __ANIMELINE_FX_H__
#define __ANIMELINE_FX_H__


	#include "BaseScaleFX.h"


	#define MAX_ANIME_LINES 32


	class LineInfo
	{
	public:
		HOBJECT		m_hObject;
		
		LTFLOAT		m_TimeToLive;
		LTFLOAT		m_Lifetime; // How long it's been alive.
		
		LTFLOAT		m_Angle, m_AngleSpeed;
		LTFLOAT		m_Scale;
	};


	class ALCREATESTRUCT : public SFXCREATESTRUCT
	{
	public:

		LTVector		m_Pos;
		LTVector		m_DirVec;
	};


	class CAnimeLineFX : public CSpecialFX
	{
		public :

			CAnimeLineFX();
			virtual ~CAnimeLineFX();

			virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
			virtual LTBOOL CreateObject(ILTClient* pClientDE);
			virtual LTBOOL Update();

		private :

			LTVector		m_BasePos;
			LTVector		m_BaseDir;

			LineInfo	m_Lines[MAX_ANIME_LINES];
			uint32		m_nLines;
	};

#endif // __ANIMELINE_FX_H__