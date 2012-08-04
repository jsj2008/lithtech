#include "stdafx.h"
#include "ShatterEffect.h"
#include "iltcustomrender.h"
#include "ShatterTypeDB.h"
#include <float.h>
#include "iperformancemonitor.h"

static CTimedSystem g_tsClientShatter("GameClient_Shatter", "GameClient");

//----------------------------------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------------------------------

//the maximum number of vertices that a piece of debris can have. This is to prevent any incredibly
//detailed pieces of debris
#define MAX_DEBRIS_VERTICES		16

//----------------------------------------------------------------------------------------------------
// CWorkingPolyStack
//
//implements the utility polygon stack class which is used to push and pop the current working stack
//of polygons that need to be divided. Each polygon can have an additional 32 bits associated which
//is used by each algorithm for optimizing the subdivision process. This is built for speed over
//robustness, so please read each function comment carefully
//----------------------------------------------------------------------------------------------------
class CWorkingPolyStack
{
public:

	//what a working polygon looks like in memory
	struct SWorkingPoly
	{
		//used internally when popping the stack
		uint32			m_nPrevPolyStart;

		//the user code
		uint32			m_nCode;

		//the number of vertices
		uint32			m_nNumVerts;

		//the actual vertices (uses over allocation strategy)
		SDebrisVert		m_Verts[1];
	};

	//an invalid polygon offset indicating that we have reached the bottom of the stack
	static const uint32 knInvalidPolyOffset = 0xFFFFFFFF;

	CWorkingPolyStack() :
		m_nTopPolyOffset(knInvalidPolyOffset)
	{
		m_MemBuffer.reserve(2 * 1024);
	}

	//called to push a polygon onto the stack. Note that the pushed polygons must have at least
	//three vertices to prevent invalid polygons from getting placed on the stack
	void PushPoly(uint32 nCode, uint32 nNumVerts, SDebrisVert* pVerts)
	{
		//don't allow invalid polygons to be pushed on the stack
		if(nNumVerts < 3)
			return;

		//the end of the list will be our new polygon's start
		uint32 nNewPolyTop = m_MemBuffer.size();

		//make room for this new polygon
		m_MemBuffer.resize(nNewPolyTop + GetWorkingPolySize(nNumVerts));

		//setup our new polygon
		SWorkingPoly* pNewPoly = (SWorkingPoly*)&m_MemBuffer[nNewPolyTop];
		pNewPoly->m_nPrevPolyStart	= m_nTopPolyOffset;
		pNewPoly->m_nCode			= nCode;
		pNewPoly->m_nNumVerts		= nNumVerts;
		memcpy(pNewPoly->m_Verts, pVerts, sizeof(SDebrisVert) * nNumVerts);

		//and now update our stack to point at the top element
		m_nTopPolyOffset = nNewPolyTop;
	}

	//called to get the top polygon on the stack. This assumes that the stack is not empty
	const SWorkingPoly* GetTopPoly() const
	{
		LTASSERT(!IsEmpty(), "Error: Called GetTopPoly on an empty stack");
		return (SWorkingPoly*)&m_MemBuffer[m_nTopPolyOffset];
	}

	//called to pop the top polygon off of the stack. This assumes that the stack is not empty
	void PopPoly()
	{
		//determine the polygon we will be moving to next
		uint32 nNewPolyTop = GetTopPoly()->m_nPrevPolyStart;

		//shrink our buffer to no longer include the top polygon
		m_MemBuffer.resize(m_nTopPolyOffset);

		//and update our internal stack head
		m_nTopPolyOffset = nNewPolyTop;		
		
	}

	//called to determine if the stack is empty or not
	bool IsEmpty() const
	{
		return m_nTopPolyOffset == knInvalidPolyOffset;
	}

private:

	//utility function that determines the size of a working poly with the specified number of vertices
	static uint32 GetWorkingPolySize(uint32 nNumVerts)
	{
		return sizeof(SWorkingPoly) + sizeof(SDebrisVert) * (nNumVerts - 1);
	}

	//this vector is the memory buffer for the poly stack
	Tuint8List		m_MemBuffer;

	//index into the buffer for the current top polygon
	uint32			m_nTopPolyOffset;
};

//----------------------------------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------------------------------

//called to get a unit random value
static float GetUnitRandom()
{
	return rand() / (float)RAND_MAX;
}

//called to get a random within a specified range
static float GetRangedRandom(float fMin, float fMax)
{
	return fMin + GetUnitRandom() * (fMax - fMin);
}

//this utility function allows for reading from a block of memory like a file. This will
//read the specified object and advance the read pointer forward by the size. Note that
//this isn't for full objects, just core types
template<class T>
const T& MemRead(const uint8*& pReadPtr)
{
	const uint8* pSrcPtr = pReadPtr;
	pReadPtr += sizeof(T);
	return *((const T*)pSrcPtr);
}

//given a working polygon and some additional data, this will create a debris piece from
//the polygon
static SDebrisPiece* ConvertWorkingPolyToDebris(uint32 nNumVerts, const SDebrisVert* pVerts,
												const LTRigidTransform& tCentroid, 
												const LTRigidTransform& tObjTransform,
												float fBinormalScale)
{
	//allocate the new debris piece with room for the vertices
	uint8* pNewPieceMem = new uint8 [SDebrisPiece::GetPieceSize(nNumVerts)];
	if(!pNewPieceMem)
		return NULL;

	//overlay the debris on top of that memory
	SDebrisPiece* pNewPiece = (SDebrisPiece*)pNewPieceMem;

	//copy over all the data that we have
	pNewPiece->m_nNumVerts	= nNumVerts;
	pNewPiece->m_tTransform = tCentroid;

	//clear out the linear velocity
	pNewPiece->m_fUpdateDelayS = 0.0f;
	pNewPiece->m_vLinearVel.Init();

	//store the binormal scale
	pNewPiece->m_fBinormalScale	= fBinormalScale;

	//we now need to tranform all of these vertices into the space of the debris, since it is in the
	//space of the world model, we can just get the inverse of the difference (the opposite difference)
	//and apply that
	LTRigidTransform tInvTransform = tCentroid.GetDifference(tObjTransform);


	//now we need to move our vertices into the space of the debris transform (a simple translation
	//since the rotation was kept from the parent), and also build up the radius
	float fMaxRadSqr = 0.0f;
	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		pNewPiece->m_Verts[nCurrVert].m_vPos = tInvTransform * pVerts[nCurrVert].m_vPos;
		pNewPiece->m_Verts[nCurrVert].m_vUV  = pVerts[nCurrVert].m_vUV;
		fMaxRadSqr = LTMAX(pNewPiece->m_Verts[nCurrVert].m_vPos.MagSqr(), fMaxRadSqr);
	}

	pNewPiece->m_fRadius = LTSqrt(fMaxRadSqr);

	return pNewPiece;
}

//given a listing of debris vertices, this will compute the positional centroid of the polygon
static LTVector CalculateCentroid(uint32 nNumVerts, const SDebrisVert* pVerts)
{
	LTVector vCentroid(LTVector::GetIdentity());
	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		vCentroid += pVerts[nCurrVert].m_vPos;
	}
	return vCentroid / (float)nNumVerts;
}

//utility funciton that given a tangent space, will generate a rotation that aligns to the normal
//and tangent, and provides a scale for the binormal to handle mirrored spaces. Note that this will
//operate properly with non-orthogonal tangent spaces, but due to the representation, the result
//will be orthogonal
static void ConvertTangentSpaceToRotation(const LTVector& vNormal, const LTVector& vTangent, 
										  const LTVector& vBinormal, LTRotation& rRot,
										  float& fBinormalScale)
{
	//calculate our rotation (ensuring we use an orthogonal tangent space)
	LTVector vPerpBinormal	= vNormal.Cross(vTangent);
	LTVector vPerpTangent	= vPerpBinormal.Cross(vNormal);
	rRot = LTRotation(-vNormal, vPerpTangent.GetUnit());

	//and determine if the binormal is mirrored on this polygon
	fBinormalScale = (vPerpBinormal.Dot(vBinormal) >= 0.0f) ? 1.0f : -1.0f;			
}



