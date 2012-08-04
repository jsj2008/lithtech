// DebugGeometry.h

#ifndef DEBUGGEOMETRY_H
#define DEBUGGEOMETRY_H

#pragma warning (disable:4530)										// Stupid STL warning...
#pragma warning (disable:4786)

#ifndef __MAP__
#include <map>
#define __MAP__
#endif

#ifndef __RENDEROBJECT_H__
#include "renderobject.h"
#endif

#ifndef __SYSDDSTRUCTS_H__
#include "sysddstructs.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#ifndef __LIST__
#include <list>
#define __LIST__
#endif

using namespace std;

// DEFINES
#define DEBUG_TEXT_MAX_LEN	100

// FORWARD DECLARATION...
class CUIFont;
class CUIPolyString;
class SharedTexture;

// Debug Text
class CDIDebugText : public CRenderObject
{
public:
	CDIDebugText();
	~CDIDebugText();

	void		Render();
	void		SetFont(CUIFont* pFont)					{ m_Font = pFont; }

	char		m_Text[DEBUG_TEXT_MAX_LEN];
	LT_VERTG	m_DPVert;

private:
	CUIFont*	m_Font;
	CUIPolyString* m_pPolyString;
};

// Debug line
class CDIDebugLine : public CRenderObject
{
public:
	CDIDebugLine();
	void		Render();

	LT_LINEF	m_DPLine;
	bool		m_bScreenSpace;
};

// Debug polygon
class CDIDebugPolygon : public CRenderObject
{
public:
	CDIDebugPolygon();

	void		Render();
	void		AddVertex(LTVector& Vert)				{ if (m_VertCount >= 4) return; m_Polygon[m_VertCount] = Vert; ++m_VertCount; }
	void		SetColor(LTRGBColor& Color)				{
		m_DPPoly3.rgba.a = Color.rgb.a; m_DPPoly3.rgba.r = Color.rgb.r; m_DPPoly3.rgba.g = Color.rgb.g; m_DPPoly3.rgba.b = Color.rgb.b;
		m_DPPoly4.rgba.a = Color.rgb.a; m_DPPoly4.rgba.r = Color.rgb.r; m_DPPoly4.rgba.g = Color.rgb.g; m_DPPoly4.rgba.b = Color.rgb.b; }

	LT_POLYF3	m_DPPoly3;
	LT_POLYF4	m_DPPoly4;
	LTVector	m_Polygon[4];
	int8		m_VertCount;
	bool		m_bScreenSpace;
};

// Debug geometry
class CDebugGeometry
{
public:
	CDebugGeometry();
	~CDebugGeometry();

	typedef		list<CDIDebugLine*>		LineList;
	typedef		list<CDIDebugPolygon*>	PolygonList;
	typedef		list<CDIDebugText*>		TextList;

	void		render();

	// Container goodies
	void		clear();
	int			size() const							{ return mLines.size() + mPolygons.size(); }

	// Set up attributes
	void		setWidth(float width)					{ mWidth = width; }
	void		setVisible(bool vis)					{ mVisible = vis; }

	// Get attributes
	bool		isVisible() const						{ return mVisible; }

	// Primitives
	void		addLine(const LTVector& from, const LTVector& to, const LTRGBColor& color, bool bScreenSpace = false);
	void		addPolygon(CDIDebugPolygon* poly, bool bScreenSpace = false);
	void		addText(const char* szText, const LTVector& position, const LTRGBColor& color);

private:
	LineList	mLines;
	PolygonList mPolygons;
	TextList	mText;
	float		mWidth;
	bool		mVisible;
	int32		mDbgText_Profile;

#ifdef __PS2
 	CUIFont*	mFont;
  	SharedTexture*	mTex;
#endif
};

// Global access functions...
extern	CDebugGeometry&			getDebugGeometry();

// Global Helper Functions...
void drawAllDebugGeometry();

#endif