// RenderStyle_LTA.cpp

#include "stdafx.h"
#if _MSC_VER >= 1300
#include <fstream>
#else
#include "fstream.h"
#endif

#include "RenderStylesEditor.h"
#include "ltamgr.h"
#
// PROTOTYPES
int			BuildParseTree(CLTAFile* str, CLTANode* root);
CLTAFile*	LTAStreamFrom_istream(istream &);
void		PrettyPrint(ostream& os, CLTANode* node, int depth);

// Load the render style into the RenderStyle...
bool CAppForm::LoadRenderStyle(const char* szFileName)
{
	CLTAReader ltaStream;

	if (ltaStream.Open(szFileName, CLTAUtil::IsFileCompressed(szFileName)))
	{
		CLTANode RootNode;

		m_pRenderStyle->SetDefaults();	// First, reset the sucker...

		CLTADefaultAlloc Allocator;
		CLTANodeReader::LoadEntireFile(ltaStream,&RootNode, &Allocator);

		vector<CLTANode*> RenderStyles;
		CLTAUtil::FindAll(RenderStyles,&RootNode,"renderstyle");
		if (RenderStyles.empty()) return false;

		CLTANode* pRenderStyle  = RenderStyles[0]; const char* szRenderStyleName = NULL;
		if (pRenderStyle->GetElement(1)) szRenderStyleName = pRenderStyle->GetElement(1)->GetValue();
		const char* szParam1; //const char* szParam2; const char* szParam3; const char* szParam4;

		// Lighting Material...
		CLTANode* pLightMaterial = CLTAUtil::ShallowFindList(pRenderStyle,"lightmaterial");
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
			m_pRenderStyle->SetLightingMaterial(lightMaterial); }

		// Render Passes...
		while (m_pRenderStyle->GetRenderPassCount()) { m_pRenderStyle->RemoveRenderPass(0); }
		vector<CLTANode*> RenderRenderPasses;
		CLTAUtil::FindAll(RenderRenderPasses,pRenderStyle,"renderpass");
		for (uint32 i = 0; i < RenderRenderPasses.size(); ++i)
		{
			RenderPassOp RenderPass;
			CLTANode* pTestMode = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"testmode");
			if (pTestMode && pTestMode->GetElement(1) && (szParam1 = pTestMode->GetElement(1)->GetValue()))
			{
				if (stricmp(szParam1,"RENDERSTYLE_NOALPHATEST")==0)								{ RenderPass.AlphaTestMode = RENDERSTYLE_NOALPHATEST; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_LESS")==0)						{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_LESS; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_LESSEQUAL")==0)				{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_LESSEQUAL; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_GREATER")==0)					{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_GREATER; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_GREATEREQUAL")==0)				{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_GREATEREQUAL; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_EQUAL")==0)					{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_EQUAL; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_NOTEQUAL")==0)					{ RenderPass.AlphaTestMode = RENDERSTYLE_ALPHATEST_NOTEQUAL; }
				else { RenderPass.AlphaTestMode = RENDERSTYLE_NOALPHATEST; OutputMsg("Warning: Unknown param %s",szParam1); }
			}
			else { RenderPass.AlphaTestMode = RENDERSTYLE_NOALPHATEST; }

			CLTANode* pZTestMode = CLTAUtil::ShallowFindList(RenderRenderPasses[i],"ztestmode");
			if (pZTestMode && pZTestMode->GetElement(1) && (szParam1 = pZTestMode->GetElement(1)->GetValue()))
			{
				if (stricmp(szParam1,"RENDERSTYLE_NOALPHATEST")==0)								{ RenderPass.ZBufferTestMode = RENDERSTYLE_NOALPHATEST; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_LESS")==0)						{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_LESS; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_LESSEQUAL")==0)				{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_LESSEQUAL; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_GREATER")==0)					{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_GREATER; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_GREATEREQUAL")==0)				{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_GREATEREQUAL; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_EQUAL")==0)					{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_EQUAL; }
				else if (stricmp(szParam1,"RENDERSTYLE_ALPHATEST_NOTEQUAL")==0)					{ RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_NOTEQUAL; }
				else { RenderPass.ZBufferTestMode = RENDERSTYLE_ALPHATEST_LESSEQUAL; OutputMsg("Warning: Unknown param %s",szParam1); }
			}
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
				if (pColorOp && pColorOp->GetElement(1) && (szParam1 = pColorOp->GetElement(1)->GetValue()))
				{
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
//OLD PS2					else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_DECAL")==0)					{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_DECAL; }
//					else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_HIGHLIGHT")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_HIGHLIGHT; }
//					else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_HIGHLIGHT2")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_HIGHLIGHT2; }
					else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_ADDSIGNED")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_ADDSIGNED; }
					else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_ADDSIGNED2X")==0)			{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_ADDSIGNED2X; }
					else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_SUBTRACT")==0)				{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_SUBTRACT; }
					else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_ADDMODALPHA")==0)			{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_ADDMODALPHA; }
					else if (stricmp(szParam1,"RENDERSTYLE_COLOROP_ADDMODINVALPHA")==0)			{ RenderPass.TextureStages[j].ColorOp = RENDERSTYLE_COLOROP_ADDMODINVALPHA; }
					else OutputMsg("Warning: Unknown param %s",szParam1);
				}
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
				if (pAlphaOp && pAlphaOp->GetElement(1) && (szParam1 = pAlphaOp->GetElement(1)->GetValue()))
				{
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
					else OutputMsg("Warning: Unknown param %s",szParam1);
				}
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
				if (pUVSource && pUVSource->GetElement(1) && (szParam1 = pUVSource->GetElement(1)->GetValue()))
				{
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
					else
					{
						RenderPass.TextureStages[j].UVSource = RENDERSTYLE_UVFROM_MODELDATA_UVSET1; OutputMsg("Warning: Unknown param %s",szParam1);
					}
				}
				CLTANode* pUAddress = CLTAUtil::ShallowFindList(TextureStages[j],"uaddress");
				if (pUAddress && pUAddress->GetElement(1) && (szParam1 = pUAddress->GetElement(1)->GetValue()))
				{
					if (stricmp(szParam1,"RENDERSTYLE_UVADDR_WRAP")==0)							{ RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_WRAP; }
					else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_CLAMP")==0)					{ RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_CLAMP; }
					else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_MIRROR")==0)					{ RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_MIRROR; }
					else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_MIRRORONCE")==0)				{ RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_MIRRORONCE; }
					else { RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_WRAP; OutputMsg("Warning: Unknown param %s",szParam1); }
				}
				else { RenderPass.TextureStages[j].UAddress = RENDERSTYLE_UVADDR_WRAP; }
				CLTANode* pVAddress = CLTAUtil::ShallowFindList(TextureStages[j],"vaddress");
				if (pVAddress && pVAddress->GetElement(1) && (szParam1 = pVAddress->GetElement(1)->GetValue())) {
					if (stricmp(szParam1,"RENDERSTYLE_UVADDR_WRAP")==0)							{ RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_WRAP; }
					else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_CLAMP")==0)					{ RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_CLAMP; }
					else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_MIRROR")==0)					{ RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_MIRROR; }
					else if (stricmp(szParam1,"RENDERSTYLE_UVADDR_MIRRORONCE")==0)				{ RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_MIRRORONCE; }
					else { RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_WRAP; OutputMsg("Warning: Unknown param %s",szParam1); } }
				else { RenderPass.TextureStages[j].VAddress = RENDERSTYLE_UVADDR_WRAP; }
				CLTANode* pTexFilter = CLTAUtil::ShallowFindList(TextureStages[j],"texfilter");
				if (pTexFilter && pTexFilter->GetElement(1) && (szParam1 = pTexFilter->GetElement(1)->GetValue())) {
					if (stricmp(szParam1,"RENDERSTYLE_TEXFILTER_POINT")==0)						{ RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_POINT; }
					else if (stricmp(szParam1,"RENDERSTYLE_TEXFILTER_BILINEAR")==0)				{ RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_BILINEAR; }
					else if (stricmp(szParam1,"RENDERSTYLE_TEXFILTER_TRILINEAR")==0)			{ RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_TRILINEAR; }
					else if (stricmp(szParam1,"RENDERSTYLE_TEXFILTER_ANISOTROPIC")==0)			{ RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_ANISOTROPIC; }
					else if (stricmp(szParam1,"RENDERSTYLE_TEXFILTER_POINT_PTMIP")==0)			{ RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_POINT_PTMIP; }
					else { RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_TRILINEAR; OutputMsg("Warning: Unknown param %s",szParam1); } }
				else { RenderPass.TextureStages[j].TexFilter = RENDERSTYLE_TEXFILTER_TRILINEAR; }
				CLTANode* pUVTransform_Enable = CLTAUtil::ShallowFindList(TextureStages[j],"uvtransform_enable");
				if (pUVTransform_Enable && pUVTransform_Enable->GetElement(1) && (szParam1 = pUVTransform_Enable->GetElement(1)->GetValue())) {
					if (stricmp(szParam1,"1")==0)												{ RenderPass.TextureStages[j].UVTransform_Enable = true; }
					else if (stricmp(szParam1,"0")==0)											{ RenderPass.TextureStages[j].UVTransform_Enable = false; }
					else { RenderPass.TextureStages[j].UVTransform_Enable = false; OutputMsg("Warning: Unknown param %s",szParam1); } }

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
				if (pUVProject_Enable && pUVProject_Enable->GetElement(1) && (szParam1 = pUVProject_Enable->GetElement(1)->GetValue()))
				{
					if (stricmp(szParam1,"1")==0)												{ RenderPass.TextureStages[j].ProjectTexCoord = true; }
					else if (stricmp(szParam1,"0")==0)											{ RenderPass.TextureStages[j].ProjectTexCoord = false; }
					else { RenderPass.TextureStages[j].ProjectTexCoord = false; OutputMsg("Warning: Unknown param %s",szParam1); }
				}
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
			m_pRenderStyle->AddRenderPass(RenderPass);

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

				m_pRenderStyle->SetRenderPass_D3DOptions(i, &D3DOptions);
			}
		}
		
		// RS D3D Options...
		CLTANode* pRSD3DOptions = CLTAUtil::ShallowFindList(pRenderStyle,"rsd3doptions");
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
			
			m_pRenderStyle->SetDirect3D_Options(rsD3DOptions);
		}

		// Debug Params (from the dialog's data)...
		CLTANode* pDebugParams = CLTAUtil::ShallowFindList(pRenderStyle,"debugparams");
		if (pDebugParams) {
			CLTANode* pUseRefRast = CLTAUtil::ShallowFindList(pDebugParams,"userefrast");
			if (pUseRefRast && pUseRefRast->GetElement(1) && (szParam1 = pUseRefRast->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"1")==0)													{ m_DebugParams.bUseRefRast = true; }
				else if (stricmp(szParam1,"0")==0)												{ m_DebugParams.bUseRefRast = false; }
				else OutputMsg("Warning: Unknown param %s",szParam1); }
			CLTANode* pRenderOnIdle = CLTAUtil::ShallowFindList(pDebugParams,"renderonidle");
			if (pRenderOnIdle && pRenderOnIdle->GetElement(1) && (szParam1 = pRenderOnIdle->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"1")==0)													{ m_DebugParams.bRenderOnIdle = true; }
				else if (stricmp(szParam1,"0")==0)												{ m_DebugParams.bRenderOnIdle = false; }
				else OutputMsg("Warning: Unknown param %s",szParam1); }
			CLTANode* pRotateModel = CLTAUtil::ShallowFindList(pDebugParams,"rotatemodel");
			if (pRotateModel && pRotateModel->GetElement(1) && (szParam1 = pRotateModel->GetElement(1)->GetValue())) {
				if (stricmp(szParam1,"1")==0)													{ m_DebugParams.bRotateModel = true; }
				else if (stricmp(szParam1,"0")==0)												{ m_DebugParams.bRotateModel = false; }
				else OutputMsg("Warning: Unknown param %s",szParam1); }
			CLTANode* pLightConfig = CLTAUtil::ShallowFindList(pDebugParams,"lightconfig");
			if (pLightConfig && pLightConfig->GetElement(1) && (szParam1 = pLightConfig->GetElement(1)->GetValue())) {
				m_DebugParams.LightConfig = atoi(szParam1); }
			else { m_DebugParams.LightConfig = 0; }
			CLTANode* pLightCount = CLTAUtil::ShallowFindList(pDebugParams,"lightcount");
			if (pLightCount && pLightCount->GetElement(1) && (szParam1 = pLightCount->GetElement(1)->GetValue())) {
				m_DebugParams.LightCount = atoi(szParam1); }
			else { m_DebugParams.LightCount = 2; }
			CLTANode* pLightColor = CLTAUtil::ShallowFindList(pDebugParams,"lightcolor");
			if (pLightColor && pLightColor->GetElement(1)) {
				if (szParam1 = pLightColor->GetElement(1)->GetValue()) { m_DebugParams.LightColor.r = (float)atof(szParam1); }
				if (szParam1 = pLightColor->GetElement(2)->GetValue()) { m_DebugParams.LightColor.g = (float)atof(szParam1); }
				if (szParam1 = pLightColor->GetElement(3)->GetValue()) { m_DebugParams.LightColor.b = (float)atof(szParam1); }
				if (szParam1 = pLightColor->GetElement(4)->GetValue()) { m_DebugParams.LightColor.a = (float)atof(szParam1); } }
			else { m_DebugParams.LightColor = FourFloatColor(0.5f,0.5f,0.5f,1.0f); }
			CLTANode* pLightRot = CLTAUtil::ShallowFindList(pDebugParams,"camerarotationspeed");
			if (pLightRot && pLightRot->GetElement(1) && (szParam1 = pLightRot->GetElement(1)->GetValue())) {
				m_DebugParams.fCameraRotationSpeed = (float)atof(szParam1); }
			else { m_DebugParams.fCameraRotationSpeed = 0.0f; }
			CLTANode* pBackGndImg_Filename = CLTAUtil::ShallowFindList(pDebugParams,"backgndimg_filename");
			if (pBackGndImg_Filename && (pBackGndImg_Filename->GetNumElements() > 1) && (szParam1 = pBackGndImg_Filename->GetElement(1)->GetValue())) {
				m_DebugParams.BackGndImg_Filename = szParam1; }
			else { m_DebugParams.BackGndImg_Filename.erase(m_DebugParams.BackGndImg_Filename.begin(),m_DebugParams.BackGndImg_Filename.end()); }
			CLTANode* pBackGndImg_Color = CLTAUtil::ShallowFindList(pDebugParams,"backgndimg_color");
			if (pBackGndImg_Color && pBackGndImg_Color->GetElement(1)) {
				if (szParam1 = pBackGndImg_Color->GetElement(1)->GetValue()) { m_DebugParams.BackGndImg_Color.r = (float)atof(szParam1); }
				if (szParam1 = pBackGndImg_Color->GetElement(2)->GetValue()) { m_DebugParams.BackGndImg_Color.g = (float)atof(szParam1); }
				if (szParam1 = pBackGndImg_Color->GetElement(3)->GetValue()) { m_DebugParams.BackGndImg_Color.b = (float)atof(szParam1); }
				if (szParam1 = pBackGndImg_Color->GetElement(4)->GetValue()) { m_DebugParams.BackGndImg_Color.a = (float)atof(szParam1); } }
			else { m_DebugParams.BackGndImg_Color = FourFloatColor(0.0f,0.05f,0.3f,1.0f); }
			CLTANode* pModelName = CLTAUtil::ShallowFindList(pDebugParams,"modelname");
			if (pModelName && pModelName->GetElement(1) && (szParam1 = pModelName->GetElement(1)->GetValue())) {
				m_DebugParams.ModelName = szParam1; }
			else {
				m_DebugParams.ModelName.erase(m_DebugParams.ModelName.begin(),m_DebugParams.ModelName.end()); }
			CLTANode* pTexture1 = CLTAUtil::ShallowFindList(pDebugParams,"texture1");
			if (pTexture1 && pTexture1->GetElement(1) && (szParam1 = pTexture1->GetElement(1)->GetValue())) {
				m_DebugParams.TextureList_Filename[0] = szParam1; }
			else {
				m_DebugParams.TextureList_Filename[0].erase(m_DebugParams.TextureList_Filename[0].begin(),m_DebugParams.TextureList_Filename[0].end()); }
			CLTANode* pTexture2 = CLTAUtil::ShallowFindList(pDebugParams,"texture2");
			if (pTexture2 && pTexture2->GetElement(1) && (szParam1 = pTexture2->GetElement(1)->GetValue())) {
				m_DebugParams.TextureList_Filename[1] = szParam1; }
			else {
				m_DebugParams.TextureList_Filename[1].erase(m_DebugParams.TextureList_Filename[1].begin(),m_DebugParams.TextureList_Filename[1].end()); 
			}
			CLTANode* pTexture3 = CLTAUtil::ShallowFindList(pDebugParams,"texture3");
			if (pTexture3 && pTexture3->GetElement(1) && (szParam1 = pTexture3->GetElement(1)->GetValue())) 
			{
				m_DebugParams.TextureList_Filename[2] = szParam1; 
			}
			else 
			{
				m_DebugParams.TextureList_Filename[2].erase(m_DebugParams.TextureList_Filename[2].begin(),m_DebugParams.TextureList_Filename[2].end()); 
			}

			CLTANode* pTexture4 = CLTAUtil::ShallowFindList(pDebugParams,"texture4");
			if (pTexture4 && pTexture4->GetElement(1) && (szParam1 = pTexture4->GetElement(1)->GetValue())) 
			{
				m_DebugParams.TextureList_Filename[3] = szParam1; 
			}
			else 
			{
				m_DebugParams.TextureList_Filename[3].erase(m_DebugParams.TextureList_Filename[3].begin(),m_DebugParams.TextureList_Filename[3].end()); 
			} 

			CLTANode* pEffectFilename = CLTAUtil::ShallowFindList(pDebugParams,"effectfilename");
			if (pEffectFilename && pEffectFilename->GetElement(1) && (szParam1 = pEffectFilename->GetElement(1)->GetValue())) 
			{
				m_DebugParams.EffectFilename = szParam1; 
			}
			else 
			{
				m_DebugParams.EffectFilename.erase(m_DebugParams.EffectFilename.begin(),m_DebugParams.EffectFilename.end()); 
			} 
		}


		//clean up the memory
		RootNode.Free(&Allocator);

		return true;
	}
	return false;
}

