
#ifndef VERTEXSHADERDLG_H
#define VERTEXSHADERDLG_H

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file
#endif

#include "resource.h"
#include "renderstyle.h"
#include "RenderStylesEditor.h"
#include "Utilities.h"
#include <string>

using namespace std;

class CVertexShaderDlg : public CDialog
{
public:
    CVertexShaderDlg() : CDialog(IDD_VERTEXSHADER) { m_bReady = false; }

	// Set by the parent dialog (tell us how to set our init conditions and where changes go)...
	void SetRenderPassData(CDirect3DData* pDirect3DData)			{ m_pDirect3DData = pDirect3DData; }

private:
	// Dialog controls...
	CEdit*				m_pEB_Filename;
	CButton*			m_pBn_Filename_Browse;
	CButton*			m_pBn_AssembleVertShader;
	CButton*			m_pBn_ExpandSkinning_Enable;
	CButton*			m_pBn_GenericVector_Enable[3];
	CStatic*			m_pST_GenericVector_ConstReg[3];
	CEdit*				m_pEB_GenericVector1_Params[4];
	CEdit*				m_pEB_GenericVector2_Params[4];
	CEdit*				m_pEB_GenericVector3_Params[4];
	CButton*			m_pBn_WorldViewTrans_Enable;
	CStatic*			m_pST_WorldViewTrans_ConstReg;
	CEdit*				m_pEB_WorldViewTrans_Param;
	CButton*			m_pBn_ProjTrans_Enable;
	CStatic*			m_pST_ProjTrans_ConstReg;
	CButton*			m_pBn_WorldViewProjTrans_Enable;
	CStatic*			m_pST_WorldViewProjTrans_ConstReg;
	CButton*			m_pBn_ViewProjTrans_Enable;
	CStatic*			m_pST_ViewProjTrans_ConstReg;
	CButton*			m_pBn_CamPos_MSpc_Enable;
	CStatic*			m_pST_CamPos_MSpc_ConstReg;
	CButton*			m_pBn_LightVectors_Enable;
	CEdit*				m_pEB_LightVectors_Param;
	CButton*			m_pBn_LightPosition_MSpc_Enable;
	CStatic*			m_pST_LightPosition_MSpc_ConstReg;
	CButton*			m_pBn_LightPosition_CSpc_Enable;
	CStatic*			m_pST_LightPosition_CSpc_ConstReg;
	CButton*			m_pBn_LightColor_Enable;
	CStatic*			m_pST_LightColor_ConstReg;
	CButton*			m_pBn_LightAtt_Enable;
	CStatic*			m_pST_LightAtt_ConstReg;
	CButton*			m_pBn_LightMaterial_AMBDIFEM_Enable;
	CStatic*			m_pST_LightMaterial_AMBDIFEM_ConstReg;
	CButton*			m_pBn_LightMaterial_Specular_Enable;
	CStatic*			m_pST_LightMaterial_Specular_ConstReg;
	CButton*			m_pBn_AmbientLight_Enable;
	CStatic*			m_pST_AmbientLight_ConstReg;
	CButton*			m_pBn_PrevWorldViewTrans_Enable;
	CStatic*			m_pST_PrevWorldViewTrans_ConstReg;
	CEdit*				m_pEB_PrevWorldViewTrans_Param;
	CButton*			m_pBn_Declaration_Stream_Position[2];
	CButton*			m_pBn_Declaration_Stream_Normal[2];
	CButton*			m_pBn_Declaration_Stream_UVSets[2];
	CEdit*				m_pEB_Declaration_Stream_UVCount[2];
	CButton*			m_pBn_Declaration_Stream_Basis[2];

	bool				m_bReady;									// Need this to ignore messages until we can set all of our data...
	CDirect3DData*		m_pDirect3DData;							// Passed in data (we set need to set this structure with any changes so the main dlg gets our changes)...

	void				InitializeDialogControls();					// Grab the Dialog's controls...
	void				SetDialogControls_From_RenderStyleData();	// Setup the controls with the passed in data...
	void				CalcConstRegs_AndSet_From_Dialog();			// We figure out what constant regs things go into by adding up the ones before them (and set them in the render style data)...
	void				SetDeclaration_From_Dialog();

public:
	// CDialog Overrides...
	virtual BOOL		OnInitDialog();

