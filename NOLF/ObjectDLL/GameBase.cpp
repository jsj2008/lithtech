// ----------------------------------------------------------------------- //
//
// MODULE  : GameBase.cpp
//
// PURPOSE : Game base object class implementation
//
// CREATED : 10/8/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameBase.h"
#include "CVarTrack.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "VersionMgr.h"

extern CVarTrack g_ShowDimsTrack;

CVarTrack	g_vtDimsAlpha;

BEGIN_CLASS(GameBase)
END_CLASS_DEFAULT_FLAGS(GameBase, BaseClass, NULL, NULL, CF_HIDDEN)
//END_CLASS_DEFAULT_FLAGS_PLUGIN(GameBase, BaseClass, NULL, NULL, CF_HIDDEN, CGameBasePlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::GameBase()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GameBase::GameBase(uint8 nType) : BaseClass(nType)
{
    m_hDimsBox          = LTNULL;
    m_pMarkList         = LTNULL;
	m_dwOriginalFlags	= 0;

	m_nSaveVersion		= CVersionMgr::GetSaveVersion();
	m_hstrSave			= LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::~GameBase()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

GameBase::~GameBase()
{
	RemoveBoundingBox();

	if (m_pMarkList)
	{
        g_pLTServer->RelinquishList(m_pMarkList);
	}

	FREE_HSTRING(m_hstrSave);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 GameBase::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			TriggerMsg(hSender, szMsg);

			// Make sure other people can read it...

            g_pLTServer->ResetRead(hRead);
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::TriggerMsg()
//
//	PURPOSE:	Process trigger messages
//
// --------------------------------------------------------------------------- //

void GameBase::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	if (!szMsg) return;

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
            uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
			if (!m_dwOriginalFlags)
			{
				m_dwOriginalFlags = dwFlags;
			}

			if (_stricmp(parse.m_Args[0], "VISIBLE") == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					if ((_stricmp(parse.m_Args[1], "1") == 0) ||
						(_stricmp(parse.m_Args[1], "TRUE") == 0))
					{
						dwFlags |= FLAG_VISIBLE;
					}
					else
					{
						if ((_stricmp(parse.m_Args[1], "0") == 0) ||
							(_stricmp(parse.m_Args[1], "FALSE") == 0))
						{
							dwFlags &= ~FLAG_VISIBLE;
						}
					}
				}
			}
			else if (_stricmp(parse.m_Args[0], "SOLID") == 0)
			{
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					if ((_stricmp(parse.m_Args[1], "1") == 0) ||
						(_stricmp(parse.m_Args[1], "TRUE") == 0))
					{
						dwFlags |= FLAG_SOLID;

						if (m_dwOriginalFlags & FLAG_RAYHIT)
						{
							dwFlags |= FLAG_RAYHIT;
						}
					}
					else
					{
						if ((_stricmp(parse.m_Args[1], "0") == 0) ||
							(_stricmp(parse.m_Args[1], "FALSE") == 0))
						{
							dwFlags &= ~FLAG_SOLID;
							dwFlags &= ~FLAG_RAYHIT;
						}
					}
				}
			}

            g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::CreateBoundingBox()
//
//	PURPOSE:	Create a bounding box
//
// ----------------------------------------------------------------------- //

void GameBase::CreateBoundingBox()
{
	if (m_hDimsBox) return;

	if (!g_vtDimsAlpha.IsInitted())
	{
        g_vtDimsAlpha.Init(g_pLTServer, "DimsAlpha", LTNULL, 1.0f);
	}

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);
	theStruct.m_Pos = vPos;

	SAFE_STRCPY(theStruct.m_Filename, "Models\\1x1_square.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "Models\\1x1_square.dtx");

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_GOTHRUWORLD;
	theStruct.m_ObjectType = OT_MODEL;

    HCLASS hClass = g_pLTServer->GetClass("BaseClass");
    LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);

	if (pModel)
	{
		m_hDimsBox = pModel->m_hObject;

        LTVector vDims;
        g_pLTServer->GetObjectDims(m_hObject, &vDims);

        LTVector vScale;
		VEC_DIVSCALAR(vScale, vDims, 0.5f);
        g_pLTServer->ScaleObject(m_hDimsBox, &vScale);
	}


    LTVector vOffset;
    LTRotation rOffset;
	vOffset.Init();
    rOffset.Init();

	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hDimsBox, LTNULL,
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
        g_pLTServer->RemoveObject(m_hDimsBox);
        m_hDimsBox = LTNULL;
	}

    LTVector vColor = GetBoundingBoxColor();

    g_pLTServer->SetObjectColor(m_hDimsBox, vColor.x, vColor.y, vColor.z, g_vtDimsAlpha.GetFloat());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::RemoveBoundingBox()
//
//	PURPOSE:	Remove the bounding box
//
// ----------------------------------------------------------------------- //

void GameBase::RemoveBoundingBox()
{
	if (m_hDimsBox)
	{
		HATTACHMENT hAttachment;
        if (g_pLTServer->FindAttachment(m_hObject, m_hDimsBox, &hAttachment) == LT_OK)
		{
            g_pLTServer->RemoveAttachment(hAttachment);
		}

        g_pLTServer->RemoveObject(m_hDimsBox);
        m_hDimsBox = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GameBase::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
        case MID_ACTIVATING:
		{
			if (IsCharacter(m_hObject))
			{
				g_pLTServer->SetObjectUserFlags(m_hObject, g_pLTServer->GetObjectUserFlags(m_hObject) | USRFLG_GAMEBASE_ACTIVE);
			}
		}
		break;

		case MID_DEACTIVATING:
		{
			if (IsCharacter(m_hObject))
			{
				g_pLTServer->SetObjectUserFlags(m_hObject, g_pLTServer->GetObjectUserFlags(m_hObject) & ~USRFLG_GAMEBASE_ACTIVE);
			}
		}
		break;

        case MID_PRECREATE:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP || nInfo == PRECREATE_NORMAL)
			{
				ObjectCreateStruct* pocs = (ObjectCreateStruct*)pData;
				if ( !(pocs->m_Name[0]) )
				{
                    static int s_nUniqueId = 0;
					sprintf(pocs->m_Name, "noname%d", s_nUniqueId++);
				}
			}

			return dwRet;
		}
		break;

		case MID_MODELSTRINGKEY:
		{
            uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			// Let the CmdMgr take a crack at it.

			ArgList* pArgList = (ArgList*)pData;

			char szBuffer[256];
			sprintf(szBuffer, "");

			for ( int iArg = 0 ; iArg < pArgList->argc ; iArg++ )
			{
				strcat(szBuffer, pArgList->argv[iArg]);
				strcat(szBuffer, " ");
			}

			szBuffer[strlen(szBuffer)-1] = 0;

			g_pCmdMgr->Process(szBuffer);

			return dwRet;
		}
		break;

		case MID_LINKBROKEN:
		{
			HandleLinkBroken((HOBJECT)pData);
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		default:
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::AddMark()
//
//	PURPOSE:	Add a mark to our list...
//
// --------------------------------------------------------------------------- //

void GameBase::AddMark(HOBJECT hMark)
{
	if (!hMark) return;

	// Create the list if necessary...

	if (!m_pMarkList)
	{
        m_pMarkList = g_pLTServer->CreateObjectList();
		if (!m_pMarkList) return;
	}

    g_pLTServer->AddObjectToList(m_pMarkList, hMark);
    g_pLTServer->CreateInterObjectLink(m_hObject, hMark);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::HandleLinkBroken()
//
//	PURPOSE:	Handle MID_LINKBROKEN engine message
//
// --------------------------------------------------------------------------- //

void GameBase::HandleLinkBroken(HOBJECT hLink)
{
	if (!m_pMarkList || !hLink) return;

	// Handle removing marks from the list...

	ObjectLink* pLink = m_pMarkList->m_pFirstLink;
    ObjectLink* pPrevious = LTNULL;

	while (pLink)
	{
		if (pLink->m_hObject == hLink)
		{
			m_pMarkList->m_nInList--;

			if (pLink == m_pMarkList->m_pFirstLink)
			{
				m_pMarkList->m_pFirstLink = pLink->m_pNext;
			}
			else if (pPrevious)
			{
				pPrevious->m_pNext = pLink->m_pNext;
			}

			// If the list is now empty, remove it...

			if (!m_pMarkList->m_nInList)
			{
                g_pLTServer->RelinquishList(m_pMarkList);
                m_pMarkList = LTNULL;
			}

			return;
		}

		pPrevious = pLink;
		pLink = pLink->m_pNext;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::GetBoundingBoxColor()
//
//	PURPOSE:	Get the color of the bounding box
//
// ----------------------------------------------------------------------- //

LTVector GameBase::GetBoundingBoxColor()
{
    LTVector vColor(1, 1, 1);
	switch (GetType())
	{
		case OT_MODEL :
			 vColor.Init(1, 0, 0);
		break;

		case OT_WORLDMODEL :
			 vColor.Init(0, 0, 1);
		break;

		case OT_NORMAL :
		default :
			 vColor.Init(1, 1, 1);
		break;
	}

	return vColor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::UpdateBoundingBox()
//
//	PURPOSE:	Update bounding box
//
// ----------------------------------------------------------------------- //

void GameBase::UpdateBoundingBox()
{
	int nVal = (int)g_ShowDimsTrack.GetFloat();

	if (nVal < 4)
	{
		switch (GetType())
		{
			case OT_WORLDMODEL :
			{
				if (nVal != 1)
				{
					RemoveBoundingBox();
					return;
				}
			}
			break;

			case OT_MODEL :
			{
				if (nVal != 2)
				{
					RemoveBoundingBox();
					return;
				}
			}
			break;

			case OT_NORMAL :
			{
				if (nVal != 3)
				{
					RemoveBoundingBox();
					return;
				}
			}
			break;

			default :
			break;
		}
	}

	CreateBoundingBox();

	if (m_hDimsBox)
	{
        LTVector vDims, vScale;
        g_pLTServer->GetObjectDims(m_hObject, &vDims);
		vScale = (vDims * 2.0);
        g_pLTServer->ScaleObject(m_hDimsBox, &vScale);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::SetNextUpdate()
//
//	PURPOSE:	Allows objects to set their next update time and at
//				time same time update autodeactivation
//
// ----------------------------------------------------------------------- //

void GameBase::SetNextUpdate(LTFLOAT fDelta, eUpdateControl eControl)
{
	if (!m_hObject) return;

	fDelta = fDelta <= 0.0f ? 0.0f : fDelta;
    g_pLTServer->SetNextUpdate(m_hObject, fDelta);

	if (eControl == eControlDeactivation)
	{
		if (fDelta == 0.0f)
		{
			if (!IsMultiplayerGame())
			{
				g_pLTServer->SetDeactivationTime(m_hObject, 0.001f);
			}
			g_pLTServer->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);
		}
		else
		{
			g_pLTServer->SetDeactivationTime(m_hObject, fDelta + 1.0f);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void GameBase::Save(HMESSAGEWRITE hWrite)
{
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwOriginalFlags);

	// Save the number of marks in the list...

    uint8 nNumInList = m_pMarkList ? m_pMarkList->m_nInList : 0;
    g_pLTServer->WriteToMessageByte(hWrite, nNumInList);

	// Save the objects marks...

	if (m_pMarkList && nNumInList)
	{
		ObjectLink* pLink = m_pMarkList->m_pFirstLink;
		while (pLink)
		{
            g_pLTServer->WriteToLoadSaveMessageObject(hWrite, pLink->m_hObject);
			pLink = pLink->m_pNext;
		}
	}

	SAVE_DWORD(m_nSaveVersion);
	SAVE_HSTRING(m_hstrSave);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameBase::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void GameBase::Load(HMESSAGEREAD hRead)
{
    m_dwOriginalFlags = g_pLTServer->ReadFromMessageDWord(hRead);

	// Load the number of marks in our list...

    uint8 nNumInList = g_pLTServer->ReadFromMessageByte(hRead);

	// Load the marks...

	if (nNumInList > 0)
	{
		if (m_pMarkList)
		{
            g_pLTServer->RelinquishList(m_pMarkList);
		}

        m_pMarkList = g_pLTServer->CreateObjectList();

		if (m_pMarkList)
		{
            HOBJECT hObj = LTNULL;
			for (int i=0; i < nNumInList; i++)
			{
                g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &hObj);
                g_pLTServer->AddObjectToList(m_pMarkList, hObj);
			}
		}
	}

	LOAD_DWORD(m_nSaveVersion);
	LOAD_HSTRING(m_hstrSave);
}