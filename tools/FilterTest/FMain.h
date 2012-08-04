//---------------------------------------------------------------------------

#ifndef FMainH
#define FMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------

#define NUM_PROPERTIES 32

class TForm_Main : public TForm
{
__published:	// IDE-managed Components
	TOpenDialog *c_OpenDialog;
	TPanel *c_Panel_Main;
	TLabel *c_Label_FileName;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label7;
	TEdit *c_Edit_FileName;
	TBitBtn *c_Button_OpenFile;
	TComboBox *c_ComboBox_Filter;
	TComboBox *c_ComboBox_Property;
	TEdit *c_Edit_Value;
	TBitBtn *c_Button_Play;
	TBitBtn *c_Button_Stop;
	TButton *c_Button_AdjSmall_Up;
	TButton *c_Button_AdjMed_Up;
	TButton *c_Button_AdjLarge_Up;
	TButton *c_Button_AdjSmall_Down;
	TButton *c_Button_AdjMed_Down;
	TButton *c_Button_AdjLarge_Down;
	TTrackBar *c_TrackBar_Pan;
	TButton *c_Button_AdjExLarge_Up;
	TButton *c_Button_AdjExLarge_Down;
	TPanel *c_Panel_Status;
	TButton *c_Button_AdjExSmall_Up;
	TButton *c_Button_AdjExSmall_Down;
	TLabel *Label4;
	TLabel *Label5;
	TLabel *Label6;
	TLabel *Label8;
	TLabel *Label9;
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall c_Button_OpenFileClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall c_ComboBox_FilterChange(TObject *Sender);
	void __fastcall c_Edit_ValueKeyPress(TObject *Sender, char &Key);
	void __fastcall c_Button_AdjSmall_UpClick(TObject *Sender);
	void __fastcall c_Button_PlayClick(TObject *Sender);
	void __fastcall c_Button_StopClick(TObject *Sender);
	void __fastcall c_Button_AdjMed_UpClick(TObject *Sender);
	void __fastcall c_Button_AdjLarge_UpClick(TObject *Sender);
	void __fastcall c_Button_AdjSmall_DownClick(TObject *Sender);
	void __fastcall c_Button_AdjMed_DownClick(TObject *Sender);
	void __fastcall c_Button_AdjLarge_DownClick(TObject *Sender);
	void __fastcall c_ComboBox_PropertyChange(TObject *Sender);
	void __fastcall c_Edit_FileNameKeyPress(TObject *Sender, char &Key);
	void __fastcall c_Edit_FileNameChange(TObject *Sender);
	void __fastcall c_TrackBar_PanChange(TObject *Sender);
	void __fastcall c_Button_AdjExLarge_UpClick(TObject *Sender);
	void __fastcall c_Button_AdjExLarge_DownClick(TObject *Sender);
	void __fastcall c_Button_AdjExSmall_UpClick(TObject *Sender);
	void __fastcall c_Button_AdjExSmall_DownClick(TObject *Sender);
private:	// User declarations
	AnsiString m_CurFile;

	HPROVIDER m_hCurFilter;
	HDIGDRIVER m_hDigDriver;
	HSAMPLE m_hSample;
	void *m_pFileBuffer;
	int m_iFileSize;

	float m_fProperties[NUM_PROPERTIES];
	int m_iNumProperties;

	bool InitMiles();

	bool OpenFile();

	void FillFilterList();
	void FillPropertyList();

	float ReadProperty();
	void AdjustProperty(float fAmount);

	void ChangeStatus(const AnsiString &sStatus);
public:		// User declarations
	__fastcall TForm_Main(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm_Main *Form_Main;
//---------------------------------------------------------------------------
#endif