	//{{AFX_MSG(CAppForm)
	afx_msg void OnVertexShaderFilename();
	afx_msg void OnVertexShaderFileBrowse();
	afx_msg void OnAConstRegChanged()								{ if (!m_bReady) return; CalcConstRegs_AndSet_From_Dialog(); g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnGenVectorChanged()								{ if (!m_bReady) return; CString WindowText; m_pEB_GenericVector1_Params[0]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param1.x = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector1_Params[1]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param1.y = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector1_Params[2]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param1.z = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector1_Params[3]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param1.w = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector2_Params[0]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param2.x = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector2_Params[1]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param2.y = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector2_Params[2]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param2.z = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector2_Params[3]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param2.w = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector3_Params[0]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param3.x = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector3_Params[1]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param3.y = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector3_Params[2]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param3.z = (float)atof(LPCSTR(WindowText)); m_pEB_GenericVector3_Params[3]->GetWindowText(WindowText); m_pDirect3DData->ConstVector_Param3.w = (float)atof(LPCSTR(WindowText)); g_AppFormView->RenderStyleDataChanged(); } 
	afx_msg void OnWorldViewTransCountChanged()						{ if (!m_bReady) return; CString WindowText; m_pEB_WorldViewTrans_Param->GetWindowText(WindowText); m_pDirect3DData->WorldViewTransform_Count = atoi(LPCSTR(WindowText)); CalcConstRegs_AndSet_From_Dialog(); g_AppFormView->RenderStyleDataChanged(); } 
	afx_msg void OnLightVectorCountChanged()						{ if (!m_bReady) return; CString WindowText; m_pEB_LightVectors_Param->GetWindowText(WindowText); m_pDirect3DData->Light_Count = atoi(LPCSTR(WindowText)); CalcConstRegs_AndSet_From_Dialog(); g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnPrevWorldViewCountChanged()						{ if (!m_bReady) return; CString WindowText; m_pEB_PrevWorldViewTrans_Param->GetWindowText(WindowText); m_pDirect3DData->PrevWorldViewTrans_Count = atoi(LPCSTR(WindowText)); CalcConstRegs_AndSet_From_Dialog(); g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnADeclareChanged()								{ if (!m_bReady) return; SetDeclaration_From_Dialog(); g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnReload()											{ if (!m_bReady) return; g_AppFormView->RenderStyleDataChanged(true); }
	afx_msg void OnExpandForSkinning()								{ if (!m_bReady) return; m_pDirect3DData->bExpandForSkinning = m_pBn_ExpandSkinning_Enable->GetCheck() ? true : false; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnVertexShaderSaveAssembled()						{
		if (m_pDirect3DData->VertexShaderFilename.empty()) return;
		char szTmp[MAX_PATH]; strcpy(szTmp,m_pDirect3DData->VertexShaderFilename.c_str());
		char* szDot = strstr(szTmp,"."); if (szDot) { *szDot = NULL; strcat(szTmp,".ash"); }
		CFileDialog BrowseBox(false,"ash",szTmp,OFN_HIDEREADONLY|OFN_CREATEPROMPT,"Assembled Vertex Shader (*.ash)|*.ash||"); 
		if (BrowseBox.DoModal() == IDOK && BrowseBox.GetPathName()) { 
			FILE* f = fopen(BrowseBox.GetPathName(),"wb"); if (!f) { Msg("Error: Couldn't open output file %s",BrowseBox.GetPathName()); return; }
			uint32 iVersion = 1; fwrite(&iVersion,1,sizeof(iVersion),f);
			bool bExpandForSkinning = m_pDirect3DData->bExpandForSkinning; fwrite(&bExpandForSkinning,1,sizeof(bExpandForSkinning),f);
			for (uint32 iType = 0; iType < VERTSHADER_TYPES_COUNT; ++iType) {
				if (!(m_pDirect3DData->bExpandForSkinning) && (iType > 0)) continue; string szCode;  // Skip out if we're just doing a rigid shader...
				LPD3DXBUFFER pVertShaderCode = NULL; LPD3DXBUFFER pErrors = NULL;					 // Compile the vertex shader...
				if (!PreProcessShaderFile(m_pDirect3DData->VertexShaderFilename.c_str(),szCode,iType) || D3DXAssembleShader(szCode.c_str(),szCode.size(),NULL,NULL,&pVertShaderCode,&pErrors) != D3D_OK) { 
					string szTmp = g_StartingDirectory; szTmp += "\\"; szTmp += m_pDirect3DData->VertexShaderFilename.c_str();
					if (!PreProcessShaderFile(szTmp.c_str(),szCode,iType) || D3DXAssembleShader(szCode.c_str(),szCode.size(),NULL,NULL,&pVertShaderCode,&pErrors) != D3D_OK) { 
						Msg("Error compiling vertex shader.\nYou must have VC installed."); if (pErrors) pErrors->Release(); return; }  }
				uint32 iSize = pVertShaderCode->GetBufferSize();
				fwrite(&iSize,1,sizeof(iSize),f);
				fwrite(pVertShaderCode->GetBufferPointer(),1,pVertShaderCode->GetBufferSize(),f); }
			fclose(f); } }
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif


