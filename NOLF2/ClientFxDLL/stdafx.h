#ifndef __STDAFX_H_
	#define __STDAFX_H_

// This removes warnings about truncating symbol names when using stl maps.
//
#pragma warning( disable : 4786 )  

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>
#include <limits.h>

#include "mfcstub.h"

#include "DebugNew.h"

#include "iltclient.h"
#include "iltserver.h"
#include "iltmessage.h"
#include "globals.h"

#include "iltmodel.h"
#include "ilttransform.h"
#include "iltphysics.h"
#include "iltmath.h"
#include "iltsoundmgr.h"
#include "ltobjref.h"
#include "ltobjectcreate.h"
#include "iltcommon.h"

#include "TemplateList.h"

extern ILTClient *g_pLTClient;


#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))

inline LTBOOL GetAttachmentSocketTransform(HOBJECT hObj, const char* pSocketName,
                                          LTVector & vPos, LTRotation & rRot)
{
    if (!hObj || !pSocketName) return LTFALSE;

	HOBJECT hAttachList[30];
    uint32 dwListSize, dwNumAttachments;

    if (g_pLTClient->Common()->GetAttachments(hObj, hAttachList,
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
        for (uint32 i=0; i < dwListSize; i++)
		{
			if (hAttachList[i])
			{
				HMODELSOCKET hSocket;

				if (g_pLTClient->GetModelLT()->GetSocket(hAttachList[i], pSocketName, hSocket) == LT_OK)
				{
					LTransform transform;
                    if (g_pLTClient->GetModelLT()->GetSocketTransform(hAttachList[i], hSocket, transform, LTTRUE) == LT_OK)
					{
						vPos = transform.m_Pos;
						rRot = transform.m_Rot;
                        return LTTRUE;
					}
				}
			}
		}
	}

    return LTFALSE;
}

#endif
