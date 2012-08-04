#include "precompile.h"
#include "modeldebug.h"

#include "d3dmeshrendobj_skel.h"
#include "d3dmeshrendobj_rigid.h"
#include "lteffectshadermgr.h"
#include "LTEffectImpl.h"

#include "setupmodel.h"
#include "d3d_utils.h"
#include "d3d_device.h"
#include "common_stuff.h"
#include "common_init.h"
#include "iltdrawprim.h"
#include "d3d_texture.h"

static ILTDrawPrim* g_pILTDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, g_pILTDrawPrimInternal, Internal);

//draws a box around the model of the size of the model dimensions
void NModelDebug::DrawModelBox(ModelInstance* pInstance)
{
	const LTVector& Pos  = pInstance->GetPos();
	const LTVector& Dims = pInstance->GetDims();

	//make sure that D3D is in a desired state
	StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);
	StateSet ssZRead(D3DRS_ZENABLE, FALSE);
	d3d_DisableTexture(0);

	//now render the actual box
	d3d_DrawWireframeBox(Pos - Dims, Pos + Dims, 0xFFFFFFFF);
}

//draws all lights touching a model
void NModelDebug::DrawTouchingLights(const CRelevantLightList& LightList)
{
	if (!g_pILTDrawPrimInternal) return; 
	g_pILTDrawPrimInternal->SetTexture(NULL);
	g_pILTDrawPrimInternal->SetTransformType(DRAWPRIM_TRANSFORM_WORLD);
	g_pILTDrawPrimInternal->SetColorOp(DRAWPRIM_NOCOLOROP);
	g_pILTDrawPrimInternal->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
	
	if (g_CV_ModelDebug_DrawTouchingLights > 1 || g_CV_DrawCastShadowLights > 1) 
	{
		g_pILTDrawPrimInternal->SetZBufferMode(DRAWPRIM_ZRO); 
	}
	else 
		g_pILTDrawPrimInternal->SetZBufferMode(DRAWPRIM_ZRW);

	g_pILTDrawPrimInternal->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pILTDrawPrimInternal->SetClipMode(DRAWPRIM_FASTCLIP);
 
	static uint32 ThisFrameCode;
	static list<CRenderLight> DrawLightInfos;	// Keep a list of drawn lights (don;t want to draw the same one twice a frame)...
	if (ThisFrameCode != g_CurFrameCode) 
	{	
		// New Frame, clear the list...
		DrawLightInfos.erase(DrawLightInfos.begin(),DrawLightInfos.end()); 
		ThisFrameCode = g_CurFrameCode; 
	}

	for (uint32 i=0; i < LightList.GetNumLights(); i++) 
	{
		const CRenderLight& Light = LightList.GetLight(i);

		//ignore it if it isn't a point or spot light
		if(	(Light.GetType() != CRenderLight::eLight_Point) &&
			(Light.GetType() != CRenderLight::eLight_Spot))
		{
			continue;
		}

		bool bAlreadyDrawn = false;			// Figure out if it's already drawn (in the list)...
		list<CRenderLight>::iterator it = DrawLightInfos.begin();
		
		while (it != DrawLightInfos.end()) 
		{
			if (*it == Light) 
			{ 
				bAlreadyDrawn = true; 
				break; 
			} 
			++it; 
		}

		if (bAlreadyDrawn) 
		{ 
			continue; 
		}
		else 
		{ 	
			DrawLightInfos.push_back(Light); 
		}

		float fLightRadius = Light.GetRadius();
		float fCirDiff	   = 0.7071f * fLightRadius;
		float fCirDiff2	   = 0.7071f * fCirDiff;
 
		if ((g_CV_DrawCastShadowLights && (Light.GetFlags() & FLAG_CASTSHADOWS)) || g_CV_ModelDebug_DrawTouchingLights) 
		{
			LT_LINEF Line;	// Draw an X at the center point (in the lights color)
			Line.rgba.a = 0xFF; Line.rgba.r = (uint8)Light.GetColor().x; Line.rgba.g = (uint8)Light.GetColor().y; Line.rgba.b = (uint8)Light.GetColor().z; 
			Line.verts[0].x = Light.GetPos().x - 20; Line.verts[0].y = Light.GetPos().y; Line.verts[0].z = Light.GetPos().z; Line.verts[1].x = Light.GetPos().x + 20; Line.verts[1].y = Light.GetPos().y; Line.verts[1].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Line,1);
			Line.verts[0].x = Light.GetPos().x; Line.verts[0].y = Light.GetPos().y - 20; Line.verts[0].z = Light.GetPos().z; Line.verts[1].x = Light.GetPos().x; Line.verts[1].y = Light.GetPos().y + 20; Line.verts[1].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Line,1);
			Line.verts[0].x = Light.GetPos().x; Line.verts[0].y = Light.GetPos().y; Line.verts[0].z = Light.GetPos().z - 20; Line.verts[1].x = Light.GetPos().x; Line.verts[1].y = Light.GetPos().y; Line.verts[1].z = Light.GetPos().z + 20;
			g_pILTDrawPrimInternal->DrawPrim(&Line,1); 
		}

		if (g_CV_ModelDebug_DrawTouchingLights == 2) 
		{
			LT_POLYF4 Quad;	// Draw a circle type thing now (in some off colors)...
			Quad.rgba.a = 0x10; Quad.rgba.r = MIN(0xFF,(uint8)((float)Light.GetColor().x * 1.2f)); Quad.rgba.g = (uint8)Light.GetColor().y; Quad.rgba.b = (uint8)Light.GetColor().z; 
			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y; Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x + fLightRadius; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z; Quad.verts[2].x = Light.GetPos().x + fCirDiff; Quad.verts[2].y = Light.GetPos().y + fCirDiff; Quad.verts[2].z = Light.GetPos().z; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y + fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y; Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x - fLightRadius; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z; Quad.verts[2].x = Light.GetPos().x - fCirDiff; Quad.verts[2].y = Light.GetPos().y + fCirDiff; Quad.verts[2].z = Light.GetPos().z; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y + fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y; Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x - fLightRadius; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z; Quad.verts[2].x = Light.GetPos().x - fCirDiff; Quad.verts[2].y = Light.GetPos().y - fCirDiff; Quad.verts[2].z = Light.GetPos().z; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y - fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y; Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x + fLightRadius; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z; Quad.verts[2].x = Light.GetPos().x + fCirDiff; Quad.verts[2].y = Light.GetPos().y - fCirDiff; Quad.verts[2].z = Light.GetPos().z; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y - fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.rgba.a = 0x10; Quad.rgba.r = (uint8)Light.GetColor().x; Quad.rgba.g = (uint8)Light.GetColor().y; Quad.rgba.b = MIN(0xFF,(uint8)((float)Light.GetColor().z * 1.2f)); 
			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y; Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z + fLightRadius; Quad.verts[2].x = Light.GetPos().x; Quad.verts[2].y = Light.GetPos().y + fCirDiff; Quad.verts[2].z = Light.GetPos().z + fCirDiff; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y + fLightRadius; Quad.verts[3].z = Light.GetPos().z; g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y; Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z - fLightRadius; Quad.verts[2].x = Light.GetPos().x; Quad.verts[2].y = Light.GetPos().y + fCirDiff; Quad.verts[2].z = Light.GetPos().z - fCirDiff; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y + fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y; Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z - fLightRadius; Quad.verts[2].x = Light.GetPos().x; Quad.verts[2].y = Light.GetPos().y - fCirDiff; Quad.verts[2].z = Light.GetPos().z - fCirDiff; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y - fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y; Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z + fLightRadius; Quad.verts[2].x = Light.GetPos().x; Quad.verts[2].y = Light.GetPos().y - fCirDiff; Quad.verts[2].z = Light.GetPos().z + fCirDiff; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y - fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1); 
		} 

		if (g_CV_ModelDebug_DrawTouchingLights == 3) 
		{
			LT_POLYF4 Quad;	// Draw a circle type thing now (in some off colors)...
			Quad.rgba.a = 0x10; Quad.rgba.r = MIN(0xFF,(uint8)((float)Light.GetColor().x * 1.2f)); Quad.rgba.g = (uint8)Light.GetColor().y; Quad.rgba.b = (uint8)Light.GetColor().z; 
			Quad.verts[0].x = Light.GetPos().x; 	Quad.verts[0].y = Light.GetPos().y;Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x + fCirDiff; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z + fCirDiff; Quad.verts[2].x = Light.GetPos().x + fCirDiff2; 	Quad.verts[2].y = Light.GetPos().y + fCirDiff; 	Quad.verts[2].z = Light.GetPos().z + fCirDiff2; 	Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y + fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y;Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x - fCirDiff; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z - fCirDiff; Quad.verts[2].x = Light.GetPos().x - fCirDiff2; Quad.verts[2].y = Light.GetPos().y + fCirDiff; Quad.verts[2].z = Light.GetPos().z - fCirDiff2; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y + fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y; Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x + fCirDiff; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z + fCirDiff; Quad.verts[2].x = Light.GetPos().x + fCirDiff2; Quad.verts[2].y = Light.GetPos().y - fCirDiff; Quad.verts[2].z = Light.GetPos().z + fCirDiff2; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y - fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y;Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x - fCirDiff; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z - fCirDiff; Quad.verts[2].x = Light.GetPos().x - fCirDiff2; Quad.verts[2].y = Light.GetPos().y - fCirDiff; Quad.verts[2].z = Light.GetPos().z - fCirDiff2; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y - fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.rgba.a = 0x10; Quad.rgba.r = (uint8)Light.GetColor().x; Quad.rgba.g = (uint8)Light.GetColor().y; Quad.rgba.b = MIN(0xFF,(uint8)((float)Light.GetColor().z * 1.2f)); 
			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y;Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x - fCirDiff; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z + fCirDiff; Quad.verts[2].x = Light.GetPos().x - fCirDiff2; Quad.verts[2].y = Light.GetPos().y + fCirDiff; Quad.verts[2].z = Light.GetPos().z + fCirDiff2; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y + fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y;Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x + fCirDiff; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z - fCirDiff; Quad.verts[2].x = Light.GetPos().x + fCirDiff2; Quad.verts[2].y = Light.GetPos().y + fCirDiff; Quad.verts[2].z = Light.GetPos().z - fCirDiff2; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y + fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y;Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x - fCirDiff; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z + fCirDiff; Quad.verts[2].x = Light.GetPos().x - fCirDiff2; Quad.verts[2].y = Light.GetPos().y - fCirDiff; Quad.verts[2].z = Light.GetPos().z + fCirDiff2; 	Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y - fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1);

			Quad.verts[0].x = Light.GetPos().x; Quad.verts[0].y = Light.GetPos().y;Quad.verts[0].z = Light.GetPos().z; Quad.verts[1].x = Light.GetPos().x + fCirDiff; Quad.verts[1].y = Light.GetPos().y; Quad.verts[1].z = Light.GetPos().z - fCirDiff; Quad.verts[2].x = Light.GetPos().x + fCirDiff2; Quad.verts[2].y = Light.GetPos().y - fCirDiff; Quad.verts[2].z = Light.GetPos().z - fCirDiff2; Quad.verts[3].x = Light.GetPos().x; Quad.verts[3].y = Light.GetPos().y - fLightRadius; Quad.verts[3].z = Light.GetPos().z;
			g_pILTDrawPrimInternal->DrawPrim(&Quad,1); 
		} 
	}
}

