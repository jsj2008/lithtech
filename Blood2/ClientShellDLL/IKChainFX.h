// ----------------------------------------------------------------------- //
//
// MODULE  : IKChainFX.h
//
// PURPOSE : Chain special fx class - Definition
//
// CREATED : 2/3/99
//
// ----------------------------------------------------------------------- //

#ifndef __IKCHAIN_FX_H__
#define __IKCHAIN_FX_H__

// ----------------------------------------------------------------------- //

#include "SpecialFX.h"

// ----------------------------------------------------------------------- //

struct IKCHAINCS : public SFXCREATESTRUCT
{
	IKCHAINCS::IKCHAINCS();

	DBYTE		byNumLinks;
	DFLOAT		fScale;
	DFLOAT		fTime;
	DBYTE		byFXType;
	DBYTE		byFXFlags;
};

// ----------------------------------------------------------------------- //

inline IKCHAINCS::IKCHAINCS()
{
	memset(this, 0, sizeof(IKCHAINCS));
}

// ----------------------------------------------------------------------- //

#define		IKCHAIN_MAX_LINKS			100

#define		IKCHAIN_FXTYPE_STRAIGHT		0
#define		IKCHAIN_FXTYPE_BUNCHED		1

#define		IKCHAIN_FXFLAG_WIGGLE		0x01
#define		IKCHAIN_FXFLAG_RETRACT		0x02
#define		IKCHAIN_FXFLAG_GRAVITY		0x04

// ----------------------------------------------------------------------- //

class CIKChainFX : public CSpecialFX
{
	public:
		CIKChainFX();
		~CIKChainFX();

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	protected:
		HLOCALOBJ	AddLink();

	private:
		HLOCALOBJ	m_hLinks[IKCHAIN_MAX_LINKS];
		DVector		m_pPoints[IKCHAIN_MAX_LINKS + 1];

		DBYTE		m_byNumLinks;
		DFLOAT		m_fScale;
		DFLOAT		m_fTime;
		DBYTE		m_byFXType;
		DBYTE		m_byFXFlags;

		DFLOAT		m_fStartTime;
};

#endif