//------------------------------------------------------------------
//
//   MODULE  : POLYTUBEFX.CPP
//
//   PURPOSE : Implements class CPolyTubeFX
//
//   CREATED : On 12/3/98 At 6:34:44 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "PolyTubeFX.h"
#include "ClientFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPolyTubeProps::CPolyTubeProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CPolyTubeProps::CPolyTubeProps() : 
	m_tmAddPtInterval	(0.1f),
	m_nMaxTrailLength	(50),
	m_tmSectionLifespan	(2.0f),
	m_uAdd				(0.0f),
	m_fTrailWidth		(20.0f),
	m_eBlendMode		(DRAWPRIM_NOBLEND),
	m_eAlphaTest		(DRAWPRIM_NOALPHATEST),
	m_eColorOp			(DRAWPRIM_NOCOLOROP),
	m_eFillMode			(DRAWPRIM_FILL),
	m_eWidthStyle		(ePTWS_Constant),
	m_eAllignment		(ePTA_Camera)
{
	m_sPath[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPolyTubeProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CPolyTubeProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "Texture" ))
		{
			fxProp.GetPath( m_sPath );
		} 
		else if( !_stricmp( fxProp.m_sName, "TrailWidth" ))
		{
			m_fTrailWidth = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "WidthStyle" ))
		{
			m_eWidthStyle = (EPTWidthStyle)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "UAdd" ))
		{
			m_uAdd = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "SectionLifespan" ))
		{
			m_tmSectionLifespan = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "TrailLen" ))
		{
			m_nMaxTrailLength = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "SectionInterval" ))
		{
			m_tmAddPtInterval = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "BlendMode" ))
		{
			m_eBlendMode = (ELTBlendMode)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "AlphaTest" ))
		{
			m_eAlphaTest = (ELTTestMode)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "ColorOp" ))
		{
			m_eColorOp = (ELTColorOp)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "FillMode" ))
		{
			m_eFillMode = (ELTDPFillMode)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "Allignment" ))
		{
			m_eAllignment = (EPTAllignment)fxProp.GetComboVal();
		}

	}

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : CPolyTubeFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CPolyTubeFX::CPolyTubeFX( )
:	CBaseFX				( CBaseFX::ePolyTubeFX ),
	m_tmElapsedEmission (0.0f),
	m_dwWidth			(1),
	m_hTexture			(LTNULL),
	m_uOffset			(0.0f),
	m_bLoadFailed		(false),
	m_bReallyClose		(false)
{

}

//------------------------------------------------------------------
//
//   FUNCTION : ~CPolyTubeFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CPolyTubeFX::~CPolyTubeFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CPolyTubeFX
//
//------------------------------------------------------------------

bool CPolyTubeFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	LTVector vSave = pBaseData->m_vPos;

	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps)) 
		return false;

	if( !GetProps()->m_nMaxTrailLength || !GetProps()->m_fTrailWidth ) 
		return LTFALSE;

	ObjectCreateStruct ocs;
	INIT_OBJECTCREATESTRUCT(ocs);

	ocs.m_ObjectType		= OT_NORMAL;
	ocs.m_Flags				= pBaseData->m_dwObjectFlags | FLAG_NOLIGHT;
	ocs.m_Flags2			|= pBaseData->m_dwObjectFlags2;
	ocs.m_Pos				= m_vCreatePos;

	m_hObject = m_pLTClient->CreateObject(&ocs);
	if( !m_hObject )
		return false;

	// Are we rendering really close?

	m_bReallyClose = !!(pBaseData->m_dwObjectFlags & FLAG_REALLYCLOSE);

	// Make sure we don't exceed the total length of the polytrail....

	LTVector vStart, vEnd;

	vStart = vSave;
	m_pLTClient->GetObjectPos(m_hParent, &vEnd);

	float fLen = (vStart - vEnd).Mag();

	if (fLen > 256.0f)
	{
		// This is too big of a distance, start polytrail at the right spot...

		vSave = m_vPos;
	}
	
	// Add an initial point

	PT_TRAIL_SECTION ts;
	ts.m_vPos		= vSave;
	ts.m_tmElapsed	= 0.0f;
	ts.m_uVal		= 0.0f;
	m_collPathPts.AddTail(ts);

	// Success !!

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CPolyTubeFX
//
//------------------------------------------------------------------

void CPolyTubeFX::Term()
{
	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);
	if (m_hTexture)
	{
		m_pLTClient->GetTexInterface()->ReleaseTextureHandle(m_hTexture);
		m_hTexture = LTNULL;
	}
	m_hObject = NULL;

}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CPolyTubeFX
//
//------------------------------------------------------------------

bool CPolyTubeFX::Update(float tmFrameTime)
{
	// Base class update first
	if (!CBaseFX::Update(tmFrameTime)) 
		return false;

	if ((!m_hTexture) && (!m_bLoadFailed))
	{
		m_pLTClient->GetTexInterface()->CreateTextureFromName(m_hTexture, GetProps()->m_sPath);

		if (m_hTexture)
		{
			// Retrieve texture dims
			uint32 nHeight;
			m_pLTClient->GetTexInterface()->GetTextureDims(m_hTexture, m_dwWidth, nHeight);
		}
		else
		{
			m_bLoadFailed = true;
		}
	}

	if ((m_collPathPts.GetSize() < 2) && IsShuttingDown())
	{
		m_collPathPts.RemoveAll();
		return true;
	}
	
	float tmAddPtInterval = GetProps()->m_tmAddPtInterval * 2.0f;

	LTRotation rRot;
	m_pLTClient->GetObjectRotation( m_hObject, &rRot );

	//increase the emission time elapse
	m_tmElapsedEmission += tmFrameTime;

	if (!IsShuttingDown() && 
	    (m_collPathPts.GetSize() < GetProps()->m_nMaxTrailLength) && 
		((m_tmElapsedEmission > GetProps()->m_tmAddPtInterval) || (m_collPathPts.GetSize() == 1)))
	{
		LTVector vNew = m_vPos;
		
		// Only add the new point if it's not the same as the last one....

		// Add a new trail section

		PT_TRAIL_SECTION ts;
		ts.m_vPos		= vNew;
		ts.m_tmElapsed	= 0.0f;

		switch( GetProps()->m_eAllignment )
		{
			case ePTA_Up:
				ts.m_vBisector = rRot.Up();
			break;

			case ePTA_Right:
				ts.m_vBisector = rRot.Right();
			break;

			case ePTA_Forward:
				ts.m_vBisector = rRot.Forward();
			break;

			case ePTA_Camera:
			default:
				ts.m_vBisector.Init();
			break;
		}
		
		
		// Compute u coordinate

		if (m_collPathPts.GetSize())
		{
			LTVector vPrev = m_collPathPts.GetTail()->m_Data.m_vPos;
			float fUPrev = m_collPathPts.GetTail()->m_Data.m_uVal;
			
			float fWidth = (float)m_dwWidth;
			float fScalar = fWidth / GetProps()->m_fTrailWidth;

			ts.m_uVal = fUPrev + ((((vNew - vPrev).Mag()) / fWidth) * fScalar);

		}
		else
		{
			ts.m_uVal = 0.0f;
		}

		m_collPathPts.AddTail(ts);
		
		m_tmElapsedEmission = 0.0f;
	}

	// Render the tube....

	if (m_collPathPts.GetSize() < 2) return true;

	CLinkListNode<PT_TRAIL_SECTION> *pNode = m_collPathPts.GetHead();

	// Fudge the last point to be the current one...

	if( !IsShuttingDown() )
		m_collPathPts.GetTail()->m_Data.m_vPos = m_vPos;

	// Transform the path

	LTMatrix mCam;
	if( m_bReallyClose || (GetProps()->m_eAllignment != ePTA_Camera))
	{
		mCam.Identity();
	}
	else
	{
		mCam = GetCamTransform(m_pLTClient, m_hCamera);
	}
	 
	while (pNode)
	{
		MatVMul(&pNode->m_Data.m_vTran, &mCam, &pNode->m_Data.m_vPos);
		pNode = pNode->m_pNext;
	}

	// Do some precalculations

	pNode = m_collPathPts.GetHead();

	float fCurU = 0.0f;

	while (pNode)
	{
		pNode->m_Data.m_tmElapsed += tmFrameTime;

		if( GetProps()->m_eAllignment == ePTA_Camera )
		{
			LTVector vBisector;
			vBisector.z = 0.0f;

			// Compute the midpoint vectors

			if (pNode == m_collPathPts.GetHead())
			{
				LTVector vStart = pNode->m_Data.m_vTran;
				LTVector vEnd   = pNode->m_pNext->m_Data.m_vTran;
				
				vBisector.x = vEnd.y - vStart.y;
				vBisector.y = -(vEnd.x - vStart.x);
			}
			else if (pNode == m_collPathPts.GetTail())
			{
				LTVector vEnd   = pNode->m_Data.m_vTran;
				LTVector vStart = pNode->m_pPrev->m_Data.m_vTran;
				
				vBisector.x = vEnd.y - vStart.y;
				vBisector.y = -(vEnd.x - vStart.x);
			}
			else
			{
				LTVector vPrev  = pNode->m_pPrev->m_Data.m_vTran;
				LTVector vStart = pNode->m_Data.m_vTran;
				LTVector vEnd   = pNode->m_pNext->m_Data.m_vTran;

				float x1 = vEnd.y - vStart.y;
				float y1 = -(vEnd.x - vStart.x);
				float z1 = vStart.z - vEnd.z;

				float x2 = vStart.y - vPrev.y;
				float y2 = -(vStart.x - vPrev.x);
				float z2 = vPrev.z - vEnd.z;
				
				vBisector.x = (x1 + x2) / 2.0f;
				vBisector.y = (y1 + y2) / 2.0f;
			}

			pNode->m_Data.m_vBisector = vBisector;
		}
		
		LTFLOAT fWidth = CalcCurWidth();
		pNode->m_Data.m_vBisector.Norm( fWidth );

		// Setup the colour
		
		float r, g, b, a;			
		CalcColour(pNode->m_Data.m_tmElapsed, GetProps()->m_tmSectionLifespan, &r, &g, &b, &a);			

		int ir = (int)(r * 255.0f);
		int ig = (int)(g * 255.0f);
		int ib = (int)(b * 255.0f);
		int ia = (int)(a * 255.0f);

		pNode->m_Data.m_red = Clamp( ir, 0, 255 );
		pNode->m_Data.m_green = Clamp( ig, 0, 255 );
		pNode->m_Data.m_blue = Clamp( ib, 0, 255 );
		pNode->m_Data.m_alpha = Clamp( ia, 0, 255 );
	
		pNode = pNode->m_pNext;
	}

	pNode = m_collPathPts.GetHead();

	pNode = m_collPathPts.GetHead();

	// Delete any dead nodes

	while (pNode->m_pNext)
	{
		CLinkListNode<PT_TRAIL_SECTION> *pDelNode= NULL;

		if (pNode->m_Data.m_tmElapsed >= GetProps()->m_tmSectionLifespan)
		{
			pDelNode = pNode;
		}
		
		pNode = pNode->m_pNext;

		if (pDelNode) m_collPathPts.Remove(pDelNode);
	}

	// Increment the offset
	m_uOffset += tmFrameTime * GetProps()->m_uAdd;

	// Success !!

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPolyTubeFX::Render
//
//  PURPOSE:	Renders the polytube to the screen using DrawPrim
//
// ----------------------------------------------------------------------- //

bool CPolyTubeFX::Render()
{
	if(!CBaseFX::Render())
		return false;

	CLinkListNode<PT_TRAIL_SECTION> *pNode = m_collPathPts.GetHead();

	//make sure that we have at least one node
	if(!pNode)
		return true;

	ILTDrawPrim *pDrawPrimLT;
	pDrawPrimLT = m_pLTClient->GetDrawPrim();

	// Draw the polylist
	if( !pDrawPrimLT )
		return false;

	pDrawPrimLT->SetTexture( m_hTexture );
	pDrawPrimLT->SetReallyClose( m_bReallyClose );
	pDrawPrimLT->SetCamera( m_hCamera );
	pDrawPrimLT->SetTransformType( (GetProps()->m_eAllignment == ePTA_Camera ? DRAWPRIM_TRANSFORM_CAMERA : DRAWPRIM_TRANSFORM_WORLD) );
	pDrawPrimLT->SetZBufferMode( DRAWPRIM_ZRO );
	pDrawPrimLT->SetFillMode( GetProps()->m_eFillMode );
	pDrawPrimLT->SetAlphaTestMode( GetProps()->m_eAlphaTest );
	pDrawPrimLT->SetAlphaBlendMode( GetProps()->m_eBlendMode );
	pDrawPrimLT->SetColorOp( GetProps()->m_eColorOp );

	uint32 nTris = 0;
	uint32 nVerts = 0;

	LT_POLYGT3 *pTri = g_pTris;
	LTVector *pVerts = g_pVerts;

	pDrawPrimLT->BeginDrawPrim();

	while (pNode->m_pNext)
	{
		LTVector vStart = pNode->m_Data.m_vTran;
		LTVector vEnd   = pNode->m_pNext->m_Data.m_vTran;
	
		LTVector vBisector1 = pNode->m_Data.m_vBisector;
		LTVector vBisector2 = pNode->m_pNext->m_Data.m_vBisector;

		*pVerts ++ = vStart + vBisector1;
		*pVerts ++ = vEnd + vBisector2;
		*pVerts ++ = vEnd - vBisector2;
		*pVerts ++ = vStart - vBisector1;

		uint8 r1 = pNode->m_Data.m_red;
		uint8 g1 = pNode->m_Data.m_green;
		uint8 b1 = pNode->m_Data.m_blue;
		uint8 a1 = pNode->m_Data.m_alpha;
		float u1 = pNode->m_Data.m_uVal + m_uOffset;

		uint8 r2 = pNode->m_pNext->m_Data.m_red;
		uint8 g2 = pNode->m_pNext->m_Data.m_green;
		uint8 b2 = pNode->m_pNext->m_Data.m_blue;
		uint8 a2 = pNode->m_pNext->m_Data.m_alpha;
		float u2 = pNode->m_pNext->m_Data.m_uVal + m_uOffset;		

		SetupVert(pTri, 0, g_pVerts[nVerts], r1, g1, b1, a1, u1, 0.0f);
		SetupVert(pTri, 1, g_pVerts[nVerts + 1], r2, g2, b2, a2, u2, 0.0f);
		SetupVert(pTri, 2, g_pVerts[nVerts + 2], r2, g2, b2, a2, u2, 1.0f);

		pTri ++;
		nTris ++;

		SetupVert(pTri, 0, g_pVerts[nVerts], r1, g1, b1, a1, u1, 0.0f);
		SetupVert(pTri, 1, g_pVerts[nVerts + 2], r2, g2, b2, a2, u2, 1.0f);
		SetupVert(pTri, 2, g_pVerts[nVerts + 3], r1, g1, b1, a1, u1, 1.0f);

		pTri ++;
		nTris ++;

		nVerts += 4;

		pNode = pNode->m_pNext;

		//see if we need to flush our buffer
		if(nTris >= MAX_BUFFER_TRIS - 2)
		{
			pDrawPrimLT->DrawPrim(g_pTris, nTris);
			nTris = 0;
			nVerts = 0;
			pTri = g_pTris;
			pVerts = g_pVerts;
		}
	}


	if(nTris > 0)
	{
		pDrawPrimLT->DrawPrim(g_pTris, nTris);
	}

	pDrawPrimLT->EndDrawPrim();

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPolyTubeFX::CalcCurWidth
//
//  PURPOSE:	Gets the current width of the trail depending on style
//
// ----------------------------------------------------------------------- //

LTFLOAT CPolyTubeFX::CalcCurWidth( )
{
	switch( GetProps()->m_eWidthStyle )
	{
		case ePTWS_Constant:
		{
			return GetProps()->m_fTrailWidth;
		}
		break;

		case ePTWS_SmallToBig:
		{
			return GetProps()->m_fTrailWidth * ( LTFLOAT(m_collPathPts.GetSize() - 2) / LTFLOAT(GetProps()->m_nMaxTrailLength) );
		}
		break;

		case ePTWS_SmallToSmall:
		{
			LTFLOAT fHalfPts = (GetProps()->m_nMaxTrailLength+1) * 0.5f;

			if( m_collPathPts.GetSize() < fHalfPts )
			{
				return GetProps()->m_fTrailWidth * ( LTFLOAT(m_collPathPts.GetSize() - 2) / LTFLOAT(GetProps()->m_nMaxTrailLength) );
			}
			else
			{
				return GetProps()->m_fTrailWidth * ( LTFLOAT(GetProps()->m_nMaxTrailLength - (m_collPathPts.GetSize() - 2 )) / LTFLOAT(GetProps()->m_nMaxTrailLength) );
			}
		}
		break;

		case ePTWS_BigToSmall:
		{
			return GetProps()->m_fTrailWidth * ( LTFLOAT(GetProps()->m_nMaxTrailLength - (m_collPathPts.GetSize() - 2 )) / LTFLOAT(GetProps()->m_nMaxTrailLength) );
		}
		break;

		default:
			return 0;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnRendererShutdown()
//
//   PURPOSE  : Handles app focus issues.....
//
//------------------------------------------------------------------

void CPolyTubeFX::OnRendererShutdown()
{
	if (m_hTexture)
	{
		m_pLTClient->GetTexInterface()->ReleaseTextureHandle(m_hTexture);
		m_hTexture = LTNULL;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetPolyTubeProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetPolyTubeProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;
	
	// Add the base props

	AddBaseProps(pList);

	// Add the class props

	fxProp.Path("Texture", "dtx|...");
	pList->AddTail(fxProp);

	fxProp.Float("TrailWidth", 20.0f);
	pList->AddTail(fxProp);

	fxProp.Combo("WidthStyle", "0,Constant,SmallToBig,SmallToSmall,BigToSmall" );
	pList->AddTail( fxProp );
	
	fxProp.Int("TrailLen", 50);
	pList->AddTail(fxProp);

	fxProp.Float("SectionLifespan", 1.0f);
	pList->AddTail(fxProp);

	fxProp.Float("UAdd", 0.0f);
	pList->AddTail(fxProp);

	fxProp.Float("SectionInterval", 0.1f);
	pList->AddTail(fxProp);

	fxProp.Combo("BlendMode", "0,None,Add,Saturate,ModSrcAlpha,ModSrcColor,ModDstColor,MulSrcColDstCol,MulSrcAlphaOne,MulSrcAlpha,MulSrcColOne,MulDstColZero" );
	pList->AddTail( fxProp );

	fxProp.Combo( "AlphaTest", "0,None,Less,LessOrEqual,Greater,GreaterOrEqual,Equal,NotEqual" );
	pList->AddTail( fxProp );

	fxProp.Combo( "ColorOp", "0,NoTexture,Modulate,Additive,NoColor" );
	pList->AddTail( fxProp );

	fxProp.Combo( "FillMode", "1,WireFrame,Fill" );
	pList->AddTail( fxProp );

	fxProp.Combo( "Allignment", "0,Camera,Up,Right,Forward" );
	pList->AddTail( fxProp );
}
