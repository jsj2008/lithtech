//---------------------------------------------------------------------------

#include <vcl.h>

#pragma hdrstop

#include <FileCtrl.hpp>
#include <stdio.h>
#include <registry.hpp>

#include <list.h>

#include "crud_proc.h"
#include "crud_util.h"

#include "FMain.h"

// Defines

#define LISTINGFILE_EXT ".lst"
#define REGKEY_CRUD "Software\\Monolith Productions\\CRUD\\"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm_CRUD *Form_CRUD;
//---------------------------------------------------------------------------
__fastcall TForm_CRUD::TForm_CRUD(TComponent* Owner)
        : TForm(Owner),
        m_bProcessing(false)
{
}
//---------------------------------------------------------------------------

// Remove non-printables and \\'s from a path, and make sure it's got a \ on it

AnsiString CleanupPath(const AnsiString &path)
{
	if (!path.Length())
    	return "";
	AnsiString result = path;
	if (!FileExists(result) || DirectoryExists(result))
		result += "\\";

    // Call the main clean-up function..
    CleanPath(result.c_str());

    return result;
}

// External access to the results window
void OutputResult(const char *pString, bool bError)
{
	if (!Form_CRUD)
    	return;

	Form_CRUD->Print(pString);

    if (bError)
    {
        FILE *pErrorLog = fopen("ErrorLog.txt", "at");
        if (pErrorLog)
        {
            fprintf(pErrorLog, "Error processing file %s:\n  ", Form_CRUD->GetCurrentFile());
            fputs(pString, pErrorLog);
            fputc('\n', pErrorLog);
            fclose(pErrorLog);
        }
        else
            Form_CRUD->Print("  ** Error writing to error log.");
    }
}

// External access to the "current file" for error reporting
void SetCurrentFile(const char *pFileName)
{
    Form_CRUD->SetCurrentFile(pFileName);
}

// Go through and update a control's children to match the enable state of their parent
void UpdateChildEnable(TWinControl *parent)
{
	for (int iEnableLoop = 0; iEnableLoop < parent->ControlCount; ++iEnableLoop)
    {
    	parent->Controls[iEnableLoop]->Enabled = parent->Enabled;
    }
}

void __fastcall TForm_CRUD::c_Button_SourceBrowseClick(TObject *Sender)
{
	c_Edit_SourceDir->Text = SelectDir(c_Edit_SourceDir->Text, "Select source directory");
}
//---------------------------------------------------------------------------

void __fastcall TForm_CRUD::c_Button_DestBrowseClick(TObject *Sender)
{
	c_Edit_DestDir->Text = SelectDir(c_Edit_DestDir->Text, "Select destination directory");
}

AnsiString TForm_CRUD::SelectDir(const AnsiString &oldDir, const AnsiString &title)
{
	c_OpenDialog_Dir->InitialDir = oldDir;
    c_OpenDialog_Dir->Title = title;
    if (c_OpenDialog_Dir->Execute())
    {
		return ExtractFilePath(CleanupPath(c_OpenDialog_Dir->FileName));
    }
    else
    	return oldDir;
}

//---------------------------------------------------------------------------
void TForm_CRUD::UpdateControlState()
{
	c_Button_SourceRemove->Enabled = c_LB_SourceFiles->SelCount > 0;
}

//---------------------------------------------------------------------------

void __fastcall TForm_CRUD::c_Button_SourceRemoveClick(TObject *Sender)
{
	// Uh, you shouldn't have gotten here...
	if (c_LB_SourceFiles->SelCount == 0)
    {
    	c_Button_SourceRemove->Enabled = false;
    	return;
    }

    // Delete the selected items from the list
	for (int iDeleteLoop = 0; iDeleteLoop < c_LB_SourceFiles->Items->Count; ++iDeleteLoop)
    {

    	if (c_LB_SourceFiles->Items->Item[iDeleteLoop]->Selected)
        	c_LB_SourceFiles->Items->Delete(iDeleteLoop--);
    }
	UpdateControlState();
}
//---------------------------------------------------------------------------


void __fastcall TForm_CRUD::c_Button_SourceAddClick(TObject *Sender)
{
	c_OpenDialog_File->Title = "Select files to add to the Source File list";
	c_OpenDialog_File->Filter = "File Listings (*" LISTINGFILE_EXT ")|*" LISTINGFILE_EXT "|All Files (*.*)|*.*";
    c_OpenDialog_File->InitialDir = c_Edit_SourceDir->Text;
    if (!c_OpenDialog_File->Execute())
    	return;

    // Add the files to the list
    for (int iInsertLoop = 0; iInsertLoop < c_OpenDialog_File->Files->Count; ++iInsertLoop)
    {
    	AnsiString fileName = c_OpenDialog_File->Files->Strings[iInsertLoop];
	    if (ExtractFileExt(fileName).AnsiCompareIC(".lst") == 0)
        	ReadFileList(fileName);
        else
        {
	    	TListItem *pNewItem = c_LB_SourceFiles->Items->Add();
    	    pNewItem->Caption = GetSourceRelative(fileName);
        	pNewItem->Checked = true;
        }
    }

	UpdateControlState();
}