//----------------------------------------------------------------------------------------------------
// CShatterEffect
//----------------------------------------------------------------------------------------------------
CShatterEffect::CShatterEffect() :
	m_nNumDebrisPieces(0),
	m_nTotalTris(0),
	m_hVertexDecl(NULL),
	m_hObject(NULL),
	m_fTotalElapsed(0.0f),
	m_hShatterType(NULL)
{
}

CShatterEffect::~CShatterEffect()
{
	Term();
}

//----------------------------------------------------------------------------------------------------
// Lifetime


//called to create a shatter effect given the provided data
bool CShatterEffect::Init(	const uint8* pWMData, const LTRigidTransform& tObjTransform, 
							const LTVector& vHitPos, const LTVector& vHitDir,
							HRECORD hShatterType)
{
	//clean up any existing data that we might have
	Term();

	//reset our elapsed time
	m_fTotalElapsed = 0.0f;

	//store our shatter type
	m_hShatterType = hShatterType;

	// Combine the direction we would like to face with our parents rotation...
	ObjectCreateStruct ocs;

	ocs.m_Flags		= FLAG_VISIBLE;
	ocs.m_Pos		= tObjTransform.m_vPos;
	ocs.m_Rotation	= tObjTransform.m_rRot;	

	//allow this object to be transparent if that is what the user wants
	if(!CShatterTypeDB::Instance().IsRenderSolid(m_hShatterType))
		ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	//create a custom render object with the associated material
	ocs.m_ObjectType = OT_CUSTOMRENDER;

	m_hObject = g_pLTClient->CreateObject( &ocs );
	if( !m_hObject )
	{
		Term();
		return false;
	}

	//setup the callback on the object so that it will render us
	g_pLTClient->GetCustomRender()->SetRenderingSpace(m_hObject, eRenderSpace_World);
	g_pLTClient->GetCustomRender()->SetRenderCallback(m_hObject, CustomRenderCallback);
	g_pLTClient->GetCustomRender()->SetCallbackUserData(m_hObject, this);

	//load up the material for this particle system, and assign it to the object
	HMATERIAL hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance(CShatterTypeDB::Instance().GetMaterialName(m_hShatterType));
	if(!hMaterial)
	{
		Term();
		return false;
	}

	g_pLTClient->GetCustomRender()->SetMaterial(m_hObject, hMaterial);
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	//create our vertex declaration
	SVertexDeclElement VertexDecl[] =
	{
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Position, 0 },
		{ 0, eVertexDeclDataType_PackedColor, eVertexDeclUsage_Color, 0 },
		{ 0, eVertexDeclDataType_Float2, eVertexDeclUsage_TexCoord, 0 },
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Normal, 0 },
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Tangent, 0 },
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Binormal, 0 }
	};

	g_pLTClient->GetCustomRender()->CreateVertexDeclaration(LTARRAYSIZE(VertexDecl), VertexDecl, m_hVertexDecl);
	if(!m_hVertexDecl)
	{
		Term();
		return false;
	}

	//now determine the shattering type and dispatch it to the appropriate function
	const char* pszShatterType = CShatterTypeDB::Instance().GetShatterMethod(m_hShatterType);

	if(LTStrIEquals(pszShatterType, "Glass"))
	{
		if(!ShatterGlass(pWMData, tObjTransform, vHitPos, vHitDir))
		{
			Term();
			return false;
		}
	}
	else if(LTStrIEquals(pszShatterType, "Tile"))
	{
		if(!ShatterTile(pWMData, tObjTransform, vHitPos, vHitDir))
		{
			Term();
			return false;
		}
	}
	else if(LTStrIEquals(pszShatterType, "Poly"))
	{
		if(!ShatterPoly(pWMData, tObjTransform, vHitPos, vHitDir))
		{
			Term();
			return false;
		}
	}
	else
	{
		LTERROR( "Error: Unexpected shattering type!");
		return false;
	}

	//success
	return true;
}

//called to free up this effect
void CShatterEffect::Term()
{
	FreeDebrisPieces();

	if (m_hObject) 
	{
		g_pLTClient->RemoveObject(m_hObject);
		m_hObject = NULL;
	}

	if(m_hVertexDecl)
	{
		g_pLTClient->GetCustomRender()->ReleaseVertexDeclaration(m_hVertexDecl);
		m_hVertexDecl = NULL;
	}	

	//free out other data
	m_hShatterType = NULL;
	m_fTotalElapsed = 0.0f;
}

//called to free all the currently allocated debris pieces
void CShatterEffect::FreeDebrisPieces()
{
	for(TDebrisList::iterator it = m_DebrisList.begin(); it != m_DebrisList.end(); it++)
	{
		delete [] (uint8*)*it;
	}
	m_DebrisList.empty();

	//and reset our counts so that they don't get out of sync
	m_nNumDebrisPieces = 0;
	m_nTotalTris = 0;
}

//----------------------------------------------------------------------------------------------------
// Shattering Common


//called to split the passed in polygon into two separate polygons, one for the front side,
//one for the back side
void CShatterEffect::SplitPoly(	const LTPlane& SplitPlane, uint32 nInVerts, const SDebrisVert* pInVerts,
								uint32& nFrontVerts, SDebrisVert* pFrontVerts, uint32 nMaxFrontVerts, 
								uint32& nBackVerts, SDebrisVert* pBackVerts, uint32 nMaxBackVerts)
{
	//sanity check!
	LTASSERT(nInVerts >= 3, "Error: Degenerate polygon passed into CShatterEffect::SplitPoly");

	//setup our output
	nFrontVerts = 0;
	nBackVerts  = 0;

	SDebrisVert Prev	= pInVerts[nInVerts - 1];
	float fPrevDist		= SplitPlane.DistTo(Prev.m_vPos);
	bool bPrevIn		= (fPrevDist >= 0.0f);

	//and now clip our list of read vertices and place the results in the write buffer
	for(uint32 nCurrVert = 0; nCurrVert < nInVerts; nCurrVert++)
	{
		//get the status of this edge
		const SDebrisVert& Curr	= pInVerts[nCurrVert];
		float fCurrDist			= SplitPlane.DistTo(Curr.m_vPos);
		bool bCurrIn			= (fCurrDist >= 0.0f);

		//check the four possible combinations
		if(bPrevIn && bCurrIn)
		{
			//Front: both are in, add the start
			if(nFrontVerts < nMaxFrontVerts)
			{
				pFrontVerts[nFrontVerts] = Prev;
				nFrontVerts++;
			}

			//Back: neither are in, don't add anything
		}
		else if(bPrevIn)
		{
			float fInterp = fPrevDist / (fPrevDist - fCurrDist);

			SDebrisVert Intersects;
			Intersects.m_vPos	= Prev.m_vPos + (Curr.m_vPos - Prev.m_vPos) * fInterp;
			Intersects.m_vUV	= Prev.m_vUV + (Curr.m_vUV - Prev.m_vUV) * fInterp;

			//Front: we are going out, add the start and intersection
			if(nFrontVerts < nMaxFrontVerts)
			{
				pFrontVerts[nFrontVerts] = Prev;
				nFrontVerts++;
			}

			if(nFrontVerts < nMaxFrontVerts)
			{
				pFrontVerts[nFrontVerts] = Intersects;
				nFrontVerts++;
			}

			//Back: we are coming back in, just add the intersection
			if(nBackVerts < nMaxBackVerts)
			{
				pBackVerts[nBackVerts] = Intersects;
				nBackVerts++;
			}
		}
		else if(bCurrIn)
		{
			float fInterp = fPrevDist / (fPrevDist - fCurrDist);

			SDebrisVert Intersects;
			Intersects.m_vPos	= Prev.m_vPos + (Curr.m_vPos - Prev.m_vPos) * fInterp;
			Intersects.m_vUV	= Prev.m_vUV + (Curr.m_vUV - Prev.m_vUV) * fInterp;

			//Front: we are coming back in, just add the intersection
			if(nFrontVerts < nMaxFrontVerts)
			{
				pFrontVerts[nFrontVerts] = Intersects;
				nFrontVerts++;
			}

			//Back: we are going out, add the start and intersection
			if(nBackVerts < nMaxBackVerts)
			{
				pBackVerts[nBackVerts] = Prev;
				nBackVerts++;
			}

			if(nBackVerts < nMaxBackVerts)
			{
				pBackVerts[nBackVerts] = Intersects;
				nBackVerts++;
			}
		}
		else
		{
			//Front: neither are in, don't add anything

			//Back: both are in, add the start
			if(nBackVerts < nMaxBackVerts)
			{
				pBackVerts[nBackVerts] = Prev;
				nBackVerts++;
			}
		}			

		//and update our previous state
		Prev		= Curr;
		fPrevDist	= fCurrDist;
		bPrevIn		= bCurrIn;
	}
}

