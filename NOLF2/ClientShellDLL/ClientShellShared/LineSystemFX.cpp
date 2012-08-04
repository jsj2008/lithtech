// ----------------------------------------------------------------------- //
//
// MODULE  : LineSystemFX.cpp
//
// PURPOSE : LineSystem special FX - Implementation
//
// CREATED : 4/12/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LineSystemFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "VarTrack.h"

#define MAX_LINES_PER_SECOND	1000			// Max lines to be created in a second
#define MAX_TOTAL_LINES			4000			// Max lines total (all systems)
#define MAX_SYSTEM_LINES		2000			// Max lines per system
#define MAX_VIEW_DIST_SQR		(10000*10000)	// Max global distance to add lines

extern CGameClientShell* g_pGameClientShell;
extern LTVector g_vWorldWindVel;

static VarTrack s_cvarTweak;

int CLineSystemFX::m_snTotalLines = 0;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::CLineSystemFX
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

CLineSystemFX::CLineSystemFX() : CBaseLineSystemFX()
{
    m_bFirstUpdate  = LTTRUE;
    m_bContinuous   = LTFALSE;

	m_fLastTime		= 0.0f;
	m_fNextUpdate	= 0.01f;

    m_RemoveLineFn  = LTNULL;
    m_pUserData     = LTNULL;

	m_fMaxViewDistSqr = 1000.0f*1000.0f;
	m_nTotalNumLines  = 0;
    m_pLines          = LTNULL;

	m_vStartOffset.Init();
	m_vEndOffset.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::~CLineSystemFX
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CLineSystemFX::~CLineSystemFX()
{
	if (m_pLines)
	{
		for (int i=0; i < m_nTotalNumLines; i++)
		{
			RemoveLine(i);
		}

		debug_deletea(m_pLines);
        m_pLines = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::Init
//
//	PURPOSE:	Init the line system
//
// ----------------------------------------------------------------------- //

LTBOOL CLineSystemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	// Set up our creation struct...

	LSCREATESTRUCT* pLS = (LSCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pLS;


	// Set our (parent's) flags...

	m_vPos = m_cs.vPos;


	// Set our max viewable distance...

	m_fMaxViewDistSqr = m_cs.fViewDist*m_cs.fViewDist;
	m_fMaxViewDistSqr = m_fMaxViewDistSqr > MAX_VIEW_DIST_SQR ? MAX_VIEW_DIST_SQR : m_fMaxViewDistSqr;


	// Adjust velocities based on global wind values...

	m_cs.vMinVel += g_vWorldWindVel;
	m_cs.vMaxVel += g_vWorldWindVel;


	// Adjust slope of lines based on global wind values...

	m_vStartOffset.y = m_cs.fLineLength / 2.0f;
	m_vEndOffset.y	 = -m_cs.fLineLength / 2.0f;

	float fVal = (float)fabs(m_cs.vMaxVel.y);

	if (fVal > 0.0)
	{
		m_vEndOffset.x = (m_cs.fLineLength * (g_vWorldWindVel.x / fVal));
	}

	if (fVal > 0.0)
	{
		m_vEndOffset.z = (m_cs.fLineLength * (g_vWorldWindVel.z / fVal));
	}

	m_bContinuous = (m_cs.fBurstWait <= 0.001f);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CLineSystemFX::CreateObject(ILTClient *pClientDE)
{
    if (!pClientDE ) return LTFALSE;

    LTBOOL bRet = CBaseLineSystemFX::CreateObject(pClientDE);

	if (bRet && m_hObject && m_hServerObject)
	{
        uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);
		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
		}
	}


	// Create lines...

	m_nTotalNumLines = int(m_cs.fLinesPerSecond * m_cs.fLineLifetime);
	if (m_nTotalNumLines > MAX_SYSTEM_LINES)
	{
		m_nTotalNumLines = MAX_SYSTEM_LINES;
	}

	m_pLines = debug_newa(LSLineStruct, m_nTotalNumLines);

	SetupSystem();

    s_cvarTweak.Init(g_pLTClient, "TweakLines", NULL, 0.0f);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

LTBOOL CLineSystemFX::Update()
{
    if (!m_hObject || !m_pClientDE || m_bWantRemove) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	// Hide/show the line system if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
			m_fLastTime = fTime;
            return LTTRUE;
		}
		else
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
		}
	}

	// Debugging aid...

	if (s_cvarTweak.GetFloat() > 0)
	{
		TweakSystem();
	}


	if (m_bFirstUpdate)
	{
		m_fLastTime = fTime;
        m_bFirstUpdate = LTFALSE;
	}
	else
	{
		UpdateSystem();
	}

	// Make sure it is time to update...

	if (fTime < m_fLastTime + m_fNextUpdate)
	{
        return LTTRUE;
	}


	// Ok, how many to add this frame....

	float fTimeDelta = fTime - m_fLastTime;

	// Make sure delta time is no less than 15 frames/sec if we're
	// continuously adding lines...

	if (m_bContinuous)
	{
		fTimeDelta = fTimeDelta > 0.0666f ? 0.0666f : fTimeDelta;
	}

	int nToAdd = (int) floor(m_cs.fLinesPerSecond * fTimeDelta);
    nToAdd = LTMIN(nToAdd, (int)(MAX_LINES_PER_SECOND * fTimeDelta));


	// Add new lines...

	AddLines(nToAdd);


	// Determine when next update should occur...

	if (m_cs.fBurstWait > 0.001f)
	{
		m_fNextUpdate = m_cs.fBurstWait * GetRandom(m_cs.fBurstWaitMin, m_cs.fBurstWaitMax);
	}
	else
	{
		m_fNextUpdate = 0.001f;
	}

	m_fLastTime = fTime;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::UpdateSystem
//
//	PURPOSE:	Update the lines in the system
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::UpdateSystem()
{
	if (!m_pLines) return;

	// Make sure delta time is no less than 15 frames/sec...

    LTFLOAT fDeltaTime = g_pGameClientShell->GetFrameTime();
	fDeltaTime = fDeltaTime > 0.0666f ? 0.0666f : fDeltaTime;

	// Update all the lines...

    LTLine line;

	for (int i=0; i < m_nTotalNumLines; i++)
	{
        if (m_pLines[i].hLTLine)
		{
            m_pClientDE->GetLineInfo(m_pLines[i].hLTLine, &line);

			line.m_Points[0].m_Pos += (m_pLines[i].vVel * fDeltaTime);
			line.m_Points[1].m_Pos += (m_pLines[i].vVel * fDeltaTime);

            m_pClientDE->SetLineInfo(m_pLines[i].hLTLine, &line);
			m_pLines[i].fLifetime -= fDeltaTime;

			// Remove dead lines...

			if (m_pLines[i].fLifetime <= 0.0f)
			{
				if (m_RemoveLineFn)
				{
					m_RemoveLineFn(m_pUserData, &(m_pLines[i]));
				}

				RemoveLine(i);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::SetupSystem
//
//	PURPOSE:	Setup the system (add ALL possible lines)
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::SetupSystem()
{
	if (!m_hServerObject) return;

	if (m_bContinuous)
	{
		for (int i=0; i < m_nTotalNumLines; i++)
		{
			if (m_snTotalLines < MAX_TOTAL_LINES)
			{
                AddLine(i, LTTRUE);
			}
			else
			{
				break;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::AddLines
//
//	PURPOSE:	Add lines to the system
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::AddLines(int nToAdd)
{
	if (!m_hServerObject || !m_pLines) return;

	int nNumAdded = 0;

	for (int i=0; nNumAdded < nToAdd && i < m_nTotalNumLines; i++)
	{
        if (!m_pLines[i].hLTLine || m_pLines[i].fLifetime <= 0.0f)
		{
			if (m_snTotalLines < MAX_TOTAL_LINES)
			{
				AddLine(i);
				nNumAdded++;
			}
			else
			{
				break;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::AddLine
//
//	PURPOSE:	Add a line to the system
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::AddLine(int nIndex, LTBOOL bSetInitialPos)
{
	if (!m_hServerObject || !m_pLines || nIndex < 0 || nIndex >= m_nTotalNumLines) return;

    LTVector vPos, vCamPos, vObjPos, vDist, vVel;
    LTLine line;

	g_pLTClient->GetObjectPos(m_hServerObject, &vObjPos);

	// Get the camera's position...If the camera is too far away from
	// the line being added, don't add it :)

	HOBJECT hCamera = g_pPlayerMgr->GetCamera();
	if (!hCamera) return;

	g_pLTClient->GetObjectPos(hCamera, &vCamPos);
	vCamPos.y = 0.0f;  // Only take x/z into account...

	vPos.x = GetRandom(-m_cs.vDims.x, m_cs.vDims.x);
	vPos.y = GetRandom(-m_cs.vDims.y, m_cs.vDims.y);
	vPos.z = GetRandom(-m_cs.vDims.z, m_cs.vDims.z);

    LTFLOAT fLifetime = m_cs.fLineLifetime;

	vVel.x = GetRandom(m_cs.vMinVel.x, m_cs.vMaxVel.x);
	vVel.y = GetRandom(m_cs.vMinVel.y, m_cs.vMaxVel.y);
	vVel.z = GetRandom(m_cs.vMinVel.z, m_cs.vMaxVel.z);


	// If we need to set the initial line pos...

	if (bSetInitialPos)
	{
		fLifetime = GetRandom(0.0f, m_cs.fLineLifetime);

		if (fLifetime >= 0.0f)
		{
			vPos += (vVel * (m_cs.fLineLifetime - fLifetime));
		}
		else
		{
			fLifetime = m_cs.fLineLifetime;
		}
	}


	vObjPos += vPos;
	vObjPos.y = 0.0f; // Only take x/z into account

	vDist = vCamPos - vObjPos;

	if (vDist.MagSqr() < m_fMaxViewDistSqr || bSetInitialPos)
	{
		line.m_Points[0].m_Pos = vPos + m_vStartOffset;
		line.m_Points[0].r	   = m_cs.vStartColor.x;
		line.m_Points[0].g	   = m_cs.vStartColor.y;
		line.m_Points[0].b	   = m_cs.vStartColor.z;
		line.m_Points[0].a	   = m_cs.fStartAlpha;

		line.m_Points[1].m_Pos = vPos + m_vEndOffset;
		line.m_Points[1].r	   = m_cs.vEndColor.x;
		line.m_Points[1].g	   = m_cs.vEndColor.y;
		line.m_Points[1].b	   = m_cs.vEndColor.z;
		line.m_Points[1].a	   = m_cs.fEndAlpha;

        if (m_pLines[nIndex].hLTLine)
		{
            m_pClientDE->SetLineInfo(m_pLines[nIndex].hLTLine, &line);
		}
		else
		{
            m_pLines[nIndex].hLTLine = m_pClientDE->AddLine(m_hObject, &line);

			m_snTotalLines++;
		}

		m_pLines[nIndex].fLifetime	= fLifetime;
		m_pLines[nIndex].vVel		= vVel;
	}
	else
	{
		RemoveLine(nIndex);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::RemoveLine
//
//	PURPOSE:	Remove the line from the system
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::RemoveLine(int nIndex)
{
	if (!m_pLines || nIndex < 0 || nIndex >= m_nTotalNumLines) return;

    if (m_pLines[nIndex].hLTLine)
	{
        m_pClientDE->RemoveLine(m_hObject, m_pLines[nIndex].hLTLine);
        m_pLines[nIndex].hLTLine = LTNULL;
		m_snTotalLines--;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLineSystemFX::TweakSystem
//
//	PURPOSE:	Tweak the particle system
//
// ----------------------------------------------------------------------- //

void CLineSystemFX::TweakSystem()
{
    LTFLOAT fIncValue = 0.01f;
    LTBOOL bChanged = LTFALSE;

    LTVector vScale;
	vScale.Init();

    uint32 dwPlayerFlags = g_pPlayerMgr->GetPlayerFlags();

	// Move faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = .5f;
	}

	// Move Red up/down...

	if ((dwPlayerFlags & BC_CFLG_FORWARD) || (dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;

        m_cs.vMinVel.y += (LTFLOAT)(fIncValue * 101.0);
        m_cs.vMaxVel.y += (LTFLOAT)(fIncValue * 101.0);
		//m_cs.vColor1
		//m_cs.vColor2

        bChanged = LTTRUE;
	}


	// Add/Subtract number of lines per second

	if ((dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) || (dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
        m_cs.fLinesPerSecond += (LTFLOAT)(fIncValue * 101.0);

		m_cs.fLinesPerSecond = m_cs.fLinesPerSecond < 0.0f ? 0.0f :
			(m_cs.fLinesPerSecond > MAX_LINES_PER_SECOND ? MAX_LINES_PER_SECOND : m_cs.fLinesPerSecond);

        bChanged = LTTRUE;
	}


	// Change line length...

	if ((dwPlayerFlags & BC_CFLG_JUMP) || (dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;

		// Adjust length and slope of lines...

		m_cs.fLineLength += (fIncValue*2.0f);

		m_vStartOffset.y = m_cs.fLineLength / 2.0f;
		m_vEndOffset.y	 = -m_cs.fLineLength / 2.0f;

		float fVal = (float)fabs(m_cs.vMaxVel.y);

		if (fVal > 0.0)
		{
			m_vEndOffset.x = (m_cs.fLineLength * (g_vWorldWindVel.x / fVal));
		}

		if (fVal > 0.0)
		{
			m_vEndOffset.z = (m_cs.fLineLength * (g_vWorldWindVel.z / fVal));
		}

        bChanged = LTTRUE;
	}


	if (bChanged)
	{
		g_pGameClientShell->CSPrint("Lines per second: %.2f", m_cs.fLinesPerSecond);
		g_pGameClientShell->CSPrint("Line length: %.2f", m_vStartOffset.y - m_vEndOffset.y);
		g_pGameClientShell->CSPrint("Velocity: %.2f", m_cs.vMinVel.y);
	}
}