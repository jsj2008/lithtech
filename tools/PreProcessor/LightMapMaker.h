//------------------------------------------------------------------
//
//	FILE	  : LightMapMaker.h
//
//	PURPOSE	  : Defines the CLightMapMaker class, which creates light
//				maps for all surfaces in a world.
//
//	CREATED	  : September 30 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __LIGHTMAP_MAKER_H__
#define __LIGHTMAP_MAKER_H__

// Includes....
#include "bdefs.h"
#include "preworld.h"
#include "classbind.h"

#include "lmpolytree.h"
#include "lightingbsp.h"

class CEditRegion;
class TLightDef;
class CFullLightMap;

#define POLYANIMREF_NONE	0xFFFFFFFF

// LMM-specific definition for a light
class CLightDef
{
public:

					CLightDef()
					{
						m_InnerColor.Init(0.0f, 0.0f, 0.0f);
						m_Pos.Init(0.0f, 0.0f, 0.0f);

						m_MaxDist				= m_MaxDistSqr = 0.0f;
						m_bClip					= true;
						m_bDirectional			= false;
						m_bLightGrid			= false;
						m_bLightWorld			= true;
						m_pObject				= NULL;
						m_A						= 1.0f;
						m_B						= 0.0f;
						m_C						= 0.0f;
						m_eAttenuation			= eAttenuation_Quartic;
						m_fSize					= 5.0f;
						m_bShadowMesh			= false;
						m_fObjectBrightScale	= 1.0f;

						//set up the field of view
						SetFOV(MATH_HALFPI);
					}

	PVector			m_Direction;
	PVector			m_Pos;

	PVector			m_InnerColor;	// Colors.
	
	PReal			m_A;			// light attenuation values
	PReal			m_B;
	PReal			m_C;

	enum EAttenuation {
		eAttenuation_Quartic, // (1-d^2/r^2)^2
		eAttenuation_Linear, // (1-d/r)
		eAttenuation_D3D // 1/(A+Bd+Cd^2)
	};
	EAttenuation	m_eAttenuation; // Use the old attenuation, or the new attenuation?

	PReal			m_MaxDist;
	PReal			m_MaxDistSqr;
	bool			m_bClip;		// Should this light clip against walls?
	bool			m_bDirectional;
	bool			m_bLightWorld;	//should this light the world
	bool			m_bLightGrid;	//this this light the lighting grid

	float			m_fObjectBrightScale;	//The amount to scale any object only lighting

	CBaseEditObj	*m_pObject;

	PReal			m_fSize;		// Size of the light source (for shadow meshing)
	bool			m_bShadowMesh;	// Cash shadow mesh shadows

	//sets the field of view of the directional light
	void			SetFOV(PReal FOV)	{ m_FOV = FOV; m_CosFOV = (PReal)cos(FOV * 0.5); }

	//gets the field of view of the directional light
	PReal			GetFOV() const		{ return m_FOV;}

	//gets the cosine of the FOV
	PReal			GetCosFOV() const	{ return m_CosFOV;}

private:

	PReal			m_FOV;			//FOV of the directional light
	PReal			m_CosFOV;		//the cosine of the FOV
};

// Used to put the LightmapMaker's worlds in a WorldTree.
class LMMWorld
{
public:
					LMMWorld();

	CPreWorld		*m_pWorld;
	
	bool			m_bUsingTransform;
	LTMatrix		m_mTransform;
	LTMatrix		m_mInverseTransform;
};

// Used to light generic points in space that aren't tied to any geometry
struct CLightingPoint
{
	enum	{ 
				LIGHT_USENORMAL			=	(1<<0),
				LIGHT_ONLYUSEAMBIENT	=	(1<<1)
			};

	uint32*  color;				// pointer to the color value to fill in
	LTVector pos;				// position of point to light
	LTVector normal;			// optional normal at point
	LTVector colorOffset;		// amount to offset resulting color (for noise, etc.)
	
	uint32	 lightingFlags;		// combination of the above flags used to determine lighting behavior
};

// The actual lightmap maker class
class CLightMapMaker
{
public:

	CLightMapMaker();
	~CLightMapMaker();

	bool				Init(CPreMainWorld *pWorld,                         
							 bool bClipLight);

	void				Term();


	//generates the lighting for the level as well as the grid for it based upon 
	//the table that is passed in. The Callee is responsible for freeing the 
	//associated memory
	bool				ProcessLight(const LightTable &Dims, PVector** pTable, const std::vector<CLightingPoint>& lightingPoints);

	// External access for calculating the lighting for points
	// Note that these functions are only available for classes/functions which have been 
	// spawned by this class.

	//calculates the lighting for a directional point in space
	PVector LightDirectionalPoint(	const PVector& vPos, const PVector& vNormal,
									bool bApplySunLight, bool bClipLight, CLightBSPStack* pStack) const;

	//this will determine if a light hits the sample point specified
	bool	DoesLightHit(const CLightDef* pLight, const PVector& vPos, bool bClipLight, CLightBSPStack* pStack) const;

protected:

	//generates a lighting grid based upon the table that is passed in. The Callee
	//is responsible for freeing the associated memory
	bool				CreateLightGrid(const LightTable &Dims, PVector** pTable, bool bAmbient);

	// light a collection of points that don't have associated geometry
	bool				LightLightingPoints( const std::vector<CLightingPoint>& lightingPoints );


	//----------------------------LIGHTMAP GENERATION----------------------------//

	//initializes the polygon's lightmap
	bool InitPolygonFullLightMap(CPrePoly* pPoly, CFullLightMap* pLightMap);

	//finishes a lightmap
	bool FinishLightMap(uint32 nPolyIndex, CFullLightMap* pLightMap);

	//uses super sampling to generate lightmap values
	bool ThreadLightPolySuperSample(uint32 nPolyIndex, uint32 nNumSamplePts,
										PVector* pSamplePts, CLightBSPStack* pStack);

	//runs through the light definitions and determines which lights effect this polygon.
	//returns the number of lights that touch the surface
	uint32 CalcLightsTouchingPoly(CPrePoly *pPoly, uint32 nPolyAnimIndex, CLightDef** pLightList, uint32 nListSize);

	//calculates the amount of shift that is necessary for the lightmap positions. This
	//allows for super sampling of lightmaps by a pixel or two, yet still have the positions
	//be correct
	PReal	GetShiftAmount(const CPrePoly* pPoly) const;

	// Calculate the edge planes for a poly. This is a list of edges from several polygons. The count
	//list tells how many polygons belong to each poly (so it would be like 3, 4, 6 it it contained 3 polys
	//with the appropriate number of edges.) Poly 0 is guaranteed to be the input polygon.
	typedef std::vector<LTPlane> TPlaneList;
	typedef std::vector<uint32> TPlaneCountList;

	uint32 AddPolyEdgesToList(const CPrePoly *pPoly, TPlaneList *pEdgePlanes);
	void CalcPolyEdgePlanes(const CPrePoly *pPoly, TPlaneList *pEdgePlanes, TPlaneCountList* pEdgeCounts);
	
	// Move a point into a poly defined by a set of edge planes
	static LTVector MovePointIntoPoly(const LTVector &vPos, const TPlaneList &cEdgePlanes, const TPlaneCountList &cEdgeCounts);

	//given a position, it will calculate the amount of light that strikes the position of a 
	//point on a polygon. This is intended for use on polygons (so the light list can
	//be cached across the surface)
	PVector LightDirectionalPoint(	const PVector& vIntersectPos, const PVector &vLightPos, CPrePoly* pPoly,
									bool bApplySunLight, bool bClipLight, 
									CLightDef** pLightList, uint32 nNumLights, CLightBSPStack* pStack);

	//given a point in space it will calculate the light that hits it
	PVector LightGridPoint( const PVector& vPos, bool bClipLight, bool bAmbient, CLightBSPStack* pStack);

	// Sub-function for LightGridPoint for calculating the ambient portion of the 
	// grid point's lighting
	LTVector CalcGridPointAmbient(const LTVector &vPos);

	//given a point in space, this function will determine if sunlight hits it or not
	bool	DoesSunlightHit( const PVector& vPos, CLightBSPStack* pStack) const;

	PVector	CalcSunLightSample(const PVector& dir) const;

protected:
  
	bool				BuildLightGroups(const LightTable &Dims);
	bool				CalculateLightFrame();
	bool				CalculateLightGroupData(const LightTable &Dims, CLightBSPStack* pStack);
	bool				CalculateLightGroupLightGrid(const LightTable &Dims, const LTVector &vNormalizedColor);

	void				GetPolyLightmapSize(CPrePoly *pPoly);
	bool				SetVertexColors();

	bool				LightPolygon(CPrePoly *pPoly, LTVector *pColorArray, CLightBSPStack* pStack);

