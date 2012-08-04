
#ifndef RENDERPASSDLG_H
#define RENDERPASSDLG_H

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file
#endif

#include "resource.h"
#include "renderstyle.h"
#include "RenderStylesEditor.h"
#include <string>
#include <vector>

using namespace std;

class CRenderPassDlg : public CDialog
{
public:
    CRenderPassDlg() : CDialog(IDD_RENDERPASS) { }

	// Set by the parent dialog (tell us how to set our init conditions and where changes go)...
	void SetRenderPassData(CRenderPassData* pRenderPassData)	{ m_pRenderPassData = pRenderPassData; }
	void LimitDialogByPlatforms(bool platformD3D, bool platformPS2, bool platformXBox); // Enable and disable dialog items according to platform

private:
	CButton*			m_pBn_DynamicLight;						// Render State Stuff...
	CEdit*				m_pEB_AlphaRef;
	CEdit*				m_pEB_TFactor;
	CComboBox*			m_pCB_BlendOp;
	CComboBox*			m_pCB_ZBuffer;
	CComboBox*			m_pCB_CullMode;
	CComboBox*			m_pCB_AlphaTest;
	CComboBox*			m_pCB_ZBufferTest;
	CComboBox*			m_pCB_FillMode;
	CComboBox*			m_pCB_TextureParam[4];					// Texture Params...
	CComboBox*			m_pCB_ColorOp[4];
	CComboBox*			m_pCB_ColorArg1[4];
	CComboBox*			m_pCB_ColorArg2[4];
	CComboBox*			m_pCB_AlphaOp[4];
	CComboBox*			m_pCB_AlphaArg1[4];
	CComboBox*			m_pCB_AlphaArg2[4];
	CComboBox*			m_pCB_UVSource[4];
	CComboBox*			m_pCB_UAddress[4];
	CComboBox*			m_pCB_VAddress[4];
	CComboBox*			m_pCB_TexFilter[4];
	CButton*			m_pCB_UVTransform_Enable[4];
	CButton*			m_pCB_UVTransform_Matrix[4];
	CButton*			m_pBn_VertexShader_Enable;				// Direct3D Options...
	CButton*			m_pBn_PixelShader_Enable;
	CButton*			m_pBn_BumpEnvMap_Enable;
	CEdit*				m_pEB_BumpEnvMapStage;
	CEdit*				m_pEB_BumpEnvMapScale;
	CEdit*				m_pEB_BumpEnvMapOffset;
	CEdit*				m_pEB_VertexShader_ID;
	CEdit*				m_pEB_PixelShader_ID;

	CRenderPassData*	m_pRenderPassData;						// Render pass data (we set need to set this structure with any changes so the main dlg gets our changes)...

