//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsDisplay.cpp: implementation of the COptionsDisplay class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "afxtempl.h"

#include "optionsbase.h"
#include "optionsdisplay.h"
#include "editprojectmgr.h"
#include "edithelpers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Externs
extern TCHAR szRegKeyCompany[];
extern TCHAR szRegKeyApp[];
extern TCHAR szRegKeyVer[];

//default values
#define DEFAULT_VERTEX_SIZE				2
#define DEFAULT_HANDLE_SIZE				4
#define DEFAULT_PERSPECTIVE_FAR_Z		20000

/************************************************************************/
// Load an item
BOOL CDisplayColorItem::Load(COptionsBase &options)
{
	// Get the color
	COLORREF cr=GetColor();

	// Create the default value
	CString sDefault;
	sDefault.Format("%d %d %d", GetRValue(cr), GetGValue(cr), GetBValue(cr));

	// Read the value from the registry
	CString sValue=options.GetStringValue(GetRegistryKey(), sDefault);

	// Parse the string
	int r=0, g=0, b=0;
	sscanf(sValue, "%d %d %d", &r, &g, &b);
	
	// Set the color
	SetColor(RGB(r, g, b));

	return TRUE;
}

/************************************************************************/
// Save an item
BOOL CDisplayColorItem::Save(COptionsBase &options)
{
	// Get the color
	COLORREF cr=GetColor();

	// Save the value as an RGB triplette
	CString sRGB;
	sRGB.Format("%d %d %d", GetRValue(cr), GetGValue(cr), GetBValue(cr));

	options.SetStringValue(GetRegistryKey(), sRGB);	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionsDisplay::COptionsDisplay()
{
	// Set the colors to their defaults
	ResetToDefaultColors();

	// Set the default display options		
	SetShadePolygons(TRUE);
	SetShowSurfaceColor(FALSE);
	SetVertexDrawRule(kVertexDrawSelectedOnly);

	SetShowLighting(TRUE);

	// Set the default D3D options
	SetZBufferLines(FALSE);
	SetDefaultD3DDevice(0);
	SetDefaultD3DMode(0);
	
	//set the defail detail texture options
	SetDetailTexEnabled(TRUE);
	SetDetailTexAdditive(TRUE);
	SetDetailTexScale(1.0f);
	SetDetailTexAngle(0.0f);

	SetVertexSize(DEFAULT_VERTEX_SIZE);
	SetHandleSize(DEFAULT_HANDLE_SIZE);

	SetPerspectiveFarZ(DEFAULT_PERSPECTIVE_FAR_Z);

	SetShowClassIcons(TRUE);
	SetClassIconSize(30);
}

COptionsDisplay::~COptionsDisplay()
{
}

/************************************************************************/
// Resets the colors to their defaults
void COptionsDisplay::ResetToDefaultColors()
{
	// Setup the color maps
	m_colorArray[kColorBackground]=					CDisplayColorItem("ColorBackground",	"Background",		RGB(72,72,72));
	m_colorArray[kColorMajorGrid]=					CDisplayColorItem("ColorMajorGrid",		"MajorGrid",		RGB(60,60,60));
	m_colorArray[kColorMinorGrid]=					CDisplayColorItem("ColorMinorGrid",		"MinorGrid",		RGB(95,95,95));	
	m_colorArray[kColorBrushDrawLines]=				CDisplayColorItem("ColorBrush",			"Brush",			RGB(255,255,255));
	m_colorArray[kColorBrushDrawLinesConcave]=		CDisplayColorItem("ColorBrushConcave",	"Brush Concave",	RGB(255,0,255));
	m_colorArray[kColorBrushDrawLinesIntersect]=	CDisplayColorItem("ColorBrushIntersect","Brush Intersect",	RGB(255,0,0));
	m_colorArray[kColorNormals]=					CDisplayColorItem("ColorNormals",		"Normals",			RGB(66,255,255));
	m_colorArray[kColorSelectedBrush]=				CDisplayColorItem("ColorSelectedBrush",	"Selected Brush",	RGB(255,255,0));
	m_colorArray[kColorFrozenBrush]=				CDisplayColorItem("ColorFrozenBrush",	"Frozen Brush",		RGB(0,0,255));
	m_colorArray[kColorObject]=						CDisplayColorItem("ColorObject",		"Object",			RGB(255,0,0));	
	m_colorArray[kColorSelectedObject]=				CDisplayColorItem("ColorSelectedObject","Selected Object",	RGB(255,255,255));
	m_colorArray[kColorObjectRadius]=				CDisplayColorItem("ColorObjectRadius",	"Object Radius",	RGB(0,255,255));
	m_colorArray[kColorObjectLink]=					CDisplayColorItem("ColorObjectLink",	"Object Link",		RGB(255,255,0));
	m_colorArray[kColorObjectFOV]=					CDisplayColorItem("ColorObjectFOV",		"Object FOV",		RGB(0,255,255));	
	m_colorArray[kColorObjectOrthoFrustum]=			CDisplayColorItem("ColorObjectOrthoFrustum","Object Ortho Frustum",		RGB(0,255,255));	
	m_colorArray[kColorObjectDims]=					CDisplayColorItem("ColorObjectDims",	"Object Dims",		RGB(255,255,255));			
	m_colorArray[kColorPath]=						CDisplayColorItem("ColorPath",			"Path",				RGB(255,255,0));			
	m_colorArray[kColorSelectedFace]=				CDisplayColorItem("ColorSelectedFace",	"Selected Face",	RGB(255,0,0));
	m_colorArray[kColorSelectedEdge]=				CDisplayColorItem("ColorSelectedEdge",	"Selected Edge",	RGB(0,255,0));
	m_colorArray[kColorBoundingBox]=				CDisplayColorItem("ColorBoundingBox",	"Bounding Box",		RGB(192,192,192));	
	m_colorArray[kColorOrigin]=						CDisplayColorItem("ColorOrigin",		"Origin",			RGB(255,255,0));	
	m_colorArray[kColorMarker]=						CDisplayColorItem("ColorMarker",		"Marker",			RGB(0,255,0));
	m_colorArray[kColorHandleOutline]=				CDisplayColorItem("ColorHandleOutline",	"Handle Outline",	RGB(255,255,255));
	m_colorArray[kColorHandleFill]=					CDisplayColorItem("ColorHandleFill",	"Handle Fill",		RGB(0,0,0));
	m_colorArray[kColorVertexOutline]=				CDisplayColorItem("ColorVertexOutline",	"Vertex Outline",	RGB(255,0,0));
	m_colorArray[kColorVertexFill]=					CDisplayColorItem("ColorVertexFill",	"Vertex Fill",		RGB(255,0,0));
	m_colorArray[kColorTaggedVerticeOutline]=		CDisplayColorItem("ColorTaggedVerticeOutline",	"Tagged Vertice Outline",	RGB(255,255,0));	
	m_colorArray[kColorTaggedVerticeFill]=			CDisplayColorItem("ColorTaggedVerticeFill",		"Tagged Vertice Fill",		RGB(255,255,0));	
	m_colorArray[kColorImmediateVertice] =			CDisplayColorItem("ColorImmediateVertice", "Immediate Vertex", RGB(50, 255, 50));
	m_colorArray[kColorObjectOrientationX] =		CDisplayColorItem("ColorObjectOrientationX", "Object X Axis", RGB(255, 0, 0));
	m_colorArray[kColorObjectOrientationY] =		CDisplayColorItem("ColorObjectOrientationY", "Object Y Axis", RGB(0, 255, 0));
	m_colorArray[kColorObjectOrientationZ] =		CDisplayColorItem("ColorObjectOrientationZ", "Object Z Axis", RGB(0, 0, 255));
	
}

/************************************************************************/
// Sets the directory to be used when loading class icons
void COptionsDisplay::SetClassIconsDir(const char* pszDir)
{
	m_sClassIconsDir  = pszDir;

	//make all the slashes standard
	m_sClassIconsDir.Replace('/', '\\');

	//add a trailing slash if needed
	if(m_sClassIconsDir.GetLength() && (m_sClassIconsDir[m_sClassIconsDir.GetLength() - 1] != '\\'))
	{
		m_sClassIconsDir += '\\';
	}

}

/************************************************************************/
// Loads the display options to the registry
BOOL COptionsDisplay::Load()
{
	// Load each color item
	int i;
	for (i=0; i < kNumColorOptions; i++)
	{
		GetColorItem(i)->Load(*this);
	}	

	CString sVal;
	sVal.Format("%f", MATH_PI / 2);
	SetViewAngle(atof(GetStringValue("ViewAngle", sVal)));
	SetUseAspectRatio(GetBoolValue("UseAspectRatio", FALSE));

	// Load the shade polygons value
	SetShadePolygons(GetBoolValue("ShadePolygons", TRUE));

	// Load the "show surface color" value
	SetShowSurfaceColor(GetBoolValue("ShowSurfaceColor", FALSE));

	// Load the vertex/handle sizes
	SetVertexSize(GetDWordValue("VertexSize", DEFAULT_VERTEX_SIZE));
	SetHandleSize(GetDWordValue("HandleSize", DEFAULT_HANDLE_SIZE));

	// Load the vertex draw rule
	SetVertexDrawRule(GetDWordValue("VertexDrawRule", kVertexDrawSelectedOnly));

	//load the Far Z clipping plane
	SetPerspectiveFarZ(GetDWordValue("PerspectiveFarZ", DEFAULT_PERSPECTIVE_FAR_Z));

	//load if the object boxes should be oriented
	SetOrientObjectBoxes(GetBoolValue("OrientObjectBoxes", TRUE));


	SetShowSelectedDecals(GetBoolValue("ShowSelectedDecals", TRUE));

	SetHideFrozenNodes(GetBoolValue("HideFrozenNodes", FALSE));

	//load the tint options
	SetTintSelected(GetBoolValue("TintSelected", TRUE));
	SetTintFrozen(GetBoolValue("TintFrozen", TRUE));

	/////////////////////////////
	// Load the D3D options
	SetSaturateLightmaps(GetBoolValue("SaturateLightmaps", TRUE));
	SetDefaultD3DDevice(GetDWordValue("DefaultD3DDevice", 0));

	// Always set the mipmap offset to zero
	SetMipMapOffset(GetDWordValue("MipMapOffset", 0));	
	//m_dwMipMapOffset=0;

	// Load the Z-Buffer lines value
	SetZBufferLines(GetBoolValue("ZBufferLines", TRUE));

	//save the class icon information
	SetShowClassIcons(GetBoolValue("ShowClassIcons", TRUE));
	SetClassIconsDir(GetStringValue("ClassIconsDir", "ClassIcons\\"));
	SetClassIconSize(GetDWordValue("ClassIconSize", 12));

	//load the detail texture options
	SetDetailTexEnabled(GetBoolValue("DetailTexEnable", TRUE));
	SetDetailTexAdditive(GetBoolValue("DetailTexAdditive", TRUE));
	SetDetailTexScale(atof(GetStringValue("DetailTexScale", "1.0")));
	SetDetailTexAngle(atof(GetStringValue("DetailTexAngle", "0.0")));

	return TRUE;
}

/************************************************************************/
// Saves the display options to the registry
BOOL COptionsDisplay::Save()
{
	// Save each color item
	int i;
	for (i=0; i < kNumColorOptions; i++)
	{
		GetColorItem(i)->Save(*this);
	}	

	CString sVal;
	sVal.Format("%f", GetViewAngle());
	SetStringValue("ViewAngle",					sVal);
	SetBoolValue("UseAspectRatio",				IsUseAspectRatio());

	// Save the shade polygons value
	SetBoolValue("ShadePolygons",				IsShadePolygons());

	// Save the "show surface color" value
	SetBoolValue("ShowSurfaceColor",			IsShowSurfaceColor());

	// Save the vertex/handle sizes
	SetDWordValue("VertexSize",					GetVertexSize());
	SetDWordValue("HandleSize",					GetHandleSize());

	// Save the vertex draw rule
	SetDWordValue("VertexDrawRule",				GetVertexDrawRule());

	// save the far Z distance
	SetDWordValue("PerspectiveFarZ",			GetPerspectiveFarZ());

	//save if we want the object boxes oriented
	SetBoolValue("OrientObjectBoxes",			IsOrientObjectBoxes());

	//save the tint options
	SetBoolValue("TintSelected",				IsTintSelected());
	SetBoolValue("TintFrozen",					IsTintFrozen());

	SetBoolValue("ShowSelectedDecals",			IsShowSelectedDecals());
	SetBoolValue("HideFrozenNodes",				IsHideFrozenNodes());

	//save the class icons options
	SetBoolValue("ShowClassIcons",				IsShowClassIcons());
	SetStringValue("ClassIconsDir",				GetClassIconsDir());
	SetDWordValue("ClassIconSize",				GetClassIconSize());

	/////////////////////////////
	// Save the D3D options

	// Save the Z-Buffer lines value
	SetBoolValue("ZBufferLines",				IsZBufferLines());
	SetBoolValue("SaturateLightmaps",			IsSaturateLightmaps());

	SetDWordValue("DefaultD3DDevice",			GetDefaultD3DDevice());
	SetDWordValue("DefaultD3DMode",				GetDefaultD3DMode());	
	SetDWordValue("MipMapOffset",				GetMipMapOffset());

	//save the detail texture settings
	SetBoolValue("DetailTexEnable",				IsDetailTexEnabled());
	SetBoolValue("DetailTexAdditive",			IsDetailTexAdditive());

	//for formatting the floats
	sVal.Format("%f", GetDetailTexScale());
	SetStringValue("DetailTexScale",			sVal);

	sVal.Format("%f", GetDetailTexAngle());
	SetStringValue("DetailTexAngle",			sVal);

	return TRUE;
}