//draws the skeleton of a model
void NModelDebug::DrawModelSkeleton(ModelInstance* pInstance)
{
	//setup the draw primitive for line rendering
	if (!g_pILTDrawPrimInternal) 
		return; 

	g_pILTDrawPrimInternal->SetTexture(NULL);
	g_pILTDrawPrimInternal->SetTransformType(DRAWPRIM_TRANSFORM_WORLD);
	g_pILTDrawPrimInternal->SetColorOp(DRAWPRIM_NOCOLOROP);
	g_pILTDrawPrimInternal->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
	g_pILTDrawPrimInternal->SetZBufferMode(DRAWPRIM_ZRW);
	g_pILTDrawPrimInternal->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pILTDrawPrimInternal->SetClipMode(DRAWPRIM_FASTCLIP);

	//figure out how long they want each axis to be from the console variable (ie typing 10
	//will make all axis 10 units long)
	float fAxisLength = LTMAX((float)g_CV_ModelDebug_DrawSkeleton.m_Val, 1.0f);

	//cache the model pointer
	Model* pModel = pInstance->GetModelDB();

	ModelDraw model_draw;
	float fDistToModel = 0.0f;
	fDistToModel = (g_ViewParams.m_Pos - pInstance->GetPos()).Mag();

	// first see which nodes we are being used by the geometry.
	for (uint32 nCurrPiece = 0; nCurrPiece < pModel->NumPieces(); nCurrPiece++) 
	{
		ModelPiece* pPiece			= pModel->GetPiece(nCurrPiece);
		CDIModelDrawable* pLOD      = pPiece->GetLODFromDist( g_CV_ModelLODOffset.m_Val, fDistToModel );	
	
		//if it doesn't have an associated LOD to render, we can't do anything
		if (!pLOD) 
			continue;
		pInstance->SetupLODNodePath(pLOD);
	}

	//the current node transform
	LTMatrix mTrans;

	//the transform of the specified child
	LTMatrix mChildTrans;

	//we just need to run through each node, and for each node render the basis space, as well
	//as a link from the node to all of its children
	for(uint32 nCurrNode = 0; nCurrNode < pModel->NumNodes(); nCurrNode++)
	{
		//figure out which transform we will be using
		if(!pInstance->GetCachedTransform(nCurrNode, mTrans))
			continue;

		//we need to draw the basis space using ModelEdit color scheme (x=r) (y=g) (z=b)
		LTVector vTransOrigin;
		mTrans.GetTranslation(vTransOrigin);

		//find out where the axis each go
		LTVector vAxis[3];
		vAxis[0] = LTVector(1.0f, 0, 0);
		vAxis[1] = LTVector(0, 1.0f, 0);
		vAxis[2] = LTVector(0, 0, 1.0f);

		//now render away
		for(uint32 nCurrAxis = 0; nCurrAxis < 3; nCurrAxis++)
		{
			LT_LINEF Line;


			//setup the line color
			Line.rgba.r = (uint8)(vAxis[nCurrAxis].x * 255.0f);
			Line.rgba.g = (uint8)(vAxis[nCurrAxis].y * 255.0f);
			Line.rgba.b = (uint8)(vAxis[nCurrAxis].z * 255.0f);
			Line.rgba.a = 255;

			//now the positions
			LTVector vTransAxis = mTrans * (vAxis[nCurrAxis] * fAxisLength);
			Line.verts[0].x = vTransAxis.x;
			Line.verts[0].y = vTransAxis.y;
			Line.verts[0].z = vTransAxis.z;

			Line.verts[1].x = vTransOrigin.x;
			Line.verts[1].y = vTransOrigin.y;
			Line.verts[1].z = vTransOrigin.z;

			//and render that line
			g_pILTDrawPrimInternal->DrawPrim(&Line, 1);
		}

		//now we need to draw a link to all of its children, with white fading to black from
		//parent to child, this way it is always visible and the relation can be established
		ModelNode* pNode = pModel->GetNode(nCurrNode);
		assert(pNode);

		for(uint32 nCurrChild = 0; nCurrChild < pNode->NumChildren(); nCurrChild++)
		{
			//now figure out the child node
			ModelNode* pChild = pNode->GetChild(nCurrChild);
			assert(pChild);

			//and figure out which transfor it maps to
			if(!pInstance->GetCachedTransform(pChild->GetNodeIndex(), mChildTrans))
				continue;

			LT_LINEG Line;

			//setup the parent
			Line.verts[0].x = vTransOrigin.x;
			Line.verts[0].y = vTransOrigin.y;
			Line.verts[0].z = vTransOrigin.z;
			Line.verts[0].rgba.r = 255;
			Line.verts[0].rgba.g = 255;
			Line.verts[0].rgba.b = 255;
			Line.verts[0].rgba.a = 255;

			//extract out the position from that matrix
			LTVector vChildPos;
			mChildTrans.GetTranslation(vChildPos);

			//and now setup the vertex
			Line.verts[1].x = vChildPos.x;
			Line.verts[1].y = vChildPos.y;
			Line.verts[1].z = vChildPos.z;
			Line.verts[1].rgba.r = 0;
			Line.verts[1].rgba.g = 0;
			Line.verts[1].rgba.b = 0;
			Line.verts[1].rgba.a = 255;

			//and render that line
			g_pILTDrawPrimInternal->DrawPrim(&Line, 1);
		}
	}
}

