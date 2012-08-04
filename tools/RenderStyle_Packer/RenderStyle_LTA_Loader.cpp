// RenderStyle_LTA.cpp
//

#include <assert.h>
#include <d3dx9.h>
#include <ltinteger.h>

#include "d3d_renderstyle.h"
#include "ltamgr.h"
#include "fstream"
#include "Utilities.h"

// ------------------------------------------------------------------------
// LoadRenderStyle( CRenderStyle , CLTANode )
// converts lta rs into in-core rs
// ------------------------------------------------------------------------
bool LoadRenderStyle( CRenderStyle* pRenderStyle, CLTANode *RootNode )

{
	// transfer from lta nodes
		CLTANode* pRenderStyleP  = RootNode;
		const char* szRenderStyleName = NULL;

		if (pRenderStyleP->GetElement(1)) szRenderStyleName = pRenderStyleP->GetElement(1)->GetValue();
	const char* szParam1; //const char* szParam2; const char* szParam3; const char* szParam4;

	// Lighting Material...
	CLTANode* pLightMaterial = CLTAUtil::ShallowFindList(pRenderStyleP,"lightmaterial");
	if (pLightMaterial) {
		LightingMaterial lightMaterial;
		CLTANode* pAmbient = CLTAUtil::ShallowFindList(pLightMaterial,"ambient");
		if (pAmbient) {
			if (pAmbient->GetElement(1) && (szParam1 = pAmbient->GetElement(1)->GetValue()))		{ lightMaterial.Ambient.r = (float)atof(szParam1); }
			if (pAmbient->GetElement(2) && (szParam1 = pAmbient->GetElement(2)->GetValue()))		{ lightMaterial.Ambient.g = (float)atof(szParam1); }
			if (pAmbient->GetElement(3) && (szParam1 = pAmbient->GetElement(3)->GetValue()))		{ lightMaterial.Ambient.b = (float)atof(szParam1); }
			if (pAmbient->GetElement(4) && (szParam1 = pAmbient->GetElement(4)->GetValue()))		{ lightMaterial.Ambient.a = (float)atof(szParam1); } }
		CLTANode* pDiffuse = CLTAUtil::ShallowFindList(pLightMaterial,"diffuse");
		if (pDiffuse) {
			if (pDiffuse->GetElement(1) && (szParam1 = pDiffuse->GetElement(1)->GetValue()))		{ lightMaterial.Diffuse.r = (float)atof(szParam1); }
			if (pDiffuse->GetElement(2) && (szParam1 = pDiffuse->GetElement(2)->GetValue()))		{ lightMaterial.Diffuse.g = (float)atof(szParam1); }
			if (pDiffuse->GetElement(3) && (szParam1 = pDiffuse->GetElement(3)->GetValue()))		{ lightMaterial.Diffuse.b = (float)atof(szParam1); }
			if (pDiffuse->GetElement(4) && (szParam1 = pDiffuse->GetElement(4)->GetValue()))		{ lightMaterial.Diffuse.a = (float)atof(szParam1); } }
		CLTANode* pSpecular = CLTAUtil::ShallowFindList(pLightMaterial,"specular");
		if (pSpecular) {
			if (pSpecular->GetElement(1) && (szParam1 = pSpecular->GetElement(1)->GetValue()))	{ lightMaterial.Specular.r = (float)atof(szParam1); }
			if (pSpecular->GetElement(2) && (szParam1 = pSpecular->GetElement(2)->GetValue()))	{ lightMaterial.Specular.g = (float)atof(szParam1); }
			if (pSpecular->GetElement(3) && (szParam1 = pSpecular->GetElement(3)->GetValue()))	{ lightMaterial.Specular.b = (float)atof(szParam1); }
			if (pSpecular->GetElement(4) && (szParam1 = pSpecular->GetElement(4)->GetValue()))	{ lightMaterial.Specular.a = (float)atof(szParam1); } }
		CLTANode* pEmissive = CLTAUtil::ShallowFindList(pLightMaterial,"emissive");
		if (pEmissive) {
			if (pEmissive->GetElement(1) && (szParam1 = pEmissive->GetElement(1)->GetValue()))	{ lightMaterial.Emissive.r = (float)atof(szParam1); }
			if (pEmissive->GetElement(2) && (szParam1 = pEmissive->GetElement(2)->GetValue()))	{ lightMaterial.Emissive.g = (float)atof(szParam1); }
			if (pEmissive->GetElement(3) && (szParam1 = pEmissive->GetElement(3)->GetValue()))	{ lightMaterial.Emissive.b = (float)atof(szParam1); }
			if (pEmissive->GetElement(4) && (szParam1 = pEmissive->GetElement(4)->GetValue()))	{ lightMaterial.Emissive.a = (float)atof(szParam1); } }
		CLTANode* pSpecPower = CLTAUtil::ShallowFindList(pLightMaterial,"specularpower");
		if (pSpecPower) {
			if (pSpecPower->GetElement(1) && (szParam1 = pSpecPower->GetElement(1)->GetValue()))	{ lightMaterial.SpecularPower = (float)atof(szParam1); } }
		pRenderStyle->SetLightingMaterial(lightMaterial); }

	// Render Passes...
	while (pRenderStyle->GetRenderPassCount()) { pRenderStyle->RemoveRenderPass(0); }
	vector<CLTANode*> RenderRenderPasses;
	CLTAUtil::FindAll(RenderRenderPasses,pRenderStyleP,"renderpass");
	for (uint32 i = 0; i < RenderRenderPasses.size(); ++i)
	{
		RenderPassOp RenderPass;
		CLTANode* pTestMode = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"testmode");
		if (pTestMode && pTestMode->GetElement(1) && (szParam1 = pTestMode->GetElement(1)->GetValue())) {
			if (stricmp(szParam1,"RENDERSTYLE_NOALPHATEST")==0)								{ RenderPass.AlphaTestMode = RENDERSTYLE_NOALPHATEST; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_LESS")==0)						{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_LESS; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_LESSEQUAL")==0)				{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_LESSEQUAL; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_GREATER")==0)					{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_GREATER; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_GREATEREQUAL")==0)				{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_GREATEREQUAL; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_EQUAL")==0)					{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_EQUAL; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_NOTEQUAL")==0)					{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_NOTEQUAL; }
			else { RenderPass.AlphaTestMode = RENDERSTYLE_NOALPHATEST; OutputMsg("Warning: Unknown param %s",szParam1); } }
		else { RenderPass.AlphaTestMode = RENDERSTYLE_NOALPHATEST; }

		CLTANode* pZTestMode = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"ztestmode");
		if (pZTestMode && pZTestMode->GetElement(1) && (szParam1 = pZTestMode->GetElement(1)->GetValue())) {
			if (stricmp(szParam1,"RENDERSTYLE_NOALPHATEST")==0)								{ RenderPass.ZBufferTestMode = RENDERSTYLE_NOALPHATEST; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_LESS")==0)						{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_LESS; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_LESSEQUAL")==0)				{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_LESSEQUAL; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_GREATER")==0)					{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_GREATER; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_GREATEREQUAL")==0)				{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_GREATEREQUAL; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_EQUAL")==0)					{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_EQUAL; }
			else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_NOTEQUAL")==0)					{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_NOTEQUAL; }
			else { RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_LESSEQUAL; OutputMsg("Warning: Unknown param %s",szParam1); } }
		else { RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_LESSEQUAL; }

		CLTANode* pFillMode = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"fillmode");
		if (pFillMode && pFillMode->GetElement(1) && (szParam1 = pFillMode->GetElement(1)->GetValue())) {
			if (stricmp(szParam1,"RENDERSTYLE_WIRE")==0)									{ RenderPass.FillMode = RENDERSTYLE_WIRE; }
			else if (stricmp(szParam1,"RENDERSTYLE_FILL")==0)								{ RenderPass.FillMode = RENDERSTYLE_FILL; }
			else { RenderPass.FillMode = RENDERSTYLE_FILL; OutputMsg("Warning: Unknown param %s",szParam1); } }
		else { RenderPass.FillMode = RENDERSTYLE_FILL; }

		CLTANode* pBlendOp = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"blendop");
		if (pBlendOp && pBlendOp->GetElement(1) && (szParam1 = pBlendOp->GetElement(1)->GetValue())) {
			if (stricmp(szParam1,"RENDERSTYLE_NOBLEND")==0)									{ RenderPass.BlendMode = RENDERSTYLE_NOBLEND; }
			else if (stricmp(szParam1,"RENDERSTYLE_BLEND_ADD")==0)							{ RenderPass.BlendMode = RENDERSTYLE_BLEND_ADD; }
			else if (stricmp(szParam1,"RENDERSTYLE_BLEND_SATURATE")==0)						{ RenderPass.BlendMode = RENDERSTYLE_BLEND_SATURATE; }
			else if (stricmp(szParam1,"RENDERSTYLE_BLEND_MOD_SRCALPHA")==0)					{ RenderPass.BlendMode = RENDERSTYLE_BLEND_MOD_SRCALPHA; }
			else if (stricmp(szParam1,"RENDERSTYLE_BLEND_MOD_SRCCOLOR")==0)					{ RenderPass.BlendMode = RENDERSTYLE_BLEND_MOD_SRCCOLOR; }
			else if (stricmp(szParam1,"RENDERSTYLE_BLEND_MOD_DSTCOLOR")==0)					{ RenderPass.BlendMode = RENDERSTYLE_BLEND_MOD_DSTCOLOR; }
			else if (stricmp(szParam1,"RENDERSTYLE_BLEND_MUL_SRCCOL_DSTCOL")==0)			{ RenderPass.BlendMode = RENDERSTYLE_BLEND_MUL_SRCCOL_DSTCOL; }
			else if (stricmp(szParam1,"RENDERSTYLE_BLEND_MUL_SRCCOL_ONE")==0)				{ RenderPass.BlendMode = RENDERSTYLE_BLEND_MUL_SRCCOL_ONE; }
			else if (stricmp(szParam1,"RENDERSTYLE_BLEND_MUL_SRCALPHA_ZERO")==0)			{ RenderPass.BlendMode = RENDERSTYLE_BLEND_MUL_SRCALPHA_ZERO; }
			else if (stricmp(szParam1,"RENDERSTYLE_BLEND_MUL_SRCALPHA_ONE")==0)				{ RenderPass.BlendMode = RENDERSTYLE_BLEND_MUL_SRCALPHA_ONE; }
			else if (stricmp(szParam1,"RENDERSTYLE_BLEND_MUL_DSTCOL_ZERO")==0)				{ RenderPass.BlendMode = RENDERSTYLE_BLEND_MUL_DSTCOL_ZERO; }
			else OutputMsg("Warning: Unknown param %s",szParam1); }

		CLTANode* pZBufferMode = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"zbuffermode");
		if (pZBufferMode && pZBufferMode->GetElement(1) && (szParam1 = pZBufferMode->GetElement(1)->GetValue())) {
			if (stricmp(szParam1,"RENDERSTYLE_ZRW")==0)										{ RenderPass.ZBufferMode = RENDERSTYLE_ZRW; }
			else if (stricmp(szParam1,"RENDERSTYLE_ZRO")==0)								{ RenderPass.ZBufferMode = RENDERSTYLE_ZRO; }
			else if (stricmp(szParam1,"RENDERSTYLE_NOZ")==0)								{ RenderPass.ZBufferMode = RENDERSTYLE_NOZ; }
			else { RenderPass.ZBufferMode = RENDERSTYLE_ZRW; OutputMsg("Warning: Unknown param %s",szParam1); } }
		else { RenderPass.ZBufferMode = RENDERSTYLE_ZRW; }

		CLTANode* pCullMode = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"cullmode");
		if (pCullMode && pCullMode->GetElement(1) && (szParam1 = pCullMode->GetElement(1)->GetValue())) {
			if (stricmp(szParam1,"RENDERSTYLE_CULL_NONE")==0)								{ RenderPass.CullMode = RENDERSTYLE_CULL_NONE; }
			else if (stricmp(szParam1,"RENDERSTYLE_CULL_CCW")==0)							{ RenderPass.CullMode = RENDERSTYLE_CULL_CCW; }
			else if (stricmp(szParam1,"RENDERSTYLE_CULL_CW")==0)							{ RenderPass.CullMode = RENDERSTYLE_CULL_CW; }
			else { RenderPass.CullMode = RENDERSTYLE_CULL_CCW; OutputMsg("Warning: Unknown param %s",szParam1); } }
		else { RenderPass.CullMode = RENDERSTYLE_CULL_CCW; }

		CLTANode* pTextureFactor = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"texturefactor");
		if (pTextureFactor && pTextureFactor->GetElement(1) && (szParam1 = pTextureFactor->GetElement(1)->GetValue())) {
			RenderPass.TextureFactor = atoi(szParam1); }
		else { RenderPass.TextureFactor = 0x80808080; }

		CLTANode* pAlphaRef = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"alpharef");
		if (pAlphaRef && pAlphaRef->GetElement(1) && (szParam1 = pAlphaRef->GetElement(1)->GetValue())) {
			RenderPass.AlphaRef = atoi(szParam1); }
		else { RenderPass.AlphaRef = 128; }

		CLTANode* pDynamicLight = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"dynamiclight");
		if (pDynamicLight && pDynamicLight->GetElement(1) && (szParam1 = pDynamicLight->GetElement(1)->GetValue())) {
			if (stricmp(szParam1,"1")==0)													{ RenderPass.DynamicLight = true; }
			else if (stricmp(szParam1,"0")==0)												{ RenderPass.DynamicLight = false; }
			else { RenderPass.DynamicLight = true; OutputMsg("Warning: Unknown param %s",szParam1); } }
		else { RenderPass.DynamicLight = true; }

		CLTANode* pUseBumpEnvMap = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"usebumpenvmap");
		if (pUseBumpEnvMap && pUseBumpEnvMap->GetElement(1) && (szParam1 = pUseBumpEnvMap->GetElement(1)->GetValue())) {
			if (stricmp(szParam1,"1")==0)													{ RenderPass.bUseBumpEnvMap = true; }
			else if (stricmp(szParam1,"0")==0)												{ RenderPass.bUseBumpEnvMap = false; }
			else { RenderPass.bUseBumpEnvMap = false; OutputMsg("Warning: Unknown param %s",szParam1); } }
		else { RenderPass.bUseBumpEnvMap = false; }

		CLTANode* pBumpEnvStage = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"bumpenvmapstage");
		if (pBumpEnvStage && pBumpEnvStage->GetElement(1) && (szParam1 = pBumpEnvStage->GetElement(1)->GetValue())) {
			RenderPass.BumpEnvMapStage = atoi(szParam1); }
		else { RenderPass.BumpEnvMapStage = 1; }

		CLTANode* pBumpEnvScale = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"bumpenvmapscale");
		if (pBumpEnvScale && pBumpEnvScale->GetElement(1) && (szParam1 = pBumpEnvScale->GetElement(1)->GetValue())) {
			RenderPass.fBumpEnvMap_Scale = (float)atof(szParam1); }
		else { RenderPass.fBumpEnvMap_Scale = 1.0f; }

		CLTANode* pBumpEnvOffset = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"bumpenvmapoffset");
		if (pBumpEnvOffset && pBumpEnvOffset->GetElement(1) && (szParam1 = pBumpEnvOffset->GetElement(1)->GetValue())) {
			RenderPass.fBumpEnvMap_Offset = (float)atof(szParam1); }
		else { RenderPass.fBumpEnvMap_Offset = 0.0f; }

		vector<CLTANode*> TextureStages;
		CLTAUtil::FindAll(TextureStages,RenderRenderPasses[i],"texturestageops");
		for (uint32 j = 0; j < TextureStages.size(); ++j) {
			assert(j < 4); if (j >= 4) continue;
			CLTANode* pTextureParam = CLTAUtil::ShallowFindList(TextureStages[j],"textureparam");
			if (pTextureParam && pTextureParam->GetElement(1) && (szParam1 = pTextureParam->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_NOTEXTURE")==0)							{ RenderPass.TextureStages[j].TextureParam = RENDERSTYLE_NOTEXTURE; }
				else if (stricmp(szParam1,"RENDERSTYLE_USE_TEXTURE1")==0)					{ RenderPass.TextureStages[j].TextureParam = RENDERSTYLE_USE_TEXTURE1; }
				else if (stricmp(szParam1,"RENDERSTYLE_USE_TEXTURE2")==0)					{ RenderPass.TextureStages[j].TextureParam = RENDERSTYLE_USE_TEXTURE2; }
				else if (stricmp(szParam1,"RENDERSTYLE_USE_TEXTURE3")==0)					{ RenderPass.TextureStages[j].TextureParam = RENDERSTYLE_USE_TEXTURE3; }
				else if (stricmp(szParam1,"RENDERSTYLE_USE_TEXTURE4")==0)					{ RenderPass.TextureStages[j].TextureParam = RENDERSTYLE_USE_TEXTURE4; }
				else OutputMsg("Warning: Unknown param %s",szParam1); }

			CLTANode* pColorOp = CLTAUtil::ShallowFindList(TextureStages[j],"colorop");
			if (pColorOp && pColorOp->GetElement(1) && (szParam1 = pColorOp->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_COLOROP_DISABLE")==0)						{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_DISABLE; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_SELECTARG1")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_SELECTARG1; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_SELECTARG2")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_SELECTARG2; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_MODULATE")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_MODULATE; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_MODULATE2X")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_MODULATE2X; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_MODULATEALPHA")==0)			{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_MODULATEALPHA; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_MODULATETEXALPHA")==0)			{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_MODULATETEXALPHA; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_MODULATETFACTOR")==0)		{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_MODULATETFACTOR; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_ADD")==0)					{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_ADD; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_DOTPRODUCT3")==0)			{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_DOTPRODUCT3; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_BUMPENVMAP")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_BUMPENVMAP; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_BUMPENVMAPLUM")==0)			{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_BUMPENVMAPLUM; }