	// Sets up a single-frame base light animation with all the polies.
	bool				SetupNewLightAnim(const char *pName);

	// Adds a new frame to the current animation and sets m_pCurFrame to the new frame.
	bool				AddAnimFrame();

	void				AddLightDef(TLightDef *pLightDef, const PVector& pos, CPrePoly *pPoly);

	void				GetLightDefs();
	void				ClearLightDefs();

	bool				ShouldBlockLight(CPreWorld *pWorld);
	bool				SetupWorldModels();

	bool				LightTouchesPoly(CLightDef *pLight, CPrePoly *pPoly, uint32 nPolyIndex);

	void				ClampMapValues( CFullLightMap *pMap, uint32 nSamplesU, uint32 nSamplesV );
	void				DownSample(CFullLightMap *pIn, CFullLightMap *pOut);
	bool				ConvertMap( CFullLightMap *pIn, CPreLightMap *pOut );

	//fills in an empty texture with the default color/ambient light/etc.
	void				InitPolyLightmapFill(CPrePoly *poly, CFullLightMap *lightmap);

	//fills in the lightmap's m_Min and m_Max;
	void				InitPolyLightmapMinMax(CPrePoly *poly, CFullLightMap *lightmap);

	// Removes any polies it can from the current light animation.
	// If it returns FALSE, the animation is invalid.
	bool				RemoveExcessPolies();

	// Builds the internal poly tree used for segment intersections
	bool				InitPolyTree();

	//builds up the BSP for polygon intersections
	bool				InitBSP();


	// Threadsafe functions.
protected:

	// Thread main function.
	static void			ThreadCB(void *pUser);

	// The main thread function.
	void				ThreadMain();

	// Does the actual work of lighting a poly.
	bool				ThreadLightPoly(uint32 iPoly, CLightBSPStack* pStack);

	// Called to get the next thread job (a poly to light).
	// Returns FALSE if there are no more jobs.
	bool				ThreadGetNextJob(uint32 &iPoly);

	// Mark that an error occured.  No more jobs will be handed out
	// and the lighting process will fail.
	void				ThreadSignalError();

	// These return FALSE if there is a fatal error.
	bool				ApplyLightsToPoly(CPrePoly *pPoly, uint32 nPolyAnimIndex, uint32 nNumLights, CLightDef **pLights, CLightBSPStack* pStack);


protected:
  
	// Our critical section.
	void				*m_ThreadCS;

	// Which poly our thread is currently on.
	uint32				m_iCurThreadPoly;

	// Did any thread signal an error?
	bool				m_bThreadError;


 protected:

	// Create the geometry subdivision lighting data
	void CreateLightingFragments();
  
	bool IsLightGroupObject(CBaseEditObj *pObject);
	const char *GetObjectLightGroupName(CBaseEditObj *pObject);

 protected:
  
	CPreMainWorld			*m_pWorld;

	// The intersection trees used for the lighting
	CLMPolyTree				m_cPolyTree_World;

	// We have one of these for each world model.
	CMoArray<LMMWorld>		m_LMMWorlds;

	//the BSP used for the lighting
	CLightingBSP			m_LightingBSP;

	// All the polies from ALL the worlds.
	CMoArray<CPrePoly*>		m_Polies;


	PVector					m_AmbientLight;
	bool					m_bClipLight;

	//whether or not the brush ambient should be applied to the light values,
	//this is disabled during light animations so that it isn't applied multiple times
	bool					m_bApplyBrushAmbient;

	// Global dir light info.

	// Was there a StaticSunLight object?
	bool					m_bStaticSunLight;
	// Do we want to use Lambert (i.e. normal) Lighting? 
	PVector					m_SunLightInner;
	PVector					m_SunLightDir;
	PReal					m_SunLightBrightScale;

	// Oversampling info.
	bool					m_bOverSample;

	// The light definitions.
	CMoArray<CLightDef*>	m_LightDefs;

	// The light map used while building maps.
	CMoArray<CFullLightMap*>	m_FullLightMaps;

	// This tells where the light data we're generating goes.
	CPreLightAnim			*m_pCurAnim;
	CPreLightFrame			*m_pCurFrame;

	// The currently active lighting group
	CPreLightGroup			*m_pCurLightGroup;

	// Length of sunlight rays - based on size of the world.
	PReal					m_fSunRayLength;

};



#endif // __LIGHTMAP_MAKER_H__


