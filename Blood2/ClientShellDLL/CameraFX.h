// ----------------------------------------------------------------------- //
//
// MODULE  : CameraFX.h
//
// PURPOSE : A special fx class for managing external camera objects
//
// CREATED : 6/28/98
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERAFX_H__
#define __CAMERAFX_H__

#include "SpecialFX.h"
#include "ClientServerShared.h"


struct CAMERACREATESTRUCT : public SFXCREATESTRUCT
{
	CAMERACREATESTRUCT::CAMERACREATESTRUCT()
	{
		memset(this, 0, sizeof(CAMERACREATESTRUCT));
	}

	DBYTE	nType;
	DBOOL	bPlayerMovement;
	DBOOL	bHidePlayer;
	DBOOL	bIsListener;
};



class CCameraFX : public CSpecialFX
{
	public :

		CCameraFX() : CSpecialFX() 
		{
			m_nType = CAMTYPE_FULLSCREEN; 
			m_bPlayerMovement = DFALSE;
			m_bHidePlayer = DFALSE;
			m_bIsListener = DTRUE;
		}

		virtual	DBOOL Update() { return !m_bWantRemove; }

		DBOOL	Init(SFXCREATESTRUCT* psfxCreateStruct)
		{
			if (!CSpecialFX::Init(psfxCreateStruct)) 
				return DFALSE;

			CAMERACREATESTRUCT* pCamera = (CAMERACREATESTRUCT*)psfxCreateStruct;

			m_nType = pCamera->nType;
			m_bPlayerMovement = pCamera->bPlayerMovement;
			m_bHidePlayer = pCamera->bHidePlayer;
			m_bIsListener = pCamera->bIsListener;

			return DTRUE;
		}

		DBYTE	GetType() const { return m_nType; }
		DBOOL	GetPlayerMovement() const { return m_bPlayerMovement; }
		DBOOL	GetHidePlayer() const { return m_bHidePlayer; }
		DBOOL	IsListener( ) const { return m_bIsListener; }

	protected :

		DBYTE	m_nType;
		DBOOL	m_bPlayerMovement;
		DBOOL	m_bHidePlayer;
		DBOOL	m_bIsListener;
};


#endif // __CAMERAFX_H__