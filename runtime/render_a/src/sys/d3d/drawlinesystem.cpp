#include "precompile.h"

class ViewParams;

#include "de_objects.h"
#include "3d_ops.h"
#include "d3d_texture.h"
#include "fixedpoint.h"
#include "tagnodes.h"
#include "drawobjects.h"
#include "d3d_draw.h"

//----------------------------------------------------------------------------
// Linesystem Vertex format
//   Uses: HW TnL, 2 texture channels, diffuse color, specular color (for fog)
//----------------------------------------------------------------------------
#define LINESYSTEMVERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_DIFFUSE)

class CLineSystemVertex
{
public:

	//position
	LTVector	m_Vec;

	//colors
	RGBColor	m_Color;
};

// ------------------------------------------------------------------- //
// Internals.
// ------------------------------------------------------------------- //

static void d3d_DrawLineSystem(const ViewParams& Params, LTObject *pObject)
{
	LSLine *pLine;
	CLineSystemVertex Verts[2];
	LineSystem *pSystem;

	pSystem = (LineSystem*)pObject;

	// Setup the main transform.
	LTMatrix mSystemTransform;
	d3d_SetupTransformation(&pObject->GetPos(), (float*)&pObject->m_Rotation, &pObject->m_Scale, &mSystemTransform);
	MatTranspose(&mSystemTransform);

	PD3DDEVICE->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&mSystemTransform);
	
	
	float fAlphaScale = (float)pSystem->m_ColorA;

	pLine = pSystem->m_LineHead.m_pNext;

	// Set states.
	d3d_DisableTexture(0);

	D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
	D3D_CALL(PD3DDEVICE->SetFVF(LINESYSTEMVERTEX_FORMAT));

	while(pLine != &pSystem->m_LineHead)
	{
		Verts[0].m_Vec = pLine->m_Points[0].m_Pos;
		Verts[0].m_Color.rgb.r = (uint8)RoundFloatToInt(pLine->m_Points[0].r * 255.0f);
		Verts[0].m_Color.rgb.g = (uint8)RoundFloatToInt(pLine->m_Points[0].g * 255.0f);
		Verts[0].m_Color.rgb.b = (uint8)RoundFloatToInt(pLine->m_Points[0].b * 255.0f);
		Verts[0].m_Color.rgb.a = (uint8)RoundFloatToInt(pLine->m_Points[0].a * fAlphaScale);
		
		Verts[1].m_Vec = pLine->m_Points[1].m_Pos;
		Verts[1].m_Color.rgb.r = (uint8)RoundFloatToInt(pLine->m_Points[1].r * 255.0f);
		Verts[1].m_Color.rgb.g = (uint8)RoundFloatToInt(pLine->m_Points[1].g * 255.0f);
		Verts[1].m_Color.rgb.b = (uint8)RoundFloatToInt(pLine->m_Points[1].b * 255.0f);
		Verts[1].m_Color.rgb.a = (uint8)RoundFloatToInt(pLine->m_Points[1].a * fAlphaScale);

		D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_LINELIST, 1, Verts, sizeof(CLineSystemVertex)));

		pLine = pLine->m_pNext;
	}

	//restore the world transform
	PD3DDEVICE->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&Params.m_mIdentity);
}	


// ------------------------------------------------------------------- //
// Externals.
// ------------------------------------------------------------------- //

void d3d_ProcessLineSystem(LTObject *pObject)
{
	if(!g_CV_DrawLineSystems)
		return;

	d3d_GetVisibleSet()->m_LineSystems.Add(pObject);
}


// Translucent line system queueing hook function for sorting
void d3d_QueueLineSystems(const ViewParams& Params, ObjectDrawList& DrawList)
{
	d3d_GetVisibleSet()->m_LineSystems.Queue(DrawList, Params, d3d_DrawLineSystem);
}

void d3d_DrawLine(const LTVector& src, const LTVector& dest, uint32 color)
{
	d3d_DrawLine(src, dest, color, color);
}

void d3d_DrawLine(const LTVector& src, const LTVector& dest, uint32 color1, uint32 color2)
{
	CLineSystemVertex verts[2];

	verts[0].m_Vec			= src;
	verts[0].m_Color.color	= color1;
	verts[1].m_Vec			= dest;
	verts[1].m_Color.color	= color2;

	D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
	D3D_CALL(PD3DDEVICE->SetFVF(LINESYSTEMVERTEX_FORMAT));
	D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_LINELIST, 1, verts, sizeof(CLineSystemVertex))); 
}




