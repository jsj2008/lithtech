#include "bdefs.h"
#include "interlink.h"
#include "servermgr.h"
#include "soundtrack.h"

static LTBOOL DoesLinkExist(LTObject *pOwner, void *pOther, uint32 linkType)
{
	LTLink *pListHead, *pCur;
	InterLink *pLink;

	pListHead = &pOwner->sd->m_Links;
	pCur = pListHead->m_pNext;
	while(pCur != pListHead)
	{
		pLink = (InterLink*)pCur->m_pData;

		if(pLink->m_Type == linkType && pLink->m_pOwner == pOwner && pLink->m_pOther == pOther)
			return LTTRUE;

		pCur = pCur->m_pNext;
	}

	return LTFALSE;
}


void DisconnectLinks(LTObject *pOwner, void *pOther, LTBOOL bDisconnectAll, LTBOOL bNotify)
{
	LTLink *pListHead, *pCur, *pNext;
	InterLink *pLink;

	pListHead = &pOwner->sd->m_Links;
	pCur = pListHead->m_pNext;
	while(pCur != pListHead)
	{
		pNext = pCur->m_pNext;
		pLink = (InterLink*)pCur->m_pData;

		if(( pLink->m_Type == LINKTYPE_INTERLINK || pLink->m_Type == LINKTYPE_SOUND ) && pLink->m_pOwner == pOwner && pLink->m_pOther == pOther)
		{
			if( pLink->m_Type == LINKTYPE_SOUND )
			{
				(( CSoundTrack * )pLink->m_pOther )->m_pInterLink = LTNULL;
			}
			else
			{
				// Notify the owner that the link is being broken.
				if( bNotify)
				{
					if(pLink->m_pOwner->sd->m_pObject)
					{
						pLink->m_pOwner->sd->m_pObject->OnLinkBroken(ServerObjToHandle(( LTObject * )pLink->m_pOther));
					}
				}

				dl_Remove(pLink->m_pOtherLink);
				g_DLinkBank.Free(pLink->m_pOtherLink);
			}

			dl_Remove(pLink->m_pOwnerLink);
			g_DLinkBank.Free(pLink->m_pOwnerLink);
			sb_Free(&g_pServerMgr->m_InterLinkBank, pLink);

			// Return if we aren't disconnecting ALL the links...
			if (!bDisconnectAll) return;
		}

		pCur = pNext;
	}
}


void BreakInterLinks(LTObject *pObj, uint32 linkType, LTBOOL bNotify)
{
	LTLink *pListHead, *pCur, *pNext;
	InterLink *pLink;
	CSoundTrack *pSoundTrack;

	pListHead = &pObj->sd->m_Links;
	pCur = pListHead->m_pNext;
	while(pCur != pListHead)
	{
		pNext = pCur->m_pNext;
		
		pLink = (InterLink*)pCur->m_pData;
		if(pLink->m_Type == linkType)
		{	
			// Kill the sound...
			if( pLink->m_Type == LINKTYPE_SOUND )
			{
				pSoundTrack = ( CSoundTrack * )pLink->m_pOther;
				if( !(pSoundTrack->m_dwFlags & PLAYSOUND_GETHANDLE ))
				{
					pSoundTrack->m_fTimeLeft = 0.0f;
					pSoundTrack->SetRemove( LTTRUE );
				}
				pSoundTrack->m_pInterLink = LTNULL;
			}
			else
			{
				// Notify the owner that the link is being broken.
				if( bNotify)
				{
					if(pLink->m_pOwner->sd->m_pObject)
					{
//						pLink->m_pOwner->sd->m_pObject->EngineMessageFn(
//							MID_LINKBROKEN,
//							ServerObjToHandle(( LTObject * )pLink->m_pOther), 
//							0.0f);
						pLink->m_pOwner->sd->m_pObject->OnLinkBroken(ServerObjToHandle(( LTObject * )pLink->m_pOther));
					}
				}

				// Detach the links between the two.
				dl_Remove(pLink->m_pOtherLink);
				
				// Free stuff.
				g_DLinkBank.Free(pLink->m_pOtherLink);
			}

			// Detach the links between the two.
			dl_Remove(pLink->m_pOwnerLink);

			// Free stuff.
			g_DLinkBank.Free(pLink->m_pOwnerLink);
			sb_Free(&g_pServerMgr->m_InterLinkBank, pLink);
		}
		
		pCur = pNext;
	}
}


LTRESULT CreateInterLink(LTObject *pOwner, void *pOther, uint32 linkType)
{

	// Don't link them if they already are.
	if( DoesLinkExist( pOwner, pOther, linkType ))
		return LT_OK;

	if (pOwner == pOther)
		return LT_ERROR;
	
	InterLink* pLink;
	LT_MEM_TRACK_ALLOC(pLink = (InterLink*)sb_Allocate(&g_pServerMgr->m_InterLinkBank), LT_MEM_TYPE_MISC);
	pLink->m_Type = linkType;
	pLink->m_pOwner	= pOwner;
	pLink->m_pOther	= pOther;

	LTLink* pOwnerLink;
	LT_MEM_TRACK_ALLOC(pOwnerLink = g_DLinkBank.Allocate(), LT_MEM_TYPE_MISC);
	
	pOwnerLink->m_pData = pLink;
	pLink->m_pOwnerLink = pOwnerLink;
	
	dl_Insert(&pOwner->sd->m_Links, pOwnerLink);

	if (linkType == LINKTYPE_SOUND)
	{
		((CSoundTrack *)pOther)->m_pInterLink = pLink;
	}
	else
	{
		LTLink* pOtherLink;
		LT_MEM_TRACK_ALLOC(pOtherLink = g_DLinkBank.Allocate(), LT_MEM_TYPE_MISC);

		pOtherLink->m_pData = pLink;
		pLink->m_pOtherLink = pOtherLink;

		dl_Insert(&((LTObject *)pOther)->sd->m_Links, pOtherLink);
	}

	return LT_OK;
}


