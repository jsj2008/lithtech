//---------------------------------------------------------------------------

#include <vcl.h>

#pragma hdrstop

#include "ltdirectsound.h"

#include "FMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm_Main *Form_Main;
//---------------------------------------------------------------------------
__fastcall TForm_Main::TForm_Main(TComponent* Owner)
	: TForm(Owner),
	m_CurFile(""),
	m_pFileBuffer(NULL),
	m_iFileSize(0),
	m_hSample(0),
	m_iNumProperties(0)
{
}

void TForm_Main::ChangeStatus(const AnsiString &sStatus)
{
	c_Panel_Status->Caption = sStatus;
}

bool TForm_Main::OpenFile()
{
	//  STOP!!!
	c_Button_Stop->Click();

	// Let go...
	if (m_hSample)
	{
		AIL_release_sample_handle(m_hSample);
		m_hSample = NULL;
	}

	// Get rid of the buffer if it's already been loaded
	if (m_pFileBuffer)
	{
		AIL_mem_free_lock(m_pFileBuffer);
		m_pFileBuffer = NULL;
	}

	// Remember the file name
	m_CurFile = c_Edit_FileName->Text;

	// Er, don't load if it doesn't exist...
	if (!FileExists(m_CurFile))
	{
		m_CurFile = "";
		return false;
	}

	m_hSample = AIL_allocate_sample_handle(m_hDigDriver);
	if (!m_hSample)
	{
		ChangeStatus("Error allocating sample handle");
		m_CurFile = "";
		return false;
	}

	char *pFileName = m_CurFile.c_str();

	// Bind to the file
	m_pFileBuffer = AIL_file_read(pFileName, NULL);
	if (!m_pFileBuffer)
	{
		ChangeStatus("Error opening " + m_CurFile + " (" + IntToStr(AIL_file_error()) + ")");
		m_CurFile = "";
		return false;
	}

	// Get the file size
	m_iFileSize = AIL_file_size(pFileName);

	AIL_init_sample(m_hSample);

	AIL_set_named_sample_file(m_hSample, pFileName, m_pFileBuffer, m_iFileSize, 0);

	AIL_set_sample_loop_count(m_hSample, 0);

	AIL_set_sample_volume(m_hSample, 127);

	AIL_set_sample_processor(m_hSample, DP_FILTER, m_hCurFilter);

	ChangeStatus("Loaded " + m_CurFile);

	return true;
}

void TForm_Main::FillFilterList()
{
	// Clear out the list
	c_ComboBox_Filter->Items->Clear();

	// Add <NONE>
	c_ComboBox_Filter->Items->AddObject("<NONE>", NULL);

	// Add the filters to the list
	HPROENUM hFilterEnum(HPROENUM_FIRST);
	HPROVIDER hFilter;
	char *pFilterName;
	while (AIL_enumerate_filters(&hFilterEnum, &hFilter, &pFilterName))
	{
		c_ComboBox_Filter->Items->AddObject(AnsiString(pFilterName), (TObject*)hFilter);
	}

	c_ComboBox_Filter->ItemIndex = 0;
}