//draws the vertex normals of a model
void NModelDebug::DrawModelVertexNormals(ModelInstance* pInstance)
{
	g_pILTDrawPrimInternal->SetTexture(NULL);
	g_pILTDrawPrimInternal->SetTransformType(DRAWPRIM_TRANSFORM_WORLD);
	g_pILTDrawPrimInternal->SetColorOp(DRAWPRIM_NOCOLOROP);
	g_pILTDrawPrimInternal->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
	g_pILTDrawPrimInternal->SetZBufferMode(DRAWPRIM_ZRW);
	g_pILTDrawPrimInternal->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pILTDrawPrimInternal->SetClipMode(DRAWPRIM_FASTCLIP);

	//cache the model pointer
	Model* pModel = pInstance->GetModelDB();

	ModelDraw model_draw;
	float fDistToModel = 0.0f;
	fDistToModel = (g_ViewParams.m_Pos - pInstance->GetPos()).Mag();

	// first see which nodes we are being used by the geometry.
	for (uint32 nCurrPiece = 0; nCurrPiece < pModel->NumPieces(); nCurrPiece++) 
	{
		ModelPiece* pPiece			= pModel->GetPiece(nCurrPiece);
		CDIModelDrawable* pLOD      = pPiece->GetLODFromDist( g_CV_ModelLODOffset.m_Val, fDistToModel );	

		switch (pLOD->GetType())
		{
		case CRenderObject::eSkelMesh :
		case CRenderObject::eRigidMesh :
			{
				CRenderStyle* pRenderStyle = NULL;
				pInstance->GetRenderStyle(pPiece->m_iRenderStyle, &pRenderStyle);
//pInstance->m_Rotation
				if (pRenderStyle)
				{
					RSD3DOptions options;
					pRenderStyle->GetDirect3D_Options(&options);
					if(options.bUseEffectShader)
					{
						LTEffectImpl* pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(options.EffectShaderID);
						IDirect3DVertexDeclaration9* pDecl = pEffect->GetVertexDeclaration();
						if(pDecl)
						{
							D3DVERTEXELEMENT9 element[256];
							UINT nElements = 0;
							if(!FAILED(pDecl->GetDeclaration(element, &nElements)))
							{
								int nBytesToNormal = 0;

								bool bTangent = false;
								int nBytesToTangent = 0;
								bool bBinormal = false;
								int nBytesToBinormal = 0;
								for(int n = 0; n < nElements; ++n)
								{
									if(element[n].Usage == D3DDECLUSAGE_NORMAL)
									{
										nBytesToNormal = element[n].Offset;
									}

									if(element[n].Usage == D3DDECLUSAGE_TANGENT)
									{
										nBytesToTangent = element[n].Offset;
										bTangent = true;
									}

									if(element[n].Usage == D3DDECLUSAGE_BINORMAL)
									{
										nBytesToBinormal = element[n].Offset;
										bBinormal = true;
									}
								}

								UINT nVertexSize = D3DXGetDeclVertexSize(element, 0);
								//dsi_ConsolePrint("Vert Size: %d", nVertexSize);								

								uint32 nVertCount =  0; 
								uint8* pVertexData = NULL;
								
								if(pLOD->GetType() == CRenderObject::eSkelMesh)
								{
									nVertCount = ((CD3DSkelMesh*)pLOD)->GetVertexCount();
									pVertexData = (uint8*)((CD3DSkelMesh*)pLOD)->GetVertexData();
								}else 
								if(pLOD->GetType() == CRenderObject::eRigidMesh)
								{
									nVertCount = ((CD3DRigidMesh*)pLOD)->GetVertexCount();
									pVertexData = (uint8*)((CD3DRigidMesh*)pLOD)->GetVertexData();
								}

								if(pVertexData)
								{
									float fVerts[3];
									float fNorms[3];
									float fTangent[3];
									float fBinormal[3];

									//++nVertexSize
									for(int i = 0; i < nVertCount; ++i)
									{					
										memcpy((void*)fVerts, &pVertexData[(i * nVertexSize)], sizeof(float) * 3);
										memcpy((void*)fNorms, &pVertexData[(i * nVertexSize) + nBytesToNormal], sizeof(float) * 3);

										if(bTangent)
										{
											memcpy((void*)fTangent, &pVertexData[(i * nVertexSize) + nBytesToTangent], sizeof(float) * 3);
										}

										if(bBinormal)
										{
											memcpy((void*)fBinormal, &pVertexData[(i * nVertexSize) + nBytesToBinormal], sizeof(float) * 3);
										}

										// Apply the model's rotation to these normals/tangents/binormals
										{
										
											LTMatrix mMat;
											mMat.Identity();

											if(bTangent)
											{											
												mMat.m[0][0] = fTangent[0];
												mMat.m[1][0] = fTangent[1];
												mMat.m[2][0] = fTangent[2];
											}

											if(bBinormal)
											{
												mMat.m[0][1] = fBinormal[0];
												mMat.m[1][1] = fBinormal[1];
												mMat.m[2][1] = fBinormal[2];
											}

											// The normal is implied
											mMat.m[0][2] = fNorms[0];
											mMat.m[1][2] = fNorms[1];
											mMat.m[2][2] = fNorms[2];

											LTMatrix mRot;
											pInstance->m_Rotation.ConvertToMatrix(mRot);
											
											//Apply the rotation to these normals
											mRot.Apply(mMat);

											if(bTangent)
											{	
												fTangent[0] = mMat.m[0][0];
												fTangent[1] = mMat.m[1][0];
												fTangent[2] = mMat.m[2][0];
											}

											if(bBinormal)
											{
												fBinormal[0] = mMat.m[0][1];
												fBinormal[1] = mMat.m[1][1];
												fBinormal[2] = mMat.m[2][1];
											}

											// The normal is implied.
											fNorms[0] = mMat.m[0][2];
											fNorms[1] = mMat.m[1][2];
											fNorms[2] = mMat.m[2][2];

											// Transform the vert position as well
											LTVector vVertPos(fVerts[0], fVerts[1], fVerts[2]);
											mRot.Apply3x3(vVertPos);
											fVerts[0] = vVertPos.x;
											fVerts[1] = vVertPos.y;
											fVerts[2] = vVertPos.z;
										}

										//dsi_ConsolePrint("[%d] Vert(%.6f %.6f %.6f) Norm(%.6f %.6f %.6f)", i, fVerts[0], fVerts[1], fVerts[2], fNorms[0], fNorms[1], fNorms[2]);
										LTVector vPos = pInstance->GetPos();

										LTVector vVertPos(fVerts[0], fVerts[1], fVerts[2]);
										LTVector vVertNormal(fNorms[0], fNorms[1], fNorms[2]);

										float fNormalScale = 5.0f;

										// NORMAL
										{
											LT_LINEG Line;

											//now the positions
											LTVector vFullPos = vPos + vVertPos;
											Line.verts[0].x = vFullPos.x;
											Line.verts[0].y = vFullPos.y;
											Line.verts[0].z = vFullPos.z;
											//setup the line color
											Line.verts[0].rgba.r = 0;
											Line.verts[0].rgba.g = 255;
											Line.verts[0].rgba.b = 0;
											Line.verts[0].rgba.a = 255;

											vFullPos += (vVertNormal * fNormalScale);
											Line.verts[1].x = vFullPos.x;
											Line.verts[1].y = vFullPos.y;
											Line.verts[1].z = vFullPos.z;
											//setup the line color
											Line.verts[1].rgba.r = 0;
											Line.verts[1].rgba.g = 0;
											Line.verts[1].rgba.b = 255;
											Line.verts[1].rgba.a = 255;

											//and render that line
											g_pILTDrawPrimInternal->DrawPrim(&Line, 1);
										}

										// TANGENT
										if(bTangent)
										{
											LTVector vVertTangent(fTangent[0], fTangent[1], fTangent[2]);

											LT_LINEG Line;

											//now the positions
											LTVector vFullPos = vPos + vVertPos;
											Line.verts[0].x = vFullPos.x;
											Line.verts[0].y = vFullPos.y;
											Line.verts[0].z = vFullPos.z;
											//setup the line color
											Line.verts[0].rgba.r = 0;
											Line.verts[0].rgba.g = 255;
											Line.verts[0].rgba.b = 0;
											Line.verts[0].rgba.a = 255;

											vFullPos += (vVertTangent * fNormalScale);
											Line.verts[1].x = vFullPos.x;
											Line.verts[1].y = vFullPos.y;
											Line.verts[1].z = vFullPos.z;
											//setup the line color
											Line.verts[1].rgba.r = 255;
											Line.verts[1].rgba.g = 255;
											Line.verts[1].rgba.b = 0;
											Line.verts[1].rgba.a = 255;

											//and render that line
											g_pILTDrawPrimInternal->DrawPrim(&Line, 1);
										}

										if(bBinormal)
										{
												//LTVector vVertBinormal = vVertNormal.Cross(vVertTangent);
												LTVector vVertBinormal(fBinormal[0], fBinormal[1], fBinormal[2]);

												LT_LINEG Line;

												//now the positions
												LTVector vFullPos = vPos + vVertPos;
												Line.verts[0].x = vFullPos.x;
												Line.verts[0].y = vFullPos.y;
												Line.verts[0].z = vFullPos.z;
												//setup the line color
												Line.verts[0].rgba.r = 0;
												Line.verts[0].rgba.g = 255;
												Line.verts[0].rgba.b = 0;
												Line.verts[0].rgba.a = 255;

												vFullPos += (vVertBinormal * fNormalScale);
												Line.verts[1].x = vFullPos.x;
												Line.verts[1].y = vFullPos.y;
												Line.verts[1].z = vFullPos.z;
												//setup the line color
												Line.verts[1].rgba.r = 255;
												Line.verts[1].rgba.g = 0;
												Line.verts[1].rgba.b = 0;
												Line.verts[1].rgba.a = 255;

												//and render that line
												g_pILTDrawPrimInternal->DrawPrim(&Line, 1);
										}
									}

								}
								((CD3DSkelMesh*)pLOD)->ReleaseVertexData();
								if(pLOD->GetType() == CRenderObject::eSkelMesh)
								{
									((CD3DSkelMesh*)pLOD)->ReleaseVertexData();
								}else 
								if(pLOD->GetType() == CRenderObject::eRigidMesh)
								{
									((CD3DRigidMesh*)pLOD)->ReleaseVertexData();
								}

							}
						}
					}
				}
			}
			break;
		default:
			break;
		}

		//if it doesn't have an associated LOD to render, we can't do anything
		if (!pLOD) 
			continue;
		pInstance->SetupLODNodePath(pLOD);
	}
}

