
#ifndef __DRAW_CANVAS_H__
#define __DRAW_CANVAS_H__

#ifndef __TAGNODES_H__
#include "tagnodes.h"
#endif

class Canvas;
class ViewParams;

class CanvasDrawMgr
{
public:
	CanvasDrawMgr();

	void	DrawCanvases(Canvas **ppList, uint32 &listSize);

public:		// Internals...
	void	DrawCanvas(Canvas *pCanvas);
};

// Global functions.
void d3d_ProcessCanvas(LTObject *pObject);
void d3d_DrawSolidCanvases(const ViewParams& Params);
void d3d_QueueTranslucentCanvases(const ViewParams& Params, ObjectDrawList& DrawList);

#endif