//				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_DECAL")==0)					{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_DECAL; }
//				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_HIGHLIGHT")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_HIGHLIGHT; }
//				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_HIGHLIGHT2")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_HIGHLIGHT2; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_ADDSIGNED")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_ADDSIGNED; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_ADDSIGNED2X")==0)			{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_ADDSIGNED2X; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_SUBTRACT")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_SUBTRACT; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_ADDMODALPHA")==0)			{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_ADDMODALPHA; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_ADDMODINVALPHA")==0)			{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_ADDMODINVALPHA; }
				else OutputMsg("Warning: Unknown param %s",szParam1); }

			CLTANode* pColorArg1 = CLTAUtil::ShallowFindList(TextureStages[j],"colorarg1");
			if (pColorArg1 && pColorArg1->GetElement(1) && (szParam1 = pColorArg1->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_COLORARG_CURRENT")==0)					{ RenderPass.TextureStages[j].ColorArg1 = RENDERSTYLE_COLORARG_CURRENT; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLORARG_DIFFUSE")==0)				{ RenderPass.TextureStages[j].ColorArg1 = RENDERSTYLE_COLORARG_DIFFUSE; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLORARG_TEXTURE")==0)				{ RenderPass.TextureStages[j].ColorArg1 = RENDERSTYLE_COLORARG_TEXTURE; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLORARG_TFACTOR")==0)				{ RenderPass.TextureStages[j].ColorArg1 = RENDERSTYLE_COLORARG_TFACTOR; }
				else OutputMsg("Warning: Unknown param %s",szParam1); }

			CLTANode* pColorArg2 = CLTAUtil::ShallowFindList(TextureStages[j],"colorarg2");
			if (pColorArg2 && pColorArg2->GetElement(1) && (szParam1 = pColorArg2->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_COLORARG_CURRENT")==0)					{ RenderPass.TextureStages[j].ColorArg2 = RENDERSTYLE_COLORARG_CURRENT; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLORARG_DIFFUSE")==0)				{ RenderPass.TextureStages[j].ColorArg2 = RENDERSTYLE_COLORARG_DIFFUSE; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLORARG_TEXTURE")==0)				{ RenderPass.TextureStages[j].ColorArg2 = RENDERSTYLE_COLORARG_TEXTURE; }
				else if (stricmp(szParam1,"RENDERSTYLE_COLORARG_TFACTOR")==0)				{ RenderPass.TextureStages[j].ColorArg2 = RENDERSTYLE_COLORARG_TFACTOR; }
				else OutputMsg("Warning: Unknown param %s",szParam1); }

			CLTANode* pAlphaOp = CLTAUtil::ShallowFindList(TextureStages[j],"alphaop");
			if (pAlphaOp && pAlphaOp->GetElement(1) && (szParam1 = pAlphaOp->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_DISABLE")==0)						{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_DISABLE; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_SELECTARG1")==0)				{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_SELECTARG1; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_SELECTARG2")==0)				{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_SELECTARG2; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_MODULATE")==0)				{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_MODULATE; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_MODULATEALPHA")==0)			{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_MODULATEALPHA; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_MODULATETEXALPHA")==0)			{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_MODULATETEXALPHA; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_MODULATETFACTOR")==0)		{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_MODULATETFACTOR; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_ADD")==0)					{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_ADD; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_ADDSIGNED")==0)				{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_ADDSIGNED; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_ADDSIGNED2X")==0)			{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_ADDSIGNED2X; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAOP_SUBTRACT")==0)				{ RenderPass.TextureStages[j].AlphaOp = RENDERSTYLE_ALPHAOP_SUBTRACT; }
				else OutputMsg("Warning: Unknown param %s",szParam1); }

			CLTANode* pAlphaArg1 = CLTAUtil::ShallowFindList(TextureStages[j],"alphaarg1");
			if (pAlphaArg1 && pAlphaArg1->GetElement(1) && (szParam1 = pAlphaArg1->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_ALPHAARG_CURRENT")==0)					{ RenderPass.TextureStages[j].AlphaArg1 = RENDERSTYLE_ALPHAARG_CURRENT; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAARG_DIFFUSE")==0)				{ RenderPass.TextureStages[j].AlphaArg1 = RENDERSTYLE_ALPHAARG_DIFFUSE; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAARG_TEXTURE")==0)				{ RenderPass.TextureStages[j].AlphaArg1 = RENDERSTYLE_ALPHAARG_TEXTURE; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAARG_TFACTOR")==0)				{ RenderPass.TextureStages[j].AlphaArg1 = RENDERSTYLE_ALPHAARG_TFACTOR; }
				else { RenderPass.TextureStages[j].AlphaArg1 = RENDERSTYLE_ALPHAARG_CURRENT; OutputMsg("Warning: Unknown param %s",szParam1); } }

			CLTANode* pAlphaArg2 = CLTAUtil::ShallowFindList(TextureStages[j],"alphaarg2");
			if (pAlphaArg2 && pAlphaArg2->GetElement(1) && (szParam1 = pAlphaArg2->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_ALPHAARG_CURRENT")==0)					{ RenderPass.TextureStages[j].AlphaArg2 = RENDERSTYLE_ALPHAARG_CURRENT; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAARG_DIFFUSE")==0)				{ RenderPass.TextureStages[j].AlphaArg2 = RENDERSTYLE_ALPHAARG_DIFFUSE; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAARG_TEXTURE")==0)				{ RenderPass.TextureStages[j].AlphaArg2 = RENDERSTYLE_ALPHAARG_TEXTURE; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHAARG_TFACTOR")==0)				{ RenderPass.TextureStages[j].AlphaArg2 = RENDERSTYLE_ALPHAARG_TFACTOR; }
				else { RenderPass.TextureStages[j].AlphaArg2 = RENDERSTYLE_ALPHAARG_CURRENT; OutputMsg("Warning: Unknown param %s",szParam1); } }

			CLTANode* pUVSource = CLTAUtil::ShallowFindList(TextureStages[j],"uvsource");
			if (pUVSource && pUVSource->GetElement(1) && (szParam1 = pUVSource->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_UVFROM_MODELDATA_UVSET1")==0)				{ RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_MODELDATA_UVSET1; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVFROM_MODELDATA_UVSET2")==0)		{ RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_MODELDATA_UVSET2; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVFROM_MODELDATA_UVSET3")==0)		{ RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_MODELDATA_UVSET3; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVFROM_MODELDATA_UVSET4")==0)		{ RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_MODELDATA_UVSET4; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVFROM_CAMERASPACENORMAL")==0)		{ RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_CAMERASPACENORMAL; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVFROM_CAMERASPACEPOSITION")==0)		{ RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_CAMERASPACEPOSITION; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVFROM_CAMERASPACEREFLTVECT")==0)	{ RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_CAMERASPACEREFLTVECT; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVFROM_WORLDSPACENORMAL")==0)		{ RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_WORLDSPACENORMAL; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVFROM_WORLDSPACEPOSITION")==0)		{ RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_WORLDSPACEPOSITION; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVFROM_WORLDSPACEREFLTVECT")==0)		{ RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_WORLDSPACEREFLTVECT; }
				else { RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_MODELDATA_UVSET1; OutputMsg("Warning: Unknown param %s",szParam1); } }

			CLTANode* pUAddress = CLTAUtil::ShallowFindList(TextureStages[j],"uaddress");
			if (pUAddress && pUAddress->GetElement(1) && (szParam1 = pUAddress->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_UVADDR_WRAP")==0)							{ RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_WRAP; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_CLAMP")==0)					{ RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_CLAMP; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_MIRROR")==0)					{ RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_MIRROR; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_MIRRORONCE")==0)				{ RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_MIRRORONCE; }
				else { RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_WRAP; OutputMsg("Warning: Unknown param %s",szParam1); } }
			else { RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_WRAP; }

			CLTANode* pVAddress = CLTAUtil::ShallowFindList(TextureStages[j],"vaddress");
			if (pVAddress && pVAddress->GetElement(1) && (szParam1 = pVAddress->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_UVADDR_WRAP")==0)							{ RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_WRAP; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_CLAMP")==0)					{ RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_CLAMP; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_MIRROR")==0)					{ RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_MIRROR; }
				else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_MIRRORONCE")==0)				{ RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_MIRRORONCE; }
				else { RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_WRAP; OutputMsg("Warning: Unknown param %s",szParam1); } }
			else { RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_WRAP; }
			CLTANode* pUVTransform_Enable = CLTAUtil::ShallowFindList(TextureStages[j],"uvtransform_enable");
			if (pUVTransform_Enable && pUVTransform_Enable->GetElement(1) && (szParam1 = pUVTransform_Enable->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"1")==0)												{ RenderPass.TextureStages[j].UVTransform_Enable = true; }
				else if (stricmp(szParam1,"0")==0)											{ RenderPass.TextureStages[j].UVTransform_Enable = false; }
				else { RenderPass.TextureStages[j].UVTransform_Enable = false; OutputMsg("Warning: Unknown param %s",szParam1); } }

			CLTANode* pTexFilter = CLTAUtil::ShallowFindList(TextureStages[j],"texfilter");
			if (pTexFilter && pTexFilter->GetElement(1) && (szParam1 = pTexFilter->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"RENDERSTYLE_TEXFILTER_POINT")==0)						{ RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_POINT; }
				else if (stricmp(szParam1,"RENDERSTYLE_TEXFILTER_BILINEAR")==0)				{ RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_BILINEAR; }
				else if (stricmp(szParam1,"RENDERSTYLE_TEXFILTER_TRILINEAR")==0)			{ RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_TRILINEAR; }
				else if (stricmp(szParam1,"RENDERSTYLE_TEXFILTER_ANISOTROPIC")==0)			{ RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_ANISOTROPIC; }
				else if (stricmp(szParam1,"RENDERSTYLE_TEXFILTER_POINT_PTMIP")==0)			{ RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_POINT_PTMIP; }
				else { RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_TRILINEAR; OutputMsg("Warning: Unknown param %s",szParam1); } }
			else { RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_TRILINEAR; }

			CLTANode* pTexCoordCount = CLTAUtil::ShallowFindList(TextureStages[j],"texcoordcount");
			if (pTexCoordCount && pTexCoordCount->GetElement(1) && (szParam1 = pTexCoordCount->GetElement(1)->GetValue()))
			{
				RenderPass.TextureStages[j].TexCoordCount = atoi(szParam1);
				if(RenderPass.TextureStages[j].TexCoordCount < 1)
					RenderPass.TextureStages[j].TexCoordCount = 1;
				if(RenderPass.TextureStages[j].TexCoordCount > 4)
					RenderPass.TextureStages[j].TexCoordCount = 4;
			}
			else { RenderPass.TextureStages[j].TexCoordCount = 2; }

			CLTANode* pUVProject_Enable = CLTAUtil::ShallowFindList(TextureStages[j],"uvproject_enable");
			if (pUVProject_Enable && pUVProject_Enable->GetElement(1) && (szParam1 = pUVProject_Enable->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"1")==0)												{ RenderPass.TextureStages[j].ProjectTexCoord = true; }
				else if (stricmp(szParam1,"0")==0)											{ RenderPass.TextureStages[j].ProjectTexCoord = false; }
				else { RenderPass.TextureStages[j].ProjectTexCoord = false; OutputMsg("Warning: Unknown param %s",szParam1); } }
			else { RenderPass.TextureStages[j].ProjectTexCoord = false; }

			CLTANode* pUVTransform_Matrix = CLTAUtil::ShallowFindList(TextureStages[j],"uvtransform_matrix");
			if (pUVTransform_Matrix) {
				if (pUVTransform_Matrix->GetElement(1)  && (szParam1 = pUVTransform_Matrix->GetElement(1)->GetValue()))  { RenderPass.TextureStages[j].UVTransform_Matrix[0]  = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(2)  && (szParam1 = pUVTransform_Matrix->GetElement(2)->GetValue()))  { RenderPass.TextureStages[j].UVTransform_Matrix[1]  = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(3)  && (szParam1 = pUVTransform_Matrix->GetElement(3)->GetValue()))  { RenderPass.TextureStages[j].UVTransform_Matrix[2]  = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(4)  && (szParam1 = pUVTransform_Matrix->GetElement(4)->GetValue()))  { RenderPass.TextureStages[j].UVTransform_Matrix[3]  = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(5)  && (szParam1 = pUVTransform_Matrix->GetElement(5)->GetValue()))  { RenderPass.TextureStages[j].UVTransform_Matrix[4]  = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(6)  && (szParam1 = pUVTransform_Matrix->GetElement(6)->GetValue()))  { RenderPass.TextureStages[j].UVTransform_Matrix[5]  = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(7)  && (szParam1 = pUVTransform_Matrix->GetElement(7)->GetValue()))  { RenderPass.TextureStages[j].UVTransform_Matrix[6]  = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(8)  && (szParam1 = pUVTransform_Matrix->GetElement(8)->GetValue()))  { RenderPass.TextureStages[j].UVTransform_Matrix[7]  = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(9)  && (szParam1 = pUVTransform_Matrix->GetElement(9)->GetValue()))  { RenderPass.TextureStages[j].UVTransform_Matrix[8]  = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(10) && (szParam1 = pUVTransform_Matrix->GetElement(10)->GetValue())) { RenderPass.TextureStages[j].UVTransform_Matrix[9]  = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(11) && (szParam1 = pUVTransform_Matrix->GetElement(11)->GetValue())) { RenderPass.TextureStages[j].UVTransform_Matrix[10] = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(12) && (szParam1 = pUVTransform_Matrix->GetElement(12)->GetValue())) { RenderPass.TextureStages[j].UVTransform_Matrix[11] = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(13) && (szParam1 = pUVTransform_Matrix->GetElement(13)->GetValue())) { RenderPass.TextureStages[j].UVTransform_Matrix[12] = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(14) && (szParam1 = pUVTransform_Matrix->GetElement(14)->GetValue())) { RenderPass.TextureStages[j].UVTransform_Matrix[13] = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(15) && (szParam1 = pUVTransform_Matrix->GetElement(15)->GetValue())) { RenderPass.TextureStages[j].UVTransform_Matrix[14] = (float)atof(szParam1); }
				if (pUVTransform_Matrix->GetElement(16) && (szParam1 = pUVTransform_Matrix->GetElement(16)->GetValue())) { RenderPass.TextureStages[j].UVTransform_Matrix[15] = (float)atof(szParam1); } }
			else {
				for (uint32 r = 0; r < 16; ++r) if ((r-(r/4))%4==0) RenderPass.TextureStages[j].UVTransform_Matrix[r] = 1.0f; else RenderPass.TextureStages[j].UVTransform_Matrix[r] = 0.0f; } }
		pRenderStyle->AddRenderPass(RenderPass);

		// D3D Options...
		vector<CLTANode*> D3DRenderPassOptions;
		CLTANode* pD3DRenderPassOptions = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"d3doptions");
		if (pD3DRenderPassOptions)
		{
			RSD3DRenderPass D3DOptions;

			// usevertexshader
			CLTANode* pUseVertexShader = CLTAUtil::ShallowFindList(pD3DRenderPassOptions, "usevertexshader");
			if (pUseVertexShader && pUseVertexShader->GetElement(1) && (szParam1 = pUseVertexShader->GetElement(1)->GetValue()))
			{
				if (stricmp(szParam1, "1") == 0)
				{
					D3DOptions.bUseVertexShader = true;
				}
				else if (stricmp(szParam1, "0") == 0)
				{
					D3DOptions.bUseVertexShader = false;
				}
				else
				{
					D3DOptions.bUseVertexShader = false;
					OutputMsg("Warning: Unknown param %s", szParam1);
				}
			}

			// vertexshaderid
			CLTANode* pVertexShaderID = CLTAUtil::ShallowFindList(pD3DRenderPassOptions, "vertexshaderid");
			if (pVertexShaderID && pVertexShaderID->GetElement(1) && (szParam1 = pVertexShaderID->GetElement(1)->GetValue()))
			{
				D3DOptions.VertexShaderID = atoi(szParam1);
			}

			// usepixelshader
			CLTANode* pUsePixelShader = CLTAUtil::ShallowFindList(pD3DRenderPassOptions, "usepixelshader");
			if (pUsePixelShader && pUsePixelShader->GetElement(1) && (szParam1 = pUsePixelShader->GetElement(1)->GetValue()))
			{
				if (stricmp(szParam1, "1") == 0)
				{
					D3DOptions.bUsePixelShader = true;
				}
				else if (stricmp(szParam1, "0") == 0)
				{
					D3DOptions.bUsePixelShader = false;
				}
				else
				{
					D3DOptions.bUsePixelShader = false;
					OutputMsg("Warning: Unknown param %s", szParam1);
				}
			}

			// pixelshaderid
			CLTANode* pPixelShaderID = CLTAUtil::ShallowFindList(pD3DRenderPassOptions, "pixelshaderid");
			if (pPixelShaderID && pPixelShaderID->GetElement(1) && (szParam1 = pPixelShaderID->GetElement(1)->GetValue()))
			{
				D3DOptions.PixelShaderID = atoi(szParam1);
			}

			pRenderStyle->SetRenderPass_D3DOptions(i, &D3DOptions);
		}
	}


	// RS D3D Options...
	CLTANode* pRSD3DOptions = CLTAUtil::ShallowFindList(pRenderStyleP,"rsd3doptions");
	if (pRSD3DOptions)
	{
		RSD3DOptions rsD3DOptions;
		CLTANode* pUseEffectShader = CLTAUtil::ShallowFindList(pRSD3DOptions,"useeffectshader");
		if (pUseEffectShader) 
		{
			if (pUseEffectShader->GetElement(1) && (szParam1 = pUseEffectShader->GetElement(1)->GetValue()))		
			{
				rsD3DOptions.bUseEffectShader = !!(int)atoi(szParam1);
			}
		}
		CLTANode* pEffectShaderID = CLTAUtil::ShallowFindList(pRSD3DOptions,"effectshaderid");
		if (pEffectShaderID) 
		{
			if (pEffectShaderID->GetElement(1) && (szParam1 = pEffectShaderID->GetElement(1)->GetValue()))
			{
				rsD3DOptions.EffectShaderID = (int)atoi(szParam1);
			}
		}

		pRenderStyle->SetDirect3D_Options(rsD3DOptions);
		
	}

	return true;
}
