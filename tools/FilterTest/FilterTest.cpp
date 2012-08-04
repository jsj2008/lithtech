//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("FilterTest.res");
USEFORM("FMain.cpp", Form_Main);
USEDEF("Mss32bc.def");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->CreateForm(__classid(TForm_Main), &Form_Main);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