static inline void draw_obb( const LTMatrix &mat, const LTVector &size )
{
	LTVector vmin(-.5f,-.5f,-.5f),vmax(.5f,.5f,.5f) ;
	LTVector vminres, vmaxres, tmpa,tmpb ;
	uint32  color = 0x00ff00ff ; // green

	vmin = vmin * size ; vmax = vmax * size ;

	LT_LINEF Line;

	//setup the line color
	Line.rgba.r = 0;
	Line.rgba.g = 255;
	Line.rgba.b = 0;
	Line.rgba.a = 255;

	//now the positions
	vminres = vmin ;
	vmaxres = vmax ;

	// top
	tmpa.x = vmin.x;
	tmpa.y = vmin.y;
	tmpa.z = vmin.z;

	tmpb.x = vmax.x;
	tmpb.y = vmin.y;
	tmpb.z = vmin.z;
	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );


	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);

	tmpa.x = vmin.x;
	tmpa.y = vmin.y;
	tmpa.z = vmin.z;

	tmpb.x = vmax.x;
	tmpb.y = vmin.y;
	tmpb.z = vmin.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );


	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);


	tmpa.x = vmax.x;
	tmpa.y = vmin.y;
	tmpa.z = vmin.z;

	tmpb.x = vmax.x;
	tmpb.y = vmin.y;
	tmpb.z = vmax.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);


	tmpa.x = vmax.x;
	tmpa.y = vmin.y;
	tmpa.z = vmax.z;

	tmpb.x = vmin.x;
	tmpb.y = vmin.y;
	tmpb.z = vmax.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);

	tmpa.x = vmin.x;
	tmpa.y = vmin.y;
	tmpa.z = vmax.z;

	tmpb.x = vmin.x;
	tmpb.y = vmin.y;
	tmpb.z = vmin.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);

	// bottom
	tmpa.x = vmax.x;
	tmpa.y = vmax.y;
	tmpa.z = vmax.z;

	tmpb.x = vmin.x;
	tmpb.y = vmax.y;
	tmpb.z = vmax.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);

	tmpa.x = vmin.x;
	tmpa.y = vmax.y;
	tmpa.z = vmax.z;

	tmpb.x = vmin.x;
	tmpb.y = vmax.y;
	tmpb.z = vmin.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);

	tmpa.x = vmin.x;
	tmpa.y = vmax.y;
	tmpa.z = vmin.z;

	tmpb.x = vmax.x;
	tmpb.y = vmax.y;
	tmpb.z = vmin.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);

	tmpa.x = vmax.x;
	tmpa.y = vmax.y;
	tmpa.z = vmin.z;

	tmpb.x = vmax.x;
	tmpb.y = vmax.y;
	tmpb.z = vmax.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);

	// sides
	tmpa.x = vmin.x;
	tmpa.y = vmin.y;
	tmpa.z = vmin.z;

	tmpb.x = vmin.x;
	tmpb.y = vmax.y;
	tmpb.z = vmin.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);

	tmpa.x = vmax.x;
	tmpa.y = vmin.y;
	tmpa.z = vmin.z;

	tmpb.x = vmax.x;
	tmpb.y = vmax.y;
	tmpb.z = vmin.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);

	tmpa.x = vmax.x;
	tmpa.y = vmin.y;
	tmpa.z = vmax.z;

	tmpb.x = vmax.x;
	tmpb.y = vmax.y;
	tmpb.z = vmax.z;

	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);


	tmpa.x = vmin.x;
	tmpa.y = vmin.y;
	tmpa.z = vmax.z;

	tmpb.x = vmin.x;
	tmpb.y = vmax.y;
	tmpb.z = vmax.z;
	mat.Apply(tmpa, vminres);
	mat.Apply(tmpb, vmaxres);

	memcpy( &Line.verts[0], &vminres, sizeof(float) *3 );
	memcpy( &Line.verts[1], &vmaxres, sizeof(float) *3 );

	g_pILTDrawPrimInternal->DrawPrim(&Line, 1);
}


