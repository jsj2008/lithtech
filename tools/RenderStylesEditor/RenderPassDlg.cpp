
#include "stdafx.h"
#include "RenderPassDlg.h"
//#include "VertexShaderDlg.h"
#include "MatrixConfigDlg.h"

BEGIN_MESSAGE_MAP(CRenderPassDlg, CDialog)
    //{{AFX_MSG_MAP(CRenderPassDlg)
	ON_BN_CLICKED(IDC_DYNAMICLIGHTING, OnDynamicLight)
	ON_EN_CHANGE(IDC_ALPHAREF, OnAlphaRef)
	ON_EN_CHANGE(IDC_TEXTUREFACTOR, OnTFactor)
	ON_CBN_SELCHANGE(IDC_BLEND, OnBlend)
	ON_CBN_SELCHANGE(IDC_ZBUFFEROP, OnZBuffer)
	ON_CBN_SELCHANGE(IDC_CULLMODE, OnCullMode)
	ON_CBN_SELCHANGE(IDC_ALPHATEST, OnAlphaTestMode)
	ON_CBN_SELCHANGE(IDC_ZBUFFERTEST, OnZBufferTestMode)
	ON_CBN_SELCHANGE(IDC_FILLMODE, OnFillMode)
	ON_CBN_SELCHANGE(IDC_TEXTURE1, OnTextureParam1)
	ON_CBN_SELCHANGE(IDC_TEXTURE2, OnTextureParam2)
	ON_CBN_SELCHANGE(IDC_TEXTURE3, OnTextureParam3)
	ON_CBN_SELCHANGE(IDC_TEXTURE4, OnTextureParam4)
	ON_CBN_SELCHANGE(IDC_COLOROP1, OnColorOp1)
	ON_CBN_SELCHANGE(IDC_COLOROP2, OnColorOp2)
	ON_CBN_SELCHANGE(IDC_COLOROP3, OnColorOp3)
	ON_CBN_SELCHANGE(IDC_COLOROP4, OnColorOp4)
	ON_CBN_SELCHANGE(IDC_COLORARG11, OnColorArg11)
	ON_CBN_SELCHANGE(IDC_COLORARG12, OnColorArg12)
	ON_CBN_SELCHANGE(IDC_COLORARG13, OnColorArg13)
	ON_CBN_SELCHANGE(IDC_COLORARG14, OnColorArg14)
	ON_CBN_SELCHANGE(IDC_COLORARG21, OnColorArg21)
	ON_CBN_SELCHANGE(IDC_COLORARG22, OnColorArg22)
	ON_CBN_SELCHANGE(IDC_COLORARG23, OnColorArg23)
	ON_CBN_SELCHANGE(IDC_COLORARG24, OnColorArg24)
  	ON_CBN_SELCHANGE(IDC_ALPHAOP1, OnAlphaOp1)
	ON_CBN_SELCHANGE(IDC_ALPHAOP2, OnAlphaOp2)
	ON_CBN_SELCHANGE(IDC_ALPHAOP3, OnAlphaOp3)
	ON_CBN_SELCHANGE(IDC_ALPHAOP4, OnAlphaOp4)
	ON_CBN_SELCHANGE(IDC_ALPHAARG11, OnAlphaArg11)
	ON_CBN_SELCHANGE(IDC_ALPHAARG12, OnAlphaArg12)
	ON_CBN_SELCHANGE(IDC_ALPHAARG13, OnAlphaArg13)
	ON_CBN_SELCHANGE(IDC_ALPHAARG14, OnAlphaArg14)
	ON_CBN_SELCHANGE(IDC_ALPHAARG21, OnAlphaArg21)
	ON_CBN_SELCHANGE(IDC_ALPHAARG22, OnAlphaArg22)
	ON_CBN_SELCHANGE(IDC_ALPHAARG23, OnAlphaArg23)
	ON_CBN_SELCHANGE(IDC_ALPHAARG24, OnAlphaArg24)
  	ON_CBN_SELCHANGE(IDC_UVSOURCE1, OnUVSource1)
	ON_CBN_SELCHANGE(IDC_UVSOURCE2, OnUVSource2)
	ON_CBN_SELCHANGE(IDC_UVSOURCE3, OnUVSource3)
	ON_CBN_SELCHANGE(IDC_UVSOURCE4, OnUVSource4)
  	ON_CBN_SELCHANGE(IDC_UADDRESS1, OnUAddress1)
	ON_CBN_SELCHANGE(IDC_UADDRESS2, OnUAddress2)
	ON_CBN_SELCHANGE(IDC_UADDRESS3, OnUAddress3)
	ON_CBN_SELCHANGE(IDC_UADDRESS4, OnUAddress4)
  	ON_CBN_SELCHANGE(IDC_VADDRESS1, OnVAddress1)
	ON_CBN_SELCHANGE(IDC_VADDRESS2, OnVAddress2)
	ON_CBN_SELCHANGE(IDC_VADDRESS3, OnVAddress3)
	ON_CBN_SELCHANGE(IDC_VADDRESS4, OnVAddress4)
  	ON_CBN_SELCHANGE(IDC_TEXFILTER1, OnTexFilter1)
	ON_CBN_SELCHANGE(IDC_TEXFILTER2, OnTexFilter2)
	ON_CBN_SELCHANGE(IDC_TEXFILTER3, OnTexFilter3)
	ON_CBN_SELCHANGE(IDC_TEXFILTER4, OnTexFilter4)
	ON_BN_CLICKED(IDC_UVTRANSFORM_ENABLE1, OnUVTransform_Enable1)
	ON_BN_CLICKED(IDC_UVTRANSFORM_ENABLE2, OnUVTransform_Enable2)
	ON_BN_CLICKED(IDC_UVTRANSFORM_ENABLE3, OnUVTransform_Enable3)
	ON_BN_CLICKED(IDC_UVTRANSFORM_ENABLE4, OnUVTransform_Enable4)
	ON_BN_CLICKED(IDC_UVTRANSFORM_MATRIX1, OnUVTransform_Matrix1)
	ON_BN_CLICKED(IDC_UVTRANSFORM_MATRIX2, OnUVTransform_Matrix2)
	ON_BN_CLICKED(IDC_UVTRANSFORM_MATRIX3, OnUVTransform_Matrix3)
	ON_BN_CLICKED(IDC_UVTRANSFORM_MATRIX4, OnUVTransform_Matrix4)
	ON_BN_CLICKED(IDC_VERTEXSHADER_ENABLE, OnD3DOptions_VertexShader_Enable)
	ON_BN_CLICKED(IDC_PIXELSHADER_ENABLE, OnD3DOptions_PixelShader_Enable)
	ON_BN_CLICKED(IDC_BUMPENVMAP, OnD3DOptions_BumpEnvMap_Enable)
	ON_EN_CHANGE(IDC_BUMPENVMAP_STAGE, OnD3DOptions_BumpEnvMap_Stage)
	ON_EN_CHANGE(IDC_BUMPENVMAP_SCALE, OnD3DOptions_BumpEnvMap_Scale)
	ON_EN_CHANGE(IDC_BUMPENVMAP_OFFSET, OnD3DOptions_BumpEnvMap_Offset)
	ON_EN_CHANGE(IDC_VERTEXSHADER_ID, OnD3DOptions_VertexShader_ID)
	ON_EN_CHANGE(IDC_PIXELSHADER_ID, OnD3DOptions_PixelShader_ID)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Grab the dialog controls...
