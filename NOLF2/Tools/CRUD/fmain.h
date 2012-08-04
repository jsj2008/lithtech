//---------------------------------------------------------------------------

#ifndef FMainH
#define FMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <CheckLst.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------

class TForm_CRUD : public TForm
{
__published:	// IDE-managed Components
	TPanel *c_Panel_Main;
	TSplitter *c_Splitter_Main;
	TPanel *c_Panel_Results;
	TButton *c_Button_Go;
	TOpenDialog *c_OpenDialog_Dir;
	TOpenDialog *c_OpenDialog_File;
	TSaveDialog *c_SaveDialog;
	TPanel *c_Panel_Disabler;
	TLabel *c_Label_Source;
	TLabel *c_Label_Dest;
	TLabel *c_Label_SFiles;
	TEdit *c_Edit_SourceDir;
	TEdit *c_Edit_DestDir;
	TButton *c_Button_SourceBrowse;
	TButton *c_Button_DestBrowse;
	TButton *c_Button_SourceAdd;
	TButton *c_Button_SourceRemove;
	TBitBtn *c_Button_LoadSource;
	TBitBtn *c_Button_SaveSource;
	TListView *c_LB_SourceFiles;
	TRichEdit *c_Memo_Results;
    TCheckBox *c_CheckBox_Recursive;
	void __fastcall c_Button_SourceBrowseClick(TObject *Sender);
	void __fastcall c_Button_DestBrowseClick(TObject *Sender);
	void __fastcall c_Button_SourceRemoveClick(TObject *Sender);
	void __fastcall c_Button_SourceAddClick(TObject *Sender);
	void __fastcall Edit_DirExit(TObject *Sender);
	void __fastcall c_LB_SourceFilesChange(TObject *Sender, TListItem *Item,
          TItemChange Change);
	void __fastcall c_LB_SourceFilesKeyPress(TObject *Sender, char &Key);
	void __fastcall c_LB_SourceFilesChanging(TObject *Sender, TListItem *Item,
          TItemChange Change, bool &AllowChange);
	void __fastcall c_Button_SaveSourceClick(TObject *Sender);
	void __fastcall c_Button_LoadSourceClick(TObject *Sender);
	void __fastcall c_Button_GoClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
	// Open a dialog for selecting a directory
	AnsiString SelectDir(const AnsiString &oldDir, const AnsiString &title);
    // Update the state of the form's controls
    void UpdateControlState();
    // Make a file name relative to the source directory
    AnsiString GetSourceRelative(const AnsiString &fileName);
    // Read a file list into the source list
    void ReadFileList(const AnsiString &fileName);

    // Item state memorization for the multi-select checkbox update
    bool m_bItemWasChecked;
    // Whether or not the processing is going...
    bool m_bProcessing;

    // The crud processor
	CCRUDProcessor m_CRUD;

	// Clear the results window
	void ClearResults();

    // The current file
    string m_sCurFile;

public:		// User declarations
	__fastcall TForm_CRUD(TComponent* Owner);
    void Print(const char *pString);
    void SetCurrentFile(const char *pString) { m_sCurFile = pString; }
    const char *GetCurrentFile() const { return m_sCurFile.begin(); }
};
//---------------------------------------------------------------------------
extern PACKAGE TForm_CRUD *Form_CRUD;
//---------------------------------------------------------------------------

// Global printing function
void OutputResult(const char *pString, bool bError = false);

// Set the file that is currently being processed
void SetCurrentFile(const char *pFileName);

#endif