	// We are taking care to allow additions to the enums without barfing up...
	uint32			GetIDfromRSEnum_Blend(uint32 RSEnum) const;
	vector<ERenStyle_ZBufferMode> m_IDtoRSEnum_ZBufferMode;		// Maps the control select ID to the CRenderStyle enum...
	uint32			GetIDfromRSEnum_ZBufferMode(uint32 RSEnum)	{ for (uint32 i=0;i<m_IDtoRSEnum_ZBufferMode.size();++i) { if (m_IDtoRSEnum_ZBufferMode[i]==(ERenStyle_ZBufferMode)RSEnum) return i; } return 0; }
	vector<ERenStyle_CullMode> m_IDtoRSEnum_CullMode;			// Maps the control select ID to the CRenderStyle enum...
	uint32			GetIDfromRSEnum_CullMode(uint32 RSEnum)		{ for (uint32 i=0;i<m_IDtoRSEnum_CullMode.size();++i) { if (m_IDtoRSEnum_CullMode[i]==(ERenStyle_CullMode)RSEnum) return i; } return 0; }
	vector<ERenStyle_TestMode> m_IDtoRSEnum_TestMode;		// Maps the control select ID to the CRenderStyle enum...
	uint32			GetIDfromRSEnum_TestMode(uint32 RSEnum) { for (uint32 i=0;i<m_IDtoRSEnum_TestMode.size();++i) { if (m_IDtoRSEnum_TestMode[i]==(ERenStyle_TestMode)RSEnum) return i; } return 0; }
	vector<ERenStyle_FillMode> m_IDtoRSEnum_FillMode;			// Maps the control select ID to the CRenderStyle enum...
	uint32			GetIDfromRSEnum_FillMode(uint32 RSEnum)		{ for (uint32 i=0;i<m_IDtoRSEnum_FillMode.size();++i) { if (m_IDtoRSEnum_FillMode[i]==(ERenStyle_FillMode)RSEnum) return i; } return 0; }
	vector<ERenStyle_TextureParam> m_IDtoRSEnum_TextureParam;	// Maps the control select ID to the CRenderStyle enum...
	uint32			GetIDfromRSEnum_TextureParam(uint32 RSEnum)	{ for (uint32 i=0;i<m_IDtoRSEnum_TextureParam.size();++i) { if (m_IDtoRSEnum_TextureParam[i]==(ERenStyle_TextureParam)RSEnum) return i; } return 0; }
	uint32			GetIDfromRSEnum_ColorOp(uint32 RSEnum) const;
	uint32			GetIDfromRSEnum_ColorArg1(uint32 RSEnum) const;
	vector<ERenStyle_ColorArg> m_IDtoRSEnum_ColorArg2;			// Maps the control select ID to the CRenderStyle enum...
	uint32			GetIDfromRSEnum_ColorArg2(uint32 RSEnum)	{ for (uint32 i=0;i<m_IDtoRSEnum_ColorArg2.size();++i) { if (m_IDtoRSEnum_ColorArg2[i]==(ERenStyle_ColorArg)RSEnum) return i; } return 0; }
	vector<ERenStyle_AlphaOp> m_IDtoRSEnum_AlphaOp;				// Maps the control select ID to the CRenderStyle enum...
	uint32			GetIDfromRSEnum_AlphaOp(uint32 RSEnum)		{ for (uint32 i=0;i<m_IDtoRSEnum_AlphaOp.size();++i) { if (m_IDtoRSEnum_AlphaOp[i]==(ERenStyle_AlphaOp)RSEnum) return i; } return 0; }
	vector<ERenStyle_AlphaArg> m_IDtoRSEnum_AlphaArg1;			// Maps the control select ID to the CRenderStyle enum...
	uint32			GetIDfromRSEnum_AlphaArg1(uint32 RSEnum)	{ for (uint32 i=0;i<m_IDtoRSEnum_AlphaArg1.size();++i) { if (m_IDtoRSEnum_AlphaArg1[i]==(ERenStyle_AlphaArg)RSEnum) return i; } return 0; }
	vector<ERenStyle_AlphaArg> m_IDtoRSEnum_AlphaArg2;			// Maps the control select ID to the CRenderStyle enum...
	uint32			GetIDfromRSEnum_AlphaArg2(uint32 RSEnum)	{ for (uint32 i=0;i<m_IDtoRSEnum_AlphaArg2.size();++i) { if (m_IDtoRSEnum_AlphaArg2[i]==(ERenStyle_AlphaArg)RSEnum) return i; } return 0; }
	vector<ERenStyle_UV_Source>	m_IDtoRSEnum_UVSource;			// Maps the control select ID to the CRenderStyle enum...
	uint32			GetIDfromRSEnum_UVSource(uint32 RSEnum)		{ for (uint32 i=0;i<m_IDtoRSEnum_UVSource.size();++i) { if (m_IDtoRSEnum_UVSource[i]==(ERenStyle_UV_Source)RSEnum) return i; } return 0; }
	uint32			GetIDfromRSEnum_UAddress(uint32 RSEnum) const;
	uint32			GetIDfromRSEnum_VAddress(uint32 RSEnum)	const;
	uint32			GetIDfromRSEnum_TexFilter(uint32 RSEnum) const;
	void			PopulateDialogControls_And_CreateIDtoRSEnum_Maps();
	void			SetDialogControls_From_RenderStyleData();

public:
	// CDialog Overrides...
	virtual BOOL OnInitDialog();

	static ERenStyle_ColorOp StringToColorOp(const char* pszString);
	void UpdateColorOp(uint32 nColorOp);

	void HandleTexTransform(uint32 nStage);