BOOL CRenderPassDlg::OnInitDialog()
{
	m_pBn_DynamicLight			= (CButton*)GetDlgItem(IDC_DYNAMICLIGHTING);		assert(m_pBn_DynamicLight);
	m_pEB_AlphaRef				= (CEdit*)GetDlgItem(IDC_ALPHAREF);					assert(m_pEB_AlphaRef);
	m_pEB_TFactor				= (CEdit*)GetDlgItem(IDC_TEXTUREFACTOR);			assert(m_pEB_TFactor);
	m_pCB_BlendOp				= (CComboBox*)GetDlgItem(IDC_BLEND);				assert(m_pCB_BlendOp);
	m_pCB_ZBuffer				= (CComboBox*)GetDlgItem(IDC_ZBUFFEROP);			assert(m_pCB_ZBuffer);
	m_pCB_CullMode				= (CComboBox*)GetDlgItem(IDC_CULLMODE);				assert(m_pCB_CullMode);
	m_pCB_AlphaTest				= (CComboBox*)GetDlgItem(IDC_ALPHATEST);			assert(m_pCB_AlphaTest);
	m_pCB_ZBufferTest			= (CComboBox*)GetDlgItem(IDC_ZBUFFERTEST);			assert(m_pCB_ZBufferTest);
	m_pCB_FillMode				= (CComboBox*)GetDlgItem(IDC_FILLMODE);				assert(m_pCB_FillMode);
	m_pCB_TextureParam[0]		= (CComboBox*)GetDlgItem(IDC_TEXTURE1);				assert(m_pCB_TextureParam[0]);
	m_pCB_TextureParam[1]		= (CComboBox*)GetDlgItem(IDC_TEXTURE2);				assert(m_pCB_TextureParam[1]);
	m_pCB_TextureParam[2]		= (CComboBox*)GetDlgItem(IDC_TEXTURE3);				assert(m_pCB_TextureParam[2]);
	m_pCB_TextureParam[3]		= (CComboBox*)GetDlgItem(IDC_TEXTURE4);				assert(m_pCB_TextureParam[3]);
	m_pCB_ColorOp[0]			= (CComboBox*)GetDlgItem(IDC_COLOROP1);				assert(m_pCB_ColorOp[0]);
	m_pCB_ColorOp[1]			= (CComboBox*)GetDlgItem(IDC_COLOROP2);				assert(m_pCB_ColorOp[1]);
	m_pCB_ColorOp[2]			= (CComboBox*)GetDlgItem(IDC_COLOROP3);				assert(m_pCB_ColorOp[2]);
	m_pCB_ColorOp[3]			= (CComboBox*)GetDlgItem(IDC_COLOROP4);				assert(m_pCB_ColorOp[3]);
	m_pCB_ColorArg1[0]			= (CComboBox*)GetDlgItem(IDC_COLORARG11);			assert(m_pCB_ColorArg1[0]);
	m_pCB_ColorArg1[1]			= (CComboBox*)GetDlgItem(IDC_COLORARG12);			assert(m_pCB_ColorArg1[1]);
	m_pCB_ColorArg1[2]			= (CComboBox*)GetDlgItem(IDC_COLORARG13);			assert(m_pCB_ColorArg1[2]);
	m_pCB_ColorArg1[3]			= (CComboBox*)GetDlgItem(IDC_COLORARG14);			assert(m_pCB_ColorArg1[3]);
	m_pCB_ColorArg2[0]			= (CComboBox*)GetDlgItem(IDC_COLORARG21);			assert(m_pCB_ColorArg2[0]);
	m_pCB_ColorArg2[1]			= (CComboBox*)GetDlgItem(IDC_COLORARG22);			assert(m_pCB_ColorArg2[1]);
	m_pCB_ColorArg2[2]			= (CComboBox*)GetDlgItem(IDC_COLORARG23);			assert(m_pCB_ColorArg2[2]);
	m_pCB_ColorArg2[3]			= (CComboBox*)GetDlgItem(IDC_COLORARG24);			assert(m_pCB_ColorArg2[3]);
	m_pCB_AlphaOp[0]			= (CComboBox*)GetDlgItem(IDC_ALPHAOP1);				assert(m_pCB_AlphaOp[0]);
	m_pCB_AlphaOp[1]			= (CComboBox*)GetDlgItem(IDC_ALPHAOP2);				assert(m_pCB_AlphaOp[1]);
	m_pCB_AlphaOp[2]			= (CComboBox*)GetDlgItem(IDC_ALPHAOP3);				assert(m_pCB_AlphaOp[2]);
	m_pCB_AlphaOp[3]			= (CComboBox*)GetDlgItem(IDC_ALPHAOP4);				assert(m_pCB_AlphaOp[3]);
	m_pCB_AlphaArg1[0]			= (CComboBox*)GetDlgItem(IDC_ALPHAARG11);			assert(m_pCB_AlphaArg1[0]);
	m_pCB_AlphaArg1[1]			= (CComboBox*)GetDlgItem(IDC_ALPHAARG12);			assert(m_pCB_AlphaArg1[1]);
	m_pCB_AlphaArg1[2]			= (CComboBox*)GetDlgItem(IDC_ALPHAARG13);			assert(m_pCB_AlphaArg1[2]);
	m_pCB_AlphaArg1[3]			= (CComboBox*)GetDlgItem(IDC_ALPHAARG14);			assert(m_pCB_AlphaArg1[3]);
	m_pCB_AlphaArg2[0]			= (CComboBox*)GetDlgItem(IDC_ALPHAARG21);			assert(m_pCB_AlphaArg2[0]);
	m_pCB_AlphaArg2[1]			= (CComboBox*)GetDlgItem(IDC_ALPHAARG22);			assert(m_pCB_AlphaArg2[1]);
	m_pCB_AlphaArg2[2]			= (CComboBox*)GetDlgItem(IDC_ALPHAARG23);			assert(m_pCB_AlphaArg2[2]);
	m_pCB_AlphaArg2[3]			= (CComboBox*)GetDlgItem(IDC_ALPHAARG24);			assert(m_pCB_AlphaArg2[3]);
	m_pCB_UVSource[0]			= (CComboBox*)GetDlgItem(IDC_UVSOURCE1);			assert(m_pCB_UVSource[0]);
	m_pCB_UVSource[1]			= (CComboBox*)GetDlgItem(IDC_UVSOURCE2);			assert(m_pCB_UVSource[1]);
	m_pCB_UVSource[2]			= (CComboBox*)GetDlgItem(IDC_UVSOURCE3);			assert(m_pCB_UVSource[2]);
	m_pCB_UVSource[3]			= (CComboBox*)GetDlgItem(IDC_UVSOURCE4);			assert(m_pCB_UVSource[3]);
	m_pCB_UAddress[0]			= (CComboBox*)GetDlgItem(IDC_UADDRESS1);			assert(m_pCB_UAddress[0]);
	m_pCB_UAddress[1]			= (CComboBox*)GetDlgItem(IDC_UADDRESS2);			assert(m_pCB_UAddress[1]);
	m_pCB_UAddress[2]			= (CComboBox*)GetDlgItem(IDC_UADDRESS3);			assert(m_pCB_UAddress[2]);
	m_pCB_UAddress[3]			= (CComboBox*)GetDlgItem(IDC_UADDRESS4);			assert(m_pCB_UAddress[3]);
	m_pCB_VAddress[0]			= (CComboBox*)GetDlgItem(IDC_VADDRESS1);			assert(m_pCB_VAddress[0]);
	m_pCB_VAddress[1]			= (CComboBox*)GetDlgItem(IDC_VADDRESS2);			assert(m_pCB_VAddress[1]);
	m_pCB_VAddress[2]			= (CComboBox*)GetDlgItem(IDC_VADDRESS3);			assert(m_pCB_VAddress[2]);
	m_pCB_VAddress[3]			= (CComboBox*)GetDlgItem(IDC_VADDRESS4);			assert(m_pCB_VAddress[3]);
	m_pCB_TexFilter[0]			= (CComboBox*)GetDlgItem(IDC_TEXFILTER1);			assert(m_pCB_TexFilter[0]);
	m_pCB_TexFilter[1]			= (CComboBox*)GetDlgItem(IDC_TEXFILTER2);			assert(m_pCB_TexFilter[1]);
	m_pCB_TexFilter[2]			= (CComboBox*)GetDlgItem(IDC_TEXFILTER3);			assert(m_pCB_TexFilter[2]);
	m_pCB_TexFilter[3]			= (CComboBox*)GetDlgItem(IDC_TEXFILTER4);			assert(m_pCB_TexFilter[3]);
	m_pCB_UVTransform_Enable[0]	= (CButton*)GetDlgItem(IDC_UVTRANSFORM_ENABLE1);	assert(m_pCB_UVTransform_Enable[0]);
	m_pCB_UVTransform_Enable[1]	= (CButton*)GetDlgItem(IDC_UVTRANSFORM_ENABLE2);	assert(m_pCB_UVTransform_Enable[1]);
	m_pCB_UVTransform_Enable[2]	= (CButton*)GetDlgItem(IDC_UVTRANSFORM_ENABLE3);	assert(m_pCB_UVTransform_Enable[2]);
	m_pCB_UVTransform_Enable[3]	= (CButton*)GetDlgItem(IDC_UVTRANSFORM_ENABLE4);	assert(m_pCB_UVTransform_Enable[3]);
	m_pCB_UVTransform_Matrix[0]	= (CButton*)GetDlgItem(IDC_UVTRANSFORM_MATRIX1);	assert(m_pCB_UVTransform_Matrix[0]);
	m_pCB_UVTransform_Matrix[1]	= (CButton*)GetDlgItem(IDC_UVTRANSFORM_MATRIX2);	assert(m_pCB_UVTransform_Matrix[1]);
	m_pCB_UVTransform_Matrix[2]	= (CButton*)GetDlgItem(IDC_UVTRANSFORM_MATRIX3);	assert(m_pCB_UVTransform_Matrix[2]);
	m_pCB_UVTransform_Matrix[3]	= (CButton*)GetDlgItem(IDC_UVTRANSFORM_MATRIX4);	assert(m_pCB_UVTransform_Matrix[3]);
	m_pBn_VertexShader_Enable	= (CButton*)GetDlgItem(IDC_VERTEXSHADER_ENABLE);	assert(m_pBn_VertexShader_Enable);
	m_pBn_PixelShader_Enable	= (CButton*)GetDlgItem(IDC_PIXELSHADER_ENABLE);		assert(m_pBn_PixelShader_Enable);
	m_pBn_BumpEnvMap_Enable		= (CButton*)GetDlgItem(IDC_BUMPENVMAP);				assert(m_pBn_BumpEnvMap_Enable);
	m_pEB_BumpEnvMapStage		= (CEdit*)GetDlgItem(IDC_BUMPENVMAP_STAGE);			assert(m_pEB_BumpEnvMapStage);
	m_pEB_BumpEnvMapScale		= (CEdit*)GetDlgItem(IDC_BUMPENVMAP_SCALE);			assert(m_pEB_BumpEnvMapScale);
	m_pEB_BumpEnvMapOffset		= (CEdit*)GetDlgItem(IDC_BUMPENVMAP_OFFSET);		assert(m_pEB_BumpEnvMapOffset);
	m_pEB_VertexShader_ID		= (CEdit*)GetDlgItem(IDC_VERTEXSHADER_ID);			assert(m_pEB_VertexShader_ID);
	m_pEB_PixelShader_ID		= (CEdit*)GetDlgItem(IDC_PIXELSHADER_ID);			assert(m_pEB_PixelShader_ID);

	SetWindowText(m_pRenderPassData->DialogText.c_str());				// Set the dialog text...

	SetWindowPos(&wndTop, m_pRenderPassData->PositionX, m_pRenderPassData->PositionY,  // Set dialog position
				 0, 0, SWP_NOSIZE);
	UpdateWindow();

	PopulateDialogControls_And_CreateIDtoRSEnum_Maps();					// Populate the Controls...
	SetDialogControls_From_RenderStyleData();							// Go ahead and setup the initial values...

	return true;
}

