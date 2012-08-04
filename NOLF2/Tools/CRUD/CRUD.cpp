//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("CRUD.res");
USEFORM("FMain.cpp", Form_CRUD);
USEUNIT("crud_proc.cpp");
USEUNIT("crud_util.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->CreateForm(__classid(TForm_CRUD), &Form_CRUD);
		Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
