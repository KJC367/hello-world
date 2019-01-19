//---------------------------------------------------------------------------

#ifndef FrmPersonH
#define FrmPersonH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
//#include "FrmQuery.h"
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include "FrmBase.h"


class Movie;    // fwd ref
//---------------------------------------------------------------------------
class TPersonForm : public TBaseForm
{
__published:	// IDE-managed Components
        TImage *graph;
        TLabel *lblScores;
        TColorDialog *ColorDialog1;
        void __fastcall FormResize(TObject *Sender);
        void __fastcall graphMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
private:	// User declarations

        void __fastcall AcceptOK(TObject *Sender);
   virtual  void  FieldList(TStringList *pStrs);
            void  UpdForm(void);
public:		// User declarations
        __fastcall TPersonForm(TComponent* Owner);
        bool  Select(Person *&pPerson);
};
//---------------------------------------------------------------------------
extern PACKAGE TPersonForm *PersonForm;
//---------------------------------------------------------------------------
#endif