void CRenderPassDlg::PopulateDialogControls_And_CreateIDtoRSEnum_Maps()
{
	m_IDtoRSEnum_TestMode.resize(7);								// AlphaTest Render State...
	m_IDtoRSEnum_TestMode[0] = RENDERSTYLE_NOALPHATEST;
	m_IDtoRSEnum_TestMode[1] = RENDERSTYLE_ALPHATEST_LESS;
	m_IDtoRSEnum_TestMode[2] = RENDERSTYLE_ALPHATEST_LESSEQUAL;
	m_IDtoRSEnum_TestMode[3] = RENDERSTYLE_ALPHATEST_GREATER;
	m_IDtoRSEnum_TestMode[4] = RENDERSTYLE_ALPHATEST_GREATEREQUAL;
	m_IDtoRSEnum_TestMode[5] = RENDERSTYLE_ALPHATEST_EQUAL;
	m_IDtoRSEnum_TestMode[6] = RENDERSTYLE_ALPHATEST_NOTEQUAL;

	m_pCB_AlphaTest->AddString("No Alpha Test");
	m_pCB_AlphaTest->AddString("Less");
	m_pCB_AlphaTest->AddString("Less or Equal");
	m_pCB_AlphaTest->AddString("Greater");
	m_pCB_AlphaTest->AddString("Greater or Equal");
	m_pCB_AlphaTest->AddString("Equal");
	m_pCB_AlphaTest->AddString("Not Equal");
	m_pCB_AlphaTest->SetCurSel(0);

	m_pCB_ZBufferTest->AddString("No Alpha Test");
	m_pCB_ZBufferTest->AddString("Less");
	m_pCB_ZBufferTest->AddString("Less or Equal");
	m_pCB_ZBufferTest->AddString("Greater");
	m_pCB_ZBufferTest->AddString("Greater or Equal");
	m_pCB_ZBufferTest->AddString("Equal");
	m_pCB_ZBufferTest->AddString("Not Equal");
	m_pCB_ZBufferTest->SetCurSel(0);

	m_IDtoRSEnum_FillMode.resize(2);									// Fill Render State...
	m_pCB_FillMode->AddString("Wireframe");								m_IDtoRSEnum_FillMode[0] = RENDERSTYLE_WIRE;
	m_pCB_FillMode->AddString("Solid");									m_IDtoRSEnum_FillMode[1] = RENDERSTYLE_FILL;
	m_pCB_FillMode->SetCurSel(0);

	// Blend Control...
	m_pCB_BlendOp->AddString("No Blend");
	m_pCB_BlendOp->AddString("Cs + Cd");
	m_pCB_BlendOp->AddString("Cs * (1 - Cd) + Cd");
	m_pCB_BlendOp->AddString("Cs * As + Cd * (1 - As)");
	m_pCB_BlendOp->AddString("Cs * Cs + Cd * (1 - Cs)");
	m_pCB_BlendOp->AddString("Cs * Cd + Cd * (1 - Cd)");
	m_pCB_BlendOp->AddString("Cs * Cs + Cd * Cd");
	m_pCB_BlendOp->AddString("Cs * Cs + Cd");
	m_pCB_BlendOp->AddString("Cs * As");
	m_pCB_BlendOp->AddString("Cs * As + Cd");
	m_pCB_BlendOp->AddString("Cs * Cd");
	m_pCB_BlendOp->SetCurSel(0);

	m_IDtoRSEnum_ZBufferMode.resize(3);									// ZBuffer Render State...
	m_pCB_ZBuffer->AddString("Read/Write");								m_IDtoRSEnum_ZBufferMode[0] = RENDERSTYLE_ZRW;
	m_pCB_ZBuffer->AddString("Read Only");								m_IDtoRSEnum_ZBufferMode[1] = RENDERSTYLE_ZRO;
	m_pCB_ZBuffer->AddString("No Z");									m_IDtoRSEnum_ZBufferMode[2] = RENDERSTYLE_NOZ;
	m_pCB_ZBuffer->SetCurSel(0);

	m_IDtoRSEnum_CullMode.resize(3);									// Cull Render State...
	m_pCB_CullMode->AddString("No Culling");							m_IDtoRSEnum_CullMode[0] = RENDERSTYLE_CULL_NONE;
	m_pCB_CullMode->AddString("CntrClockWise");							m_IDtoRSEnum_CullMode[1] = RENDERSTYLE_CULL_CCW;
	m_pCB_CullMode->AddString("Clockwise");								m_IDtoRSEnum_CullMode[2] = RENDERSTYLE_CULL_CW;
	m_pCB_CullMode->SetCurSel(0);

	m_IDtoRSEnum_TextureParam.resize(5);
	for (uint32 i=0;i<4;++i) {
		m_pCB_TextureParam[i]->AddString("No Texture");					m_IDtoRSEnum_TextureParam[0] = RENDERSTYLE_NOTEXTURE;
		m_pCB_TextureParam[i]->AddString("Texture 1");					m_IDtoRSEnum_TextureParam[1] = RENDERSTYLE_USE_TEXTURE1;
		m_pCB_TextureParam[i]->AddString("Texture 2");					m_IDtoRSEnum_TextureParam[2] = RENDERSTYLE_USE_TEXTURE2;
		m_pCB_TextureParam[i]->AddString("Texture 3");					m_IDtoRSEnum_TextureParam[3] = RENDERSTYLE_USE_TEXTURE3;
		m_pCB_TextureParam[i]->AddString("Texture 4");					m_IDtoRSEnum_TextureParam[4] = RENDERSTYLE_USE_TEXTURE4;			assert(4 < m_IDtoRSEnum_TextureParam.size());
		m_pCB_TextureParam[i]->SetCurSel(0); }

	for (i=0;i<4;++i) {
		m_pCB_ColorOp[i]->AddString("Disable");
		m_pCB_ColorOp[i]->AddString("Select Arg1");
		m_pCB_ColorOp[i]->AddString("Select Arg2");
		m_pCB_ColorOp[i]->AddString("Modulate");
		m_pCB_ColorOp[i]->AddString("Modulate2x");
		m_pCB_ColorOp[i]->AddString("Mod CurAlpha");
		m_pCB_ColorOp[i]->AddString("Mod TexAlpha");
		m_pCB_ColorOp[i]->AddString("Mod TFactor");
		m_pCB_ColorOp[i]->AddString("Add");
		m_pCB_ColorOp[i]->AddString("DotProduct3");
		m_pCB_ColorOp[i]->AddString("BumpEnvMap");
		m_pCB_ColorOp[i]->AddString("BumpEnvMapLum");
		m_pCB_ColorOp[i]->AddString("AddSigned");
		m_pCB_ColorOp[i]->AddString("AddSigned2x");
		m_pCB_ColorOp[i]->AddString("Subtract");
		m_pCB_ColorOp[i]->AddString("Add Mod Alpha");
		m_pCB_ColorOp[i]->AddString("Add Mod Inv Alpha");
		m_pCB_ColorOp[i]->SetCurSel(0);
	}

	for (i=0;i<4;++i) {
		m_pCB_ColorArg1[i]->AddString("Current");
		m_pCB_ColorArg1[i]->AddString("Diffuse");
		m_pCB_ColorArg1[i]->AddString("Texture");
		m_pCB_ColorArg1[i]->AddString("TFactor");
		m_pCB_ColorArg1[i]->SetCurSel(0); }

	m_IDtoRSEnum_ColorArg2.resize(4);
	for (i=0;i<4;++i) {
		m_pCB_ColorArg2[i]->AddString("Current");						m_IDtoRSEnum_ColorArg2[0] = RENDERSTYLE_COLORARG_CURRENT;
		m_pCB_ColorArg2[i]->AddString("Diffuse");						m_IDtoRSEnum_ColorArg2[1] = RENDERSTYLE_COLORARG_DIFFUSE;
		m_pCB_ColorArg2[i]->AddString("Texture");						m_IDtoRSEnum_ColorArg2[2] = RENDERSTYLE_COLORARG_TEXTURE;
		m_pCB_ColorArg2[i]->AddString("TFactor");						m_IDtoRSEnum_ColorArg2[3] = RENDERSTYLE_COLORARG_TFACTOR;			assert(3 < m_IDtoRSEnum_ColorArg2.size());
		m_pCB_ColorArg2[i]->SetCurSel(0); }

	for (i=0;i<4;++i)
	{
		m_pCB_AlphaOp[i]->AddString("Disable");							m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_DISABLE);
		m_pCB_AlphaOp[i]->AddString("Select Arg1");						m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_SELECTARG1);
		m_pCB_AlphaOp[i]->AddString("Select Arg2");						m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_SELECTARG2);
		m_pCB_AlphaOp[i]->AddString("Modulate");						m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_MODULATE);
		m_pCB_AlphaOp[i]->AddString("Mod CurAlpha");					m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_MODULATEALPHA);
		m_pCB_AlphaOp[i]->AddString("Mod TexAlpha");					m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_MODULATETEXALPHA);
		m_pCB_AlphaOp[i]->AddString("Mod TFactor");						m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_MODULATETFACTOR);
		m_pCB_AlphaOp[i]->AddString("Add");								m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_ADD);
		m_pCB_AlphaOp[i]->AddString("AddSigned");						m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_ADDSIGNED);
		m_pCB_AlphaOp[i]->AddString("AddSigned2x");						m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_ADDSIGNED2X);
		m_pCB_AlphaOp[i]->AddString("Subtract");						m_IDtoRSEnum_AlphaOp.push_back(RENDERSTYLE_ALPHAOP_SUBTRACT);
		m_pCB_AlphaOp[i]->SetCurSel(0);
	}

	m_IDtoRSEnum_AlphaArg1.resize(4);
	for (i=0;i<4;++i) {
		m_pCB_AlphaArg1[i]->AddString("Current");						m_IDtoRSEnum_AlphaArg1[0] = RENDERSTYLE_ALPHAARG_CURRENT;
		m_pCB_AlphaArg1[i]->AddString("Diffuse");						m_IDtoRSEnum_AlphaArg1[1] = RENDERSTYLE_ALPHAARG_DIFFUSE;
		m_pCB_AlphaArg1[i]->AddString("Texture");						m_IDtoRSEnum_AlphaArg1[2] = RENDERSTYLE_ALPHAARG_TEXTURE;
		m_pCB_AlphaArg1[i]->AddString("TFactor");						m_IDtoRSEnum_AlphaArg1[3] = RENDERSTYLE_ALPHAARG_TFACTOR;			assert(3 < m_IDtoRSEnum_AlphaArg1.size());
		m_pCB_AlphaArg1[i]->SetCurSel(0); }

	m_IDtoRSEnum_AlphaArg2.resize(4);
	for (i=0;i<4;++i) {
		m_pCB_AlphaArg2[i]->AddString("Current");						m_IDtoRSEnum_AlphaArg2[0] = RENDERSTYLE_ALPHAARG_CURRENT;
		m_pCB_AlphaArg2[i]->AddString("Diffuse");						m_IDtoRSEnum_AlphaArg2[1] = RENDERSTYLE_ALPHAARG_DIFFUSE;
		m_pCB_AlphaArg2[i]->AddString("Texture");						m_IDtoRSEnum_AlphaArg2[2] = RENDERSTYLE_ALPHAARG_TEXTURE;
		m_pCB_AlphaArg2[i]->AddString("TFactor");						m_IDtoRSEnum_AlphaArg2[3] = RENDERSTYLE_ALPHAARG_TFACTOR;			assert(3 < m_IDtoRSEnum_AlphaArg2.size());
		m_pCB_AlphaArg2[i]->SetCurSel(0); }

	for (i=0;i<4;++i)
	{
		m_pCB_UVSource[i]->AddString("UV Set 0");						m_IDtoRSEnum_UVSource.push_back(RENDERSTYLE_UVFROM_MODELDATA_UVSET1);
		m_pCB_UVSource[i]->AddString("UV Set 1");						m_IDtoRSEnum_UVSource.push_back(RENDERSTYLE_UVFROM_MODELDATA_UVSET2);
		m_pCB_UVSource[i]->AddString("UV Set 2");						m_IDtoRSEnum_UVSource.push_back(RENDERSTYLE_UVFROM_MODELDATA_UVSET3);
		m_pCB_UVSource[i]->AddString("UV Set 3");						m_IDtoRSEnum_UVSource.push_back(RENDERSTYLE_UVFROM_MODELDATA_UVSET4);
		m_pCB_UVSource[i]->AddString("CS Normal");						m_IDtoRSEnum_UVSource.push_back(RENDERSTYLE_UVFROM_CAMERASPACENORMAL);
		m_pCB_UVSource[i]->AddString("CS Position");					m_IDtoRSEnum_UVSource.push_back(RENDERSTYLE_UVFROM_CAMERASPACEPOSITION);
		m_pCB_UVSource[i]->AddString("CS RefctVec");					m_IDtoRSEnum_UVSource.push_back(RENDERSTYLE_UVFROM_CAMERASPACEREFLTVECT);
		m_pCB_UVSource[i]->AddString("WS Normal");						m_IDtoRSEnum_UVSource.push_back(RENDERSTYLE_UVFROM_WORLDSPACENORMAL);
		m_pCB_UVSource[i]->AddString("WS Position");					m_IDtoRSEnum_UVSource.push_back(RENDERSTYLE_UVFROM_WORLDSPACEPOSITION);
		m_pCB_UVSource[i]->AddString("WS RefctVec");					m_IDtoRSEnum_UVSource.push_back(RENDERSTYLE_UVFROM_WORLDSPACEREFLTVECT);
		m_pCB_UVSource[i]->SetCurSel(0);
	}

	for (i=0;i<4;++i) {
		m_pCB_UAddress[i]->AddString("Wrap");
		m_pCB_UAddress[i]->AddString("Clamp");
		m_pCB_UAddress[i]->AddString("Mirror");
		m_pCB_UAddress[i]->AddString("MirrorOnce");
		m_pCB_UAddress[i]->SetCurSel(0); }

	for (i=0;i<4;++i) {
		m_pCB_VAddress[i]->AddString("Wrap");
		m_pCB_VAddress[i]->AddString("Clamp");
		m_pCB_VAddress[i]->AddString("Mirror");
		m_pCB_VAddress[i]->AddString("MirrorOnce");
		m_pCB_VAddress[i]->SetCurSel(0); }

	for (i=0;i<4;++i) {
		m_pCB_TexFilter[i]->AddString("Point");
		m_pCB_TexFilter[i]->AddString("BiLinear");
		m_pCB_TexFilter[i]->AddString("TriLinear");
		m_pCB_TexFilter[i]->AddString("Anisotropic");
		m_pCB_TexFilter[i]->AddString("Point w/ PtMip");
		m_pCB_TexFilter[i]->SetCurSel(0); }
}

void CRenderPassDlg::SetDialogControls_From_RenderStyleData()
{
	// Render States...
	m_pBn_DynamicLight->SetCheck(m_pRenderPassData->DynamicLight);
	char szTmp[32]; sprintf(szTmp,"%d",m_pRenderPassData->AlphaRef); m_pEB_AlphaRef->SetWindowText(szTmp);
	sprintf(szTmp,"%x",m_pRenderPassData->TextureFactor); m_pEB_TFactor->SetWindowText(szTmp);
	m_pCB_BlendOp->SetCurSel(GetIDfromRSEnum_Blend(m_pRenderPassData->Blend));
	m_pCB_ZBuffer->SetCurSel(GetIDfromRSEnum_ZBufferMode(m_pRenderPassData->ZBuffer));
	m_pCB_CullMode->SetCurSel(GetIDfromRSEnum_CullMode(m_pRenderPassData->CullMode));
	m_pCB_AlphaTest->SetCurSel(GetIDfromRSEnum_TestMode(m_pRenderPassData->AlphaTestMode));
	m_pCB_ZBufferTest->SetCurSel(GetIDfromRSEnum_TestMode(m_pRenderPassData->ZBufferTestMode));
	m_pCB_FillMode->SetCurSel(GetIDfromRSEnum_FillMode(m_pRenderPassData->FillMode));

	// Texture Stage Ops...
	for (uint32 i=0;i<4;++i)
	{
		m_pCB_TextureParam[i]->SetCurSel(GetIDfromRSEnum_TextureParam(m_pRenderPassData->TextureParam[i]));
		m_pCB_ColorOp[i]->SetCurSel(GetIDfromRSEnum_ColorOp(m_pRenderPassData->ColorOp[i]));
		m_pCB_ColorArg1[i]->SetCurSel(GetIDfromRSEnum_ColorArg1(m_pRenderPassData->ColorArg1[i]));
		m_pCB_ColorArg2[i]->SetCurSel(GetIDfromRSEnum_ColorArg2(m_pRenderPassData->ColorArg2[i]));
		m_pCB_AlphaOp[i]->SetCurSel(GetIDfromRSEnum_AlphaOp(m_pRenderPassData->AlphaOp[i]));
		m_pCB_AlphaArg1[i]->SetCurSel(GetIDfromRSEnum_AlphaArg1(m_pRenderPassData->AlphaArg1[i]));
		m_pCB_AlphaArg2[i]->SetCurSel(GetIDfromRSEnum_AlphaArg2(m_pRenderPassData->AlphaArg2[i]));
		m_pCB_UVSource[i]->SetCurSel(GetIDfromRSEnum_UVSource(m_pRenderPassData->UVSource[i]));
		m_pCB_UAddress[i]->SetCurSel(GetIDfromRSEnum_UAddress(m_pRenderPassData->UAddress[i]));
		m_pCB_VAddress[i]->SetCurSel(GetIDfromRSEnum_VAddress(m_pRenderPassData->VAddress[i]));
		m_pCB_TexFilter[i]->SetCurSel(GetIDfromRSEnum_TexFilter(m_pRenderPassData->TexFilter[i]));
		m_pCB_UVTransform_Enable[i]->SetCheck(m_pRenderPassData->UVTransform_Enable[i]);
	}

	// Direct3D Data...

	// vertex shader
	m_pBn_VertexShader_Enable->SetCheck(m_pRenderPassData->VertexShader_Enable);
	sprintf(szTmp, "%d", m_pRenderPassData->VertexShader_ID);
	m_pEB_VertexShader_ID->SetWindowText(szTmp);

	// pixel shader
	m_pBn_PixelShader_Enable->SetCheck(m_pRenderPassData->PixelShader_Enable);
	sprintf(szTmp, "%d", m_pRenderPassData->PixelShader_ID);
	m_pEB_PixelShader_ID->SetWindowText(szTmp);

	m_pBn_BumpEnvMap_Enable->SetCheck(m_pRenderPassData->bUseBumpEnvMap);
	sprintf(szTmp,"%d", m_pRenderPassData->BumpEnvMapStage);
	m_pEB_BumpEnvMapStage->SetWindowText(szTmp);
	sprintf(szTmp,"%3.3f", m_pRenderPassData->fBumpEnvMap_Scale);
	m_pEB_BumpEnvMapScale->SetWindowText(szTmp);
	sprintf(szTmp,"%3.3f", m_pRenderPassData->fBumpEnvMap_Offset);
	m_pEB_BumpEnvMapOffset->SetWindowText(szTmp);
}

