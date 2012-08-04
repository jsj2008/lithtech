// ----------------------------------------------------------------------- //
//
// MODULE  : KeyFramer.h
//
// PURPOSE : KeyFramer definition
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#ifndef __KEYFRAMER_H__
#define __KEYFRAMER_H__

#include "cpp_engineobjects_de.h"
#include "dynarray.h"
#include "KeyData.h"
#include "B2BaseClass.h"


struct KEYNODE
{
	KEYNODE()	{ pPrev = NULL; pNext = NULL; }

	KeyData		keyData;
	KEYNODE*	pPrev;
	KEYNODE*	pNext;
};

class KeyFramer : public B2BaseClass
{
	public :

		KeyFramer();
		~KeyFramer();

		void		GoActive();
		void		Pause()				{ m_bActive = DFALSE; }
		void		Resume()			{ m_bActive = DTRUE; }

	protected :

		KEYNODE*	GetNextPositionKey (KEYNODE* pNode);
		void		ProcessKey (KEYNODE* pNode);

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD		ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		HSTRING		m_hstrObjectName;
		HSTRING		m_hstrBaseKeyName;
		DBOOL		m_bStartActive;
		DBOOL		m_bLooping;

		DBOOL		m_bActive;
		ObjectList*	m_pObjectList;

		CDynArray<DVector>	 m_pOffsets;
		CDynArray<DRotation> m_pRotations;
		
		KEYNODE*	m_pKeys;
		KEYNODE*	m_pCurKey;
		KEYNODE*	m_pPosition1;
		KEYNODE*	m_pPosition2;

		DFLOAT		m_fCurTime;
		DBOOL		m_bFirstUpdate;
		int			m_nNumKeys;
		HSOUNDDE	m_hsndKeySound;

	private :

		DBOOL	ReadProp(ObjectCreateStruct *pData);
		DBOOL	InitialUpdate(DVector *pMovement);
		DBOOL	Update(DVector *pMovement);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		DBOOL	BuildObjectList();

		DBOOL	CreateKeyList();
};

#endif // __KEYFRAMER_H__
