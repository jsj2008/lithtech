#ifndef LIGHTATTENUATION_H
#define LIGHTATTENUATION_H

#pragma once

#include "resource.h"
#include "proplist.h"


class CAttenuationFunction
{
public:
	float m_A;
	float m_B;
	float m_C;

	void Initialize( float lightRadius, const LTVector& coefs, const LTVector& exps );
	float Evaluate( float dist );		// function value at dist
};


struct LightAttenuationPreset
{
	CString m_Name;		// preset name
	LTVector m_Coefs;	// coefficients
	LTVector m_Exps;	// exponents
};


class CLightAttenuationDlg : public CDialog
{
public:
	CPropList* m_pPropList;		// the properties of the current light
	
	CLightAttenuationDlg( CPropList* pPropList, CWnd* pParent = NULL );

private:
	int m_GraphWidth;			// width of the graphs in pixels
	CAttenuationFunction m_Att;	// function used to attenuate light

	bool m_ShowColors;

	float m_LightRadius;		// radius of the light
	LTVector m_InnerColor;
	LTVector m_OuterColor;
	LTVector m_Coefs;
	LTVector m_Exps;
	char m_Type[MAX_STRINGPROP_LEN+1];

	CMoArray<LightAttenuationPreset> m_Presets;	// list of light presets

	// controls
	CComboBox* m_pPresetCombo;
	CEdit* m_pCoefA;
	CEdit* m_pCoefB;
	CEdit* m_pCoefC;
	CEdit* m_pExpA;
	CEdit* m_pExpB;
	CEdit* m_pExpC;
	CSpinButtonCtrl* m_pCoefASpin;
	CSpinButtonCtrl* m_pCoefBSpin;
	CSpinButtonCtrl* m_pCoefCSpin;
	CSpinButtonCtrl* m_pExpASpin;
	CSpinButtonCtrl* m_pExpBSpin;
	CSpinButtonCtrl* m_pExpCSpin;

	void DrawAttenuation();
	void DrawGraph( float* values );
	void DrawGradient( float* values );

	void SetupControls();
	void UpdateFields( bool updateSpinners );
	void UpdatePresets();
	void SelectNone();

	void LoadPresets();
	bool SavePreset( const LightAttenuationPreset& preset, bool warn=true );

	void CoefChanged( CEdit* edit, float* val, CSpinButtonCtrl* spinner );
	void ExpChanged( CEdit* edit, float* val, CSpinButtonCtrl* spinner );

protected:
	BOOL OnInitDialog();
	void OnOK();
	void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	void OnCoefAChange() { CoefChanged( m_pCoefA, &(m_Coefs.x), m_pCoefASpin ); }
	void OnCoefBChange() { CoefChanged( m_pCoefB, &(m_Coefs.y), m_pCoefBSpin ); }
	void OnCoefCChange() { CoefChanged( m_pCoefC, &(m_Coefs.z), m_pCoefCSpin ); }
	void OnExpAChange() { ExpChanged( m_pExpA, &(m_Exps.x), m_pExpASpin ); }
	void OnExpBChange() { ExpChanged( m_pExpB, &(m_Exps.y), m_pExpBSpin ); }
	void OnExpCChange() { ExpChanged( m_pExpC, &(m_Exps.z), m_pExpCSpin ); }
	void OnPresetSelect();
	void OnShowColors();
	void OnSavePreset();
	void OnPaint();

	DECLARE_MESSAGE_MAP()
};


#endif // LIGHTATTENUATION_H