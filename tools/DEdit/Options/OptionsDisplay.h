//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsDisplay.h: interface for the COptionsDisplay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONSDISPLAY_H__5E3D9E56_D8AA_11D2_BE02_0060971BDC6D__INCLUDED_)
#define AFX_OPTIONSDISPLAY_H__5E3D9E56_D8AA_11D2_BE02_0060971BDC6D__INCLUDED_

#include "regmgr.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "optionsbase.h"

class CDisplayColorItem
{
public:
	// Constructors
	CDisplayColorItem() {}
	CDisplayColorItem(CString sKey, CString sName, COLORREF crDefault)
	{
		SetRegistryKey(sKey);
		SetDisplayName(sName);
		SetColor(crDefault);
	}

	// Load/Save
	BOOL		Load(COptionsBase &options);
	BOOL		Save(COptionsBase &options);

	// Access to member variables
	void		SetRegistryKey(CString sKey)		{ m_sRegistryKey=sKey; }
	void		SetDisplayName(CString sName)		{ m_sDisplayName=sName; }

	void SetColor(COLORREF cr)				
	{ 
		m_crColor = cr;
		m_D3DColor = RGB( (((cr) & 0xFF0000)>>16), (((cr) & 0xFF00)>>8), ((cr) & 0xFF) );
	}

	CString&	GetRegistryKey()					{ return m_sRegistryKey; }
	CString&	GetDisplayName()					{ return m_sDisplayName; }
	COLORREF	GetColor() const					{ return m_crColor; }
	DWORD		GetD3DColor() const					{ return m_D3DColor; }

protected:
	CString		m_sRegistryKey;
	CString		m_sDisplayName;
	COLORREF	m_crColor;
	DWORD		m_D3DColor;
};

class COptionsDisplay : public COptionsBase
{
public:
	// These are the various color options
	enum
	{
		kColorBackground=0,
		kColorMajorGrid,
		kColorMinorGrid,				
		kColorBrushDrawLines,
		kColorBrushDrawLinesConcave,
		kColorBrushDrawLinesIntersect,
		kColorNormals,
		kColorSelectedBrush,
		kColorFrozenBrush,
		kColorObject,
		kColorSelectedObject,
		kColorObjectRadius,
		kColorObjectLink,
		kColorObjectFOV,
		kColorObjectOrthoFrustum,
		kColorObjectDims,
		kColorPath,
		kColorSelectedFace,
		kColorSelectedEdge,
		kColorBoundingBox,
		kColorOrigin,
		kColorMarker,
		kColorHandleOutline,
		kColorHandleFill,
		kColorVertexOutline,
		kColorVertexFill,
		kColorTaggedVerticeOutline,
		kColorTaggedVerticeFill,
		kColorImmediateVertice,
		kColorObjectOrientationX,
		kColorObjectOrientationY,
		kColorObjectOrientationZ,
		kNumColorOptions
	};

	// These are the vertex drawing rules
	enum
	{
		kVertexDrawAll=0,
		kVertexDrawSelectedOnly
	};

public:
	COptionsDisplay();
	virtual ~COptionsDisplay();

	// Loads/Saves the display options to the registry
	BOOL		Load();
	BOOL		Save();

	// Resets the colors to their defaults
	void				ResetToDefaultColors();

	//returns a D3D color based on a color type
	DWORD GetD3DColor(uint32 nColorType) const
	{
		ASSERT(nColorType < kNumColorOptions);
		return m_colorArray[nColorType].GetD3DColor();	
	}

	// Returns a color based on a color type. (See the enum above)
	COLORREF GetColor(uint32 nColorType) const
	{
		ASSERT(nColorType < kNumColorOptions);
		return m_colorArray[nColorType].GetColor();	
	}

	// Returns the color item based on type
	CDisplayColorItem* GetColorItem(uint32 nColorType)
	{
		ASSERT(nColorType < kNumColorOptions);
		return &m_colorArray[nColorType];
	}

	// Sets/Gets other rendering options
	BOOL				IsShadePolygons() const			{ return m_bShadePolygons; }
	void				SetShadePolygons(BOOL bShade)	{ m_bShadePolygons=bShade; }

	BOOL				IsShowSurfaceColor() const		{ return m_bShowSurfaceColor; }
	void				SetShowSurfaceColor(BOOL bShow)	{ m_bShowSurfaceColor=bShow; }

	int					GetVertexSize()	const			{ return m_nVertexSize; }
	void				SetVertexSize(int nSize)		{ m_nVertexSize=nSize; }

	int					GetHandleSize()	const			{ return m_nHandleSize; }
	void				SetHandleSize(int nSize)		{ m_nHandleSize=nSize; }

	int					GetVertexDrawRule()	const		{ return m_nVertexDrawRule; }
	void				SetVertexDrawRule(int nRule)	{ m_nVertexDrawRule=nRule; }

	int					GetPerspectiveFarZ() const		{ return m_nPerspectiveFarZ; }
	void				SetPerspectiveFarZ(int nFarZ)	{ m_nPerspectiveFarZ = (nFarZ < 1) ? 1 : nFarZ; }

	//determines which brushes should and should not be tinted
	BOOL				IsTintSelected() const			{ return m_bTintSelected; }
	void				SetTintSelected(BOOL bVal)		{ m_bTintSelected = bVal; }

	BOOL				IsTintFrozen() const			{ return m_bTintFrozen; }
	void				SetTintFrozen(BOOL bVal)		{ m_bTintFrozen = bVal; }

	//determines if the object bounding boxes should be oriented
	BOOL				IsOrientObjectBoxes() const		{ return m_bOrientObjectBoxes; }
	void				SetOrientObjectBoxes(BOOL bVal)	{ m_bOrientObjectBoxes = bVal; }

	BOOL				IsShowSelectedDecals() const	{ return m_bShowSelectedDecals; }
	void				SetShowSelectedDecals(BOOL bVal){ m_bShowSelectedDecals = bVal; }

	//class icons options
	BOOL				IsShowClassIcons() const		{ return m_bShowClassIcons; }
	void				SetShowClassIcons(BOOL bVal)	{ m_bShowClassIcons = bVal; }

	const char*			GetClassIconsDir() const		{ return m_sClassIconsDir; }
	void				SetClassIconsDir(const char* pszDir);

	DWORD				GetClassIconSize() const		{ return m_nClassIconSize; }
	void				SetClassIconSize(DWORD nVal)	{ m_nClassIconSize = nVal; }

	BOOL				IsHideFrozenNodes() const		{ return m_bHideFrozenNodes; }
	void				SetHideFrozenNodes(BOOL bVal)	{ m_bHideFrozenNodes = bVal; }

	BOOL				IsShowLighting() const			{ return m_bShowLighting; }
	void				SetShowLighting(BOOL bVal)		{ m_bShowLighting = bVal; }


	// Gets/Sets D3D options
	BOOL				IsZBufferLines() const			{ return m_bZBufferLines; }
	void				SetZBufferLines(BOOL bZBuffer)	{ m_bZBufferLines=bZBuffer; }

	BOOL				IsSaturateLightmaps() const		{ return m_bSaturateLightmaps; }
	void				SetSaturateLightmaps(BOOL bVal)	{ m_bSaturateLightmaps = bVal; }

	int					GetDefaultD3DMode() const		{ return m_nDefaultD3DMode; }
	void				SetDefaultD3DMode(int nMode)	{ m_nDefaultD3DMode=nMode; }

	int					GetMipMapOffset() const			{ return m_dwMipMapOffset; }
	void				SetMipMapOffset(DWORD dwOffset)	{ m_dwMipMapOffset=dwOffset; }


	int					GetDefaultD3DDevice() const			{ return m_nDefaultD3DDevice; }
	void				SetDefaultD3DDevice(int nDevice)	{ m_nDefaultD3DDevice=nDevice; }

	float				GetViewAngle() const				{ return m_fViewAngle; }
	void				SetViewAngle(float fAngle)			{ m_fViewAngle = fAngle; }

	BOOL				IsUseAspectRatio() const			{ return m_bUseAspectRatio; }
	void				SetUseAspectRatio(BOOL bVal)		{ m_bUseAspectRatio = bVal; }

	//D3D detail texture settings
	BOOL				IsDetailTexEnabled() const			{ return m_bDetailTexEnable; }
	void				SetDetailTexEnabled(BOOL bEnable)	{ m_bDetailTexEnable = bEnable; }

	BOOL				IsDetailTexAdditive() const			{ return m_bDetailTexAdditive; }
	void				SetDetailTexAdditive(BOOL bVal)		{ m_bDetailTexAdditive = bVal; }

	float				GetDetailTexScale()	const			{ return m_fDetailTexScale; }
	void				SetDetailTexScale(float fVal)		{ m_fDetailTexScale = fVal; }

	float				GetDetailTexAngle() const			{ return m_fDetailTexAngle; }
	void				SetDetailTexAngle(float fVal)		{ m_fDetailTexAngle = fVal; }

protected:
	CDisplayColorItem	m_colorArray[kNumColorOptions];		// The colors
	BOOL				m_bShadePolygons;					// True if the polygons should be shaded
	BOOL				m_bShowSurfaceColor;				// True if the surface color boxes should be displayed
	BOOL				m_bAutoLoadLightmaps;				// True if lightmaps should be loaded with the world
	BOOL				m_bOrientObjectBoxes;				// True if the object boxes should be oriented to their orientation
	int					m_nVertexDrawRule;					// Sets the drawing rules for vertices (all, selected only, etc)

	int					m_nVertexSize;						// The size of a vertex
	int					m_nHandleSize;						// The size of a handle

	int					m_nPerspectiveFarZ;					//the distance for the Far Z clipping plane

	BOOL				m_bHideFrozenNodes;					//determines if frozen nodes should be rendered or not

	BOOL				m_bShowLighting;					//determines if lighting information should be rendered or not

	BOOL				m_bShowSelectedDecals;				//determines if selected decals should be rendered on the world

	float				m_fViewAngle;						//the view angle of the perspective viees in radians
	BOOL				m_bUseAspectRatio;					//whether or not to use the aspect ratio for the camera

	//tinting options
	BOOL				m_bTintSelected;
	BOOL				m_bTintFrozen;

	//class icon options
	BOOL				m_bShowClassIcons;					//determines if class icons should be shown
	CString				m_sClassIconsDir;					//the directory where the class icons are heard
	DWORD				m_nClassIconSize;					//the size of the class icons

	//////////////////
	// D3D options
	BOOL				m_bZBufferLines;					// True if the lines should be Z-Buffered in D3D mode
	BOOL				m_bSaturateLightmaps;				// True if lightmaps should be saturated (2 * src * dest)
	int					m_nDefaultD3DDevice;				// The default D3D device
	int					m_nDefaultD3DMode;					// The default D3D mode
	DWORD				m_dwMipMapOffset;					// The mipmap offset

	//detail texture options
	BOOL				m_bDetailTexEnable;					//if detail textures are enabled
	BOOL				m_bDetailTexAdditive;				//to use additive blend mode
	float				m_fDetailTexScale;					//the scale of detail textures
	float				m_fDetailTexAngle;					//the angle of rotation
};

#endif // !defined(AFX_OPTIONSDISPLAY_H__5E3D9E56_D8AA_11D2_BE02_0060971BDC6D__INCLUDED_)