void TForm_Main::FillPropertyList()
{
	// Clear the property list
	c_ComboBox_Property->Items->Clear();
	m_iNumProperties = 0;

	// Enumerate the available properties
	RIB_INTERFACE_ENTRY ribAttrib;
	HINTENUM enumerator(HINTENUM_FIRST);
	while (AIL_enumerate_filter_sample_attributes(m_hCurFilter, &enumerator, &ribAttrib))
	{
		float fValue;
		AIL_filter_sample_attribute(m_hSample, ribAttrib.entry_name, &fValue);
		if (m_iNumProperties < NUM_PROPERTIES)
		{
			m_fProperties[m_iNumProperties] = fValue;
			++m_iNumProperties;
		}
		c_ComboBox_Property->Items->AddObject(AnsiString(ribAttrib.entry_name), reinterpret_cast<TObject*>(*(int*)&fValue));
	}

	// Enable/disable the property related stuff..
	bool bEnabled = c_ComboBox_Property->Items->Count > 0;
	c_ComboBox_Property->Enabled = bEnabled;
	c_Edit_Value->Enabled = bEnabled;
	c_Button_AdjSmall_Up->Enabled = bEnabled;
	c_Button_AdjSmall_Down->Enabled = bEnabled;
	c_Button_AdjMed_Up->Enabled = bEnabled;
	c_Button_AdjMed_Down->Enabled = bEnabled;
	c_Button_AdjLarge_Up->Enabled = bEnabled;
	c_Button_AdjLarge_Down->Enabled = bEnabled;

	// Select the first item
	if (bEnabled)
	{
		c_ComboBox_Property->ItemIndex = 0;
		// Act like a change happened, because it just did...
		c_ComboBox_PropertyChange(c_ComboBox_Property);
	}
}

float TForm_Main::ReadProperty()
{
	float fValue;
	AIL_filter_sample_attribute(m_hSample, c_ComboBox_Property->Text.c_str(), &fValue);
	return fValue;
}

void TForm_Main::AdjustProperty(float fAmount)
{
	if (!m_hSample)
		return;

	// Get the current value
	float fValue = atof(c_Edit_Value->Text.c_str());
	// Adjust
	fValue += fAmount;
	// Tell Miles
	AIL_set_filter_sample_preference(m_hSample, c_ComboBox_Property->Text.c_str(), &fValue);
	// Get it back out in case we hit a max/min
	fValue = ReadProperty();
	// Put it back in the edit
	AnsiString holdString;
	holdString.printf("%0.4f", fValue);
	c_Edit_Value->Text = holdString;

	// Update the property cache
	int iCurProperty = 0;
	RIB_INTERFACE_ENTRY ribAttrib;
	HINTENUM enumerator(HINTENUM_FIRST);
	while ((iCurProperty < m_iNumProperties) &&
		AIL_enumerate_filter_sample_attributes(m_hCurFilter, &enumerator, &ribAttrib))
	{
		AIL_filter_sample_attribute(m_hSample, ribAttrib.entry_name, &m_fProperties[iCurProperty]);
		++iCurProperty;
	}
}

bool TForm_Main::InitMiles()
{
	AIL_startup();

	PCMWAVEFORMAT format;

	format.wf.wFormatTag      = WAVE_FORMAT_PCM;
	format.wf.nChannels       = 2;
	format.wf.nSamplesPerSec  = 44100;
	format.wf.nBlockAlign     = 4;
	format.wf.nAvgBytesPerSec = 44100 * format.wf.nBlockAlign;
	format.wBitsPerSample     = 16;

	if (AIL_waveOutOpen(&m_hDigDriver, NULL, WAVE_MAPPER, (LPWAVEFORMAT) &format) != 0)
		return false;

	return true;
}

//---------------------------------------------------------------------------

void __fastcall TForm_Main::c_ComboBox_FilterChange(TObject *Sender)
{
	m_hCurFilter = reinterpret_cast<HPROVIDER>(c_ComboBox_Filter->Items->Objects[c_ComboBox_Filter->ItemIndex]);

	// re-bind the filter if we've got a sample
	if (m_hSample)
		AIL_set_sample_processor(m_hSample, DP_FILTER, m_hCurFilter);

	FillPropertyList();
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Edit_ValueKeyPress(TObject *Sender,
	  char &Key)
{
	if (Key == 13)
	{
		AdjustProperty(0.0f);
	}
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::FormCreate(TObject *Sender)
{
	if (!InitMiles())
	{
		ShowMessage("Error initializing Miles");
		Close();
		return;
	}

	FillFilterList();

	// Open a file off the command line
	if (ParamCount() > 0)
	{
		c_Edit_FileName->Text = ParamStr(1);
		OpenFile();
	}
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::FormDestroy(TObject *Sender)
{
	// Stop..
	c_Button_Stop->Click();

	if (m_hDigDriver)
		AIL_waveOutClose(m_hDigDriver);

	AIL_shutdown();
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Button_OpenFileClick(TObject *Sender)
{
	c_OpenDialog->FileName = m_CurFile;

	if (!c_OpenDialog->Execute())
		return;

	c_Edit_FileName->Text = c_OpenDialog->FileName;

	OpenFile();
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Button_PlayClick(TObject *Sender)
{
	if (!m_hSample)
		return;

	AIL_set_sample_processor(m_hSample, DP_FILTER, m_hCurFilter);

	// Reset the current property, just in case it got lost..
	//AdjustProperty(0.0f);
	// Play
	c_TrackBar_PanChange(Sender);
	AIL_set_sample_ms_position(m_hSample, 0);
	AIL_start_sample(m_hSample);

	// Reset the property values based on our cache
	int iCurProperty = 0;
	RIB_INTERFACE_ENTRY ribAttrib;
	HINTENUM enumerator(HINTENUM_FIRST);
	while ((iCurProperty < m_iNumProperties) &&
		AIL_enumerate_filter_sample_attributes(m_hCurFilter, &enumerator, &ribAttrib))
	{
		AIL_set_filter_sample_preference(m_hSample, ribAttrib.entry_name, &m_fProperties[iCurProperty]);
		++iCurProperty;
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Button_StopClick(TObject *Sender)
{
	if (!m_hSample)
		return;

	// Stop
	AIL_stop_sample(m_hSample);
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Button_AdjSmall_UpClick(TObject *Sender)
{
	AdjustProperty(0.1f);
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Button_AdjMed_UpClick(TObject *Sender)
{
	AdjustProperty(1.0f);
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Button_AdjLarge_UpClick(TObject *Sender)
{
	AdjustProperty(100.0f);
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Button_AdjSmall_DownClick(TObject *Sender)
{
	AdjustProperty(-0.1f);
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Button_AdjMed_DownClick(TObject *Sender)
{
	AdjustProperty(-1.0f);
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Button_AdjLarge_DownClick(TObject *Sender)
{
	AdjustProperty(-100.0f);
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_ComboBox_PropertyChange(TObject *Sender)
{
	AnsiString holdString;
	holdString.printf("%0.4f", ReadProperty());
	c_Edit_Value->Text = holdString;
}

//---------------------------------------------------------------------------
void __fastcall TForm_Main::c_Edit_FileNameKeyPress(TObject *Sender,
	  char &Key)
{
	if (Key == 13)
	{
		OpenFile();
	}
}
//---------------------------------------------------------------------------



void __fastcall TForm_Main::c_Edit_FileNameChange(TObject *Sender)
{
	bool bSuccess = OpenFile();

	// Disable the controls...
	c_Button_Play->Enabled = bSuccess;
	c_Button_Stop->Enabled = bSuccess;
}
//---------------------------------------------------------------------------

void __fastcall TForm_Main::c_TrackBar_PanChange(TObject *Sender)
{
	AIL_set_sample_pan( m_hSample, c_TrackBar_Pan->Position );
}
//---------------------------------------------------------------------------

void __fastcall TForm_Main::c_Button_AdjExLarge_UpClick(TObject *Sender)
{
	AdjustProperty(1000.0f);
}
//---------------------------------------------------------------------------

void __fastcall TForm_Main::c_Button_AdjExLarge_DownClick(TObject *Sender)
{
	AdjustProperty(-1000.0f);
}
//---------------------------------------------------------------------------

void __fastcall TForm_Main::c_Button_AdjExSmall_UpClick(TObject *Sender)
{
	AdjustProperty(0.01f);
}
//---------------------------------------------------------------------------

void __fastcall TForm_Main::c_Button_AdjExSmall_DownClick(TObject *Sender)
{
	AdjustProperty(-0.01f);
}
//---------------------------------------------------------------------------