//draws the obbs of a model
void NModelDebug::DrawModelOBBS( ModelInstance * pInstance )
{
	//setup the draw primitive for line rendering
	if (!g_pILTDrawPrimInternal) 
		return; 

	// check if model has obbs.
	const uint32 num_obb  = pInstance->NumCollisionObjects();

	if( num_obb <= 0)
		return ;

	// set to plain vanilla rendering.
	g_pILTDrawPrimInternal->SetTexture(NULL);
	g_pILTDrawPrimInternal->SetTransformType(DRAWPRIM_TRANSFORM_WORLD);
	g_pILTDrawPrimInternal->SetColorOp(DRAWPRIM_NOCOLOROP);
	g_pILTDrawPrimInternal->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
	g_pILTDrawPrimInternal->SetZBufferMode(DRAWPRIM_ZRW);
	g_pILTDrawPrimInternal->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pILTDrawPrimInternal->SetClipMode(DRAWPRIM_FASTCLIP);

	// draw the obbs. 
	LTMatrix obb_mat ;
	ModelOBB obb ;

	// iterate through available obbs and draw them.
	for( uint32 obb_cnt = 0; obb_cnt < num_obb ; obb_cnt++ )
	{
		pInstance->UpdateCollisionObject( obb_cnt,  obb );

		// setup the coordinate frame of the obb.
		obb_mat.Identity();
		obb_mat.SetBasisVectors( &obb.m_Basis[0],
			&obb.m_Basis[1],
			&obb.m_Basis[2]);
		obb_mat.SetTranslation( obb.m_Pos );

		draw_obb(obb_mat, obb.m_Size);
	}
}