void CRenderPassDlg::HandleTexTransform(uint32 nStage)
{
	MtxCfDlg_Data PassedCfgDlg_Data;
	PassedCfgDlg_Data.ProjectUVEnable = &(m_pRenderPassData->ProjectTexCoord[nStage]);
	PassedCfgDlg_Data.TexCoordCount = &(m_pRenderPassData->TexCoordCount[nStage]);
	PassedCfgDlg_Data.pMatrix = &(m_pRenderPassData->UVTransform_Matrix[nStage][0]);

	bool	SavedProjEnable		= m_pRenderPassData->ProjectTexCoord[nStage];
	uint32	SavedTexCoordCount	= m_pRenderPassData->TexCoordCount[nStage];
	D3DXMATRIX SavedMatrix		= m_pRenderPassData->UVTransform_Matrix[nStage];

	CMatrixConfigDlg dlg; dlg.SetMatrixData(&PassedCfgDlg_Data);
	if (dlg.DoModal() != IDOK)
	{
		m_pRenderPassData->UVTransform_Matrix[nStage] = SavedMatrix;
		m_pRenderPassData->ProjectTexCoord[nStage] = SavedProjEnable;
		m_pRenderPassData->TexCoordCount[nStage] = SavedTexCoordCount;
	}
	g_AppFormView->RenderStyleDataChanged();
}

// Button presses for configuring the texture transforms...
void CRenderPassDlg::OnUVTransform_Matrix1()
{
	HandleTexTransform(0);
}
void CRenderPassDlg::OnUVTransform_Matrix2()
{
	HandleTexTransform(1);
}
void CRenderPassDlg::OnUVTransform_Matrix3()
{
	HandleTexTransform(2);
}
void CRenderPassDlg::OnUVTransform_Matrix4()
{
	HandleTexTransform(3);
}

// Button presses for configuring the d3d vertex shader passes (starts up a thread)...
/*
UINT StartDlgThread_ConfigVertexShader(LPVOID lpvParam) {
	CDirect3DData* pDirect3DData = (CDirect3DData*)lpvParam; pDirect3DData->bDialogOpen = true;
    CVertexShaderDlg dlg; dlg.SetRenderPassData(pDirect3DData);
	CDirect3DData OriginalDirect3DData = *pDirect3DData;	// Make a copy so we can reset it if they cancel...
	if (!dlg.Create(IDD_VERTEXSHADER)) return 0; dlg.ShowWindow(SW_SHOW);
	if (dlg.RunModalLoop(MLF_SHOWONIDLE) == IDCANCEL) { *pDirect3DData = OriginalDirect3DData; }
	dlg.DestroyWindow(); pDirect3DData->bDialogOpen = false; g_AppFormView->RenderStyleDataChanged();
	return 0; }
void CRenderPassDlg::OnD3DOptions_VertexShader_Config() {
	if (m_pRenderPassData->Direct3DData.bDialogOpen) return;			// Already started up this dialog - skip out...
	::AfxBeginThread((AFX_THREADPROC)StartDlgThread_ConfigVertexShader,&m_pRenderPassData->Direct3DData,THREAD_PRIORITY_NORMAL);
	g_AppFormView->RenderStyleDataChanged(); }
 */


void CRenderPassDlg::OnTextureParam1()
{
	m_pRenderPassData->TextureParam[0] = m_IDtoRSEnum_TextureParam[m_pCB_TextureParam[0]->GetCurSel()];
/*
	if (g_AppFormView->m_PlatformPS2)
	{
		switch (m_pRenderPassData->TextureParam[0])
		{
		case RENDERSTYLE_NOTEXTURE:
				m_pCB_ColorArg1[0]->SelectString(-1, "Diffuse");
				OnColorArg11();
				break;
		case RENDERSTYLE_USE_TEXTURE1:
				m_pCB_ColorArg1[0]->SelectString(-1, "Texture");
				OnColorArg11();
				break;
		case RENDERSTYLE_USE_TEXTURE2:
				m_pCB_ColorArg1[0]->SelectString(-1, "Texture");
				OnColorArg11();
				break;
		case RENDERSTYLE_USE_TEXTURE3:
				m_pCB_ColorArg1[0]->SelectString(-1, "Texture");
				OnColorArg11();
				break;
		case RENDERSTYLE_USE_TEXTURE4:
				m_pCB_ColorArg1[0]->SelectString(-1, "Texture");
				OnColorArg11();
				break;
		default:
			ASSERT(false);
		}
	}
*/

	g_AppFormView->RenderStyleDataChanged();

}