// Save the render style from the RenderStyle Data members...
bool CAppForm::SaveRenderStyle(const char* szFileName)
{
	if (!m_pRenderStyle) return false;

	char buffer[512], filePath[512], projectPath[512], drive[8];

	// Record the file-relative and project-relative paths
	_splitpath(m_RenderStyle_FileName, drive, buffer, filePath, filePath);
	sprintf(filePath, "%s%s", drive, buffer);

	_splitpath(m_ProjectPath, drive, buffer, projectPath, projectPath);
	sprintf(projectPath, "%s%s", drive, buffer);


	ofstream ltaFile(szFileName);
	if (ltaFile) {

		CLTADefaultAlloc Allocator;
		CLTANodeBuilder fnListBuild(&Allocator);
		fnListBuild.Push("renderstyle");
			fnListBuild.AddValue("Nameless");

			// Lighting Material...
			fnListBuild.Push("lightmaterial");
				LightingMaterial lightMaterial;
				if (m_pRenderStyle->GetLightingMaterial(&lightMaterial)) {
					fnListBuild.Push("ambient");
					fnListBuild.AddValue(lightMaterial.Ambient.r); fnListBuild.AddValue(lightMaterial.Ambient.g); fnListBuild.AddValue(lightMaterial.Ambient.b); fnListBuild.AddValue(lightMaterial.Ambient.a);
					fnListBuild.Pop();
					fnListBuild.Push("diffuse");
					fnListBuild.AddValue(lightMaterial.Diffuse.r); fnListBuild.AddValue(lightMaterial.Diffuse.g); fnListBuild.AddValue(lightMaterial.Diffuse.b); fnListBuild.AddValue(lightMaterial.Diffuse.a);
					fnListBuild.Pop();
					fnListBuild.Push("specular");
					fnListBuild.AddValue(lightMaterial.Specular.r); fnListBuild.AddValue(lightMaterial.Specular.g); fnListBuild.AddValue(lightMaterial.Specular.b); fnListBuild.AddValue(lightMaterial.Specular.a);
					fnListBuild.Pop();
					fnListBuild.Push("emissive");
					fnListBuild.AddValue(lightMaterial.Emissive.r); fnListBuild.AddValue(lightMaterial.Emissive.g); fnListBuild.AddValue(lightMaterial.Emissive.b); fnListBuild.AddValue(lightMaterial.Emissive.a);
					fnListBuild.Pop();
					fnListBuild.Push("specularpower");
					fnListBuild.AddValue(lightMaterial.SpecularPower);
					fnListBuild.Pop(); }
			fnListBuild.Pop();

			// Render Passes...
			for (uint32 i = 0; i < m_pRenderStyle->GetRenderPassCount(); ++i)
			{
				RenderPassOp RenderPass; m_pRenderStyle->GetRenderPass(i,&RenderPass);
				fnListBuild.Push("renderpass");
					fnListBuild.Push("testmode");
					switch (RenderPass.AlphaTestMode) {
					case RENDERSTYLE_NOALPHATEST			: fnListBuild.AddValue("RENDERSTYLE_NOALPHATEST"); break;
					case RENDERSTYLE_ALPHATEST_LESS			: fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_LESS"); break;
					case RENDERSTYLE_ALPHATEST_LESSEQUAL	: fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_LESSEQUAL"); break;
					case RENDERSTYLE_ALPHATEST_GREATER		: fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_GREATER"); break;
					case RENDERSTYLE_ALPHATEST_GREATEREQUAL : fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_GREATEREQUAL"); break;
					case RENDERSTYLE_ALPHATEST_EQUAL		: fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_EQUAL"); break;
					case RENDERSTYLE_ALPHATEST_NOTEQUAL		: fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_NOTEQUAL"); break;
					default									: assert(0 && "Unknown value"); }
					fnListBuild.Pop();

					fnListBuild.Push("ztestmode");
					switch (RenderPass.ZBufferTestMode) {
					case RENDERSTYLE_NOALPHATEST			: fnListBuild.AddValue("RENDERSTYLE_NOALPHATEST"); break;
					case RENDERSTYLE_ALPHATEST_LESS			: fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_LESS"); break;
					case RENDERSTYLE_ALPHATEST_LESSEQUAL	: fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_LESSEQUAL"); break;
					case RENDERSTYLE_ALPHATEST_GREATER		: fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_GREATER"); break;
					case RENDERSTYLE_ALPHATEST_GREATEREQUAL : fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_GREATEREQUAL"); break;
					case RENDERSTYLE_ALPHATEST_EQUAL		: fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_EQUAL"); break;
					case RENDERSTYLE_ALPHATEST_NOTEQUAL		: fnListBuild.AddValue("RENDERSTYLE_ALPHATEST_NOTEQUAL"); break;
					default									: assert(0 && "Unknown value"); }
					fnListBuild.Pop();

					fnListBuild.Push("fillmode");
					switch (RenderPass.FillMode) {
					case RENDERSTYLE_WIRE					: fnListBuild.AddValue("RENDERSTYLE_WIRE"); break;
					case RENDERSTYLE_FILL					: fnListBuild.AddValue("RENDERSTYLE_FILL"); break;
					default									: assert(0 && "Unknown value"); }
					fnListBuild.Pop();
					fnListBuild.Push("blendop");
					switch (RenderPass.BlendMode) {
					case RENDERSTYLE_NOBLEND							: fnListBuild.AddValue("RENDERSTYLE_NOBLEND"); break;
					case RENDERSTYLE_BLEND_ADD							: fnListBuild.AddValue("RENDERSTYLE_BLEND_ADD"); break;
					case RENDERSTYLE_BLEND_SATURATE						: fnListBuild.AddValue("RENDERSTYLE_BLEND_SATURATE"); break;
					case RENDERSTYLE_BLEND_MOD_SRCALPHA					: fnListBuild.AddValue("RENDERSTYLE_BLEND_MOD_SRCALPHA"); break;
					case RENDERSTYLE_BLEND_MOD_SRCCOLOR					: fnListBuild.AddValue("RENDERSTYLE_BLEND_MOD_SRCCOLOR"); break;
					case RENDERSTYLE_BLEND_MOD_DSTCOLOR					: fnListBuild.AddValue("RENDERSTYLE_BLEND_MOD_DSTCOLOR"); break;
					case RENDERSTYLE_BLEND_MUL_SRCCOL_DSTCOL			: fnListBuild.AddValue("RENDERSTYLE_BLEND_MUL_SRCCOL_DSTCOL"); break;
					case RENDERSTYLE_BLEND_MUL_SRCCOL_ONE				: fnListBuild.AddValue("RENDERSTYLE_BLEND_MUL_SRCCOL_ONE"); break;
					case RENDERSTYLE_BLEND_MUL_SRCALPHA_ZERO			: fnListBuild.AddValue("RENDERSTYLE_BLEND_MUL_SRCALPHA_ZERO"); break;
					case RENDERSTYLE_BLEND_MUL_SRCALPHA_ONE				: fnListBuild.AddValue("RENDERSTYLE_BLEND_MUL_SRCALPHA_ONE"); break;
					case RENDERSTYLE_BLEND_MUL_DSTCOL_ZERO				: fnListBuild.AddValue("RENDERSTYLE_BLEND_MUL_DSTCOL_ZERO"); break;
					default												: assert(0 && "Unknown value"); }
					fnListBuild.Pop();
					fnListBuild.Push("zbuffermode");
					switch (RenderPass.ZBufferMode) {
					case RENDERSTYLE_ZRW								: fnListBuild.AddValue("RENDERSTYLE_ZRW"); break;
					case RENDERSTYLE_ZRO								: fnListBuild.AddValue("RENDERSTYLE_ZRO"); break;
					case RENDERSTYLE_NOZ								: fnListBuild.AddValue("RENDERSTYLE_NOZ"); break;
					default												: assert(0 && "Unknown value"); }
					fnListBuild.Pop();
					fnListBuild.Push("cullmode");
					switch (RenderPass.CullMode) {
					case RENDERSTYLE_CULL_NONE							: fnListBuild.AddValue("RENDERSTYLE_CULL_NONE"); break;
					case RENDERSTYLE_CULL_CCW							: fnListBuild.AddValue("RENDERSTYLE_CULL_CCW"); break;
					case RENDERSTYLE_CULL_CW							: fnListBuild.AddValue("RENDERSTYLE_CULL_CW"); break;
					default												: assert(0 && "Unknown value"); }
					fnListBuild.Pop();
					fnListBuild.Push("texturefactor");
					char szTmp[32]; sprintf(szTmp,"%i",RenderPass.TextureFactor); fnListBuild.AddValue(szTmp);
					fnListBuild.Pop();
					fnListBuild.Push("alpharef");
					sprintf(szTmp,"%d",RenderPass.AlphaRef); fnListBuild.AddValue(szTmp);
					fnListBuild.Pop();
					fnListBuild.Push("dynamiclight");
					fnListBuild.AddValue(RenderPass.DynamicLight ? "1" : "0");
					fnListBuild.Pop();
					fnListBuild.Push("usebumpenvmap");
					fnListBuild.AddValue(RenderPass.bUseBumpEnvMap ? "1" : "0");
					fnListBuild.Pop();
					fnListBuild.Push("bumpenvmapstage");
					char szBuf[32]; sprintf(szBuf,"%d",RenderPass.BumpEnvMapStage); fnListBuild.AddValue(szBuf);
					fnListBuild.Pop();
					fnListBuild.Push("bumpenvmapscale");
					sprintf(szBuf,"%f",RenderPass.fBumpEnvMap_Scale); fnListBuild.AddValue(szBuf);
					fnListBuild.Pop();
					fnListBuild.Push("bumpenvmapoffset");
					sprintf(szBuf,"%f",RenderPass.fBumpEnvMap_Offset); fnListBuild.AddValue(szBuf);
					fnListBuild.Pop();

					for (uint32 j = 0; j < 4; ++j) {
						fnListBuild.Push("texturestageops");
							fnListBuild.Push("textureparam");
							switch (RenderPass.TextureStages[j].TextureParam) {
							case RENDERSTYLE_NOTEXTURE					: fnListBuild.AddValue("RENDERSTYLE_NOTEXTURE"); break;
							case RENDERSTYLE_USE_TEXTURE1				: fnListBuild.AddValue("RENDERSTYLE_USE_TEXTURE1"); break;
							case RENDERSTYLE_USE_TEXTURE2				: fnListBuild.AddValue("RENDERSTYLE_USE_TEXTURE2"); break;
							case RENDERSTYLE_USE_TEXTURE3				: fnListBuild.AddValue("RENDERSTYLE_USE_TEXTURE3"); break;
							case RENDERSTYLE_USE_TEXTURE4				: fnListBuild.AddValue("RENDERSTYLE_USE_TEXTURE4"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("colorop");
							switch (RenderPass.TextureStages[j].ColorOp) {
							case RENDERSTYLE_COLOROP_DISABLE			: fnListBuild.AddValue("RENDERSTYLE_COLOROP_DISABLE"); break;
							case RENDERSTYLE_COLOROP_SELECTARG1			: fnListBuild.AddValue("RENDERSTYLE_COLOROP_SELECTARG1"); break;
							case RENDERSTYLE_COLOROP_SELECTARG2			: fnListBuild.AddValue("RENDERSTYLE_COLOROP_SELECTARG2"); break;
							case RENDERSTYLE_COLOROP_MODULATE			: fnListBuild.AddValue("RENDERSTYLE_COLOROP_MODULATE"); break;
							case RENDERSTYLE_COLOROP_MODULATE2X			: fnListBuild.AddValue("RENDERSTYLE_COLOROP_MODULATE2X"); break;
							case RENDERSTYLE_COLOROP_MODULATEALPHA		: fnListBuild.AddValue("RENDERSTYLE_COLOROP_MODULATEALPHA"); break;
							case RENDERSTYLE_COLOROP_MODULATETEXALPHA		: fnListBuild.AddValue("RENDERSTYLE_COLOROP_MODULATETEXALPHA"); break;
							case RENDERSTYLE_COLOROP_MODULATETFACTOR	: fnListBuild.AddValue("RENDERSTYLE_COLOROP_MODULATETFACTOR"); break;
							case RENDERSTYLE_COLOROP_ADD				: fnListBuild.AddValue("RENDERSTYLE_COLOROP_ADD"); break;
							case RENDERSTYLE_COLOROP_DOTPRODUCT3		: fnListBuild.AddValue("RENDERSTYLE_COLOROP_DOTPRODUCT3"); break;
							case RENDERSTYLE_COLOROP_BUMPENVMAP			: fnListBuild.AddValue("RENDERSTYLE_COLOROP_BUMPENVMAP"); break;
							case RENDERSTYLE_COLOROP_BUMPENVMAPLUM		: fnListBuild.AddValue("RENDERSTYLE_COLOROP_BUMPENVMAPLUM"); break;
							case RENDERSTYLE_COLOROP_ADDSIGNED			:  fnListBuild.AddValue("RENDERSTYLE_COLOROP_ADDSIGNED"); break;
							case RENDERSTYLE_COLOROP_ADDSIGNED2X		:  fnListBuild.AddValue("RENDERSTYLE_COLOROP_ADDSIGNED2X"); break;
							case RENDERSTYLE_COLOROP_SUBTRACT			:  fnListBuild.AddValue("RENDERSTYLE_COLOROP_SUBTRACT"); break;
							case RENDERSTYLE_COLOROP_ADDMODALPHA		:  fnListBuild.AddValue("RENDERSTYLE_COLOROP_ADDMODALPHA"); break;
							case RENDERSTYLE_COLOROP_ADDMODINVALPHA	:  fnListBuild.AddValue("RENDERSTYLE_COLOROP_ADDMODINVALPHA"); break; 

//	OLD PS2				case RENDERSTYLE_COLOROP_DECAL				: fnListBuild.AddValue("RENDERSTYLE_COLOROP_DECAL"); break;
//							case RENDERSTYLE_COLOROP_HIGHLIGHT			: fnListBuild.AddValue("RENDERSTYLE_COLOROP_HIGHLIGHT"); break;
//							case RENDERSTYLE_COLOROP_HIGHLIGHT2			: fnListBuild.AddValue("RENDERSTYLE_COLOROP_HIGHLIGHT2"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("colorarg1");
							switch (RenderPass.TextureStages[j].ColorArg1) {
							case RENDERSTYLE_COLORARG_CURRENT			: fnListBuild.AddValue("RENDERSTYLE_COLORARG_CURRENT"); break;
							case RENDERSTYLE_COLORARG_DIFFUSE			: fnListBuild.AddValue("RENDERSTYLE_COLORARG_DIFFUSE"); break;
							case RENDERSTYLE_COLORARG_TEXTURE			: fnListBuild.AddValue("RENDERSTYLE_COLORARG_TEXTURE"); break;
							case RENDERSTYLE_COLORARG_TFACTOR			: fnListBuild.AddValue("RENDERSTYLE_COLORARG_TFACTOR"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("colorarg2");
							switch (RenderPass.TextureStages[j].ColorArg2) {
							case RENDERSTYLE_COLORARG_CURRENT			: fnListBuild.AddValue("RENDERSTYLE_COLORARG_CURRENT"); break;
							case RENDERSTYLE_COLORARG_DIFFUSE			: fnListBuild.AddValue("RENDERSTYLE_COLORARG_DIFFUSE"); break;
							case RENDERSTYLE_COLORARG_TEXTURE			: fnListBuild.AddValue("RENDERSTYLE_COLORARG_TEXTURE"); break;
							case RENDERSTYLE_COLORARG_TFACTOR			: fnListBuild.AddValue("RENDERSTYLE_COLORARG_TFACTOR"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("alphaop");
							switch (RenderPass.TextureStages[j].AlphaOp) {
							case RENDERSTYLE_ALPHAOP_DISABLE			: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_DISABLE"); break;
							case RENDERSTYLE_ALPHAOP_SELECTARG1			: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_SELECTARG1"); break;
							case RENDERSTYLE_ALPHAOP_SELECTARG2			: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_SELECTARG2"); break;
							case RENDERSTYLE_ALPHAOP_MODULATE			: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_MODULATE"); break;
							case RENDERSTYLE_ALPHAOP_MODULATEALPHA		: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_MODULATEALPHA"); break;
							case RENDERSTYLE_ALPHAOP_MODULATETEXALPHA		: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_MODULATETEXALPHA"); break;
							case RENDERSTYLE_ALPHAOP_MODULATETFACTOR	: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_MODULATETFACTOR"); break;
							case RENDERSTYLE_ALPHAOP_ADD				: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_ADD"); break;
							case RENDERSTYLE_ALPHAOP_ADDSIGNED			: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_ADDSIGNED"); break;
							case RENDERSTYLE_ALPHAOP_ADDSIGNED2X		: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_ADDSIGNED2X"); break;
							case RENDERSTYLE_ALPHAOP_SUBTRACT			: fnListBuild.AddValue("RENDERSTYLE_ALPHAOP_SUBTRACT"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("alphaarg1");
							switch (RenderPass.TextureStages[j].AlphaArg1) {
							case RENDERSTYLE_ALPHAARG_CURRENT			: fnListBuild.AddValue("RENDERSTYLE_ALPHAARG_CURRENT"); break;
							case RENDERSTYLE_ALPHAARG_DIFFUSE			: fnListBuild.AddValue("RENDERSTYLE_ALPHAARG_DIFFUSE"); break;
							case RENDERSTYLE_ALPHAARG_TEXTURE			: fnListBuild.AddValue("RENDERSTYLE_ALPHAARG_TEXTURE"); break;
							case RENDERSTYLE_ALPHAARG_TFACTOR			: fnListBuild.AddValue("RENDERSTYLE_ALPHAARG_TFACTOR"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("alphaarg2");
							switch (RenderPass.TextureStages[j].AlphaArg2) {
							case RENDERSTYLE_ALPHAARG_CURRENT			: fnListBuild.AddValue("RENDERSTYLE_ALPHAARG_CURRENT"); break;
							case RENDERSTYLE_ALPHAARG_DIFFUSE			: fnListBuild.AddValue("RENDERSTYLE_ALPHAARG_DIFFUSE"); break;
							case RENDERSTYLE_ALPHAARG_TEXTURE			: fnListBuild.AddValue("RENDERSTYLE_ALPHAARG_TEXTURE"); break;
							case RENDERSTYLE_ALPHAARG_TFACTOR			: fnListBuild.AddValue("RENDERSTYLE_ALPHAARG_TFACTOR"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("uvsource");
							switch (RenderPass.TextureStages[j].UVSource) {
							case RENDERSTYLE_UVFROM_MODELDATA_UVSET1	: fnListBuild.AddValue("RENDERSTYLE_UVFROM_MODELDATA_UVSET1"); break;
							case RENDERSTYLE_UVFROM_MODELDATA_UVSET2	: fnListBuild.AddValue("RENDERSTYLE_UVFROM_MODELDATA_UVSET2"); break;
							case RENDERSTYLE_UVFROM_MODELDATA_UVSET3	: fnListBuild.AddValue("RENDERSTYLE_UVFROM_MODELDATA_UVSET3"); break;
							case RENDERSTYLE_UVFROM_MODELDATA_UVSET4	: fnListBuild.AddValue("RENDERSTYLE_UVFROM_MODELDATA_UVSET4"); break;
							case RENDERSTYLE_UVFROM_CAMERASPACENORMAL	: fnListBuild.AddValue("RENDERSTYLE_UVFROM_CAMERASPACENORMAL"); break;
							case RENDERSTYLE_UVFROM_CAMERASPACEPOSITION	: fnListBuild.AddValue("RENDERSTYLE_UVFROM_CAMERASPACEPOSITION"); break;
							case RENDERSTYLE_UVFROM_CAMERASPACEREFLTVECT: fnListBuild.AddValue("RENDERSTYLE_UVFROM_CAMERASPACEREFLTVECT"); break;
							case RENDERSTYLE_UVFROM_WORLDSPACENORMAL	: fnListBuild.AddValue("RENDERSTYLE_UVFROM_WORLDSPACENORMAL"); break;
							case RENDERSTYLE_UVFROM_WORLDSPACEPOSITION	: fnListBuild.AddValue("RENDERSTYLE_UVFROM_WORLDSPACEPOSITION"); break;
							case RENDERSTYLE_UVFROM_WORLDSPACEREFLTVECT : fnListBuild.AddValue("RENDERSTYLE_UVFROM_WORLDSPACEREFLTVECT"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("uaddress");
							switch (RenderPass.TextureStages[j].UAddress) {
							case RENDERSTYLE_UVADDR_WRAP				: fnListBuild.AddValue("RENDERSTYLE_UVADDR_WRAP"); break;
							case RENDERSTYLE_UVADDR_CLAMP				: fnListBuild.AddValue("RENDERSTYLE_UVADDR_CLAMP"); break;
							case RENDERSTYLE_UVADDR_MIRROR				: fnListBuild.AddValue("RENDERSTYLE_UVADDR_MIRROR"); break;
							case RENDERSTYLE_UVADDR_MIRRORONCE			: fnListBuild.AddValue("RENDERSTYLE_UVADDR_MIRRORONCE"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("vaddress");
							switch (RenderPass.TextureStages[j].VAddress) {
							case RENDERSTYLE_UVADDR_WRAP				: fnListBuild.AddValue("RENDERSTYLE_UVADDR_WRAP"); break;
							case RENDERSTYLE_UVADDR_CLAMP				: fnListBuild.AddValue("RENDERSTYLE_UVADDR_CLAMP"); break;
							case RENDERSTYLE_UVADDR_MIRROR				: fnListBuild.AddValue("RENDERSTYLE_UVADDR_MIRROR"); break;
							case RENDERSTYLE_UVADDR_MIRRORONCE			: fnListBuild.AddValue("RENDERSTYLE_UVADDR_MIRRORONCE"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("texfilter");
							switch (RenderPass.TextureStages[j].TexFilter) {
							case RENDERSTYLE_TEXFILTER_POINT			: fnListBuild.AddValue("RENDERSTYLE_TEXFILTER_POINT"); break;
							case RENDERSTYLE_TEXFILTER_BILINEAR			: fnListBuild.AddValue("RENDERSTYLE_TEXFILTER_BILINEAR"); break;
							case RENDERSTYLE_TEXFILTER_TRILINEAR		: fnListBuild.AddValue("RENDERSTYLE_TEXFILTER_TRILINEAR"); break;
							case RENDERSTYLE_TEXFILTER_ANISOTROPIC		: fnListBuild.AddValue("RENDERSTYLE_TEXFILTER_ANISOTROPIC"); break;
							case RENDERSTYLE_TEXFILTER_POINT_PTMIP		: fnListBuild.AddValue("RENDERSTYLE_TEXFILTER_POINT_PTMIP"); break;
							default										: assert(0 && "Unknown value"); }
							fnListBuild.Pop();
							fnListBuild.Push("texcoordcount");
							fnListBuild.AddValue((int32)RenderPass.TextureStages[j].TexCoordCount);
							fnListBuild.Pop();
							fnListBuild.Push("uvtransform_enable");
							fnListBuild.AddValue(RenderPass.TextureStages[j].UVTransform_Enable ? "1" : "0");
							fnListBuild.Pop();
							fnListBuild.Push("uvproject_enable");
							fnListBuild.AddValue(RenderPass.TextureStages[j].ProjectTexCoord ? "1" : "0");
							fnListBuild.Pop();
							fnListBuild.Push("uvtransform_matrix"); char szTmp[32];
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[0]);  fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[1]);  fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[2]);  fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[3]);  fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[4]);  fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[5]);  fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[6]);  fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[7]);  fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[8]);  fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[9]);  fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[10]); fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[11]); fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[12]); fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[13]); fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[14]); fnListBuild.AddValue(szTmp);
							sprintf(szTmp,"%f",RenderPass.TextureStages[j].UVTransform_Matrix[15]); fnListBuild.AddValue(szTmp);
							fnListBuild.Pop();
						fnListBuild.Pop(); }

					// D3D Options...
					RSD3DRenderPass D3DOptions;
					if (m_pRenderStyle->GetRenderPass_D3DOptions(i,&D3DOptions))
					{
						fnListBuild.Push("d3doptions");
							fnListBuild.Push("usevertexshader");
							fnListBuild.AddValue(D3DOptions.bUseVertexShader ? "1" : "0");
							fnListBuild.Pop();

							fnListBuild.Push("vertexshaderid");
							sprintf(szBuf, "%d", D3DOptions.VertexShaderID);
							fnListBuild.AddValue(szBuf);
							fnListBuild.Pop();

							fnListBuild.Push("usepixelshader");
							fnListBuild.AddValue(D3DOptions.bUsePixelShader ? "1" : "0");
							fnListBuild.Pop();

							fnListBuild.Push("pixelshaderid");
							sprintf(szBuf, "%d", D3DOptions.PixelShaderID);
							fnListBuild.AddValue(szBuf);
							fnListBuild.Pop();
						fnListBuild.Pop();
  					}

					fnListBuild.Pop();
			}
		RSD3DOptions rsD3DOptions;
		if (m_pRenderStyle->GetDirect3D_Options(&rsD3DOptions))
		{
			char szBuf[16];
			fnListBuild.Push("rsd3doptions");
				
				fnListBuild.Push("useeffectshader");
				fnListBuild.AddValue(rsD3DOptions.bUseEffectShader ? "1" : "0");
				fnListBuild.Pop();

				fnListBuild.Push("effectshaderid");
				sprintf(szBuf, "%d", rsD3DOptions.EffectShaderID);
				fnListBuild.AddValue(szBuf);
				fnListBuild.Pop();

			fnListBuild.Pop();
		}

		// Debug Params (from the dialog's data)...
		fnListBuild.Push("debugparams");
			fnListBuild.Push("userefrast");
			fnListBuild.AddValue(m_DebugParams.bUseRefRast ? "1" : "0");
			fnListBuild.Pop();
			fnListBuild.Push("renderonidle");
			fnListBuild.AddValue(m_DebugParams.bRenderOnIdle ? "1" : "0");
			fnListBuild.Pop();
			fnListBuild.Push("rotatemodel");
			fnListBuild.AddValue(m_DebugParams.bRotateModel ? "1" : "0");
			fnListBuild.Pop();
			fnListBuild.Push("lightconfig"); char szTmp[32];
			sprintf(szTmp,"%d",m_DebugParams.LightConfig); fnListBuild.AddValue(szTmp);
			fnListBuild.Pop();
			fnListBuild.Push("lightcount");
			sprintf(szTmp,"%d",m_DebugParams.LightCount); fnListBuild.AddValue(szTmp);
			fnListBuild.Pop();
			fnListBuild.Push("lightcolor");
			sprintf(szTmp,"%f",m_DebugParams.LightColor.r); fnListBuild.AddValue(szTmp);
			sprintf(szTmp,"%f",m_DebugParams.LightColor.g); fnListBuild.AddValue(szTmp);
			sprintf(szTmp,"%f",m_DebugParams.LightColor.b); fnListBuild.AddValue(szTmp);
			sprintf(szTmp,"%f",m_DebugParams.LightColor.a); fnListBuild.AddValue(szTmp);
			fnListBuild.Pop();
			fnListBuild.Push("camerarotationspeed");
			sprintf(szTmp,"%f",m_DebugParams.fCameraRotationSpeed); fnListBuild.AddValue(szTmp);
			fnListBuild.Pop();
			if (!m_DebugParams.BackGndImg_Filename.empty()) {
				fnListBuild.Push("backgndimg_filename");
				string szTmp1 = "\""; szTmp1 += m_DebugParams.BackGndImg_Filename.c_str(); szTmp1 += "\"";
				fnListBuild.AddValue(szTmp1.c_str());
				fnListBuild.Pop(); }
			fnListBuild.Push("backgndimg_color");
			sprintf(szTmp,"%f",m_DebugParams.BackGndImg_Color.r); fnListBuild.AddValue(szTmp);
			sprintf(szTmp,"%f",m_DebugParams.BackGndImg_Color.g); fnListBuild.AddValue(szTmp);
			sprintf(szTmp,"%f",m_DebugParams.BackGndImg_Color.b); fnListBuild.AddValue(szTmp);
			sprintf(szTmp,"%f",m_DebugParams.BackGndImg_Color.a); fnListBuild.AddValue(szTmp);
			fnListBuild.Pop();
			fnListBuild.Push("modelname");
			fnListBuild.AddValue(m_DebugParams.ModelName.c_str());
			fnListBuild.Pop();
			if (!m_DebugParams.TextureList_Filename[0].empty()) {
				fnListBuild.Push("texture1"); string szTmp = "\""; szTmp += m_DebugParams.TextureList_Filename[0].c_str(); szTmp += "\"";
				fnListBuild.AddValue(szTmp.c_str());
				fnListBuild.Pop(); }
			if (!m_DebugParams.TextureList_Filename[1].empty()) {
				fnListBuild.Push("texture2"); string szTmp = "\""; szTmp += m_DebugParams.TextureList_Filename[1].c_str(); szTmp += "\"";
				fnListBuild.AddValue(szTmp.c_str());
				fnListBuild.Pop(); }
			if (!m_DebugParams.TextureList_Filename[2].empty()) {
				fnListBuild.Push("texture3"); string szTmp = "\""; szTmp += m_DebugParams.TextureList_Filename[2].c_str(); szTmp += "\"";
				fnListBuild.AddValue(szTmp.c_str());
				fnListBuild.Pop(); }
			if (!m_DebugParams.TextureList_Filename[3].empty()) {
				fnListBuild.Push("texture4"); string szTmp = "\""; szTmp += m_DebugParams.TextureList_Filename[3].c_str(); szTmp += "\"";
				fnListBuild.AddValue(szTmp.c_str());
				fnListBuild.Pop(); }
			if (!m_DebugParams.EffectFilename.empty()) {
				fnListBuild.Push("effectfilename"); string szTmp = "\""; szTmp += m_DebugParams.EffectFilename.c_str(); szTmp += "\"";
				fnListBuild.AddValue(szTmp.c_str());
				fnListBuild.Pop(); }
		fnListBuild.Pop();

		fnListBuild.Pop();
		CLTANode* pNode = fnListBuild.DetachHead();
		PrettyPrint(ltaFile,pNode,0);
		Allocator.FreeNode(pNode);


		// Check if folder is labelled DirTypeRenderStyles, and add file
		// if it is not

		char path[512], drive[8];

		_splitpath(szFileName, drive, path, buffer, buffer);
		sprintf(buffer, "%s%s", drive, path);

		if (!DoesDirHaveFile(buffer, "DirTypeRenderStyles"))
		{
			FILE *fp;
			sprintf(path, "%sDirTypeRenderStyles", buffer);

			if( fp = fopen(path, "wb") )
				fclose(fp);
		}

		return true; }
	return false;
}

// ------------------------------------------------------------------------
// PrettyPrint( out-stream, root-parsenode, depth )
// ------------------------------------------------------------------------
static char* s_tabs = NULL;
static int   s_tabsSize = 0;
static int	 Push = 0;

static void Tabs_init(int num_tabs)
{
	if (s_tabs == NULL) {
		s_tabs = new char [num_tabs];
		s_tabsSize = num_tabs ;
		for (int i = 0; i < num_tabs-1; i++) {
			s_tabs[i] = '\t'; }
		s_tabs[i] = '\0'; } // terminatestring
}

static char* Tabs_indent(int size)
{
	if (size < s_tabsSize) return &(s_tabs[(s_tabsSize-1)-size]);
	else  return &(s_tabs[s_tabsSize-1]);
}

void PrettyPrint( ostream & os,  CLTANode * node, int depth )
{
	int anon_list = 0; // is this an anonymous list no name in 1st place
	if (s_tabs == NULL ) Tabs_init(30);

	if (node != NULL) {
		if (!node->IsList()) {
			if (node->IsString()) os << "\""<< node->GetValue() <<"\" ";
			else os << node->GetValue() << " "; }
		else {
			if (node->GetNumElements() > 0)
				if (!node->GetElement(0)->IsList()) {
					anon_list = 1;
					if (Push == 1) os << endl << Tabs_indent(depth) << "(";
					else { os << endl<<Tabs_indent(depth)<<  "(" ; }
					Push = 0 ; }
				else {
					os <<  "(";
					Push = 1; }

			for (int i = 0; i < (int)node->GetNumElements(); i++) {
				CLTANode *child = node->GetElement(i);
				if (Push) depth--;
				PrettyPrint(os,child,depth +1);
				if ((i+1) % 5 == 0 && anon_list)
					os << endl << Tabs_indent(depth) <<  " "; }
			if (node->GetNumElements()) os << ")" ;
			else os << "()"; } }
	else {
		os << "error in parse tree" << endl; }
}
