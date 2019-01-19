//---------------------------------------------------------------------------

#ifndef FrmQueryH
#define FrmQueryH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include "FrmBase.h"
#include <Buttons.hpp>


class Movie;    // fwd ref
//---------------------------------------------------------------------------
class TQueryForm : public TBaseForm
{
__published:	// IDE-managed Components
        TLabel *lblScores;
        TColorDialog *ColorDialog1;
        TButton *btnRun;
        TCheckBox *chkFirstOnly;
        TBitBtn *gbtnExclude;
        TBitBtn *gbtnDontCare;
        TBitBtn *gbtnInclude;
        void __fastcall btnRunClick(TObject *Sender);
private:	// User declarations

   ControlAlignment ca;
   struct RangeCtl
   {
      RangeCtl       *pNext;
      int            idx;
      TStringList    *pStrs;
      TComboBox      *pLwrCB;
      TComboBox      *pUprCB;
               int   GetLimit(bool bUpper);
   };
   RangeCtl       **ppRngAdd;
   RangeCtl       *pRngMovie;
   RangeCtl       *pRngNexus;
   ControlGroup   ctlGrpGenre;
   ControlGroup   ctlGrpRole;

        void __fastcall AcceptOK(TObject *Sender);
        TComboBox *BldComboBox(TStrings *pStrs, int idxDef);
        RangeCtl  *BldRange(TStrings *pStrs, int idxRnk, cchar *pszName,
                        int idxLwr=0, int idxUpr=-1);

   virtual  void  FieldList(TStringList *pStrs);

            void  UpdForm(void);
public:		// User declarations
        __fastcall TQueryForm(TComponent* Owner);
        void  BuildForm(void);
}; // TQueryForm

//---------------------------------------------------------------------------
extern PACKAGE TQueryForm *QueryForm;
//---------------------------------------------------------------------------
#endif
