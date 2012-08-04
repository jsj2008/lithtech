
#include "stdafx.h"
#include "VertexShaderDlg.h"

BEGIN_MESSAGE_MAP(CVertexShaderDlg, CDialog)
    //{{AFX_MSG_MAP(CVertexShaderDlg)
	ON_EN_CHANGE(IDC_VERTSHADER_FILENAME, OnVertexShaderFilename)
	ON_BN_CLICKED(IDC_VERTEXSHADER_BROWSE, OnVertexShaderFileBrowse)
	ON_BN_CLICKED(IDC_SAVEASSEMBLED, OnVertexShaderSaveAssembled)
	ON_BN_CLICKED(IDC_RELOAD, OnReload)
	ON_BN_CLICKED(IDC_EXPANDMACROS_FORSKINNING, OnExpandForSkinning)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_GENVECTOR1, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_GENVECTOR2, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_GENVECTOR3, OnAConstRegChanged)
 	ON_EN_CHANGE(IDC_VERTSHADER_PARAM1_GENVECTOR1, OnGenVectorChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_PARAM2_GENVECTOR1, OnGenVectorChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_PARAM3_GENVECTOR1, OnGenVectorChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_PARAM4_GENVECTOR1, OnGenVectorChanged)
 	ON_EN_CHANGE(IDC_VERTSHADER_PARAM1_GENVECTOR2, OnGenVectorChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_PARAM2_GENVECTOR2, OnGenVectorChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_PARAM3_GENVECTOR2, OnGenVectorChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_PARAM4_GENVECTOR2, OnGenVectorChanged)
 	ON_EN_CHANGE(IDC_VERTSHADER_PARAM1_GENVECTOR3, OnGenVectorChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_PARAM2_GENVECTOR3, OnGenVectorChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_PARAM3_GENVECTOR3, OnGenVectorChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_PARAM4_GENVECTOR3, OnGenVectorChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_WORLDVIEWTRANSFORM, OnAConstRegChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_NUM_WORLDVIEWTRANSFORM, OnWorldViewTransCountChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_VIEWTRANSFORM, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_PROJTRANSFORM, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_WORLDVIEWPROJTRANSFORM, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_VIEWPROJTRANSFORM, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_CAMPOS_MSPC, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_LIGHTVECTORS, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_LIGHTPOSITION_MSPC, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_LIGHTPOSITION_CSPC, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_LIGHTCOLOR, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_LIGHTATT, OnAConstRegChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_NUM_LIGHT, OnLightVectorCountChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_MATERIAL_AMBDIFDEM, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_MATERIAL_SPECULAR, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_AMBIENTLIGHT, OnAConstRegChanged)
	ON_BN_CLICKED(IDC_VERTSHADER_ENABLE_PREVWORLDVIEWTRANS, OnAConstRegChanged)
	ON_EN_CHANGE(IDC_VERTSHADER_NUM_PREVWORLDVIEWTRANS, OnPrevWorldViewCountChanged)
	ON_BN_CLICKED(IDC_VERTEXSHADER_DECLARE_POSITION1, OnADeclareChanged)
	ON_BN_CLICKED(IDC_VERTEXSHADER_DECLARE_NORMAL3, OnADeclareChanged)
	ON_BN_CLICKED(IDC_VERTEXSHADER_DECLARE_UV1, OnADeclareChanged)
	ON_EN_CHANGE(IDC_VERTEXSHADER_DECLARE_UV_COUNT1, OnADeclareChanged)
	ON_BN_CLICKED(IDC_VERTEXSHADER_DECLARE_BASISVECTORS1, OnADeclareChanged)
	ON_BN_CLICKED(IDC_VERTEXSHADER_DECLARE_POSITION2, OnADeclareChanged)
	ON_BN_CLICKED(IDC_VERTEXSHADER_DECLARE_NORMAL4, OnADeclareChanged)
	ON_BN_CLICKED(IDC_VERTEXSHADER_DECLARE_UV2, OnADeclareChanged)
	ON_EN_CHANGE(IDC_VERTEXSHADER_DECLARE_UV_COUNT2, OnADeclareChanged)
	ON_BN_CLICKED(IDC_VERTEXSHADER_DECLARE_BASISVECTORS2, OnADeclareChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CVertexShaderDlg::OnInitDialog()
{
	InitializeDialogControls();							// Grab the Dialog's controls...
	SetDialogControls_From_RenderStyleData();			// Setup the controls with the passed in data...
	CalcConstRegs_AndSet_From_Dialog();					// Just to make sure we have valid data...let this function set the const regs...

	return true;
}

// Grab the Dialog's controls...
void CVertexShaderDlg::InitializeDialogControls()
{
	m_pEB_Filename							= (CEdit*)GetDlgItem(IDC_VERTSHADER_FILENAME);							assert(m_pEB_Filename);
	m_pBn_Filename_Browse					= (CButton*)GetDlgItem(IDC_VERTEXSHADER_BROWSE);						assert(m_pBn_Filename_Browse);
	m_pBn_AssembleVertShader				= (CButton*)GetDlgItem(IDC_SAVEASSEMBLED);								assert(m_pBn_AssembleVertShader);
	m_pBn_ExpandSkinning_Enable				= (CButton*)GetDlgItem(IDC_EXPANDMACROS_FORSKINNING);					assert(m_pBn_ExpandSkinning_Enable);
	m_pBn_GenericVector_Enable[0]			= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_GENVECTOR1);				assert(m_pBn_GenericVector_Enable[0]);
	m_pBn_GenericVector_Enable[1]			= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_GENVECTOR2);				assert(m_pBn_GenericVector_Enable[1]);
	m_pBn_GenericVector_Enable[2]			= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_GENVECTOR3);				assert(m_pBn_GenericVector_Enable[2]);
	m_pST_GenericVector_ConstReg[0]			= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_GENVECTOR1);				assert(m_pST_GenericVector_ConstReg[0]);
	m_pST_GenericVector_ConstReg[1]			= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_GENVECTOR2);				assert(m_pST_GenericVector_ConstReg[1]);
	m_pST_GenericVector_ConstReg[2]			= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_GENVECTOR3);				assert(m_pST_GenericVector_ConstReg[2]);
	m_pEB_GenericVector1_Params[0]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM1_GENVECTOR1);					assert(m_pEB_GenericVector1_Params[0]);
	m_pEB_GenericVector1_Params[1]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM2_GENVECTOR1);					assert(m_pEB_GenericVector1_Params[1]);
	m_pEB_GenericVector1_Params[2]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM3_GENVECTOR1);					assert(m_pEB_GenericVector1_Params[2]);
	m_pEB_GenericVector1_Params[3]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM4_GENVECTOR1);					assert(m_pEB_GenericVector1_Params[3]);
	m_pEB_GenericVector2_Params[0]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM1_GENVECTOR2);					assert(m_pEB_GenericVector2_Params[0]);
	m_pEB_GenericVector2_Params[1]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM2_GENVECTOR2);					assert(m_pEB_GenericVector2_Params[1]);
	m_pEB_GenericVector2_Params[2]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM3_GENVECTOR2);					assert(m_pEB_GenericVector2_Params[2]);
	m_pEB_GenericVector2_Params[3]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM4_GENVECTOR2);					assert(m_pEB_GenericVector2_Params[3]);
	m_pEB_GenericVector3_Params[0]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM1_GENVECTOR3);					assert(m_pEB_GenericVector3_Params[0]);
	m_pEB_GenericVector3_Params[1]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM2_GENVECTOR3);					assert(m_pEB_GenericVector3_Params[1]);
	m_pEB_GenericVector3_Params[2]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM3_GENVECTOR3);					assert(m_pEB_GenericVector3_Params[2]);
	m_pEB_GenericVector3_Params[3]			= (CEdit*)GetDlgItem(IDC_VERTSHADER_PARAM4_GENVECTOR3);					assert(m_pEB_GenericVector3_Params[3]);
	m_pBn_WorldViewTrans_Enable				= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_WORLDVIEWTRANSFORM);		assert(m_pBn_WorldViewTrans_Enable);
	m_pST_WorldViewTrans_ConstReg			= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_WORLDVIEWTRANSFORM);		assert(m_pST_WorldViewTrans_ConstReg);
	m_pEB_WorldViewTrans_Param				= (CEdit*)GetDlgItem(IDC_VERTSHADER_NUM_WORLDVIEWTRANSFORM);			assert(m_pEB_WorldViewTrans_Param);
	m_pBn_ProjTrans_Enable					= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_PROJTRANSFORM);			assert(m_pBn_ProjTrans_Enable);
	m_pST_ProjTrans_ConstReg				= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_PROJTRANSFORM);			assert(m_pST_ProjTrans_ConstReg);
	m_pBn_WorldViewProjTrans_Enable			= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_WORLDVIEWPROJTRANSFORM);	assert(m_pBn_WorldViewProjTrans_Enable);
	m_pST_WorldViewProjTrans_ConstReg		= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_WORLDVIEWPROJTRANSFORM);	assert(m_pST_WorldViewProjTrans_ConstReg);
	m_pBn_ViewProjTrans_Enable				= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_VIEWPROJTRANSFORM);		assert(m_pBn_ViewProjTrans_Enable);
	m_pST_ViewProjTrans_ConstReg			= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_VIEWPROJTRANSFORM);		assert(m_pST_ViewProjTrans_ConstReg);
	m_pBn_CamPos_MSpc_Enable				= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_CAMPOS_MSPC);				assert(m_pBn_CamPos_MSpc_Enable);
	m_pST_CamPos_MSpc_ConstReg				= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_CAMPOS_MSPC);			assert(m_pST_CamPos_MSpc_ConstReg);
	m_pEB_LightVectors_Param				= (CEdit*)GetDlgItem(IDC_VERTSHADER_NUM_LIGHT);							assert(m_pEB_LightVectors_Param);
	m_pBn_LightPosition_MSpc_Enable			= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_LIGHTPOSITION_MSPC);		assert(m_pBn_LightPosition_MSpc_Enable);
	m_pST_LightPosition_MSpc_ConstReg		= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_LIGHTPOSITION_MSPC);		assert(m_pST_LightPosition_MSpc_ConstReg);
	m_pBn_LightPosition_CSpc_Enable			= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_LIGHTPOSITION_CSPC);		assert(m_pBn_LightPosition_MSpc_Enable);
	m_pST_LightPosition_CSpc_ConstReg		= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_LIGHTPOSITION_CSPC);		assert(m_pST_LightPosition_MSpc_ConstReg);
	m_pBn_LightColor_Enable					= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_LIGHTCOLOR);				assert(m_pBn_LightColor_Enable);
	m_pST_LightColor_ConstReg				= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_LIGHTCOLOR);				assert(m_pST_LightColor_ConstReg);
	m_pBn_LightAtt_Enable					= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_LIGHTATT);					assert(m_pBn_LightAtt_Enable);
	m_pST_LightAtt_ConstReg					= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_LIGHTATT);				assert(m_pST_LightAtt_ConstReg);
	m_pBn_LightMaterial_AMBDIFEM_Enable		= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_MATERIAL_AMBDIFDEM);		assert(m_pBn_LightMaterial_AMBDIFEM_Enable);
	m_pST_LightMaterial_AMBDIFEM_ConstReg	= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_MATERIAL_AMBDIFDEM);		assert(m_pST_LightMaterial_AMBDIFEM_ConstReg);
	m_pBn_LightMaterial_Specular_Enable		= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_MATERIAL_SPECULAR);		assert(m_pBn_LightMaterial_Specular_Enable);
	m_pST_LightMaterial_Specular_ConstReg	= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_MATERIAL_SPECULAR);		assert(m_pST_LightMaterial_Specular_ConstReg);
	m_pBn_AmbientLight_Enable				= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_AMBIENTLIGHT);				assert(m_pBn_AmbientLight_Enable);
	m_pST_AmbientLight_ConstReg				= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_AMBIENTLIGHT);			assert(m_pST_AmbientLight_ConstReg);
	m_pBn_PrevWorldViewTrans_Enable			= (CButton*)GetDlgItem(IDC_VERTSHADER_ENABLE_PREVWORLDVIEWTRANS);		assert(m_pBn_PrevWorldViewTrans_Enable);
	m_pST_PrevWorldViewTrans_ConstReg		= (CStatic*)GetDlgItem(IDC_VERTSHADER_CONSTREG_PREVWORLDVIEWTRANS);		assert(m_pST_PrevWorldViewTrans_ConstReg);
	m_pEB_PrevWorldViewTrans_Param			= (CEdit*)GetDlgItem(IDC_VERTSHADER_NUM_PREVWORLDVIEWTRANS);			assert(m_pEB_PrevWorldViewTrans_Param);
	m_pBn_Declaration_Stream_Position[0]	= (CButton*)GetDlgItem(IDC_VERTEXSHADER_DECLARE_POSITION1);				assert(m_pBn_Declaration_Stream_Position[0]);
	m_pBn_Declaration_Stream_Normal[0]		= (CButton*)GetDlgItem(IDC_VERTEXSHADER_DECLARE_NORMAL3);				assert(m_pBn_Declaration_Stream_Normal[0]);
	m_pBn_Declaration_Stream_UVSets[0]		= (CButton*)GetDlgItem(IDC_VERTEXSHADER_DECLARE_UV1);					assert(m_pBn_Declaration_Stream_UVSets[0]);
	m_pEB_Declaration_Stream_UVCount[0]		= (CEdit*)GetDlgItem(IDC_VERTEXSHADER_DECLARE_UV_COUNT1);				assert(m_pEB_Declaration_Stream_UVCount[0]);
	m_pBn_Declaration_Stream_Basis[0]		= (CButton*)GetDlgItem(IDC_VERTEXSHADER_DECLARE_BASISVECTORS1);			assert(m_pBn_Declaration_Stream_Basis[0]);
	m_pBn_Declaration_Stream_Position[1]	= (CButton*)GetDlgItem(IDC_VERTEXSHADER_DECLARE_POSITION2);				assert(m_pBn_Declaration_Stream_Position[1]);
	m_pBn_Declaration_Stream_Normal[1]		= (CButton*)GetDlgItem(IDC_VERTEXSHADER_DECLARE_NORMAL4);				assert(m_pBn_Declaration_Stream_Normal[1]);
	m_pBn_Declaration_Stream_UVSets[1]		= (CButton*)GetDlgItem(IDC_VERTEXSHADER_DECLARE_UV2);					assert(m_pBn_Declaration_Stream_UVSets[1]);
	m_pEB_Declaration_Stream_UVCount[1]		= (CEdit*)GetDlgItem(IDC_VERTEXSHADER_DECLARE_UV_COUNT2);				assert(m_pEB_Declaration_Stream_UVCount[1]);
	m_pBn_Declaration_Stream_Basis[1]		= (CButton*)GetDlgItem(IDC_VERTEXSHADER_DECLARE_BASISVECTORS2);			assert(m_pBn_Declaration_Stream_Basis[1]);
	CSpinButtonCtrl* pSpin					= (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN1);								if (pSpin) pSpin->SetRange32(0,8);
	pSpin									= (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN7);								if (pSpin) pSpin->SetRange32(0,8);
}

