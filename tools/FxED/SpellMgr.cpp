//------------------------------------------------------------------
//
//   MODULE  : SPELLMGR.CPP
//
//   PURPOSE : Implements class CSpellMgr
//
//   CREATED : On 10/29/98 At 6:08:04 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "SpellMgr.h"

//------------------------------------------------------------------
//
//   FUNCTION : CSpellMgr()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CSpellMgr::CSpellMgr()
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CSpellMgr
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CSpellMgr::~CSpellMgr()
{
	// Call Term()

	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CSpellMgr
//
//------------------------------------------------------------------

BOOL CSpellMgr::Init()
{
	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CSpellMgr
//
//------------------------------------------------------------------

void CSpellMgr::Term()
{
	CLinkListNode<CSpell *> *pNode = m_collSpells.GetHead();

	while (pNode)
	{
		delete pNode->m_Data;

		pNode = pNode->m_pNext;
	}

	m_collSpells.RemoveAll();
}

//------------------------------------------------------------------
//
//   FUNCTION : AddSpell()
//
//   PURPOSE  : Adds a new spell
//
//------------------------------------------------------------------

CSpell* CSpellMgr::AddSpell()
{
	CSpell *pNewSpell = new CSpell;
	if (!pNewSpell) return NULL;

	// Create a guid

	GUID guid;
	CoCreateGuid(&guid);

	int *pInt = (int *)&guid;

	char sTmp[256];
	sTmp[0] = 0;

	for (int i = 0; i < 4; i ++)
	{
		char sInt[10];

		itoa(*pInt, sInt, 36);

		strcat(sTmp, sInt);

		pInt ++;
	}

	sprintf(sTmp, "Spell1");
	pNewSpell->SetGuid(sTmp);

	// Add it to the list of spells

	m_collSpells.AddTail(pNewSpell);
	
	// Success !!

	return pNewSpell;
}

//------------------------------------------------------------------
//
//   FUNCTION : DeleteSpell()
//
//   PURPOSE  : Deletes a spell
//
//------------------------------------------------------------------

void CSpellMgr::DeleteSpell(CSpell *pSpell)
{
	CLinkListNode<CSpell *> *pNode = m_collSpells.GetHead();

	while (pNode)
	{
		if (pNode->m_Data == pSpell)
		{
			// Found it in the list....

			delete pSpell;

			m_collSpells.Remove(pNode);

			// Success !!
			
			return;
		}
		
		pNode = pNode->GetNext();
	}

	// Shouldn't EVER EVER get here....

	ASSERT(FALSE);
}

//------------------------------------------------------------------
//
//   FUNCTION : FindSpellByName()
//
//   PURPOSE  : Locates a named spell and returns it
//
//------------------------------------------------------------------

CSpell* CSpellMgr::FindSpellByName(CString sName)
{
	CLinkListNode<CSpell *> *pNode = m_collSpells.GetHead();

	while (pNode)
	{
		if (pNode->m_Data->GetName() == sName) return pNode->m_Data;
		
		pNode = pNode->m_pNext;
	}

	// Failure....

	return NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : SpellExists()
//
//   PURPOSE  : Checks for the existence of a spell
//
//------------------------------------------------------------------

BOOL CSpellMgr::SpellExists(CSpell *pSpell)
{
	CLinkListNode<CSpell *> *pNode = m_collSpells.GetHead();

	while (pNode)
	{
		if (pNode->m_Data == pSpell) return TRUE;
		
		pNode = pNode->m_pNext;
	}

	// Failure

	return FALSE;
}

//------------------------------------------------------------------
//
//   FUNCTION : FindResource()
//
//   PURPOSE  : Locates all usages of a resource
//
//------------------------------------------------------------------

void CSpellMgr::FindResource(const char *sRez, CLinkList<SPELLNAME> *pList)
{
	CLinkListNode<CSpell *> *pSpellNode = m_collSpells.GetHead();

	while (pSpellNode)
	{
		CLinkListNode<CTrack *> *pTrackNode = pSpellNode->m_Data->GetPhase(1)->GetTracks()->GetHead();

		while (pTrackNode)
		{
			CLinkListNode<CKey *> *pKeyNode = pTrackNode->m_Data->GetKeys()->GetHead();

			while (pKeyNode)
			{
				CFastListNode<FX_PROP> *pFxNode = pKeyNode->m_Data->GetCollProps()->GetHead();

				while (pFxNode)
				{
					if (pFxNode->m_Data.m_nType == FX_PROP::PATH)
					{
						// Possible resource.... check it....

						char sTmp[256];
						strcpy(sTmp, pFxNode->m_Data.m_data.m_sVal);
						
						char *sRes = sTmp;

						if (strlen(sRes) > 4)
						{
							// Check for a resource 

							if (sTmp[3] == '|')
							{
								char *sRealRes = sTmp + 4;
								sTmp[3] = 0;

								if ((!stricmp(sTmp, "SPR")) ||
									(!stricmp(sTmp, "ABC")) ||
									(!stricmp(sTmp, "DTX")) ||
									(!stricmp(sTmp, "WAV")))
								{
									if (!stricmp(sRez, sRealRes))
									{
										// We've found one....

										SPELLNAME spName;
										strcpy(spName.m_sName, (char *)(LPCSTR)pSpellNode->m_Data->GetName());

										pList->AddTail(spName);
									}
								}
							}
						}
					}
					
					pFxNode = pFxNode->m_pNext;					
				}

				pKeyNode = pKeyNode->m_pNext;
			}

			pTrackNode = pTrackNode->m_pNext;
		}

		pSpellNode = pSpellNode->m_pNext;
	}
}