//----------------------------------------------------------------------------------------------------
// Glass shattering

//utility function to generate radial planes that are randomly distributed within fixed cels with a 
//certain amount of padding along each cel so that the planes don't come too close together.
static void GlassGenerateRadialPlanes(const LTVector& vObjHitPos, uint32 nNumPlanes, LTPlane* pPlanes)
{
	//we will generate the plane around the forward of the object facing, meaning that we can create
	//the normal by blending between the right and up vectors, and since this is object space, that is simply
	//(1.0, 0.0, 0.0) for the right and (0.0, 1.0, 0.0) for the up (we take advantage of this to optimize
	//the normal calculation)

	//calculate the range for each plane's region, keeping a certain percentage as a buffer to prevent the
	//planes from becoming too close
	static const float kfAngleActualRange = 0.9f;
	float fAngRange = (MATH_PI / (float)nNumPlanes) * kfAngleActualRange;

	//and now actually create the radial planes, we want them to be randomized, but not too much, so randomize
	//each within an certain equal region across the +Y hemisphere
	for(uint32 nCurrPlane = 0; nCurrPlane < nNumPlanes; nCurrPlane++)
	{
		//determine the angular range
		float fAngMin = nCurrPlane * MATH_PI / nNumPlanes;

		//now determine the angle of the plane around the forward axis of the object
		float fAngle = fAngMin + GetUnitRandom() * fAngRange;

		//now generate the plane normal by blending between the axis
		LTVector vNormal(cosf(fAngle), sinf(fAngle), 0.0f);

		//and generate the actual plane so it passes through the point of impact
		pPlanes[nCurrPlane].Init(vNormal, vObjHitPos);
	}
}

//this function will take in an object space centroid and hit position, and from that project the points onto
//the vector to the centroid. It will return the vector to the centroid and also the projected range along that
//vector.
static void CalculateCentroidProjection(const LTVector& vObjHitPos, const LTVector& vCentroid, 
										const LTVector& vNormal, uint32 nNumVerts, const SDebrisVert* pVerts,
										LTVector& vToCentroid, float& fProjMin, float& fProjMax)
{
	//form a vector that goes to the centroid of the polygon from the impact position in the plane
	//of the polygon
	vToCentroid = vCentroid - vObjHitPos;

	//project it into the plane of the polygon and ensure it is of unit length
	vToCentroid -= vNormal * vNormal.Dot(vToCentroid);

	if(vToCentroid == LTVector::GetIdentity())
		vToCentroid.Init(0.0f, 1.0f, 0.0f);

	vToCentroid.Normalize();

	//we now need to determine the maximum projection along this axis
	fProjMin = pVerts[0].m_vPos.Dot(vToCentroid);
	fProjMax = fProjMin;

	for(uint32 nCurrVert = 1; nCurrVert < nNumVerts; nCurrVert++)
	{
		float fProjVal = pVerts[nCurrVert].m_vPos.Dot(vToCentroid);
		fProjMin = LTMIN(fProjMin, fProjVal);
		fProjMax = LTMAX(fProjMax, fProjVal);
	}
}

//given a polygon, this will determine if it contains any points within a cylinder of the specified radius
//that goes along the provided line in object space
static bool DoesPolyOverlapCylinder(const LTVector& vObjHitPos, const LTVector& vObjHitDir, float fRadiusSqr,
									uint32 nNumVerts, const SDebrisVert* pVerts)
{
	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		const LTVector& vVert = pVerts[nCurrVert].m_vPos;

		//find the vector perpendicular to the line of fire for this point
		LTVector vPerpImpact = vVert - vObjHitPos;
		vPerpImpact -= vObjHitDir * vObjHitDir.Dot(vPerpImpact);

		//and determine if it is within the radius
		if(vPerpImpact.MagSqr() <= fRadiusSqr)
			return true;
	}

	//no points inside
	return false;

}

