// ----------------------------------------------------------------------- //
//
// MODULE  : CameraFX.h
//
// PURPOSE : Camera special fx class - Definition
//
// CREATED : 5/20/98
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERA_FX_H__
#define __CAMERA_FX_H__

#include "SpecialFX.h"

struct CAMCREATESTRUCT : public SFXCREATESTRUCT
{
	CAMCREATESTRUCT::CAMCREATESTRUCT();

	LTBOOL bAllowPlayerMovement;
	uint8 nCameraType;
	LTBOOL bIsListener;
};

inline CAMCREATESTRUCT::CAMCREATESTRUCT()
{
	memset(this, 0, sizeof(CAMCREATESTRUCT));
}

class CCameraFX : public CSpecialFX
{
	public :

		CCameraFX() : CSpecialFX() 
		{
			m_bAllowPlayerMovement  = LTFALSE;
			m_nCameraType			= 0; 
			m_bIsListener			= LTFALSE;
		}

		virtual LTBOOL Update() { return !m_bWantRemove; }

		LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
			if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

			CAMCREATESTRUCT* pCAM = (CAMCREATESTRUCT*)psfxCreateStruct;

			m_bAllowPlayerMovement	= pCAM->bAllowPlayerMovement;
			m_nCameraType			= pCAM->nCameraType;
			m_bIsListener			= pCAM->bIsListener;

			return LTTRUE;
		}

		LTBOOL	AllowPlayerMovement()	const { return m_bAllowPlayerMovement; }
		uint8	GetType()				const { return m_nCameraType; }
		LTBOOL	IsListener()			const { return m_bIsListener; }

	protected :

		LTBOOL	m_bAllowPlayerMovement;
		uint8	m_nCameraType;
		LTBOOL	m_bIsListener;
};

#endif // __CAMERA_FX_H__