	//{{AFX_MSG(CAppForm)
	afx_msg void OnBlend();
	afx_msg void OnZBuffer()									{ m_pRenderPassData->ZBuffer = m_IDtoRSEnum_ZBufferMode[m_pCB_ZBuffer->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnCullMode()									{ m_pRenderPassData->CullMode = m_IDtoRSEnum_CullMode[m_pCB_CullMode->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaTestMode()								{ m_pRenderPassData->AlphaTestMode = m_IDtoRSEnum_TestMode[m_pCB_AlphaTest->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnZBufferTestMode()							{ m_pRenderPassData->ZBufferTestMode = m_IDtoRSEnum_TestMode[m_pCB_ZBufferTest->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnFillMode()									{ m_pRenderPassData->FillMode = m_IDtoRSEnum_FillMode[m_pCB_FillMode->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnTFactor()									{ CString WindowText; m_pEB_TFactor->GetWindowText(WindowText); uint32 iTmp = 0x80808080; sscanf(LPCSTR(WindowText),"%x",&iTmp); m_pRenderPassData->TextureFactor = iTmp; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnDynamicLight()								{ m_pRenderPassData->DynamicLight = m_pBn_DynamicLight->GetCheck() ? true : false; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaRef()									{ CString WindowText; m_pEB_AlphaRef->GetWindowText(WindowText); uint32 iTmp = 128; sscanf(LPCSTR(WindowText),"%d",&iTmp); m_pRenderPassData->AlphaRef = iTmp; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnTextureParam1();
	afx_msg void OnTextureParam2()								{ m_pRenderPassData->TextureParam[1] = m_IDtoRSEnum_TextureParam[m_pCB_TextureParam[1]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnTextureParam3()								{ m_pRenderPassData->TextureParam[2] = m_IDtoRSEnum_TextureParam[m_pCB_TextureParam[2]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnTextureParam4()								{ m_pRenderPassData->TextureParam[3] = m_IDtoRSEnum_TextureParam[m_pCB_TextureParam[3]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
  	afx_msg void OnColorOp1();
	afx_msg void OnColorOp2();
	afx_msg void OnColorOp3();
	afx_msg void OnColorOp4();
  	afx_msg void OnColorArg11();
	afx_msg void OnColorArg12();
	afx_msg void OnColorArg13();
	afx_msg void OnColorArg14();
  	afx_msg void OnColorArg21()									{ m_pRenderPassData->ColorArg2[0] = m_IDtoRSEnum_ColorArg2[m_pCB_ColorArg2[0]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnColorArg22()									{ m_pRenderPassData->ColorArg2[1] = m_IDtoRSEnum_ColorArg2[m_pCB_ColorArg2[1]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnColorArg23()									{ m_pRenderPassData->ColorArg2[2] = m_IDtoRSEnum_ColorArg2[m_pCB_ColorArg2[2]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnColorArg24()									{ m_pRenderPassData->ColorArg2[3] = m_IDtoRSEnum_ColorArg2[m_pCB_ColorArg2[3]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
  	afx_msg void OnAlphaOp1()									{ m_pRenderPassData->AlphaOp[0] = m_IDtoRSEnum_AlphaOp[m_pCB_AlphaOp[0]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaOp2()									{ m_pRenderPassData->AlphaOp[1] = m_IDtoRSEnum_AlphaOp[m_pCB_AlphaOp[1]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaOp3()									{ m_pRenderPassData->AlphaOp[2] = m_IDtoRSEnum_AlphaOp[m_pCB_AlphaOp[2]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaOp4()									{ m_pRenderPassData->AlphaOp[3] = m_IDtoRSEnum_AlphaOp[m_pCB_AlphaOp[3]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
  	afx_msg void OnAlphaArg11()									{ m_pRenderPassData->AlphaArg1[0] = m_IDtoRSEnum_AlphaArg1[m_pCB_AlphaArg1[0]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaArg12()									{ m_pRenderPassData->AlphaArg1[1] = m_IDtoRSEnum_AlphaArg1[m_pCB_AlphaArg1[1]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaArg13()									{ m_pRenderPassData->AlphaArg1[2] = m_IDtoRSEnum_AlphaArg1[m_pCB_AlphaArg1[2]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaArg14()									{ m_pRenderPassData->AlphaArg1[3] = m_IDtoRSEnum_AlphaArg1[m_pCB_AlphaArg1[3]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
  	afx_msg void OnAlphaArg21()									{ m_pRenderPassData->AlphaArg2[0] = m_IDtoRSEnum_AlphaArg2[m_pCB_AlphaArg2[0]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaArg22()									{ m_pRenderPassData->AlphaArg2[1] = m_IDtoRSEnum_AlphaArg2[m_pCB_AlphaArg2[1]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaArg23()									{ m_pRenderPassData->AlphaArg2[2] = m_IDtoRSEnum_AlphaArg2[m_pCB_AlphaArg2[2]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnAlphaArg24()									{ m_pRenderPassData->AlphaArg2[3] = m_IDtoRSEnum_AlphaArg2[m_pCB_AlphaArg2[3]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnUVSource1()									{ m_pRenderPassData->UVSource[0] = m_IDtoRSEnum_UVSource[m_pCB_UVSource[0]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnUVSource2()									{ m_pRenderPassData->UVSource[1] = m_IDtoRSEnum_UVSource[m_pCB_UVSource[1]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnUVSource3()									{ m_pRenderPassData->UVSource[2] = m_IDtoRSEnum_UVSource[m_pCB_UVSource[2]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnUVSource4()									{ m_pRenderPassData->UVSource[3] = m_IDtoRSEnum_UVSource[m_pCB_UVSource[3]->GetCurSel()]; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnUAddress1();
	afx_msg void OnUAddress2();
	afx_msg void OnUAddress3();
	afx_msg void OnUAddress4();
	afx_msg void OnVAddress1();
	afx_msg void OnVAddress2();
	afx_msg void OnVAddress3();
	afx_msg void OnVAddress4();
	afx_msg void OnTexFilter1();
	afx_msg void OnTexFilter2();
	afx_msg void OnTexFilter3();
	afx_msg void OnTexFilter4();
	afx_msg void OnUVTransform_Enable1()						{ m_pRenderPassData->UVTransform_Enable[0] = m_pCB_UVTransform_Enable[0]->GetCheck() ? true : false; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnUVTransform_Enable2()						{ m_pRenderPassData->UVTransform_Enable[1] = m_pCB_UVTransform_Enable[1]->GetCheck() ? true : false; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnUVTransform_Enable3()						{ m_pRenderPassData->UVTransform_Enable[2] = m_pCB_UVTransform_Enable[2]->GetCheck() ? true : false; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnUVTransform_Enable4()						{ m_pRenderPassData->UVTransform_Enable[3] = m_pCB_UVTransform_Enable[3]->GetCheck() ? true : false; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnUVTransform_Matrix1();
	afx_msg void OnUVTransform_Matrix2();
	afx_msg void OnUVTransform_Matrix3();
	afx_msg void OnUVTransform_Matrix4();
	afx_msg void OnD3DOptions_VertexShader_Enable()				{ m_pRenderPassData->VertexShader_Enable = m_pBn_VertexShader_Enable->GetCheck() ? true : false; g_AppFormView->RenderStyleDataChanged(true); }
	afx_msg void OnD3DOptions_PixelShader_Enable()				{ m_pRenderPassData->PixelShader_Enable = m_pBn_PixelShader_Enable->GetCheck() ? true : false; g_AppFormView->RenderStyleDataChanged(true); }
	afx_msg void OnD3DOptions_BumpEnvMap_Enable()				{ m_pRenderPassData->bUseBumpEnvMap = m_pBn_BumpEnvMap_Enable->GetCheck() ? true : false; g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnD3DOptions_BumpEnvMap_Stage()				{ CString WindowText; m_pEB_BumpEnvMapStage->GetWindowText(WindowText); m_pRenderPassData->BumpEnvMapStage = atoi(LPCSTR(WindowText)); g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnD3DOptions_BumpEnvMap_Scale()				{ CString WindowText; m_pEB_BumpEnvMapScale->GetWindowText(WindowText); m_pRenderPassData->fBumpEnvMap_Scale = (float)atof(LPCSTR(WindowText)); g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnD3DOptions_BumpEnvMap_Offset()				{ CString WindowText; m_pEB_BumpEnvMapOffset->GetWindowText(WindowText); m_pRenderPassData->fBumpEnvMap_Offset = (float)atof(LPCSTR(WindowText)); g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnD3DOptions_VertexShader_ID()					{ CString WindowText; m_pEB_VertexShader_ID->GetWindowText(WindowText); m_pRenderPassData->VertexShader_ID = atoi(LPCSTR(WindowText)); g_AppFormView->RenderStyleDataChanged(); }
	afx_msg void OnD3DOptions_PixelShader_ID()					{ CString WindowText; m_pEB_PixelShader_ID->GetWindowText(WindowText); m_pRenderPassData->PixelShader_ID = atoi(LPCSTR(WindowText)); g_AppFormView->RenderStyleDataChanged(); }
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif


