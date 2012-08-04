//------------------------------------------------------------------
//
//   MODULE  : LIGHTNINGFX.CPP
//
//   PURPOSE : Implements class CLightningFX
//
//   CREATED : On 10/12/98 At 5:07:14 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "LightningFX.h"
#include "ClientFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLightningProps::CLightningProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CLightningProps::CLightningProps() :	
	m_eBlendMode			( DRAWPRIM_NOBLEND ),
	m_eAlphaTest			( DRAWPRIM_NOALPHATEST ),
	m_eColorOp				( DRAWPRIM_NOCOLOROP ),
	m_eFillMode				( DRAWPRIM_FILL ),
	m_eAllignment			( ePTA_Camera ),
	m_nMinNumBolts			( 1 ),
	m_nMaxNumBolts			( 5 ),
	m_nMinSegmentsPerBolt	( 10 ),
	m_nMaxSegmentsPerBolt	( 50 ),
	m_fMinBoltWidth			( 0.5f ),
	m_fMaxBoltWidth			( 3.0f ),
	m_fMinPerturb			( 0.0f ),
	m_fMaxPerturb			( 35.0f ),
	m_fMinLifetime			( 0.1f ),
	m_fMaxLifetime			( 3.0f ),
	m_fMinDelay				( 0.0f ),
	m_fMaxDelay				( 0.1f ),
	m_fPulse				( 0.0f ),
	m_fOmniDirectionalRadius( 0.0f )
{
	m_szNodeAttractors[0]	= '\0';
	m_szSocketAttractors[0]	= '\0';
	m_szTexture[0]			= '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLightningProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CLightningProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
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
			fxProp.GetPath( m_szTexture );
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
		else if( !_stricmp( fxProp.m_sName, "NodeAttractors" ))
		{
			fxProp.GetStringVal( m_szNodeAttractors );
		}
		else if( !_stricmp( fxProp.m_sName, "SocketAttractors" ))
		{
			fxProp.GetStringVal( m_szSocketAttractors );
		}
		else if( !_stricmp( fxProp.m_sName, "OmniDirectionalRadius" ))
		{
			m_fOmniDirectionalRadius = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MinNumBolts" ))
		{
			m_nMinNumBolts = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MaxNumBolts" ))
		{
			m_nMaxNumBolts = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MinSegmentsPerBolt" ))
		{
			m_nMinSegmentsPerBolt = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MaxSegmentsPerBolt" ))
		{
			m_nMaxSegmentsPerBolt = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MinBoltWidth" ))
		{
			m_fMinBoltWidth = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MaxBoltWidth" ))
		{
			m_fMaxBoltWidth = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MinPerturb" ))
		{
			m_fMinPerturb = (float)fabs( fxProp.GetFloatVal());
		}
		else if( !_stricmp( fxProp.m_sName, "MaxPerturb" ))
		{
			m_fMaxPerturb = (float)fabs( fxProp.GetFloatVal() );
		}
		else if( !_stricmp( fxProp.m_sName, "MinLifetime" ))
		{
			m_fMinLifetime = (float)fabs( fxProp.GetFloatVal() );
		}
		else if( !_stricmp( fxProp.m_sName, "MaxLifetime" ))
		{
			m_fMaxLifetime = (float)fabs( fxProp.GetFloatVal() );
		}
		else if( !_stricmp( fxProp.m_sName, "MinDelay" ))
		{
			m_fMinDelay = (float)fabs( fxProp.GetFloatVal() );
		}
		else if( !_stricmp( fxProp.m_sName, "MaxDelay" ))
		{
			m_fMaxDelay = (float)fabs( fxProp.GetFloatVal() );
		}
		else if( !_stricmp( fxProp.m_sName, "Pulse" ))
		{
			m_fPulse = fxProp.GetFloatVal();
		}
	}

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : CLightningFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CLightningFX::CLightningFX()
:	CBaseFX					( CBaseFX::eLightningFX ),
	m_hTexture				( LTNULL ),
	m_vTargetPos			( 0.0f, 0.0f, 0.0f ),
	m_hTarget				( LTNULL ),
	m_bReallyClose			( false ),
	m_fDelay				( 0.0f ),
	m_tmElapsedEmission		( 0.0f )
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CLightningFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CLightningFX::~CLightningFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CLightningFX
//
//------------------------------------------------------------------

bool CLightningFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	LTVector vSave = pBaseData->m_vPos;

	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps)) 
		return false;

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

	
	// Create the max number of bolts 

	CLightningBolt *pBolt = LTNULL;
	PT_TRAIL_SECTION ts;

	for( uint32 nBolts = 0; nBolts < GetProps()->m_nMaxNumBolts; ++nBolts )
	{
		pBolt = debug_new( CLightningBolt );

		pBolt->m_nNumSegments = GetRandom( (int)GetProps()->m_nMinSegmentsPerBolt, (int)GetProps()->m_nMaxSegmentsPerBolt );

		// Add all the trail sections now since we don't need to constantly create and delete them...

		for( uint32 nSegs = 0; nSegs < pBolt->m_nNumSegments; ++nSegs )
		{
			ts.m_vPos = m_vCreatePos;
			pBolt->m_collPathPts.AddTail( ts );
		}

		m_lstBolts.push_back( pBolt );
	}
	

	// Setup the target data so we now where the lightning is going...

	if( pBaseData->m_bUseTargetData )
	{
		if( pBaseData->m_hTarget )
		{
			m_hTarget = pBaseData->m_hTarget;
		}
		else if( m_hParent )
		{
			m_hTarget = m_hParent;
		}
		else
		{
			m_hTarget = LTNULL;
		}
		
		m_vTargetPos = pBaseData->m_vTargetPos;
	}
	else
	{
		// Use our parent as the target if we have one otherwise just use ourselves...
		
		m_hTarget = (m_hParent ? m_hParent : m_hObject);
		m_vTargetPos = m_vCreatePos;
	}

	// Load the texture if one was specified...
	
	if( !m_hTexture && GetProps()->m_szTexture[0] )
	{
		m_pLTClient->GetTexInterface()->CreateTextureFromName( m_hTexture, GetProps()->m_szTexture );
	}

	// Create a list of attractor nodes 

	if( m_hTarget )
	{
		ILTModel		*pModelLT = m_pLTClient->GetModelLT();
		ILTCommon		*pCommonLT = m_pLTClient->Common();
		HMODELNODE		hNode = -1;
		HMODELSOCKET	hSocket = -1;
		HATTRACTOR		hAttractor = INVALID_ATTRACTOR;
		CAttractor		cAttractor;

		// Add any nodes to our attractor list...

		if( GetProps()->m_szNodeAttractors[0] )
		{
			ConParse parse( GetProps()->m_szNodeAttractors );
			while( pCommonLT->Parse( &parse ) == LT_OK )
			{
				if( parse.m_nArgs > 0 && parse.m_Args[0] )
				{
					if( pModelLT->GetNode( m_hTarget, parse.m_Args[0], hAttractor ) == LT_OK )
					{
						cAttractor.m_hModel		= m_hTarget;
						cAttractor.m_hAttractor	= hAttractor;
						cAttractor.m_eType		= CAttractor::eNode;
					
						m_lstAttractors.push_back( cAttractor );
					}
				}
			}
		}

		// Add any sockets to our attractor list...

		if( GetProps()->m_szSocketAttractors[0] )
		{
			ConParse parse( GetProps()->m_szSocketAttractors );
			while( pCommonLT->Parse( &parse ) == LT_OK )
			{
				if( parse.m_nArgs > 0 && parse.m_Args[0] )
				{
					if( pModelLT->GetSocket( m_hTarget, parse.m_Args[0], hAttractor ) == LT_OK )
					{
						cAttractor.m_hModel		= m_hTarget;
						cAttractor.m_hAttractor = hAttractor;
						cAttractor.m_eType		= CAttractor::eSocket;
					
						m_lstAttractors.push_back( cAttractor );
					}
				}
			}
		}
	}

	m_tmElapsedEmission = 0.0f;
	m_fDelay = 0.0f;

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CLightningFX
//
//------------------------------------------------------------------

void CLightningFX::Term()
{
	if( m_hObject )
		m_pLTClient->RemoveObject(m_hObject);
	
	m_hObject = NULL;
	
	if( m_hTexture )
	{
		m_pLTClient->GetTexInterface()->ReleaseTextureHandle(m_hTexture);
		m_hTexture = LTNULL;
	}
		
	LightningBolts::iterator iter;
	for( iter = m_lstBolts.begin(); iter != m_lstBolts.end(); ++iter )
	{
		debug_delete( *iter );
	}

	m_lstBolts.clear();
}

//------------------------------------------------------------------
//
//   FUNCTION : EmitBolts()
//
//   PURPOSE  : Decides how many bolts to show and sets them up
//
//------------------------------------------------------------------

void CLightningFX::EmitBolts( float tmFrameTime )
{
	// Make sure enough time between emissions has passed...
	m_tmElapsedEmission += tmFrameTime;

	if( m_fDelay < m_tmElapsedEmission )
	{
		LTransform		lTrans;
		LTVector		vAttractorPos;
		ILTModel		*pModelLT = m_pLTClient->GetModelLT();

		uint32	nActiveBolts = GetRandom( (int)GetProps()->m_nMinNumBolts, (int)GetProps()->m_nMaxNumBolts );
		uint32	nBolt;

		bool	bCanUseAttractors = (m_lstAttractors.size() > 0);
		bool	bCanUseRadius = (GetProps()->m_fOmniDirectionalRadius >= 1.0f);

		CLightningBolt *pBolt = LTNULL;
		LightningBolts::iterator iter;

		for( nBolt = 0, iter = m_lstBolts.begin(); iter != m_lstBolts.end(), nBolt < nActiveBolts; ++iter, ++nBolt )
		{
			pBolt = *iter;
			
			pBolt->m_fWidth = GetRandom( GetProps()->m_fMinBoltWidth, GetProps()->m_fMaxBoltWidth );
			pBolt->m_fLifetime = GetRandom( GetProps()->m_fMinLifetime, GetProps()->m_fMaxLifetime );
			pBolt->m_tmElapsed = 0.0f;
			pBolt->m_bActive = true;
			
			// Grab the position of the object to compensate for offset
		
			if( m_hTarget )
			{
				m_pLTClient->GetObjectPos( m_hTarget, &vAttractorPos );
			}
			else
			{
				vAttractorPos = m_vTargetPos;
			}

			// Decide if we should use an attractor or radius for the end pos...
			
			if( bCanUseAttractors && (!bCanUseRadius || GetRandom(0,1)) )
			{
				uint8	nIndex = GetRandom( 0, m_lstAttractors.size() - 1 );
				CAttractor cAttractor = m_lstAttractors[nIndex];

				if( cAttractor.GetTransform( lTrans, true ) == LT_OK )
				{
					vAttractorPos = lTrans.m_Pos;
				}
			}	
			else if( bCanUseRadius )
			{
				LTVector vRandomPos;
				vRandomPos.x = GetRandom( -1.0f, 1.0f );
				vRandomPos.y = GetRandom( -1.0f, 1.0f );
				vRandomPos.z = GetRandom( -1.0f, 1.0f );
				
				vRandomPos.Normalize();
				vRandomPos *= GetRandom( -GetProps()->m_fOmniDirectionalRadius, GetProps()->m_fOmniDirectionalRadius );

				vAttractorPos = m_vPos + vRandomPos;

				IntersectQuery	iQuery;
				IntersectInfo	iInfo;

				iQuery.m_From	= m_vPos;
				iQuery.m_To		= vAttractorPos;

				if( m_pLTClient->IntersectSegment( &iQuery, &iInfo ))
				{
					vAttractorPos = iInfo.m_Point;
				}
			}

			
			LTVector vNew = m_vPos;
			LTVector vDir = vAttractorPos - vNew;
						
			float fStep = vDir.Length() / (float)pBolt->m_nNumSegments;
			float fPerturb = GetRandom( GetProps()->m_fMinPerturb, GetProps()->m_fMaxPerturb );
			
			vDir.Normalize();
			LTRotation rRot = LTRotation( vDir, LTVector( 0.0f, 1.0f, 0.0f ));
				
			CLinkListNode<PT_TRAIL_SECTION> *pNode = pBolt->m_collPathPts.GetHead();
			while( pNode )
			{
				pNode->m_Data.m_vPos = vNew;
				pNode->m_Data.m_tmElapsed = 0.0f;
				pNode->m_Data.m_vBisector.Init();
												
				// Add in some perturb going in the direction of the attractor pos for the next section...
				
				vNew +=	(rRot.Forward() * fStep );
				vNew += (rRot.Up() * GetRandom( -fPerturb, fPerturb ));
				vNew += (rRot.Right() * GetRandom( -fPerturb, fPerturb ));

				// Make sure the last section goes to the end pos...

				if( !pNode->m_pNext )
					pNode->m_Data.m_vPos = vAttractorPos;

				pNode = pNode->m_pNext;
			}
		}

		// Decide when the next emission will be...

		m_tmElapsedEmission = 0.0f;
		m_fDelay = GetRandom( GetProps()->m_fMinDelay, GetProps()->m_fMaxDelay );
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : PreRender()
//
//   PURPOSE  : Handle some precalculations of the bolts before they render
//
//------------------------------------------------------------------

void CLightningFX::PreRender( float tmFrameTime )
{
	
	LTVector vPulse;
	LTVector vF(0.0f, 0.0f, 1.0f);
	LTVector vU(0.0f, 1.0f, 0.0f);
	LTVector vR(1.0f, 0.0f, 0.0f);

	
	// Transform the bolt 

	LTMatrix mCam;
	if( m_bReallyClose )
	{
		mCam.Identity();
	}
	else
	{
		mCam = GetCamTransform(m_pLTClient, m_hCamera);
	}
	 
	
	CLightningBolt *pBolt = LTNULL;
	LightningBolts::iterator iter;
	for( iter = m_lstBolts.begin(); iter != m_lstBolts.end(); ++iter )
	{
		pBolt = *iter;

		// Skip this bolt if there are not enough segments...

		if( pBolt->m_collPathPts.GetSize() < 2 || !pBolt->m_bActive )
			continue;

		CLinkListNode<PT_TRAIL_SECTION> *pNode = pBolt->m_collPathPts.GetHead();

		//as long as some amount of time has passed, apply a pulse onto the bolt to make
		//it jitter
		if(tmFrameTime > 0.001f)
		{
			while (pNode)
			{
				vPulse = pNode->m_Data.m_vPos;
				vPulse += (vF * GetRandom( -GetProps()->m_fPulse, GetProps()->m_fPulse ));
				vPulse += (vU * GetRandom( -GetProps()->m_fPulse, GetProps()->m_fPulse ));
				vPulse += (vR * GetRandom( -GetProps()->m_fPulse, GetProps()->m_fPulse ));

				if( pNode == pBolt->m_collPathPts.GetHead() || !pNode->m_pNext )
				{
					MatVMul(&pNode->m_Data.m_vTran, &mCam, &pNode->m_Data.m_vPos);
				}
				else
				{
					MatVMul(&pNode->m_Data.m_vTran, &mCam, &vPulse);
				}

				pNode = pNode->m_pNext;
			}
		}

		// Do some precalculations

		float fScale;
		CalcScale( pBolt->m_tmElapsed, pBolt->m_fLifetime, &fScale );
		float fWidth = pBolt->m_fWidth * fScale;

		// Setup the colour
		
		float r, g, b, a;			
		CalcColour( pBolt->m_tmElapsed, pBolt->m_fLifetime, &r, &g, &b, &a );			

		int ir = Clamp( (int)(r * 255.0f), 0, 255 );
		int ig = Clamp( (int)(g * 255.0f), 0, 255 );
		int ib = Clamp( (int)(b * 255.0f), 0, 255 );
		int ia = Clamp( (int)(a * 255.0f), 0, 255 );

		LTVector vStart, vEnd, vPrev, vBisector;
		vBisector.z = 0.0f;

		pNode = pBolt->m_collPathPts.GetHead();

		while( pNode )
		{
			if( GetProps()->m_eAllignment == ePTA_Camera )
			{
				// Compute the midpoint vectors

				if( pNode == pBolt->m_collPathPts.GetHead() )
				{
					vStart = pNode->m_Data.m_vTran;
					vEnd   = pNode->m_pNext->m_Data.m_vTran;
					
					vBisector.x = vEnd.y - vStart.y;
					vBisector.y = -(vEnd.x - vStart.x);
				}
				else if( pNode == pBolt->m_collPathPts.GetTail() )
				{
					vEnd   = pNode->m_Data.m_vTran;
					vStart = pNode->m_pPrev->m_Data.m_vTran;
					
					vBisector.x = vEnd.y - vStart.y;
					vBisector.y = -(vEnd.x - vStart.x);
				}
				else
				{
					vPrev  = pNode->m_pPrev->m_Data.m_vTran;
					vStart = pNode->m_Data.m_vTran;
					vEnd   = pNode->m_pNext->m_Data.m_vTran;

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
			
			// Set the width for this section...

			pNode->m_Data.m_vBisector.Norm( fWidth );
			
			// Set the color for this section...

			pNode->m_Data.m_red = ir;
			pNode->m_Data.m_green = ig;
			pNode->m_Data.m_blue = ib;
			pNode->m_Data.m_alpha = ia;
		
			pNode = pNode->m_pNext;
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates the lightning
//
//------------------------------------------------------------------

bool CLightningFX::Update(float tmFrameTime)
{
	// Base class update first
	
	if (!CBaseFX::Update(tmFrameTime)) 
		return false;

	
	if( !IsShuttingDown() )
	{
		EmitBolts( tmFrameTime );		
	}

	// Check to see if any bolts have expired...

	CLightningBolt *pBolt = LTNULL;
	LightningBolts::iterator iter;
	for( iter = m_lstBolts.begin(); iter != m_lstBolts.end(); ++iter )
	{
		pBolt = *iter;

		//adjust our start time
		pBolt->m_tmElapsed += tmFrameTime;

		if( pBolt->m_fLifetime < pBolt->m_tmElapsed )
			pBolt->m_bActive = false;
	}

	// Setup the bolts for rendering

	PreRender( tmFrameTime );

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Render()
//
//   PURPOSE  : Renders the lightning using DrawPrim
//
//------------------------------------------------------------------

bool CLightningFX::Render()
{
	if(!CBaseFX::Render())
		return false;
	
	// Render the bolts...

	uint32 nTris = 0;
	uint32 nVerts = 0;

	LT_POLYGT3 *pTri = g_pTris;
	LTVector *pVerts = g_pVerts;


	ILTDrawPrim *pDrawPrimLT;
	pDrawPrimLT = m_pLTClient->GetDrawPrim();

	if(!pDrawPrimLT)
		return false;

	// Draw the polylist
	pDrawPrimLT->SetTexture( m_hTexture );
	pDrawPrimLT->SetReallyClose( m_bReallyClose );
	pDrawPrimLT->SetCamera( m_hCamera );
	pDrawPrimLT->SetTransformType( DRAWPRIM_TRANSFORM_CAMERA );
	pDrawPrimLT->SetZBufferMode( DRAWPRIM_ZRO );
	pDrawPrimLT->SetFillMode( GetProps()->m_eFillMode );
	pDrawPrimLT->SetAlphaTestMode( GetProps()->m_eAlphaTest );
	pDrawPrimLT->SetAlphaBlendMode( GetProps()->m_eBlendMode );
	pDrawPrimLT->SetColorOp( GetProps()->m_eColorOp );

	pDrawPrimLT->BeginDrawPrim();

	CLinkListNode<PT_TRAIL_SECTION> *pNode = LTNULL;
	CLightningBolt *pBolt = LTNULL;
	LightningBolts::iterator iter;
	
	for( iter = m_lstBolts.begin(); iter != m_lstBolts.end(); ++iter )
	{
		pBolt = *iter;

		if( !pBolt->m_bActive )
			continue;

		pNode = pBolt->m_collPathPts.GetHead();
	
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
			
			uint8 r2 = pNode->m_pNext->m_Data.m_red;
			uint8 g2 = pNode->m_pNext->m_Data.m_green;
			uint8 b2 = pNode->m_pNext->m_Data.m_blue;
			uint8 a2 = pNode->m_pNext->m_Data.m_alpha;
			

			SetupVert(pTri, 0, g_pVerts[nVerts], r1, g1, b1, a1, 0.0f, 0.0f);
			SetupVert(pTri, 1, g_pVerts[nVerts + 1], r2, g2, b2, a2, 1.0f, 0.0f);
			SetupVert(pTri, 2, g_pVerts[nVerts + 2], r2, g2, b2, a2, 1.0f, 1.0f);

			pTri ++;
			nTris ++;

			SetupVert(pTri, 0, g_pVerts[nVerts], r1, g1, b1, a1, 0.0f, 0.0f);
			SetupVert(pTri, 1, g_pVerts[nVerts + 2], r2, g2, b2, a2, 1.0f, 1.0f);
			SetupVert(pTri, 2, g_pVerts[nVerts + 3], r1, g1, b1, a1, 0.0f, 1.0f);

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
	}

	if(nTris > 0)
	{
		pDrawPrimLT->DrawPrim(g_pTris, nTris);
	}

	pDrawPrimLT->EndDrawPrim();

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : OnRendererShutdown()
//
//   PURPOSE  : Handles app focus issues.....
//
//------------------------------------------------------------------

void CLightningFX::OnRendererShutdown()
{
	if (m_hTexture)
	{
		m_pLTClient->GetTexInterface()->ReleaseTextureHandle(m_hTexture);
		m_hTexture = LTNULL;
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : fxGetLightningProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetLightningProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;
	
	// Add the base props

	AddBaseProps(pList);
	

	fxProp.Path("Texture", "dtx|...");
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

	fxProp.String( "NodeAttractors", "" );
	pList->AddTail( fxProp );

	fxProp.String( "SocketAttractors", "" );
	pList->AddTail( fxProp );

	fxProp.Float( "OmniDirectionalRadius", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Int( "MinNumBolts", 1 );
	pList->AddTail( fxProp );

	fxProp.Int( "MaxNumBolts", 5 );
	pList->AddTail( fxProp );

	fxProp.Int( "MinSegmentsPerBolt", 10 );
	pList->AddTail( fxProp );

	fxProp.Int( "MaxSegmentsPerBolt", 50 );
	pList->AddTail( fxProp );

	fxProp.Float( "MinBoltWidth", 0.5f );
	pList->AddTail(fxProp);

	fxProp.Float( "MaxBoltWidth", 3.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MinPerturb", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MaxPerturb", 35.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MinLifetime", 0.1f );
	pList->AddTail( fxProp );

	fxProp.Float( "MaxLifetime", 3.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MinDelay", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MaxDelay", 0.1f );
	pList->AddTail( fxProp );

	fxProp.Float( "Pulse", 5.0f );
	pList->AddTail( fxProp );
}