void CVertexShaderDlg::CalcConstRegs_AndSet_From_Dialog()
{
	uint32 iConstReg = 0; char szTmp[16];
	if (m_pBn_GenericVector_Enable[0]->GetCheck()) { m_pST_GenericVector_ConstReg[0]->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->ConstVector_ConstReg1 = iConstReg; iConstReg += 1; }
	else { m_pST_GenericVector_ConstReg[0]->SetWindowText(""); m_pDirect3DData->ConstVector_ConstReg1 = -1; }
	if (m_pBn_GenericVector_Enable[1]->GetCheck()) { m_pST_GenericVector_ConstReg[1]->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->ConstVector_ConstReg2 = iConstReg; iConstReg += 1; }
	else { m_pST_GenericVector_ConstReg[1]->SetWindowText(""); m_pDirect3DData->ConstVector_ConstReg2 = -1; }
	if (m_pBn_GenericVector_Enable[2]->GetCheck()) { m_pST_GenericVector_ConstReg[2]->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->ConstVector_ConstReg3 = iConstReg; iConstReg += 1; }
	else { m_pST_GenericVector_ConstReg[2]->SetWindowText(""); m_pDirect3DData->ConstVector_ConstReg3 = -1; }

	if (m_pBn_WorldViewTrans_Enable->GetCheck()) { m_pST_WorldViewTrans_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->WorldViewTransform_ConstReg = iConstReg; iConstReg += m_pDirect3DData->WorldViewTransform_Count * 4; }
	else { m_pST_WorldViewTrans_ConstReg->SetWindowText(""); m_pDirect3DData->WorldViewTransform_ConstReg = -1; }
	if (m_pBn_ProjTrans_Enable->GetCheck()) { m_pST_ProjTrans_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->ProjTransform_ConstReg = iConstReg; iConstReg += 4; }
	else { m_pST_ProjTrans_ConstReg->SetWindowText(""); m_pDirect3DData->ProjTransform_ConstReg = -1; }
	if (m_pBn_WorldViewProjTrans_Enable->GetCheck()) { m_pST_WorldViewProjTrans_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->WorldViewProjTransform_ConstReg = iConstReg; iConstReg += 4; }
	else { m_pST_WorldViewProjTrans_ConstReg->SetWindowText(""); m_pDirect3DData->WorldViewProjTransform_ConstReg = -1; }
	if (m_pBn_ViewProjTrans_Enable->GetCheck()) { m_pST_ViewProjTrans_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->ViewProjTransform_ConstReg = iConstReg; iConstReg += 4; }
	else { m_pST_ViewProjTrans_ConstReg->SetWindowText(""); m_pDirect3DData->ViewProjTransform_ConstReg = -1; }

	if (m_pBn_CamPos_MSpc_Enable->GetCheck()) { m_pST_CamPos_MSpc_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->CamPos_MSpc_ConstReg = iConstReg; iConstReg += 1; }
	else { m_pST_CamPos_MSpc_ConstReg->SetWindowText(""); m_pDirect3DData->CamPos_MSpc_ConstReg = -1; }

	if (m_pBn_LightPosition_MSpc_Enable->GetCheck()) { m_pST_LightPosition_MSpc_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->LightPosition_MSpc_ConstReg = iConstReg; iConstReg += m_pDirect3DData->Light_Count; }
	else { m_pST_LightPosition_MSpc_ConstReg->SetWindowText(""); m_pDirect3DData->LightPosition_MSpc_ConstReg = -1; }
	if (m_pBn_LightPosition_CSpc_Enable->GetCheck()) { m_pST_LightPosition_CSpc_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->LightPosition_CSpc_ConstReg = iConstReg; iConstReg += m_pDirect3DData->Light_Count; }
	else { m_pST_LightPosition_CSpc_ConstReg->SetWindowText(""); m_pDirect3DData->LightPosition_CSpc_ConstReg = -1; }
	if (m_pBn_LightColor_Enable->GetCheck()) { m_pST_LightColor_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->LightColor_ConstReg = iConstReg; iConstReg += m_pDirect3DData->Light_Count; }
	else { m_pST_LightColor_ConstReg->SetWindowText(""); m_pDirect3DData->LightColor_ConstReg = -1; }
	if (m_pBn_LightAtt_Enable->GetCheck()) { m_pST_LightAtt_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->LightAtt_ConstReg = iConstReg; iConstReg += m_pDirect3DData->Light_Count; }
	else { m_pST_LightAtt_ConstReg->SetWindowText(""); m_pDirect3DData->LightAtt_ConstReg = -1; }
	if (m_pBn_LightMaterial_AMBDIFEM_Enable->GetCheck()) { m_pST_LightMaterial_AMBDIFEM_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->Material_AmbDifEm_ConstReg = iConstReg; iConstReg += 3; }
	else { m_pST_LightMaterial_AMBDIFEM_ConstReg->SetWindowText(""); m_pDirect3DData->Material_AmbDifEm_ConstReg = -1; }
	if (m_pBn_LightMaterial_Specular_Enable->GetCheck()) { m_pST_LightMaterial_Specular_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->Material_Specular_ConstReg = iConstReg; iConstReg += 1; }
	else { m_pST_LightMaterial_Specular_ConstReg->SetWindowText(""); m_pDirect3DData->Material_Specular_ConstReg = -1; }
	if (m_pBn_AmbientLight_Enable->GetCheck()) { m_pST_AmbientLight_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->AmbientLight_ConstReg = iConstReg; iConstReg += 1; }
	else { m_pST_AmbientLight_ConstReg->SetWindowText(""); m_pDirect3DData->AmbientLight_ConstReg = -1; }
	if (m_pBn_PrevWorldViewTrans_Enable->GetCheck()) { m_pST_PrevWorldViewTrans_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); m_pDirect3DData->PrevWorldViewTrans_ConstReg = iConstReg; iConstReg += m_pDirect3DData->PrevWorldViewTrans_Count * 4 ; }
	else { m_pST_PrevWorldViewTrans_ConstReg->SetWindowText(""); m_pDirect3DData->PrevWorldViewTrans_ConstReg = -1; }

	m_pDirect3DData->Last_ConstReg = iConstReg;

	sprintf(szTmp,"%d",m_pDirect3DData->Light_Count);
	if (GetDlgItem(IDC_VERTSHADER_NUM_LIGHTPOSITION))	{ ((CStatic*)GetDlgItem(IDC_VERTSHADER_NUM_LIGHTPOSITION))->SetWindowText(szTmp); } 
	if (GetDlgItem(IDC_VERTSHADER_NUM_LIGHTCOLOR))		{ ((CStatic*)GetDlgItem(IDC_VERTSHADER_NUM_LIGHTCOLOR))->SetWindowText(szTmp); }
	if (GetDlgItem(IDC_VERTSHADER_NUM_LIGHTATT))		{ ((CStatic*)GetDlgItem(IDC_VERTSHADER_NUM_LIGHTATT))->SetWindowText(szTmp); }
}