uint32 CRenderPassDlg::GetIDfromRSEnum_ColorArg1(uint32 RSEnum) const
{
	char str[32];
	switch (RSEnum)
	{
		case RENDERSTYLE_COLORARG_CURRENT:
			strcpy(str, "Current");
			return m_pCB_ColorArg1[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLORARG_DIFFUSE:
			strcpy(str, "Diffuse");
			return m_pCB_ColorArg1[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLORARG_TEXTURE:
			strcpy(str, "Texture");
			return m_pCB_ColorArg1[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLORARG_TFACTOR:
			strcpy(str, "TFactor");
			return m_pCB_ColorArg1[0]->FindStringExact(-1, str);
			break;
		default:
			ASSERT(false);
	}
	return -1;
}

void CRenderPassDlg::OnColorArg11()
{
	char selectionText[32];

	m_pCB_ColorArg1[0]->GetLBText(m_pCB_ColorArg1[0]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Current") == 0)  m_pRenderPassData->ColorArg1[0] = RENDERSTYLE_COLORARG_CURRENT;
	else if (strcmp(selectionText, "Diffuse") == 0)  m_pRenderPassData->ColorArg1[0] = RENDERSTYLE_COLORARG_DIFFUSE;
	else if (strcmp(selectionText, "Texture") == 0)  m_pRenderPassData->ColorArg1[0] = RENDERSTYLE_COLORARG_TEXTURE;
	else if (strcmp(selectionText, "TFactor") == 0)  m_pRenderPassData->ColorArg1[0] = RENDERSTYLE_COLORARG_TFACTOR;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}

void CRenderPassDlg::OnColorArg12()
{
	char selectionText[32];

	m_pCB_ColorArg1[1]->GetLBText(m_pCB_ColorArg1[1]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Current") == 0)  m_pRenderPassData->ColorArg1[1] = RENDERSTYLE_COLORARG_CURRENT;
	else if (strcmp(selectionText, "Diffuse") == 0)  m_pRenderPassData->ColorArg1[1] = RENDERSTYLE_COLORARG_DIFFUSE;
	else if (strcmp(selectionText, "Texture") == 0)  m_pRenderPassData->ColorArg1[1] = RENDERSTYLE_COLORARG_TEXTURE;
	else if (strcmp(selectionText, "TFactor") == 0)  m_pRenderPassData->ColorArg1[1] = RENDERSTYLE_COLORARG_TFACTOR;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}

void CRenderPassDlg::OnColorArg13()
{
	char selectionText[32];

	m_pCB_ColorArg1[2]->GetLBText(m_pCB_ColorArg1[2]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Current") == 0)  m_pRenderPassData->ColorArg1[2] = RENDERSTYLE_COLORARG_CURRENT;
	else if (strcmp(selectionText, "Diffuse") == 0)  m_pRenderPassData->ColorArg1[2] = RENDERSTYLE_COLORARG_DIFFUSE;
	else if (strcmp(selectionText, "Texture") == 0)  m_pRenderPassData->ColorArg1[2] = RENDERSTYLE_COLORARG_TEXTURE;
	else if (strcmp(selectionText, "TFactor") == 0)  m_pRenderPassData->ColorArg1[2] = RENDERSTYLE_COLORARG_TFACTOR;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}

void CRenderPassDlg::OnColorArg14()
{
	char selectionText[32];

	m_pCB_ColorArg1[3]->GetLBText(m_pCB_ColorArg1[3]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Current") == 0)  m_pRenderPassData->ColorArg1[3] = RENDERSTYLE_COLORARG_CURRENT;
	else if (strcmp(selectionText, "Diffuse") == 0)  m_pRenderPassData->ColorArg1[3] = RENDERSTYLE_COLORARG_DIFFUSE;
	else if (strcmp(selectionText, "Texture") == 0)  m_pRenderPassData->ColorArg1[3] = RENDERSTYLE_COLORARG_TEXTURE;
	else if (strcmp(selectionText, "TFactor") == 0)  m_pRenderPassData->ColorArg1[3] = RENDERSTYLE_COLORARG_TFACTOR;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


uint32 CRenderPassDlg::GetIDfromRSEnum_ColorOp(uint32 RSEnum) const
{
	char str[32];
	switch (RSEnum)
	{
		case RENDERSTYLE_COLOROP_DISABLE:
			strcpy(str, "Disable");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_SELECTARG1:
			strcpy(str, "Select Arg1");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_SELECTARG2:
			strcpy(str, "Select Arg2");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_MODULATE:
			strcpy(str, "Modulate");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_MODULATE2X:
			strcpy(str, "Modulate2x");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_MODULATEALPHA:
			strcpy(str, "Mod CurAlpha");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_MODULATETEXALPHA:
			strcpy(str, "Mod TexAlpha");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_MODULATETFACTOR:
			strcpy(str, "Mod TFactor");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_ADD:
			strcpy(str, "Add");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_DOTPRODUCT3:
			strcpy(str, "DotProduct3");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_BUMPENVMAP:
			strcpy(str, "BumpEnvMap");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_BUMPENVMAPLUM:
			strcpy(str, "BumpEnvMapLum");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		// OLD PS2 case RENDERSTYLE_COLOROP_DECAL:
		//	strcpy(str, "Decal");
		//	return m_pCB_ColorOp[0]->FindStringExact(-1, str);
		//	break;
		//case RENDERSTYLE_COLOROP_HIGHLIGHT:
		//	strcpy(str, "Highlight");
		//	return m_pCB_ColorOp[0]->FindStringExact(-1, str);
		//	break;
		//case RENDERSTYLE_COLOROP_HIGHLIGHT2:
		//	strcpy(str, "Highlight2");
		//	return m_pCB_ColorOp[0]->FindStringExact(-1, str);
		//	break;
		case RENDERSTYLE_COLOROP_ADDSIGNED:
			strcpy(str, "AddSigned");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_ADDSIGNED2X:
			strcpy(str, "AddSigned2x");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_SUBTRACT:
			strcpy(str, "Subtract");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_ADDMODALPHA:
			strcpy(str, "Add Mod Alpha");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_COLOROP_ADDMODINVALPHA:
			strcpy(str, "Add Mod Inv Alpha");
			return m_pCB_ColorOp[0]->FindStringExact(-1, str);
			break;
		default:
			ASSERT(false);
	}
	return -1;
}

ERenStyle_ColorOp CRenderPassDlg::StringToColorOp(const char* pszString)
{
	if (strcmp(pszString, "Disable") == 0)				return RENDERSTYLE_COLOROP_DISABLE;
	else if (strcmp(pszString, "Select Arg1") == 0)		return RENDERSTYLE_COLOROP_SELECTARG1;
	else if (strcmp(pszString, "Select Arg2") == 0)		return RENDERSTYLE_COLOROP_SELECTARG2;
	else if (strcmp(pszString, "Modulate") == 0)		return RENDERSTYLE_COLOROP_MODULATE;
	else if (strcmp(pszString, "Modulate2x") == 0)		return RENDERSTYLE_COLOROP_MODULATE2X;
	else if (strcmp(pszString, "Mod CurAlpha") == 0)	return RENDERSTYLE_COLOROP_MODULATEALPHA;
	else if (strcmp(pszString, "Mod TexAlpha") == 0)	return RENDERSTYLE_COLOROP_MODULATETEXALPHA;
	else if (strcmp(pszString, "Mod TFactor") == 0)		return RENDERSTYLE_COLOROP_MODULATETFACTOR;
	else if (strcmp(pszString, "Add") == 0)				return RENDERSTYLE_COLOROP_ADD;
	else if (strcmp(pszString, "DotProduct3") == 0)		return RENDERSTYLE_COLOROP_DOTPRODUCT3;
	else if (strcmp(pszString, "BumpEnvMap") == 0)		return RENDERSTYLE_COLOROP_BUMPENVMAP;
	else if (strcmp(pszString, "BumpEnvMapLum") == 0)	return RENDERSTYLE_COLOROP_BUMPENVMAPLUM;
//	OLD PS2 else if (strcmp(pszString, "Decal") == 0)			return RENDERSTYLE_COLOROP_DECAL;
//	else if (strcmp(pszString, "Highlight") == 0)		return RENDERSTYLE_COLOROP_HIGHLIGHT;
//	else if (strcmp(pszString, "Highlight2") == 0)		return RENDERSTYLE_COLOROP_HIGHLIGHT2;
	else if (strcmp(pszString, "AddSigned") == 0)		return RENDERSTYLE_COLOROP_ADDSIGNED;
	else if (strcmp(pszString, "AddSigned2x") == 0)		return RENDERSTYLE_COLOROP_ADDSIGNED2X;
	else if (strcmp(pszString, "Subtract") == 0)		return RENDERSTYLE_COLOROP_SUBTRACT;
	else if (strcmp(pszString, "Add Mod Alpha") == 0)	return RENDERSTYLE_COLOROP_ADDMODALPHA;
	else if (strcmp(pszString, "Add Mod Inv Alpha") == 0)	return RENDERSTYLE_COLOROP_ADDMODINVALPHA;
	else ASSERT(false);

	return RENDERSTYLE_COLOROP_DISABLE;
}

void CRenderPassDlg::UpdateColorOp(uint32 nColorOp)
{
	CString sSelectionText;
	m_pCB_ColorOp[nColorOp]->GetLBText(m_pCB_ColorOp[nColorOp]->GetCurSel(), sSelectionText);
	m_pRenderPassData->ColorOp[nColorOp] = StringToColorOp(sSelectionText);
	g_AppFormView->RenderStyleDataChanged();
}

void CRenderPassDlg::OnColorOp1()
{
	UpdateColorOp(0);
}

void CRenderPassDlg::OnColorOp2()
{
	UpdateColorOp(1);
}

void CRenderPassDlg::OnColorOp3()
{
	UpdateColorOp(2);
}

void CRenderPassDlg::OnColorOp4()
{
	UpdateColorOp(3);
}


uint32 CRenderPassDlg::GetIDfromRSEnum_Blend(uint32 RSEnum) const
{
	char str[32];
	switch (RSEnum)
	{
		case RENDERSTYLE_NOBLEND:
			strcpy(str, "No Blend");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_BLEND_ADD:
			strcpy(str, "Cs + Cd");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_BLEND_SATURATE:
			strcpy(str, "Cs * (1 - Cd) + Cd");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_BLEND_MOD_SRCALPHA:
			strcpy(str, "Cs * As + Cd * (1 - As)");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_BLEND_MOD_SRCCOLOR:
			strcpy(str, "Cs * Cs + Cd * (1 - Cs)");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_BLEND_MOD_DSTCOLOR:
			strcpy(str, "Cs * Cd + Cd * (1 - Cd)");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_BLEND_MUL_SRCCOL_DSTCOL:
			strcpy(str, "Cs * Cs + Cd * Cd");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_BLEND_MUL_SRCCOL_ONE:
			strcpy(str, "Cs * Cs + Cd");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_BLEND_MUL_SRCALPHA_ZERO:
			strcpy(str, "Cs * As");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_BLEND_MUL_SRCALPHA_ONE:
			strcpy(str, "Cs * As + Cd");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_BLEND_MUL_DSTCOL_ZERO:
			strcpy(str, "Cs * Cd");
			return m_pCB_BlendOp->FindStringExact(-1, str);
			break;
		default:
			ASSERT(false);
	}
	return -1;
}

void CRenderPassDlg::OnBlend()
{
	char selectionText[32];

	m_pCB_BlendOp->GetLBText(m_pCB_BlendOp->GetCurSel(), selectionText);

	if (strcmp(selectionText, "No Blend") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_NOBLEND;
	else if (strcmp(selectionText, "Cs + Cd") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_BLEND_ADD;
	else if (strcmp(selectionText, "Cs * (1 - Cd) + Cd") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_BLEND_SATURATE;
	else if (strcmp(selectionText, "Cs * As + Cd * (1 - As)") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_BLEND_MOD_SRCALPHA;
	else if (strcmp(selectionText, "Cs * Cs + Cd * (1 - Cs)") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_BLEND_MOD_SRCCOLOR;
	else if (strcmp(selectionText, "Cs * Cd + Cd * (1 - Cd)") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_BLEND_MOD_DSTCOLOR;
	else if (strcmp(selectionText, "Cs * Cs + Cd * Cd") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_BLEND_MUL_SRCCOL_DSTCOL;
	else if (strcmp(selectionText, "Cs * Cs + Cd") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_BLEND_MUL_SRCCOL_ONE;
	else if (strcmp(selectionText, "Cs * As") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_BLEND_MUL_SRCALPHA_ZERO;
	else if (strcmp(selectionText, "Cs * As + Cd") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_BLEND_MUL_SRCALPHA_ONE;
	else if (strcmp(selectionText, "Cs * Cd") == 0)  m_pRenderPassData->Blend = RENDERSTYLE_BLEND_MUL_DSTCOL_ZERO;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


uint32 CRenderPassDlg::GetIDfromRSEnum_TexFilter(uint32 RSEnum) const
{
	char str[32];
	switch (RSEnum)
	{
		case RENDERSTYLE_TEXFILTER_POINT:
			strcpy(str, "Point");
			return m_pCB_TexFilter[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_TEXFILTER_BILINEAR:
			strcpy(str, "BiLinear");
			return m_pCB_TexFilter[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_TEXFILTER_TRILINEAR:
			strcpy(str, "TriLinear");
			return m_pCB_TexFilter[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_TEXFILTER_ANISOTROPIC:
			strcpy(str, "Anisotropic");
			return m_pCB_TexFilter[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_TEXFILTER_POINT_PTMIP:
			strcpy(str, "Point w/ PtMip");
			return m_pCB_TexFilter[0]->FindStringExact(-1, str);
			break;
		default:
			ASSERT(false);
	}
	return -1;
}


void CRenderPassDlg::OnTexFilter1()
{
	char selectionText[32];

	m_pCB_TexFilter[0]->GetLBText(m_pCB_TexFilter[0]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Point") == 0)  m_pRenderPassData->TexFilter[0] = RENDERSTYLE_TEXFILTER_POINT;
	else if (strcmp(selectionText, "BiLinear") == 0)  m_pRenderPassData->TexFilter[0] = RENDERSTYLE_TEXFILTER_BILINEAR;
	else if (strcmp(selectionText, "TriLinear") == 0)  m_pRenderPassData->TexFilter[0] = RENDERSTYLE_TEXFILTER_TRILINEAR;
	else if (strcmp(selectionText, "Anisotropic") == 0)  m_pRenderPassData->TexFilter[0] = RENDERSTYLE_TEXFILTER_ANISOTROPIC;
	else if (strcmp(selectionText, "Point w/ PtMip") == 0)  m_pRenderPassData->TexFilter[0] = RENDERSTYLE_TEXFILTER_POINT_PTMIP;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


void CRenderPassDlg::OnTexFilter2()
{
	char selectionText[32];

	m_pCB_TexFilter[1]->GetLBText(m_pCB_TexFilter[1]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Point") == 0)  m_pRenderPassData->TexFilter[1] = RENDERSTYLE_TEXFILTER_POINT;
	else if (strcmp(selectionText, "BiLinear") == 0)  m_pRenderPassData->TexFilter[1] = RENDERSTYLE_TEXFILTER_BILINEAR;
	else if (strcmp(selectionText, "TriLinear") == 0)  m_pRenderPassData->TexFilter[1] = RENDERSTYLE_TEXFILTER_TRILINEAR;
	else if (strcmp(selectionText, "Anisotropic") == 0)  m_pRenderPassData->TexFilter[1] = RENDERSTYLE_TEXFILTER_ANISOTROPIC;
	else if (strcmp(selectionText, "Point w/ PtMip") == 0)  m_pRenderPassData->TexFilter[1] = RENDERSTYLE_TEXFILTER_POINT_PTMIP;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


void CRenderPassDlg::OnTexFilter3()
{
	char selectionText[32];

	m_pCB_TexFilter[2]->GetLBText(m_pCB_TexFilter[2]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Point") == 0)  m_pRenderPassData->TexFilter[2] = RENDERSTYLE_TEXFILTER_POINT;
	else if (strcmp(selectionText, "BiLinear") == 0)  m_pRenderPassData->TexFilter[2] = RENDERSTYLE_TEXFILTER_BILINEAR;
	else if (strcmp(selectionText, "TriLinear") == 0)  m_pRenderPassData->TexFilter[2] = RENDERSTYLE_TEXFILTER_TRILINEAR;
	else if (strcmp(selectionText, "Anisotropic") == 0)  m_pRenderPassData->TexFilter[2] = RENDERSTYLE_TEXFILTER_ANISOTROPIC;
	else if (strcmp(selectionText, "Point w/ PtMip") == 0)  m_pRenderPassData->TexFilter[2] = RENDERSTYLE_TEXFILTER_POINT_PTMIP;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


void CRenderPassDlg::OnTexFilter4()
{
	char selectionText[32];

	m_pCB_TexFilter[3]->GetLBText(m_pCB_TexFilter[3]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Point") == 0)  m_pRenderPassData->TexFilter[3] = RENDERSTYLE_TEXFILTER_POINT;
	else if (strcmp(selectionText, "BiLinear") == 0)  m_pRenderPassData->TexFilter[3] = RENDERSTYLE_TEXFILTER_BILINEAR;
	else if (strcmp(selectionText, "TriLinear") == 0)  m_pRenderPassData->TexFilter[3] = RENDERSTYLE_TEXFILTER_TRILINEAR;
	else if (strcmp(selectionText, "Anisotropic") == 0)  m_pRenderPassData->TexFilter[3] = RENDERSTYLE_TEXFILTER_ANISOTROPIC;
	else if (strcmp(selectionText, "Point w/ PtMip") == 0)  m_pRenderPassData->TexFilter[3] = RENDERSTYLE_TEXFILTER_POINT_PTMIP;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


uint32 CRenderPassDlg::GetIDfromRSEnum_UAddress(uint32 RSEnum) const
{
	char str[32];
	switch (RSEnum)
	{
		case RENDERSTYLE_UVADDR_WRAP:
			strcpy(str, "Wrap");
			return m_pCB_UAddress[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_UVADDR_CLAMP:
			strcpy(str, "Clamp");
			return m_pCB_UAddress[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_UVADDR_MIRROR:
			strcpy(str, "Mirror");
			return m_pCB_UAddress[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_UVADDR_MIRRORONCE:
			strcpy(str, "MirrorOnce");
			return m_pCB_UAddress[0]->FindStringExact(-1, str);
			break;
		default:
			ASSERT(false);
	}
	return -1;
}


void CRenderPassDlg::OnUAddress1()
{
	char selectionText[32];

	m_pCB_UAddress[0]->GetLBText(m_pCB_UAddress[0]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Wrap") == 0)  m_pRenderPassData->UAddress[0] = RENDERSTYLE_UVADDR_WRAP;
	else if (strcmp(selectionText, "Clamp") == 0)  m_pRenderPassData->UAddress[0] = RENDERSTYLE_UVADDR_CLAMP;
	else if (strcmp(selectionText, "Mirror") == 0)  m_pRenderPassData->UAddress[0] = RENDERSTYLE_UVADDR_MIRROR;
	else if (strcmp(selectionText, "MirrorOnce") == 0)  m_pRenderPassData->UAddress[0] = RENDERSTYLE_UVADDR_MIRRORONCE;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


void CRenderPassDlg::OnUAddress2()
{
	char selectionText[32];

	m_pCB_UAddress[1]->GetLBText(m_pCB_UAddress[1]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Wrap") == 0)  m_pRenderPassData->UAddress[1] = RENDERSTYLE_UVADDR_WRAP;
	else if (strcmp(selectionText, "Clamp") == 0)  m_pRenderPassData->UAddress[1] = RENDERSTYLE_UVADDR_CLAMP;
	else if (strcmp(selectionText, "Mirror") == 0)  m_pRenderPassData->UAddress[1] = RENDERSTYLE_UVADDR_MIRROR;
	else if (strcmp(selectionText, "MirrorOnce") == 0)  m_pRenderPassData->UAddress[1] = RENDERSTYLE_UVADDR_MIRRORONCE;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


void CRenderPassDlg::OnUAddress3()
{
	char selectionText[32];

	m_pCB_UAddress[2]->GetLBText(m_pCB_UAddress[2]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Wrap") == 0)  m_pRenderPassData->UAddress[2] = RENDERSTYLE_UVADDR_WRAP;
	else if (strcmp(selectionText, "Clamp") == 0)  m_pRenderPassData->UAddress[2] = RENDERSTYLE_UVADDR_CLAMP;
	else if (strcmp(selectionText, "Mirror") == 0)  m_pRenderPassData->UAddress[2] = RENDERSTYLE_UVADDR_MIRROR;
	else if (strcmp(selectionText, "MirrorOnce") == 0)  m_pRenderPassData->UAddress[2] = RENDERSTYLE_UVADDR_MIRRORONCE;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}



void CRenderPassDlg::OnUAddress4()
{
	char selectionText[32];

	m_pCB_UAddress[3]->GetLBText(m_pCB_UAddress[3]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Wrap") == 0)  m_pRenderPassData->UAddress[3] = RENDERSTYLE_UVADDR_WRAP;
	else if (strcmp(selectionText, "Clamp") == 0)  m_pRenderPassData->UAddress[3] = RENDERSTYLE_UVADDR_CLAMP;
	else if (strcmp(selectionText, "Mirror") == 0)  m_pRenderPassData->UAddress[3] = RENDERSTYLE_UVADDR_MIRROR;
	else if (strcmp(selectionText, "MirrorOnce") == 0)  m_pRenderPassData->UAddress[3] = RENDERSTYLE_UVADDR_MIRRORONCE;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


uint32 CRenderPassDlg::GetIDfromRSEnum_VAddress(uint32 RSEnum) const
{
	char str[32];
	switch (RSEnum)
	{
		case RENDERSTYLE_UVADDR_WRAP:
			strcpy(str, "Wrap");
			return m_pCB_VAddress[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_UVADDR_CLAMP:
			strcpy(str, "Clamp");
			return m_pCB_VAddress[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_UVADDR_MIRROR:
			strcpy(str, "Mirror");
			return m_pCB_VAddress[0]->FindStringExact(-1, str);
			break;
		case RENDERSTYLE_UVADDR_MIRRORONCE:
			strcpy(str, "MirrorOnce");
			return m_pCB_VAddress[0]->FindStringExact(-1, str);
			break;
		default:
			ASSERT(false);
	}
	return -1;
}


void CRenderPassDlg::OnVAddress1()
{
	char selectionText[32];

	m_pCB_VAddress[0]->GetLBText(m_pCB_VAddress[0]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Wrap") == 0)  m_pRenderPassData->VAddress[0] = RENDERSTYLE_UVADDR_WRAP;
	else if (strcmp(selectionText, "Clamp") == 0)  m_pRenderPassData->VAddress[0] = RENDERSTYLE_UVADDR_CLAMP;
	else if (strcmp(selectionText, "Mirror") == 0)  m_pRenderPassData->VAddress[0] = RENDERSTYLE_UVADDR_MIRROR;
	else if (strcmp(selectionText, "MirrorOnce") == 0)  m_pRenderPassData->VAddress[0] = RENDERSTYLE_UVADDR_MIRRORONCE;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


void CRenderPassDlg::OnVAddress2()
{
	char selectionText[32];

	m_pCB_VAddress[1]->GetLBText(m_pCB_VAddress[1]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Wrap") == 0)  m_pRenderPassData->VAddress[1] = RENDERSTYLE_UVADDR_WRAP;
	else if (strcmp(selectionText, "Clamp") == 0)  m_pRenderPassData->VAddress[1] = RENDERSTYLE_UVADDR_CLAMP;
	else if (strcmp(selectionText, "Mirror") == 0)  m_pRenderPassData->VAddress[1] = RENDERSTYLE_UVADDR_MIRROR;
	else if (strcmp(selectionText, "MirrorOnce") == 0)  m_pRenderPassData->VAddress[1] = RENDERSTYLE_UVADDR_MIRRORONCE;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}


void CRenderPassDlg::OnVAddress3()
{
	char selectionText[32];

	m_pCB_VAddress[2]->GetLBText(m_pCB_VAddress[2]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Wrap") == 0)  m_pRenderPassData->VAddress[2] = RENDERSTYLE_UVADDR_WRAP;
	else if (strcmp(selectionText, "Clamp") == 0)  m_pRenderPassData->VAddress[2] = RENDERSTYLE_UVADDR_CLAMP;
	else if (strcmp(selectionText, "Mirror") == 0)  m_pRenderPassData->VAddress[2] = RENDERSTYLE_UVADDR_MIRROR;
	else if (strcmp(selectionText, "MirrorOnce") == 0)  m_pRenderPassData->VAddress[2] = RENDERSTYLE_UVADDR_MIRRORONCE;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}



void CRenderPassDlg::OnVAddress4()
{
	char selectionText[32];

	m_pCB_VAddress[3]->GetLBText(m_pCB_VAddress[3]->GetCurSel(), selectionText);

	if (strcmp(selectionText, "Wrap") == 0)  m_pRenderPassData->VAddress[3] = RENDERSTYLE_UVADDR_WRAP;
	else if (strcmp(selectionText, "Clamp") == 0)  m_pRenderPassData->VAddress[3] = RENDERSTYLE_UVADDR_CLAMP;
	else if (strcmp(selectionText, "Mirror") == 0)  m_pRenderPassData->VAddress[3] = RENDERSTYLE_UVADDR_MIRROR;
	else if (strcmp(selectionText, "MirrorOnce") == 0)  m_pRenderPassData->VAddress[3] = RENDERSTYLE_UVADDR_MIRRORONCE;
	else ASSERT(false);

	g_AppFormView->RenderStyleDataChanged();
}

