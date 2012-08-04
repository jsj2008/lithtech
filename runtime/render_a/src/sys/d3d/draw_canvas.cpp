#include "precompile.h"

#include "de_objects.h"
#include "draw_canvas.h"
#include "d3d_convar.h"
#include "drawobjects.h"
#include "common_draw.h"

class ViewParams;


class Tester_Canvas
{
public:
	static bool Filter(LTObject *pObj) {
		return !(((Canvas*)pObj)->cd.m_ClientFlags & CF_SOLIDCANVAS); }
};

CanvasDrawMgr g_CanvasDrawMgr;

// --------------------------------------------------------------------------- //
// CanvasDrawMgr.
// --------------------------------------------------------------------------- //
CanvasDrawMgr::CanvasDrawMgr()
{
}

void CanvasDrawMgr::DrawCanvases(Canvas **pList, uint32 &listSize)
{
	for (uint32 i=0; i < listSize; ++i) {
		DrawCanvas(pList[i]); }
	listSize = 0;
}

void CanvasDrawMgr::DrawCanvas(Canvas *pCanvas)
{
	if (pCanvas->m_Fn) {
		pCanvas->m_Fn(pCanvas, pCanvas->m_pFnUserData); }
}

// --------------------------------------------------------------------------- //
// Global functions.
// --------------------------------------------------------------------------- //
void d3d_DrawCanvasCB(const ViewParams& Params, LTObject *pCanvas)
{
	g_CanvasDrawMgr.DrawCanvas((Canvas*)pCanvas);
}

void d3d_ProcessCanvas(LTObject *pObject)
{
	if (!g_CV_DrawCanvases.m_Val) 
		return;

	if(pObject->ToCanvas()->cd.m_ClientFlags & CF_SOLIDCANVAS)
	{
		d3d_GetVisibleSet()->m_SolidCanvases.Add(pObject);
	}
	else
	{
		d3d_GetVisibleSet()->m_TranslucentCanvases.Add(pObject);
	}
}

void d3d_DrawSolidCanvases(const ViewParams& Params)
{
	d3d_GetVisibleSet()->m_SolidCanvases.Draw(Params, d3d_DrawCanvasCB);
}

void d3d_QueueTranslucentCanvases(const ViewParams& Params, ObjectDrawList& DrawList)
{
	d3d_GetVisibleSet()->m_TranslucentCanvases.Queue(DrawList, Params, d3d_DrawCanvasCB);
}