void CVertexShaderDlg::SetDeclaration_From_Dialog()
{
	for (uint32 z = 0; z < 2; ++z) {
		m_pDirect3DData->bDeclaration_Stream_Position[z]		= m_pBn_Declaration_Stream_Position[z]->GetCheck() ? true : false;
		m_pDirect3DData->bDeclaration_Stream_Normal[z]			= m_pBn_Declaration_Stream_Normal[z]->GetCheck() ? true : false;
		m_pDirect3DData->bDeclaration_Stream_UVSets[z]			= m_pBn_Declaration_Stream_UVSets[z]->GetCheck() ? true : false;
		CString WindowText; m_pEB_Declaration_Stream_UVCount[z]->GetWindowText(WindowText);
		m_pDirect3DData->Declaration_Stream_UVCount[z]			= atoi(LPCSTR(WindowText));
		m_pDirect3DData->bDeclaration_Stream_BasisVectors[z]	= m_pBn_Declaration_Stream_Basis[z]->GetCheck() ? true : false; }
}

// Setup the controls with the passed in data...
void CVertexShaderDlg::SetDialogControls_From_RenderStyleData()
{
	if (!m_pDirect3DData->VertexShaderFilename.empty()) { m_pEB_Filename->SetWindowText(m_pDirect3DData->VertexShaderFilename.c_str()); }
	else { m_pEB_Filename->SetWindowText(""); }
	m_pBn_ExpandSkinning_Enable->SetCheck(m_pDirect3DData->bExpandForSkinning);

	uint32 iConstReg = 0; char szTmp[16];
	if (m_pDirect3DData->ConstVector_ConstReg1 > -1) { m_pBn_GenericVector_Enable[0]->SetCheck(true); m_pST_GenericVector_ConstReg[0]->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += 1; }
	else { m_pBn_GenericVector_Enable[0]->SetCheck(false); m_pST_GenericVector_ConstReg[0]->SetWindowText(""); }
	if (m_pDirect3DData->ConstVector_ConstReg2 > -1) { m_pBn_GenericVector_Enable[1]->SetCheck(true); m_pST_GenericVector_ConstReg[1]->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += 1; }
	else { m_pBn_GenericVector_Enable[1]->SetCheck(false); m_pST_GenericVector_ConstReg[1]->SetWindowText(""); }
	if (m_pDirect3DData->ConstVector_ConstReg3 > -1) { m_pBn_GenericVector_Enable[2]->SetCheck(true); m_pST_GenericVector_ConstReg[2]->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += 1; }
	else { m_pBn_GenericVector_Enable[2]->SetCheck(false); m_pST_GenericVector_ConstReg[2]->SetWindowText(""); }
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param1.x); m_pEB_GenericVector1_Params[0]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param1.y); m_pEB_GenericVector1_Params[1]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param1.z); m_pEB_GenericVector1_Params[2]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param1.w); m_pEB_GenericVector1_Params[3]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param2.x); m_pEB_GenericVector2_Params[0]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param2.y); m_pEB_GenericVector2_Params[1]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param2.z); m_pEB_GenericVector2_Params[2]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param2.w); m_pEB_GenericVector2_Params[3]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param3.x); m_pEB_GenericVector3_Params[0]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param3.y); m_pEB_GenericVector3_Params[1]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param3.z); m_pEB_GenericVector3_Params[2]->SetWindowText(szTmp);
	sprintf(szTmp,"%5.3f",m_pDirect3DData->ConstVector_Param3.w); m_pEB_GenericVector3_Params[3]->SetWindowText(szTmp);

	if (m_pDirect3DData->WorldViewTransform_ConstReg > -1) { m_pBn_WorldViewTrans_Enable->SetCheck(true); m_pST_WorldViewTrans_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += m_pDirect3DData->WorldViewTransform_Count * 4; }
	else { m_pBn_WorldViewTrans_Enable->SetCheck(false); m_pST_WorldViewTrans_ConstReg->SetWindowText(""); }
	if (m_pDirect3DData->ProjTransform_ConstReg > -1) { m_pBn_ProjTrans_Enable->SetCheck(true); m_pST_ProjTrans_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += 4; }
	else { m_pBn_ProjTrans_Enable->SetCheck(false); m_pST_ProjTrans_ConstReg->SetWindowText(""); }
	if (m_pDirect3DData->WorldViewProjTransform_ConstReg > -1) { m_pBn_WorldViewProjTrans_Enable->SetCheck(true); m_pST_WorldViewProjTrans_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += 4; }
	else { m_pBn_WorldViewProjTrans_Enable->SetCheck(false); m_pST_WorldViewProjTrans_ConstReg->SetWindowText(""); }
	if (m_pDirect3DData->ViewProjTransform_ConstReg > -1) { m_pBn_ViewProjTrans_Enable->SetCheck(true); m_pST_ViewProjTrans_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += 4; }
	else { m_pBn_ViewProjTrans_Enable->SetCheck(false); m_pST_ViewProjTrans_ConstReg->SetWindowText(""); }

	if (m_pDirect3DData->CamPos_MSpc_ConstReg > -1) { m_pBn_CamPos_MSpc_Enable->SetCheck(true); m_pST_CamPos_MSpc_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += 1; }
	else { m_pBn_CamPos_MSpc_Enable->SetCheck(false); m_pST_CamPos_MSpc_ConstReg->SetWindowText(""); }

	if (m_pDirect3DData->LightPosition_MSpc_ConstReg > -1) { m_pBn_LightPosition_MSpc_Enable->SetCheck(true); m_pST_LightPosition_MSpc_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += m_pDirect3DData->Light_Count; }
	else { m_pBn_LightPosition_MSpc_Enable->SetCheck(false); m_pST_LightPosition_MSpc_ConstReg->SetWindowText(""); }
	if (m_pDirect3DData->LightPosition_CSpc_ConstReg > -1) { m_pBn_LightPosition_CSpc_Enable->SetCheck(true); m_pST_LightPosition_CSpc_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += m_pDirect3DData->Light_Count; }
	else { m_pBn_LightPosition_CSpc_Enable->SetCheck(false); m_pST_LightPosition_CSpc_ConstReg->SetWindowText(""); }
	if (m_pDirect3DData->LightColor_ConstReg > -1) { m_pBn_LightColor_Enable->SetCheck(true); m_pST_LightColor_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += m_pDirect3DData->Light_Count; }
	else { m_pBn_LightColor_Enable->SetCheck(false); m_pST_LightColor_ConstReg->SetWindowText(""); }
	if (m_pDirect3DData->LightAtt_ConstReg > -1) { m_pBn_LightAtt_Enable->SetCheck(true); m_pST_LightAtt_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += m_pDirect3DData->Light_Count; }
	else { m_pBn_LightAtt_Enable->SetCheck(false); m_pST_LightAtt_ConstReg->SetWindowText(""); }
	if (m_pDirect3DData->Material_AmbDifEm_ConstReg > -1) { m_pBn_LightMaterial_AMBDIFEM_Enable->SetCheck(true); m_pST_LightMaterial_AMBDIFEM_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += 3; }
	else { m_pBn_LightMaterial_AMBDIFEM_Enable->SetCheck(false); m_pST_LightMaterial_AMBDIFEM_ConstReg->SetWindowText(""); }
	if (m_pDirect3DData->Material_Specular_ConstReg > -1) { m_pBn_LightMaterial_Specular_Enable->SetCheck(true); m_pST_LightMaterial_Specular_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += 1; }
	else { m_pBn_LightMaterial_Specular_Enable->SetCheck(false); m_pST_LightMaterial_Specular_ConstReg->SetWindowText(""); }
	if (m_pDirect3DData->AmbientLight_ConstReg > -1) { m_pBn_AmbientLight_Enable->SetCheck(true); m_pST_AmbientLight_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += 1; }
	else { m_pBn_AmbientLight_Enable->SetCheck(false); m_pST_AmbientLight_ConstReg->SetWindowText(""); }
	if (m_pDirect3DData->PrevWorldViewTrans_ConstReg > -1) { m_pBn_PrevWorldViewTrans_Enable->SetCheck(true); m_pST_PrevWorldViewTrans_ConstReg->SetWindowText(itoa(iConstReg,szTmp,10)); iConstReg += m_pDirect3DData->PrevWorldViewTrans_Count * 4; }
	else { m_pBn_PrevWorldViewTrans_Enable->SetCheck(false); m_pST_PrevWorldViewTrans_ConstReg->SetWindowText(""); }

	for (uint32 z = 0; z < 2; ++z) {
		m_pBn_Declaration_Stream_Position[z]->SetCheck(m_pDirect3DData->bDeclaration_Stream_Position[z]);
		m_pBn_Declaration_Stream_Normal[z]->SetCheck(m_pDirect3DData->bDeclaration_Stream_Normal[z]);
		m_pBn_Declaration_Stream_UVSets[z]->SetCheck(m_pDirect3DData->bDeclaration_Stream_UVSets[z]);
		m_pEB_Declaration_Stream_UVCount[z]->SetWindowText(itoa(m_pDirect3DData->Declaration_Stream_UVCount[z],szTmp,10));
		m_pBn_Declaration_Stream_Basis[z]->SetCheck(m_pDirect3DData->bDeclaration_Stream_BasisVectors[z]); }

	m_pEB_WorldViewTrans_Param->SetWindowText(itoa(m_pDirect3DData->WorldViewTransform_Count,szTmp,10));
	m_pEB_LightVectors_Param->SetWindowText(itoa(m_pDirect3DData->Light_Count,szTmp,10));
	m_pEB_PrevWorldViewTrans_Param->SetWindowText(itoa(m_pDirect3DData->PrevWorldViewTrans_Count,szTmp,10));
	if (GetDlgItem(IDC_VERTSHADER_NUM_LIGHTPOSITION))	{ ((CStatic*)GetDlgItem(IDC_VERTSHADER_NUM_LIGHTPOSITION))->SetWindowText(szTmp); } 
	if (GetDlgItem(IDC_VERTSHADER_NUM_LIGHTCOLOR))		{ ((CStatic*)GetDlgItem(IDC_VERTSHADER_NUM_LIGHTCOLOR))->SetWindowText(szTmp); }
	if (GetDlgItem(IDC_VERTSHADER_NUM_LIGHTATT))		{ ((CStatic*)GetDlgItem(IDC_VERTSHADER_NUM_LIGHTATT))->SetWindowText(szTmp); }

	m_bReady = true;
}

