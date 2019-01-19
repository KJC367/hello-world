//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("MovieMain.cpp", Form1);
USEFORM("FrmMovie.cpp", MoviesForm);
USEFORM("FrmPerson.cpp", PersonForm);
USEFORM("FrmBase.cpp", QueryForm);
USEFORM("FrmType.cpp", TypeForm);
USEFORM("FrmReport.cpp", ReportForm);
USEFORM("frmGraph.cpp", GraphForm);
USEFORM("FrmQuery.cpp", Form2);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->CreateForm(__classid(TForm1), &Form1);
                 Application->CreateForm(__classid(TMoviesForm), &MoviesForm);
                 Application->CreateForm(__classid(TPersonForm), &PersonForm);
              //   Application->CreateForm(__classid(TBaseForm), &BaseForm);
                 Application->CreateForm(__classid(TTypeForm), &TypeForm);
                 Application->CreateForm(__classid(TReportForm), &ReportForm);
                 Application->CreateForm(__classid(TGraphForm), &GraphForm);
                 Application->CreateForm(__classid(TQueryForm), &QueryForm);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        catch (...)
        {
                 try
                 {
                         throw Exception("");
                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
        return 0;
}
//---------------------------------------------------------------------------
