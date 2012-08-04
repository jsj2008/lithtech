#ifndef IMPORTCUBEMAPDLG_H
#define IMPORTCUBEMAPDLG_H
#pragma once


enum CubeMapFace { ePosX=0, eNegX, ePosY, eNegY, ePosZ, eNegZ };

class CImportCubeMapDlg : public CDialog
{
public:
	CImportCubeMapDlg( CWnd* parent );
	~CImportCubeMapDlg();

	// filename input and output
	CString m_InputName[6];
	CString m_OutputName;

private:
	CEdit* m_InputNameEdit[6];
	CEdit* m_OutputNameEdit;

	void ChooseFile( CubeMapFace face );

protected:
	// user controls
	void OnPosX() { ChooseFile( ePosX ); }
	void OnNegX() { ChooseFile( eNegX ); }
	void OnPosY() { ChooseFile( ePosY ); }
	void OnNegY() { ChooseFile( eNegY ); }
	void OnPosZ() { ChooseFile( ePosZ ); }
	void OnNegZ() { ChooseFile( eNegZ ); }

	void OnImportGeneratedCubeMap();

	// base class overrides
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};


#endif // IMPORTCUBEMAPDLG_H