// ----------------------------------------------------------------------- //
//
// MODULE  : DialogQueue.h
//
// PURPOSE : Dialog queue structures
//
// CREATED : 2/12/99
//
// ----------------------------------------------------------------------- //

#ifndef __DIALOGQUEUE_H__
#define __DIALOGQUEUE_H__

#include "BaseCharacter.h"

struct DialogQueueCharacter
{
	char				m_szDialogFile[_MAX_PATH+1];
	CharacterSoundType	m_eCharacterSoundType;
};

struct DialogQueueTransmission
{
	char				m_szDialogFile[_MAX_PATH+1];
	DDWORD				m_nStringID;
};


struct DialogQueueElement
{
	DialogQueueElement( ) { memset( this, 0, sizeof( DialogQueueElement )); }

	HOBJECT		m_hObject;
	void *		m_pData;

	DLink		m_Link;
};



#endif // __DIALOGQUEUE_H__