void CVertexShaderDlg::OnVertexShaderFilename()							
{ 
	char buffer[512], filePath[512], projectPath[512], drive[8];

	if (!m_bReady) return; 
	CString WindowText; 
	m_pEB_Filename->GetWindowText(WindowText); 
	m_pDirect3DData->VertexShaderFilename = WindowText;

	// Record the file-relative and project-relative paths
	_splitpath(g_AppFormView->m_RenderStyle_FileName, drive, buffer, filePath, filePath);
	sprintf(filePath, "%s%s", drive, buffer); 

	_splitpath(g_AppFormView->m_ProjectPath, drive, buffer, projectPath, projectPath);
	sprintf(projectPath, "%s%s", drive, buffer);


	if (!WindowText.IsEmpty()) 
	{
		// If the texture path is explicit, strip out the file or project paths, if possible
		_splitpath(WindowText, drive, buffer, buffer, buffer);

		if (drive[0] != '\0')
		{
			if (g_AppFormView->StripPath(WindowText, filePath, buffer))
			{
				m_pEB_Filename->SetWindowText(buffer);
				m_pDirect3DData->VertexShaderFilename = buffer;
			}
			else if (g_AppFormView->StripPath(WindowText, projectPath, buffer))
			{
				m_pEB_Filename->SetWindowText(buffer);
				m_pDirect3DData->VertexShaderFilename = buffer;
			}
		}
	}

	g_AppFormView->RenderStyleDataChanged(); 
}

void CVertexShaderDlg::OnVertexShaderFileBrowse()			
{ 
	if (!m_bReady) return; 
	CFileDialog BrowseBox(true,"vsh",m_pDirect3DData->VertexShaderFilename.c_str(),OFN_HIDEREADONLY|OFN_CREATEPROMPT,"Vertex Shaders (*.vsh)|*.vsh|Assembled Vertex Shaders (*.ash)|*.ash||"); 
	if (BrowseBox.DoModal() == IDOK) 
	{
		m_pDirect3DData->VertexShaderFilename =  BrowseBox.GetPathName(); 
		m_pEB_Filename->SetWindowText(BrowseBox.GetPathName()); 
	} 
	g_AppFormView->RenderStyleDataChanged(); 
}