AnsiString TForm_CRUD::GetSourceRelative(const AnsiString &fileName)
{
	AnsiString fileDir = ExtractFilePath(fileName);
    fileDir.Delete(c_Edit_SourceDir->Text.Length() + 1, fileDir.Length());
	if (fileDir.AnsiCompareIC(c_Edit_SourceDir->Text) == 0)
    	return fileName.SubString(fileDir.Length() + 1, fileName.Length());
    else
    	return fileName;
}

//---------------------------------------------------------------------------


void __fastcall TForm_CRUD::Edit_DirExit(TObject *Sender)
{
	((TEdit*)Sender)->Text = ExtractFilePath(CleanupPath(((TEdit*)Sender)->Text));
}
//---------------------------------------------------------------------------




void __fastcall TForm_CRUD::c_LB_SourceFilesChange(TObject *Sender,
      TListItem *Item, TItemChange Change)
{
	if ((Change == ctState) && (Item->Checked != m_bItemWasChecked))
    {
		bool bChecked = Item->Checked;

	    // Update all of the selected items' check states
        for (int iCheckLoop = 0; iCheckLoop < c_LB_SourceFiles->Items->Count; ++iCheckLoop)
        {
        	TListItem *pCheckItem = c_LB_SourceFiles->Items->Item[iCheckLoop];
            if (pCheckItem->Selected)
            	pCheckItem->Checked = bChecked;
        }
    }

	UpdateControlState();
}
//---------------------------------------------------------------------------



void __fastcall TForm_CRUD::c_LB_SourceFilesKeyPress(TObject *Sender,
      char &Key)
{
	switch (Key)
    {
    	case '+' :
        	c_Button_SourceAddClick(c_Button_SourceAdd);
            break;
        case '-' :
        	c_Button_SourceRemoveClick(c_Button_SourceRemove);
            break;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm_CRUD::c_LB_SourceFilesChanging(TObject *Sender,
      TListItem *Item, TItemChange Change, bool &AllowChange)
{
	m_bItemWasChecked = Item->Checked;
}
//---------------------------------------------------------------------------

// Print out a string in the results box

void TForm_CRUD::Print(const char *pString)
{
	AnsiString newLine(pString);
	c_Memo_Results->Lines->Add(newLine);

    FILE *pLog = fopen("crud.log", "at");
    if (!pLog)
	    return;
    fprintf(pLog, "%s\n", pString);
    fclose(pLog);
}

void TForm_CRUD::ClearResults()
{
	c_Memo_Results->Lines->Clear();

	DeleteFile("crud.log");
}

void __fastcall TForm_CRUD::c_Button_SaveSourceClick(TObject *Sender)
{
	c_SaveDialog->Title = "Select a file name for the Source File list";
    c_SaveDialog->InitialDir = "";
    if (!c_SaveDialog->Execute())
    	return;

	AnsiString fileName = c_SaveDialog->FileName;
	if (ExtractFileExt(fileName) == "")
		fileName += ".lst";

	FILE *pFile = fopen(fileName.c_str(), "wt");
	if (!pFile)
    {
    	ShowMessage("Error opening file " + c_SaveDialog->FileName);
        return;
    }

    // Write out the lines to the file
    for (int iWriteLoop = 0; iWriteLoop < c_LB_SourceFiles->Items->Count; ++iWriteLoop)
    {
    	TListItem *pItem = c_LB_SourceFiles->Items->Item[iWriteLoop];

        fputs(pItem->Caption.c_str(), pFile);
        fputs("\n", pFile);
    }

    // Close the file...
    fclose(pFile);
}
//---------------------------------------------------------------------------

void __fastcall TForm_CRUD::c_Button_LoadSourceClick(TObject *Sender)
{
	c_OpenDialog_File->Title = "Select a Source File list file to load";
	c_OpenDialog_File->Filter = "File Listings (*" LISTINGFILE_EXT ")|*" LISTINGFILE_EXT "|All Files (*.*)|*.*";
    c_OpenDialog_File->InitialDir = c_Edit_SourceDir->Text;
    if (!c_OpenDialog_File->Execute())
    	return;

    ReadFileList(c_OpenDialog_File->FileName);
}
//---------------------------------------------------------------------------
void TForm_CRUD::ReadFileList(const AnsiString &fileName)
{
	// Open the file
    FILE *pFile = fopen(fileName.c_str(), "rt");

    char nextFile[256];
    while (!feof(pFile))
    {
    	// Read a line from the file
		if (!fgets(nextFile, 255, pFile))
        	break;
        CleanPath(nextFile);
        if (!nextFile[0])
        	continue;

        // Add a new item for the new line
        TListItem *pNewItem = c_LB_SourceFiles->Items->Add();
        pNewItem->Caption = AnsiString(nextFile);
        pNewItem->Checked = true;
    }

    // Remember to close the file..
	fclose(pFile);
}


void __fastcall TForm_CRUD::c_Button_GoClick(TObject *Sender)
{
	static bWasCancelled;

	// Tell the processing to stop if we get in here while it's processing..
	if (m_bProcessing)
	{
		bWasCancelled = true;
		m_CRUD.Stop();
		// Tell the user we're trying to stop
		c_Button_Go->Caption = "Stopping..";
		Application->ProcessMessages();
		return;
	}

	// Change focus to the results window so it will scroll
	c_Memo_Results->SetFocus();

	// Oh, and clear it out, too..
	ClearResults();

	bWasCancelled = false;

	TFileList fileList;

    for (int iAddLoop = 0; iAddLoop < c_LB_SourceFiles->Items->Count; ++iAddLoop)
    {
    	TListItem *pItem = c_LB_SourceFiles->Items->Item[iAddLoop];
        if (pItem->Checked)
	    	fileList.push_back(pItem->Caption.c_str());
    }

    // Remember that we're processing
    m_bProcessing = true;
    // Change the button
    c_Button_Go->Caption = "&Cancel";
    // Turn off everything else
    c_Panel_Disabler->Enabled = false;
    // Update the childrens' enabled state
	UpdateChildEnable(c_Panel_Disabler);
    // Get one last update in before it starts..
	Application->ProcessMessages();

	// Ok, go process
	CCRUDResults results;
	m_CRUD.ProcessFiles(fileList, c_Edit_SourceDir->Text.c_str(), c_Edit_DestDir->Text.c_str(), c_CheckBox_Recursive->Checked, results);

	// Turn everything back on
	c_Panel_Disabler->Enabled = true;
	// Update the childrens' enabled state
	UpdateChildEnable(c_Panel_Disabler);
	// Bring back the button
	c_Button_Go->Caption = "&Go";
	// Remember that we're done
	m_bProcessing = false;
	// Add a message if it got cancelled
	if (bWasCancelled)
		Print("Processing cancelled by user.");
	// Or tell them how it went..
	else
	{
		AnsiString sMessage;
		sMessage.printf("Done processing in %s.  Found %d files.  (%d copied)",
			FormatDateTime("n 'min.' s 'sec.'", results.m_ProcessingTime), results.m_iFilesFound, results.m_iFilesCopied);
		Print(sMessage.c_str());
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm_CRUD::FormClose(TObject *Sender,
      TCloseAction &Action)
{
	// Make sure we stop processing!
    if (m_bProcessing)
    {
    	c_Button_GoClick(this);
    }

	// Read the source and destination directories

	TRegistry *reg = new TRegistry;

    try
    {
	    reg->RootKey = HKEY_CURRENT_USER;

	    if (reg->OpenKey(REGKEY_CRUD, true))
	    {
        	try
            {
			    reg->WriteString("SourceDir", c_Edit_SourceDir->Text);
		        reg->WriteString("DestDir", c_Edit_DestDir->Text);
                reg->WriteInteger("WindowPosX", Left);
                reg->WriteInteger("WindowPosY", Top);
				reg->WriteInteger("WindowSizeX", Width);
                reg->WriteInteger("WindowSizeY", Height);
				reg->WriteInteger("SplitSizeY", c_Panel_Main->Height);
            }
			__except (1)
            {
            	// We don't care if we fail here..
            }
		    reg->CloseKey();
	    }
    }
    __finally
    {
    	delete reg;
    }

    // Move to the bottom of the results list
    c_Memo_Results->SelStart = c_Memo_Results->Text.Length();
    c_Memo_Results->SelLength = 1;
}
//---------------------------------------------------------------------------



void __fastcall TForm_CRUD::FormCreate(TObject *Sender)
{
	// Get the source and destination directories out of the registry
	TRegistry *reg = new TRegistry;

    try
    {
	    reg->RootKey = HKEY_CURRENT_USER;
		if (reg->OpenKey(REGKEY_CRUD, false))
	    {
        	try
            {
		    	c_Edit_SourceDir->Text = reg->ReadString("SourceDir");
		        c_Edit_DestDir->Text = reg->ReadString("DestDir");
	            c_Panel_Main->Height = reg->ReadInteger("SplitSizeY");
	            Left = reg->ReadInteger("WindowPosX");
	            Top = reg->ReadInteger("WindowPosY");
				Width = reg->ReadInteger("WindowSizeX");
	            Height = reg->ReadInteger("WindowSizeY");
                Position = poDesigned;
			}
            __except (1)
            {
            	// We don't care if this data isn't in the registry..
            }
		    reg->CloseKey();
	    }
    }
    __finally
    {
    	delete reg;
    }

    // Read a file list if they specify one on the command line
    if (ParamCount() > 0)
		ReadFileList(ParamStr(1));

	Print("CRUD v.1.0 Ready and Waiting");
}
//---------------------------------------------------------------------------