//called to apply the glass shattering algorithm and create the debris
bool CShatterEffect::ShatterGlass(	const uint8* pWMData, const LTRigidTransform& tObjTransform, 
									const LTVector& vHitPos, const LTVector& vHitDir)
{
	//track performance
	CTimedSystemBlock TimeBlock(g_tsClientShatter);

	//free any existing data that we may have
	FreeDebrisPieces();

	//our current read position in the worldmodel data
	const uint8* pMemFile = pWMData;

	//prototype implementation. Just grab each polygon and make a piece of debris out of it
	uint32 nNumPolys = MemRead<uint32>(pMemFile);

	//------------------------
	// Generate radial planes

	//create all of the primary fracture planes (limit is 32 to allow the working poly stack 32 bit code to work)
	static const uint32 knMaxRadialPlanes = 32;
	LTPlane RadialPlanes[knMaxRadialPlanes];

	//determine the number of radial planes that we want to create
	uint32 nMinRadial = LTCLAMP(CShatterTypeDB::Instance().GetGlassMinRadialFractures(m_hShatterType), 0, knMaxRadialPlanes);
	uint32 nMaxRadial = LTCLAMP(CShatterTypeDB::Instance().GetGlassMaxRadialFractures(m_hShatterType), nMinRadial, knMaxRadialPlanes);
	uint32 nNumRadial = nMinRadial + (rand() % (nMaxRadial - nMinRadial + 1));

	//a bitmask that contains all of the radial plane bits set (used for detecting when a shard has been
	//fully radially subdivided)
	uint32 nFullPlaneMask = (1 << nNumRadial) - 1;

	//transform the hit information into object space
	LTRigidTransform tInvObjTransform = tObjTransform.GetInverse();
	LTVector vObjHitPos = tInvObjTransform * vHitPos;
	LTVector vObjHitDir = tInvObjTransform.m_rRot.RotateVector(vHitDir);

	//generate our shattering planes for the glass
	GlassGenerateRadialPlanes(vObjHitPos, nNumRadial, RadialPlanes);

	//cache splitting information for the debris pieces
	float fMinPieceLen = LTMAX(CShatterTypeDB::Instance().GetGlassMinDebrisLength(m_hShatterType), 1.0f);
	float fMaxPieceLen = LTMAX(CShatterTypeDB::Instance().GetGlassMaxDebrisLength(m_hShatterType), fMinPieceLen);
	float fPieceLenRange = fMaxPieceLen - fMinPieceLen;

	//calculate the immediate radius squared
	float fImmediateRadiusSqr = CShatterTypeDB::Instance().GetGlassImmediateRadius(m_hShatterType);
	fImmediateRadiusSqr *= fImmediateRadiusSqr;

	//cache other shatter information
	float fImmediateMinVelocity = CShatterTypeDB::Instance().GetGlassImmediateMinVelocity(m_hShatterType);
	float fImmediateMaxVelocity = CShatterTypeDB::Instance().GetGlassImmediateMaxVelocity(m_hShatterType);
	float fVelocityFalloff = CShatterTypeDB::Instance().GetGlassImmediateFalloff(m_hShatterType);
	float fGlassFallDelay = CShatterTypeDB::Instance().GetGlassFallDelay(m_hShatterType);
	float fGlassFallDistanceDelay = CShatterTypeDB::Instance().GetGlassFallDistanceDelay(m_hShatterType);
	float fIndirectMinVelocity = CShatterTypeDB::Instance().GetGlassIndirectMinVelocity(m_hShatterType);
	float fIndirectMaxVelocity = CShatterTypeDB::Instance().GetGlassIndirectMaxVelocity(m_hShatterType);

	//cache the angular velocity range for the pieces
	float fImmediateMinRotation = MATH_DEGREES_TO_RADIANS(CShatterTypeDB::Instance().GetImmediateMinRotation(m_hShatterType));
	float fImmediateMaxRotation = MATH_DEGREES_TO_RADIANS(CShatterTypeDB::Instance().GetImmediateMaxRotation(m_hShatterType));
	float fIndirectMinRotation = MATH_DEGREES_TO_RADIANS(CShatterTypeDB::Instance().GetIndirectMinRotation(m_hShatterType));
	float fIndirectMaxRotation = MATH_DEGREES_TO_RADIANS(CShatterTypeDB::Instance().GetIndirectMaxRotation(m_hShatterType));

	//-----------------------------
	// Subidivide each source poly

	//buffers to hold the output results of the splitting operation
	SDebrisVert SplitVerts[2][MAX_DEBRIS_VERTICES];

	//TEMP:JO Find a better means of managing this memory
	CWorkingPolyStack PolyStack;

	//now run through each of those polygons and create a piece of debris
	for(uint32 nCurrPoly = 0; nCurrPoly < nNumPolys; nCurrPoly++)
	{
		//read in the polygon parameters
		LTVector vNormal	= MemRead<LTVector>(pMemFile);
		LTVector vTangent	= MemRead<LTVector>(pMemFile);
		LTVector vBinormal	= MemRead<LTVector>(pMemFile);

		// We are getting an NaN vector from introduced into this system at some
		// point. Verify our inputs are good.
		LTASSERT(!LTIsNaN(vNormal), "CShatterEffect::ShatterGlass: Normal is NaN");
		LTASSERT(!LTIsNaN(vTangent), "CShatterEffect::ShatterGlass: Tangent is NaN");
		LTASSERT(!LTIsNaN(vBinormal), "CShatterEffect::ShatterGlass: Binormal is NaN");

		//and now the number of vertices
		uint32 nNumVerts = MemRead<uint32>(pMemFile);

		//we can now push the polygon onto our working stack directly from memory
		PolyStack.PushPoly(0, nNumVerts, (SDebrisVert*)pMemFile);

		//skip forward in the file over all of the vertices
		pMemFile += nNumVerts * sizeof(SDebrisVert);

		//determine the world space normal, and build up a rotation that we should use for all of the
		//created debris pieces
		float fBinormalScale;
		LTRotation rObjDebrisRot;
		ConvertTangentSpaceToRotation(vNormal, vTangent, vBinormal, rObjDebrisRot, fBinormalScale);

		LTRotation rDebrisRot = tObjTransform.m_rRot * rObjDebrisRot;

		//now we need to go into the recursive splitting mode
		while(!PolyStack.IsEmpty())
		{
			//we now need to subdivide our top polygon
			const CWorkingPolyStack::SWorkingPoly* pWorkPoly = PolyStack.GetTopPoly();
			uint32 nCurrPolyCode = pWorkPoly->m_nCode;

			//see if we need to perform any radial subdivision on this top polygon
			if(nCurrPolyCode != nFullPlaneMask)
			{
				//-----------------
				// Radial subdivision

				//we still need to perform radial subdivision, determine which plane to use
				for(uint32 nCurrPlane = 0; nCurrPlane < nNumRadial; nCurrPlane++)
				{
					const uint32 nPlaneCode = (1<<nCurrPlane);
					if(!(pWorkPoly->m_nCode & nPlaneCode))
					{
						uint32 nFrontVerts = 0;
						uint32 nBackVerts = 0;
                        
						//this is the plane that we need to subdivide with
						SplitPoly(RadialPlanes[nCurrPlane], pWorkPoly->m_nNumVerts, pWorkPoly->m_Verts, 
									nFrontVerts, SplitVerts[0], MAX_DEBRIS_VERTICES,
									nBackVerts, SplitVerts[1], MAX_DEBRIS_VERTICES);

						//now we can pop off our polygon and add the two new ones
						PolyStack.PopPoly();

						//also set our working poly to NULL. Much easier to catch NULL pointers than bad mem
						pWorkPoly = NULL;

						PolyStack.PushPoly(nCurrPolyCode | nPlaneCode, nFrontVerts, SplitVerts[0]);
						PolyStack.PushPoly(nCurrPolyCode | nPlaneCode, nBackVerts, SplitVerts[1]);

						//no need to test further planes
						break;
					}
				}
			}
			else
			{
				//-----------------
				// Shard subdivision

				//determine the centroid of this polygon which is needed to determine the primary axis
				//which is used to control how to fracture the shard
				LTVector vCentroid = CalculateCentroid(pWorkPoly->m_nNumVerts, pWorkPoly->m_Verts);

				//project our polygon along a vector from the point of impact to the centroid
				LTVector vToCentroid;
				float fProjMin, fProjMax;
				CalculateCentroidProjection(vObjHitPos, vCentroid, vNormal, pWorkPoly->m_nNumVerts, 
											pWorkPoly->m_Verts, vToCentroid, fProjMin, fProjMax);
				
				//determine a random length of which we will clip this piece to
				float fMaxLength = GetUnitRandom() * fPieceLenRange + fMinPieceLen;
				float fProjDist = fProjMax - fProjMin;

				if(fProjDist > fMaxLength)
				{
					//this piece is too long, we need to split it along a plane that is perpendicular to
					//the direction to the centroid. Pick a distance along that value to make the plane from
					//(we want it to be about 25-75% of the polygon)
					float fPlaneDist = GetUnitRandom() * fProjDist * 0.5f + fProjMin + fProjDist * 0.25f;

					LTPlane SplitPlane(vToCentroid, fPlaneDist);

					//and now split our polygon
					uint32 nFrontVerts = 0;
					uint32 nBackVerts = 0;

					//this is the plane that we need to subdivide with
					SplitPoly(	SplitPlane, pWorkPoly->m_nNumVerts, pWorkPoly->m_Verts, 
								nFrontVerts, SplitVerts[0], MAX_DEBRIS_VERTICES,
								nBackVerts, SplitVerts[1], MAX_DEBRIS_VERTICES);

					//now we can pop off our polygon and add the two new ones
					PolyStack.PopPoly();

					//also set our working poly to NULL. Much easier to catch NULL pointers than bad mem
					pWorkPoly = NULL;

					PolyStack.PushPoly(nCurrPolyCode, nFrontVerts, SplitVerts[0]);
					PolyStack.PushPoly(nCurrPolyCode, nBackVerts, SplitVerts[1]);
				}
				else
				{
					//this polygon is small enough to just be used as is
					LTRigidTransform tCentroid(tObjTransform * vCentroid, rDebrisRot);

					SDebrisPiece* pNewPiece = ConvertWorkingPolyToDebris(pWorkPoly->m_nNumVerts, pWorkPoly->m_Verts, tCentroid, tObjTransform, fBinormalScale);

					if(!pNewPiece)
						return false;

					//determine the distance of the centroid from the line of force of the bullet
					LTVector vPerpImpact = tCentroid.m_vPos - vHitPos;
					vPerpImpact -= vHitDir * vHitDir.Dot(vPerpImpact);

					//and now apply a velocity based upon that distance
					float fCentroidDist = vPerpImpact.Mag();

					if(DoesPolyOverlapCylinder(vObjHitPos, vObjHitDir, fImmediateRadiusSqr, pWorkPoly->m_nNumVerts, pWorkPoly->m_Verts))
					{
						//scale the distance accordingly
						float fScaledDist = fCentroidDist * fVelocityFalloff;

						//determine how much force should be applied
						float fForce = fImmediateMinVelocity + GetUnitRandom() * (fImmediateMaxVelocity - fImmediateMinVelocity);

						pNewPiece->m_vLinearVel = vHitDir * fForce / LTMAX(fScaledDist, 1.0f);

						//determine the angular velocity
						pNewPiece->m_vAngularVel.Init(	GetRangedRandom(fImmediateMinRotation, fImmediateMaxRotation),
														GetRangedRandom(fImmediateMinRotation, fImmediateMaxRotation),
														GetRangedRandom(fImmediateMinRotation, fImmediateMaxRotation));
					}
					else
					{
						//delay this piece from updating for the specified amount of time
						pNewPiece->m_fUpdateDelayS = fGlassFallDelay + fCentroidDist * fGlassFallDistanceDelay;

						//determine a random velocity for this piece that will take effect after the
						//delay has expired
						float fForce = fIndirectMinVelocity + GetUnitRandom() * (fIndirectMaxVelocity - fIndirectMinVelocity);

						pNewPiece->m_vLinearVel = vHitDir * fForce;

						//determine the angular velocity
						pNewPiece->m_vAngularVel.Init(	GetRangedRandom(fIndirectMinRotation, fIndirectMaxRotation),
														GetRangedRandom(fIndirectMinRotation, fIndirectMaxRotation),
														GetRangedRandom(fIndirectMinRotation, fIndirectMaxRotation));
					}										

					m_DebrisList.push_back(pNewPiece);

					//and update our counts
					m_nNumDebrisPieces++;
					m_nTotalTris += pNewPiece->m_nNumVerts - 2;

					//now we can pop off our polygon off of the stack
					PolyStack.PopPoly();

					//also set our working poly to NULL. Much easier to catch NULL pointers than bad mem
					pWorkPoly = NULL;
				}
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------------------------------------
// Tile shattering

//given the range of split planes on each axis, this will pack it into a working code. The bottom
//value is inclusive
static uint32 TileGenerateWorkingCode(uint32 nUMin, uint32 nUMax, uint32 nVMin, uint32 nVMax)
{
	LTASSERT((nUMin <= 0xFF) && (nUMax <= 0xFF) && (nVMin <= 0xFF) && (nVMax <= 0xFF), "Error: Found plane index that could not be packed into a working code for the tile shattering");
	return (nUMin << 24) | (nUMax << 16) | (nVMin << 8) | nVMax;
}

//given a working code, this will get the plane ranges for both axis
static void TileGetURangeFromWorkingCode(uint32 nWorkCode, uint32& nUMin, uint32& nUMax, uint32& nVMin, uint32& nVMax)
{
	nUMin = (nWorkCode >> 24) & 0xFF;
	nUMax = (nWorkCode >> 16) & 0xFF;
	nVMin = (nWorkCode >> 8) & 0xFF;
	nVMax = (nWorkCode >> 0) & 0xFF;
}

//given an axis range as well as an offset and subdivision ratio, this will determine the offset of the first
//plane that should be within the range
static float TileGetFirstOffset(float fRangeMin, float fOffset, float fSubDivLen)
{
	float fOffsetMin = fRangeMin - fOffset;

	if(fOffsetMin < 0.0f)
	{
		return fRangeMin + fmodf(-fOffsetMin, fSubDivLen);
	}
	else
	{
		return fRangeMin + (fSubDivLen - fmodf(fOffsetMin, fSubDivLen));
	}
}

//called to apply the tile shattering algorithm and create the debris
bool CShatterEffect::ShatterTile(	const uint8* pWMData, const LTRigidTransform& tObjTransform, 
									const LTVector& vHitPos, const LTVector& vHitDir)
{
	//track performance
	CTimedSystemBlock TimeBlock(g_tsClientShatter);

	//free any existing data that we may have
	FreeDebrisPieces();

	//our current read position in the worldmodel data
	const uint8* pMemFile = pWMData;

	//prototype implementation. Just grab each polygon and make a piece of debris out of it
	uint32 nNumPolys = MemRead<uint32>(pMemFile);

	//buffers to hold the output results of the splitting operation
	SDebrisVert SplitVerts[2][MAX_DEBRIS_VERTICES];

	//TEMP:JO Find a better means of managing this memory
	CWorkingPolyStack PolyStack;

	//transform the hit information into object space
	LTRigidTransform tInvObjTransform = tObjTransform.GetInverse();
	LTVector vObjHitPos = tInvObjTransform * vHitPos;
	LTVector vObjHitDir = tInvObjTransform.m_rRot.RotateVector(vHitDir);

	//calculate the immediate radius squared
	float fImmediateRadiusSqr = CShatterTypeDB::Instance().GetGlassImmediateRadius(m_hShatterType);
	fImmediateRadiusSqr *= fImmediateRadiusSqr;

	//cache other shatter information
	float fImmediateMinVelocity = CShatterTypeDB::Instance().GetGlassImmediateMinVelocity(m_hShatterType);
	float fImmediateMaxVelocity = CShatterTypeDB::Instance().GetGlassImmediateMaxVelocity(m_hShatterType);
	float fVelocityFalloff = CShatterTypeDB::Instance().GetGlassImmediateFalloff(m_hShatterType);
	float fGlassFallDelay = CShatterTypeDB::Instance().GetGlassFallDelay(m_hShatterType);
	float fGlassFallDistanceDelay = CShatterTypeDB::Instance().GetGlassFallDistanceDelay(m_hShatterType);
	float fIndirectMinVelocity = CShatterTypeDB::Instance().GetGlassIndirectMinVelocity(m_hShatterType);
	float fIndirectMaxVelocity = CShatterTypeDB::Instance().GetGlassIndirectMaxVelocity(m_hShatterType);

	//cache certain subdivision properties
	float fTileWidth   = CShatterTypeDB::Instance().GetTileWidth(m_hShatterType);
	float fTileHeight  = CShatterTypeDB::Instance().GetTileHeight(m_hShatterType);
	float fTileUOffset = CShatterTypeDB::Instance().GetTileUOffset(m_hShatterType);
	float fTileVOffset = CShatterTypeDB::Instance().GetTileVOffset(m_hShatterType);

	//cache the angular velocity range for the pieces
	float fImmediateMinRotation = MATH_DEGREES_TO_RADIANS(CShatterTypeDB::Instance().GetImmediateMinRotation(m_hShatterType));
	float fImmediateMaxRotation = MATH_DEGREES_TO_RADIANS(CShatterTypeDB::Instance().GetImmediateMaxRotation(m_hShatterType));
	float fIndirectMinRotation = MATH_DEGREES_TO_RADIANS(CShatterTypeDB::Instance().GetIndirectMinRotation(m_hShatterType));
	float fIndirectMaxRotation = MATH_DEGREES_TO_RADIANS(CShatterTypeDB::Instance().GetIndirectMaxRotation(m_hShatterType));


	//now for each polygon we need to run through, determine the splitting planes for both the P and
	//Q axis, and split the polygons appropriately
	for(uint32 nCurrPoly = 0; nCurrPoly < nNumPolys; nCurrPoly++)
	{
		//read in the polygon parameters
		LTVector vNormal	= MemRead<LTVector>(pMemFile);
		LTVector vTangent	= MemRead<LTVector>(pMemFile);
		LTVector vBinormal	= MemRead<LTVector>(pMemFile);

		//determine the world space normal, and build up a rotation that we should use for all of the
		//created debris pieces
		float fBinormalScale;
		LTRotation rObjDebrisRot;
		ConvertTangentSpaceToRotation(vNormal, vTangent, vBinormal, rObjDebrisRot, fBinormalScale);

		LTRotation rDebrisRot = tObjTransform.m_rRot * rObjDebrisRot;

		//and now the number of vertices
		uint32 nNumVerts = MemRead<uint32>(pMemFile);

		//determine the vertices that we will be working with
		SDebrisVert* pVertices = (SDebrisVert*)pMemFile;

		//we need to determine the extents across this face. There are four extents we care about:
		//one for the U extents across the poly and also the extents of the projection onto the tangent
		//one for the V extents across the poly and also the extents of the projection onto the binormal
		LTVector2 vURange(FLT_MAX, -FLT_MAX);
		LTVector2 vTangentRange(FLT_MAX, -FLT_MAX);
		LTVector2 vVRange(FLT_MAX, -FLT_MAX);
		LTVector2 vBinormalRange(FLT_MAX, -FLT_MAX);

		for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
		{
			const SDebrisVert& Vert = pVertices[nCurrVert];

			//handle updating the U and tangent extents
			if(Vert.m_vUV.x < vURange.x)
			{
				vURange.x = Vert.m_vUV.x;
				vTangentRange.x = vTangent.Dot(Vert.m_vPos);
			}
			if(Vert.m_vUV.x > vURange.y)
			{
				vURange.y = Vert.m_vUV.x;
				vTangentRange.y = vTangent.Dot(Vert.m_vPos);
			}

			//handle updating the V and binormal extents
			if(Vert.m_vUV.y < vVRange.x)
			{
				vVRange.x = Vert.m_vUV.y;
				vBinormalRange.x = vBinormal.Dot(Vert.m_vPos);
			}
			if(Vert.m_vUV.y > vVRange.y)
			{
				vVRange.y = Vert.m_vUV.y;
				vBinormalRange.y = vBinormal.Dot(Vert.m_vPos);
			}
		}

		//determine the first planes texture coordinate for each value
		float fFirstU = TileGetFirstOffset(vURange.x, fTileUOffset, fTileWidth);
		float fFirstV = TileGetFirstOffset(vVRange.x, fTileVOffset, fTileHeight);

		//the maximum number of planes we allow for subdividing on an axis
		static const uint32 knMaxAxisPlanes = 255;

		//we now need to determine the number of planes we will need to subdivide along for each axis
		uint32 nUPlanes = (uint32)LTCLAMP(((vURange.y - fFirstU) / fTileWidth) + 1.0f, 0.0f, (float)knMaxAxisPlanes);
		uint32 nVPlanes = (uint32)LTCLAMP(((vVRange.y - fFirstV) / fTileHeight) + 1.0f, 0.0f, (float)knMaxAxisPlanes);

		//we can now push the polygon onto our working stack directly from memory
		PolyStack.PushPoly(TileGenerateWorkingCode(0, nUPlanes, 0, nVPlanes), nNumVerts, pVertices);

		//and move past them in the file
		pMemFile += sizeof(SDebrisVert) * nNumVerts;

		//now the algorithm is actually quite simple. Just keep popping a poly, decide which plane to use
		//to split, and either split it, or if no plane is appropriate, create a debris piece
		while(!PolyStack.IsEmpty())
		{
			//we now need to subdivide our top polygon
			const CWorkingPolyStack::SWorkingPoly* pWorkPoly = PolyStack.GetTopPoly();

			//get the tile ranges for this working poly
			uint32 nUMin, nUMax, nVMin, nVMax;
			TileGetURangeFromWorkingCode(pWorkPoly->m_nCode, nUMin, nUMax, nVMin, nVMax);

			//see if there are any more planes to subdivide on the U axis
			if(nUMin < nUMax)
			{
				//find the texture coordinate of this first plane
				float fCoord = fFirstU + (float)nUMin * fTileWidth;

				//and find the projection onto the texture axis distance
				float fUniformCoord = (fCoord - vURange.x) / (vURange.y - vURange.x);
				float fProjDist = vTangentRange.x + fUniformCoord * (vTangentRange.y - vTangentRange.x);

				//and now build up the plane
				LTPlane SplitPlane(vTangent, -fProjDist);

				//and split the polygon
				uint32 nFrontVerts, nBackVerts;
				SplitPoly(	SplitPlane, pWorkPoly->m_nNumVerts, pWorkPoly->m_Verts, 
							nFrontVerts, SplitVerts[0], MAX_DEBRIS_VERTICES,
							nBackVerts, SplitVerts[1], MAX_DEBRIS_VERTICES);

				//now we can pop off our polygon and add the two new ones
				PolyStack.PopPoly();

				//also set our working poly to NULL. Much easier to catch NULL pointers than bad mem
				pWorkPoly = NULL;

				PolyStack.PushPoly(TileGenerateWorkingCode(nUMin, nUMin, nVMin, nVMax), nFrontVerts, SplitVerts[0]);
				PolyStack.PushPoly(TileGenerateWorkingCode(nUMin + 1, nUMax, nVMin, nVMax), nBackVerts, SplitVerts[1]);
			}
			//see if there are any more planes to subdivide on the V axis
			else if(nVMin < nVMax)
			{
				//find the texture coordinate of this first plane
				float fCoord = fFirstV + (float)nVMin * fTileHeight;

				//and find the projection onto the texture axis distance
				float fUniformCoord = (fCoord - vVRange.x) / (vVRange.y - vVRange.x);
				float fProjDist = vBinormalRange.x + fUniformCoord * (vBinormalRange.y - vBinormalRange.x);

				//and now build up the plane
				LTPlane SplitPlane(vBinormal, -fProjDist);

				//and split the polygon
				uint32 nFrontVerts, nBackVerts;
				SplitPoly(	SplitPlane, pWorkPoly->m_nNumVerts, pWorkPoly->m_Verts, 
							nFrontVerts, SplitVerts[0], MAX_DEBRIS_VERTICES,
							nBackVerts, SplitVerts[1], MAX_DEBRIS_VERTICES);

				//now we can pop off our polygon and add the two new ones
				PolyStack.PopPoly();

				//also set our working poly to NULL. Much easier to catch NULL pointers than bad mem
				pWorkPoly = NULL;

				PolyStack.PushPoly(TileGenerateWorkingCode(nUMin, nUMax, nVMin, nVMin), nFrontVerts, SplitVerts[0]);
				PolyStack.PushPoly(TileGenerateWorkingCode(nUMin, nUMax, nVMin + 1, nVMax), nBackVerts, SplitVerts[1]);
			}
			//otherwise if we can't split in either range, we need to create our piece of debris
			else
			{
				//determine the centroid for this piece of debris
				LTVector vCentroid = CalculateCentroid(pWorkPoly->m_nNumVerts, pWorkPoly->m_Verts);
				LTRigidTransform tCentroid(tObjTransform * vCentroid, rDebrisRot);

				SDebrisPiece* pNewPiece = ConvertWorkingPolyToDebris(pWorkPoly->m_nNumVerts, pWorkPoly->m_Verts, tCentroid, tObjTransform, fBinormalScale);

				if(!pNewPiece)
					return false;

				//determine the distance of the centroid from the line of force of the bullet
				LTVector vPerpImpact = tCentroid.m_vPos - vHitPos;
				vPerpImpact -= vHitDir * vHitDir.Dot(vPerpImpact);

				//and now apply a velocity based upon that distance
				float fCentroidDist = vPerpImpact.Mag();

				if(DoesPolyOverlapCylinder(vObjHitPos, vObjHitDir, fImmediateRadiusSqr, pWorkPoly->m_nNumVerts, pWorkPoly->m_Verts))
				{
					//scale the distance accordingly
					float fScaledDist = fCentroidDist * fVelocityFalloff;

					//determine how much force should be applied
					float fForce = fImmediateMinVelocity + GetUnitRandom() * (fImmediateMaxVelocity - fImmediateMinVelocity);

					pNewPiece->m_vLinearVel = vHitDir * fForce / LTMAX(fScaledDist, 1.0f);

					//determine the angular velocity
					pNewPiece->m_vAngularVel.Init(	GetRangedRandom(fImmediateMinRotation, fImmediateMaxRotation),
													GetRangedRandom(fImmediateMinRotation, fImmediateMaxRotation),
													GetRangedRandom(fImmediateMinRotation, fImmediateMaxRotation));
				}
				else
				{
					//delay this piece from updating for the specified amount of time
					pNewPiece->m_fUpdateDelayS = fGlassFallDelay + fCentroidDist * fGlassFallDistanceDelay;

					//determine a random velocity for this piece that will take effect after the
					//delay has expired
					float fForce = fIndirectMinVelocity + GetUnitRandom() * (fIndirectMaxVelocity - fIndirectMinVelocity);

					pNewPiece->m_vLinearVel = vHitDir * fForce;

					//TEMP:JO
					pNewPiece->m_fUpdateDelayS += GetUnitRandom() * 1.0f;

					//determine the angular velocity
					pNewPiece->m_vAngularVel.Init(	GetRangedRandom(fIndirectMinRotation, fIndirectMaxRotation),
													GetRangedRandom(fIndirectMinRotation, fIndirectMaxRotation),
													GetRangedRandom(fIndirectMinRotation, fIndirectMaxRotation));
				}

				m_DebrisList.push_back(pNewPiece);

				//and update our counts
				m_nNumDebrisPieces++;
				m_nTotalTris += pNewPiece->m_nNumVerts - 2;

				//now we can pop off our polygon off of the stack
				PolyStack.PopPoly();

				//also set our working poly to NULL. Much easier to catch NULL pointers than bad mem
				pWorkPoly = NULL;
			}
		}
	}

	return true;
}



//----------------------------------------------------------------------------------------------------
// Poly shattering

//called to shatter the world model data on a per polygon basis
bool CShatterEffect::ShatterPoly(	const uint8* pWMData, const LTRigidTransform& tObjTransform, 
									const LTVector& vHitPos, const LTVector& vHitDir)
{
	//track performance
	CTimedSystemBlock TimeBlock(g_tsClientShatter);

	//our current read position in the worldmodel data
	const uint8* pMemFile = pWMData;

	//prototype implementation. Just grab each polygon and make a piece of debris out of it
	uint32 nNumPolys = MemRead<uint32>(pMemFile);

	//free any existing data that we may have
	FreeDebrisPieces();

	//now run through each of those polygons and create a piece of debris
	for(uint32 nCurrPoly = 0; nCurrPoly < nNumPolys; nCurrPoly++)
	{
		//read in the polygon parameters
		LTVector vNormal	= MemRead<LTVector>(pMemFile);
		LTVector vTangent	= MemRead<LTVector>(pMemFile);
		LTVector vBinormal	= MemRead<LTVector>(pMemFile);

		//and now the number of vertices
		uint32 nNumVerts = MemRead<uint32>(pMemFile);

		//we are now at the vertices, so determine the centroid
		const SDebrisVert* pVertices = (const SDebrisVert*)pMemFile;

		//determine the world space normal, and build up a rotation that we should use for all of the
		//created debris pieces
		float fBinormalScale;
		LTRotation rObjDebrisRot;
		ConvertTangentSpaceToRotation(vNormal, vTangent, vBinormal, rObjDebrisRot, fBinormalScale);

		LTRotation rDebrisRot = tObjTransform.m_rRot * rObjDebrisRot;

		//setup our centroid transform
		LTRigidTransform tCentroid;
		tCentroid.m_vPos = tObjTransform * CalculateCentroid(nNumVerts, pVertices);
		tCentroid.m_rRot = rDebrisRot;

		//create our actual debris piece from all the data that we have collected
		SDebrisPiece* pNewPiece = ConvertWorkingPolyToDebris(nNumVerts, pVertices, tCentroid, tObjTransform, fBinormalScale);
		if(!pNewPiece)
			return false;

		//and now move on past our vertices in our memory file
		pMemFile += sizeof(SDebrisVert) * nNumVerts;

        //add this onto our list of debris
		m_DebrisList.push_back(pNewPiece);

		//and update our counts
		m_nNumDebrisPieces++;
		m_nTotalTris += pNewPiece->m_nNumVerts - 2;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------
// Updating

//given a piece of debris and an impact point, this will determine if the piece of debris should
//just be deactivated
static bool CanDeactivatePiece(const LTVector& vVelocity, const IntersectInfo& Info)
{
	//we can only deactivate if this hit the main world model
	if(!IsMainWorld(Info.m_hObject))
		return false;

	//and now look at the slope of the polygon we impacted
	//(note, this check for performance assumes gravity is along the -Y direction)
	if(Info.m_Plane.Normal().y <= 0.001f)
		return false;

	//we landed on geometry that we can potentially stick to, see if our velocity is small enough
	//to consider it a stick (note we don't consider -Y vel since that just moves us back to the surface)
	LTVector vClampedVel(vVelocity.x, LTMAX(vVelocity.y, 0.0f), vVelocity.z);

	//determine how fast we are moving
	static const float kfMinVelocity = 2.0f;
	if(vClampedVel.MagSqr() >= kfMinVelocity * kfMinVelocity)
		return false;

	//all of our tests passed, we can disable this piece
	return true;
}

//given the current orientation of a piece of debris, and the normal that was impacted, this will return
//the orientation that can be used to align the piece of debris to the surface
static LTRotation GetDebrisSurfaceAlignment(const LTRotation& rCurrRot, const LTVector& vPlaneNormal)
{
	//determine whether we should align to the positive or negative axis
	LTVector vForward = rCurrRot.Forward();
	LTVector vAlignNormal = (vForward.Dot(vPlaneNormal) > 0.0f) ? vPlaneNormal : -vPlaneNormal;

	//and align our piece of glass to the surface we just hit
	LTVector vCurrUp = rCurrRot.Up();
	LTVector vProjUp = vCurrUp - vCurrUp.Dot(vAlignNormal) * vAlignNormal;

	if(vProjUp.NearlyEquals(LTVector::GetIdentity(), 0.01f))
	{
		//too close to perpendicular, just build a random orthonormal
		vProjUp = vAlignNormal.BuildOrthonormal();
	}
	else
	{
		//we can use our normalized projected up vector
		vProjUp.Normalize();
	}

	return LTRotation(vAlignNormal, vProjUp);
}

//called to update this shatter effect by the specified time interval
bool CShatterEffect::Update(float fElapsedS)
{
	//track performance
	CTimedSystemBlock TimeBlock(g_tsClientShatter);

	//just bail immediately if we don't have any pieces of debris
	if(!m_nNumDebrisPieces)
		return false;

	//bail if we are paused
	if(fElapsedS == 0.0f)
		return true;

	//update our total elapsed time and see if we are done
	m_fTotalElapsed += fElapsedS;
	float fMaxLifetime = CShatterTypeDB::Instance().GetLifetime(m_hShatterType);

	if(m_fTotalElapsed >= fMaxLifetime)
	{
		//our time has expired, so report that to the owner so it can clean us up
		return false;
	}

	//also handle the updating of the alpha of the object
	float fStartFading = CShatterTypeDB::Instance().GetFadeoutStart(m_hShatterType);
	if(m_fTotalElapsed > fStartFading)
	{
		float fAlpha = 1.0f - (m_fTotalElapsed - fStartFading) / (fMaxLifetime - fStartFading);
		if(CShatterTypeDB::Instance().IsRenderSolid(m_hShatterType))
			g_pLTClient->SetObjectColor(m_hObject, fAlpha, fAlpha, fAlpha, 1.0f);
		else
			g_pLTClient->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, fAlpha);
	}

	//we need to run through each piece of debris and update it based upon the elapsed time, as well
	//as update the visibility information associated with this object
	LTVector vMin(FLT_MAX, FLT_MAX, FLT_MAX);
	LTVector vMax(-vMin);

	//determine the acceleration to apply onto all of the debris pieces for this time interval
	LTVector vAcceleration;
	g_pLTClient->Physics()->GetGlobalForce(vAcceleration);
	vAcceleration *= CShatterTypeDB::Instance().GetGravityScale(m_hShatterType);

	//cache the coefficient of restitution
	float fCOR = CShatterTypeDB::Instance().GetBounceStrength(m_hShatterType);

	//get the main world that we are going to test against
	HOBJECT hMainWorld = g_pLTClient->GetMainWorldModel();

	//structures we need for the intersection query
	IntersectQuery		iQuery;
	IntersectInfo		iInfo;

    for(uint32 nCurrDebris = 0; nCurrDebris < m_nNumDebrisPieces; nCurrDebris++)
	{
		//determine the actual piece of debris we will be working with
		SDebrisPiece* pDebris = m_DebrisList[nCurrDebris];

		//skip over this piece of debris if it isn't active yet
		if(m_fTotalElapsed <= pDebris->m_fUpdateDelayS)
		{
			//just update the visibility for this piece and keep going
			vMin.Min(pDebris->m_tTransform.m_vPos - LTVector(pDebris->m_fRadius, pDebris->m_fRadius, pDebris->m_fRadius));
			vMax.Max(pDebris->m_tTransform.m_vPos + LTVector(pDebris->m_fRadius, pDebris->m_fRadius, pDebris->m_fRadius));
			continue;
		}

		//scale the update time accordingly
		float fDebrisUpdateS = LTMIN(fElapsedS, m_fTotalElapsed - pDebris->m_fUpdateDelayS);

		//now we need to handle the updating of this piece of debris

		//handle updating position using second order derivatives (acceleration->velocity->position)
		pDebris->m_vLinearVel += vAcceleration * fDebrisUpdateS;
		LTVector vDestPos = pDebris->m_tTransform.m_vPos + pDebris->m_vLinearVel * fDebrisUpdateS;

		//handle updating the rotation of this piece
		LTRotation rRot(pDebris->m_vAngularVel.x * fDebrisUpdateS, pDebris->m_vAngularVel.y * fDebrisUpdateS, pDebris->m_vAngularVel.z * fDebrisUpdateS);
		pDebris->m_tTransform.m_rRot = pDebris->m_tTransform.m_rRot * rRot;

		//we now need to perform a ray cast to see if we can move the glass to the desired position
		iQuery.m_From	= pDebris->m_tTransform.m_vPos;
		iQuery.m_To		= vDestPos;

		if( g_pLTClient->IntersectSegmentAgainst( iQuery, &iInfo, hMainWorld ) )
		{
			const LTVector& vHitNormal = iInfo.m_Plane.Normal();

			//move to the point of impact (so it doesn't look like we bounced in mid-air)
			vDestPos = iInfo.m_Point + vHitNormal * 0.1f;

			//and dampen the linear velocity
			pDebris->m_vLinearVel *= fCOR;

			//align this piece of debris so it will rest on the surface
			pDebris->m_tTransform.m_rRot = GetDebrisSurfaceAlignment(pDebris->m_tTransform.m_rRot, vHitNormal);

			//handle disabling of debris pieces, before we bounce our velocity
			if(CanDeactivatePiece(pDebris->m_vLinearVel, iInfo))
			{
				//we can deactivate this piece for the rest of the simulation
				pDebris->m_fUpdateDelayS = fMaxLifetime;
			}
			else
			{
				//and now reflect our vector over the axis
				pDebris->m_vLinearVel = -vHitNormal * (2.0f * pDebris->m_vLinearVel.Dot(vHitNormal));
			}

		}		
		
		pDebris->m_tTransform.m_vPos = vDestPos;

		//and now update the visible box around this object
		vMin.Min(pDebris->m_tTransform.m_vPos - LTVector(pDebris->m_fRadius, pDebris->m_fRadius, pDebris->m_fRadius));
		vMax.Max(pDebris->m_tTransform.m_vPos + LTVector(pDebris->m_fRadius, pDebris->m_fRadius, pDebris->m_fRadius));
	}

	//and now setup this new visibility information on our object
	LTVector vObjPos;
	g_pLTClient->GetObjectPos(m_hObject, &vObjPos);

	g_pLTClient->GetCustomRender()->SetVisBoundingBox(m_hObject, vMin - vObjPos, vMax - vObjPos);

	return true;
}

//----------------------------------------------------------------------------------------------------
// Rendering

//hook for the custom render object, this will just call into the render function
void CShatterEffect::CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
{
	LTASSERT(pUser, "Error: Invalid user data provided to the custom render callback");
	((CShatterEffect*)pUser)->RenderShatterEffect(pInterface);
}

//function that handles the actual custom rendering
void CShatterEffect::RenderShatterEffect(ILTCustomRenderCallback* pInterface)
{
	//track performance
	CTimedSystemBlock TimeBlock(g_tsClientShatter);

	//the actual sprite vertex as seen by the device
	struct SRenderShatterVert
	{
		LTVector	m_vPos;
		uint32		m_nPackedColor;
		LTVector2	m_vUV;
		LTVector	m_vNormal;
		LTVector	m_vTangent;
		LTVector	m_vBinormal;
	};

	static const uint32 knRenderVertSize = sizeof(SRenderShatterVert);
	static const uint32 knRenderTriSize = sizeof(SRenderShatterVert) * 3;

	//setup our vertex declaration
	if(pInterface->SetVertexDeclaration(m_hVertexDecl) != LT_OK)
		return;

	//sanity check to ensure that we can at least render a sprite
	LTASSERT(DYNAMIC_RENDER_VERTEX_STREAM_SIZE >= knRenderTriSize, "Error: Dynamic vertex buffer size is too small to render a shatter effect");

	//determine the maximum number of triangles that we can calculate per batch
	uint32 nMaxTrisPerBatch = DYNAMIC_RENDER_VERTEX_STREAM_SIZE / knRenderTriSize;

	//keep track of the number of triangles that we have left to render
	uint32 nNumTrisLeftToRender = m_nTotalTris;

	//determine the color to use for this
	uint32 nColor = 0xFFFFFFFF;

	//and also keep track of how many more triangles we have left in the buffer, along with
	//how many are in the buffer currently
	uint32 nNumTrisLeftInBuffer = 0;
	uint32 nNumTrisInBuffer = 0;
	SRenderShatterVert* pDestBuffer = NULL;

	//the current output buffer that we will fill in with triangles
	SDynamicVertexBufferLockRequest LockRequest;

	//run through and render each of our debris pieces
	for(uint32 nCurrDebris = 0; nCurrDebris < m_nNumDebrisPieces; nCurrDebris++)
	{
		//determine the piece of debris that we will be working with
		const SDebrisPiece* pDebris = m_DebrisList[nCurrDebris];

		//cache the transform of the debris
		const LTRigidTransform& tTransform = pDebris->m_tTransform;

		//extract our vectors from the rotation
		LTMatrix3x4 mDebrisOr;
		tTransform.m_rRot.ConvertToMatrix(mDebrisOr);

		//now we need to determine what the tangent space is for this piece of debris
		LTVector vWSNormal, vWSTangent, vWSBinormal;
		mDebrisOr.GetBasisVectors(vWSBinormal, vWSTangent, vWSNormal);

		//and apply our scale to our binormal for mirred space
		vWSNormal *= pDebris->m_fBinormalScale;

		//now at this point we need to fan our debris triangles and render each triangle
		LTVector vFanPt		= tTransform * pDebris->m_Verts[0].m_vPos;
		LTVector2 vFanUV	= pDebris->m_Verts[0].m_vUV;
		LTVector vPrevPt	= tTransform * pDebris->m_Verts[1].m_vPos;
		LTVector2 vPrevUV	= pDebris->m_Verts[1].m_vUV;

		//and now fan and render our vertices as room permits
		for(uint32 nCurrPt = 2; nCurrPt < pDebris->m_nNumVerts; nCurrPt++)
		{
			//tranform the current position (we already have a matrix, so that is about 2x faster
			//than applying a transform multiply so use that)
			LTVector vCurrPt;
			mDebrisOr.Transform3x3(pDebris->m_Verts[nCurrPt].m_vPos, vCurrPt);
			vCurrPt += tTransform.m_vPos;

			LTVector2 vCurrUV = pDebris->m_Verts[nCurrPt].m_vUV;

			//and now make sure that we have room in the output buffer
			if(!nNumTrisLeftInBuffer)
			{
				//we need to create a new buffer to render into, flush our current buffer
				if(nNumTrisInBuffer > 0)
				{
					//flush our current tris
					pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
					pInterface->Render(	eCustomRenderPrimType_TriangleList, LockRequest.m_nStartIndex, nNumTrisInBuffer * 3);
					nNumTrisInBuffer = 0;
				}

				//and now lock down our new buffer to fill in
				uint32 nNumBatchTris = LTMIN(nNumTrisLeftToRender, nMaxTrisPerBatch);
				if(pInterface->LockDynamicVertexBuffer(nNumBatchTris * 3, LockRequest) != LT_OK)
					return;

				//and update all of our buffers to reflect the new data
				pDestBuffer = (SRenderShatterVert*)LockRequest.m_pData;
				nNumTrisLeftInBuffer = nNumBatchTris;
			}

			//we can now fill in our vertices for this triangle

			//fan vertex
			pDestBuffer->m_vPos			= vFanPt;
			pDestBuffer->m_nPackedColor	= nColor;
			pDestBuffer->m_vUV			= vFanUV;
			pDestBuffer->m_vNormal		= vWSNormal;
			pDestBuffer->m_vTangent		= vWSTangent;
			pDestBuffer->m_vBinormal	= vWSBinormal;
			pDestBuffer++;

			//previous position
			pDestBuffer->m_vPos			= vPrevPt;
			pDestBuffer->m_nPackedColor	= nColor;
			pDestBuffer->m_vUV			= vPrevUV;
			pDestBuffer->m_vNormal		= vWSNormal;
			pDestBuffer->m_vTangent		= vWSTangent;
			pDestBuffer->m_vBinormal	= vWSBinormal;
			pDestBuffer++;

			//current position
			pDestBuffer->m_vPos			= vCurrPt;
			pDestBuffer->m_nPackedColor	= nColor;
			pDestBuffer->m_vUV			= vCurrUV;
			pDestBuffer->m_vNormal		= vWSNormal;
			pDestBuffer->m_vTangent		= vWSTangent;
			pDestBuffer->m_vBinormal	= vWSBinormal;
			pDestBuffer++;

			//and update all of our counters
			nNumTrisLeftToRender--;
			nNumTrisLeftInBuffer--;
			nNumTrisInBuffer++;

			//and make our current vertex the previous one
			vPrevPt = vCurrPt;
			vPrevUV = vCurrUV;
		}
	}

	//flush anything that we have left in our buffer
	if(nNumTrisInBuffer > 0)
	{
		//flush our current tris
		pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
		pInterface->Render(	eCustomRenderPrimType_TriangleList, LockRequest.m_nStartIndex, nNumTrisInBuffer * 3);
		nNumTrisInBuffer = 0;
	}

	//at this point we should have 0 remaining tris exactly
	LTASSERT(nNumTrisLeftToRender == 0, "Error: Mismatch in the number of triangles to render and those rendered");
}


