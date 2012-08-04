// DebugGeometry.cpp

// Includes
#include "bdefs.h"
#include "debuggeometry.h"
#include "render.h"
#include "sysconsole_impl.h"
#include "cuipolystring_impl.h"
#include "iltfontmanager.h"
#include "ilttexinterface.h"
#include "iltdrawprim.h"

static ILTDrawPrim *g_pILTDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, g_pILTDrawPrimInternal, Internal);

static ILTFontManager*  g_IFontManager;
define_holder(ILTFontManager, g_IFontManager);
static ILTTexInterface* g_ITexInterface;
define_holder(ILTTexInterface, g_ITexInterface);

#ifndef MAKEFOURCC_PC
#define MAKEFOURCC_PC(ch3, ch2, ch1, ch0)								\
		((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |			\
		((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))
#endif

// NOTE: This can only only implement the CDebugGeometry class - it is included in 
//	the engine (DI objects like the DebugLines, etc.) should have their code only in the
//	the renderer (with the headers shared in both)...
#ifdef __D3D
CDebugGeometry::CDebugGeometry() : mVisible(true), mWidth(0.5f) {}
CDebugGeometry::~CDebugGeometry() { 
	clear(); }
#elif defined(__XBOX)
CDebugGeometry::CDebugGeometry() : mVisible(true), mWidth(0.5f) {}
CDebugGeometry::~CDebugGeometry() { 
	clear(); }
#else
CDebugGeometry::CDebugGeometry() {}
CDebugGeometry::~CDebugGeometry() {}
#endif

// Get a particular debug geometry object (selected by an ID)
CDebugGeometry& getDebugGeometry()
{
	static CDebugGeometry sDebugGeometry;
	return sDebugGeometry;
}

void CDebugGeometry::clear()
{
#ifdef __D3D
	RenderStruct* pRendStruct = r_GetRenderStruct();
	if (pRendStruct) if (!pRendStruct->m_bInitted) pRendStruct = NULL;	// Renderer already shut down...
#endif
#ifdef __XBOX
	RenderStruct* pRendStruct = r_GetRenderStruct();
	if (pRendStruct) if (!pRendStruct->m_bInitted) pRendStruct = NULL;	// Renderer already shut down...
#endif
    
	while (!mLines.empty())
    {					// Clear out mLines...
		CDIDebugLine* pLine = *(mLines.begin()); 
		mLines.pop_front();
#ifdef __D3D
	 	if (pRendStruct) pRendStruct->DestroyRenderObject(pLine);
#endif
#ifdef __XBOX
	 	if (pRendStruct) pRendStruct->DestroyRenderObject(pLine);
#endif
    }

	while (!mPolygons.empty()) {				// Clear out mPolygons...
		CDIDebugPolygon* pPoly = *(mPolygons.begin()); 
		mPolygons.pop_front();
#ifdef __D3D
		if (pRendStruct) pRendStruct->DestroyRenderObject(pPoly); 
#endif
#ifdef __XBOX
		if (pRendStruct) pRendStruct->DestroyRenderObject(pPoly); 
#endif
    }
    

	while (!mText.empty()) {					// Clear out mText...
		CDIDebugText* pText = *(mText.begin()); 
		mText.pop_front();
#ifdef __D3D
	 	if (pRendStruct) pRendStruct->DestroyRenderObject(pText); 
#endif
#ifdef __XBOX
	 	if (pRendStruct) pRendStruct->DestroyRenderObject(pText); 
#endif
    }
}

void CDebugGeometry::addLine(const LTVector& from, const LTVector& to, const LTRGBColor& color, bool bScreenSpace)
{
	// The renderer creates the render object (DD Object) and passes it back to us...		
	CDIDebugLine* pLine = NULL;	
#ifdef __D3D
	pLine = (CDIDebugLine*)r_GetRenderStruct()->CreateRenderObject(CRenderObject::eDebugLine); if (!pLine) return;
#endif
#ifdef __XBOX
	pLine = (CDIDebugLine*)r_GetRenderStruct()->CreateRenderObject(CRenderObject::eDebugLine); if (!pLine) return;
#endif
    if (!pLine) {
		return;
	}	
	
	pLine->m_DPLine.verts[0].x = from.x;  pLine->m_DPLine.verts[0].y = from.y;  pLine->m_DPLine.verts[0].z = from.z;
	pLine->m_DPLine.verts[1].x = to.x;    pLine->m_DPLine.verts[1].y = to.y;    pLine->m_DPLine.verts[1].z = to.z;
	pLine->m_DPLine.rgba.a = color.rgb.a; pLine->m_DPLine.rgba.r = color.rgb.r; pLine->m_DPLine.rgba.g = color.rgb.g; pLine->m_DPLine.rgba.b = color.rgb.b;
	pLine->m_bScreenSpace = bScreenSpace;
	
		
	mLines.push_back(pLine);
}

void CDebugGeometry::addPolygon(CDIDebugPolygon* poly, bool bScreenSpace)
{
	CDIDebugPolygon* pPoly = NULL;
#ifdef __D3D
	pPoly = (CDIDebugPolygon*)r_GetRenderStruct()->CreateRenderObject(CRenderObject::eDebugPolygon); if (!pPoly) return;
#endif
#ifdef __XBOX
	pPoly = (CDIDebugPolygon*)r_GetRenderStruct()->CreateRenderObject(CRenderObject::eDebugPolygon); if (!pPoly) return;
#endif
    if (!pPoly) return;
	pPoly->m_bScreenSpace  = bScreenSpace;
	pPoly->m_DPPoly3	   = poly->m_DPPoly3;
	pPoly->m_DPPoly4	   = poly->m_DPPoly4;
	pPoly->m_VertCount	   = poly->m_VertCount;
	pPoly->m_Polygon[0]	   = poly->m_Polygon[0];
	pPoly->m_Polygon[1]	   = poly->m_Polygon[1];
	pPoly->m_Polygon[2]	   = poly->m_Polygon[2];
	pPoly->m_Polygon[3]	   = poly->m_Polygon[3];
		
	mPolygons.push_back(pPoly);
}

void CDebugGeometry::addText(const char* szText, const LTVector& position, const LTRGBColor& color)
{

	CDIDebugText* pText = NULL;
#ifdef __D3D
	pText = (CDIDebugText*)r_GetRenderStruct()->CreateRenderObject(CRenderObject::eDebugText); if (!pText) return;
#endif
#ifdef __XBOX
	pText = (CDIDebugText*)r_GetRenderStruct()->CreateRenderObject(CRenderObject::eDebugText); if (!pText) return;
#endif
    if (!pText) return;

#ifdef __D3D
	if (!GETCONSOLE()->GetInitialized()) return;
	pText->SetFont(GETCONSOLE()->GetFont()); 
#endif
#ifdef __XBOX
	if (!GETCONSOLE()->GetInitialized()) return;
	pText->SetFont(GETCONSOLE()->GetFont()); 
#endif
	
	pText->m_DPVert.rgba.a = color.rgb.a; pText->m_DPVert.rgba.r = color.rgb.r; pText->m_DPVert.rgba.g = color.rgb.g; pText->m_DPVert.rgba.b = color.rgb.b;
	
	pText->m_DPVert.x = position.x; pText->m_DPVert.y = position.y; pText->m_DPVert.z = position.z;
	LTStrCpy(pText->m_Text,szText,DEBUG_TEXT_MAX_LEN); 
	mText.push_back(pText);
}

// Renders debug stuff
void CDebugGeometry::render()
{
	if (!mVisible) return;
	
	for (LineList::iterator it = mLines.begin();it != mLines.end();++it) {
		CRenderObject* pLine = (*it);
		pLine->Render(); }							// Draw the Lines...

	for (PolygonList::iterator ip = mPolygons.begin();ip != mPolygons.end();++ip) {
		CRenderObject* pPoly = (*ip);
		pPoly->Render(); }							// Draw the Poly List...

	for (TextList::iterator iz = mText.begin();iz != mText.end();++iz) {
		CRenderObject* pText = (*iz);
		pText->Render(); }							// Draw the Text List...
}

void drawAllDebugGeometry()
{
	getDebugGeometry().render();
}

// Device Independent Debug Geometry...
CDIDebugText::CDIDebugText()
{ 
	m_Type		  = eDebugText; 
	m_pPolyString = NULL;
}

CDIDebugText::~CDIDebugText()
{ 
	if (m_pPolyString) delete m_pPolyString; 
}

#ifndef RGBA_MAKE
#define RGBA_MAKE(r, g, b, a)		((uint32) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#endif
#ifdef __D3D	// While we're keeping the old drawtext method...
void r_GenericTextPrint(char *pMsg, const LTRect *pRect, uint32 textColor);
#endif
#ifdef __XBOX	// While we're keeping the old drawtext method...
void r_GenericTextPrint(char *pMsg, const LTRect *pRect, uint32 textColor);
#endif
extern int32 g_ScreenWidth, g_ScreenHeight;
void CDIDebugText::Render()
{
	if (!m_pPolyString && m_Font) {
		uint32 i32Color = RGBA_MAKE(m_DPVert.rgba.r,m_DPVert.rgba.g,m_DPVert.rgba.b,m_DPVert.rgba.a);
		//m_Font->SetColor(i32Color);
		LT_MEM_TRACK_ALLOC(m_pPolyString = new CUIPolyString(m_Font, m_Text, m_DPVert.x, m_DPVert.y),LT_MEM_TYPE_MISC);
	}

	if (m_pPolyString) {
		m_pPolyString->Render(); } 

/*LTRect rc; rc.left = m_DPVert.x; rc.top = m_DPVert.y;	
rc.bottom = rc.top + 20; rc.right = min(rc.left + 100,g_ScreenWidth);;
uint32 i32Color = RGBA_MAKE(m_DPVert.rgba.r,m_DPVert.rgba.g,m_DPVert.rgba.b,m_DPVert.rgba.a);
r_GenericTextPrint(m_Text,&rc,i32Color);*/
}

// Device Independent Debug Geometry...
CDIDebugLine::CDIDebugLine()
{ 
	m_Type = eDebugLine; 
}

void CDIDebugLine::Render()
{
	if (g_pILTDrawPrimInternal) {
		if (m_bScreenSpace) {
			g_pILTDrawPrimInternal->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN); }
		else g_pILTDrawPrimInternal->SetTransformType(DRAWPRIM_TRANSFORM_WORLD);
		g_pILTDrawPrimInternal->SetTexture(NULL);
		g_pILTDrawPrimInternal->SetZBufferMode(DRAWPRIM_NOZ);
		g_pILTDrawPrimInternal->SetAlphaBlendMode(DRAWPRIM_NOBLEND);
		g_pILTDrawPrimInternal->SetColorOp(DRAWPRIM_MODULATE);
		g_pILTDrawPrimInternal->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		g_pILTDrawPrimInternal->SetClipMode(DRAWPRIM_FASTCLIP);
		g_pILTDrawPrimInternal->SetFillMode(DRAWPRIM_FILL);
		
		g_pILTDrawPrimInternal->DrawPrim(&m_DPLine,1); }
}

// Device Independent Debug Geometry...
CDIDebugPolygon::CDIDebugPolygon()
{ 
	m_Type = eDebugPolygon; 
	m_VertCount = 0; 
}

void CDIDebugPolygon::Render()
{
	for (int32 i=0;i<m_VertCount;++i) {
		if (m_VertCount == 3) {
			m_DPPoly3.verts[i].x = m_Polygon[i].x; m_DPPoly3.verts[i].y = m_Polygon[i].y; m_DPPoly3.verts[i].z = m_Polygon[i].z; }
		else if (m_VertCount == 4) {
			m_DPPoly4.verts[i].x = m_Polygon[i].x; m_DPPoly4.verts[i].y = m_Polygon[i].y; m_DPPoly4.verts[i].z = m_Polygon[i].z; } }

	if (g_pILTDrawPrimInternal) {
		if (m_bScreenSpace) {
			g_pILTDrawPrimInternal->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN); }
		else {
			g_pILTDrawPrimInternal->SetTransformType(DRAWPRIM_TRANSFORM_WORLD); }
		g_pILTDrawPrimInternal->SetTexture(NULL);
		g_pILTDrawPrimInternal->SetZBufferMode(DRAWPRIM_NOZ);
		g_pILTDrawPrimInternal->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		g_pILTDrawPrimInternal->SetColorOp(DRAWPRIM_MODULATE);
		g_pILTDrawPrimInternal->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		g_pILTDrawPrimInternal->SetClipMode(DRAWPRIM_FASTCLIP);
		g_pILTDrawPrimInternal->SetFillMode(DRAWPRIM_FILL);

		if (m_VertCount == 3) { g_pILTDrawPrimInternal->DrawPrim(&m_DPPoly3,1); }
		else { g_pILTDrawPrimInternal->DrawPrim(&m_DPPoly4,1); } }
}
