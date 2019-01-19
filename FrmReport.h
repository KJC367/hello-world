//---------------------------------------------------------------------------

#ifndef FrmReportH
#define FrmReportH
//---------------------------------------------------------------------------
#include <Buttons.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
#include <StdCtrls.hpp>

//#include "FrmQuery.h"
#include "MovieMain.h"
#include "FrmBase.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class TReportForm : public TBaseForm
{
__published:	// IDE-managed Components
private:	// User declarations
   struct ExtColCfg : public TReportForm::ColCfg
   {
      enum CatFld {catBASE=Nexus::fldMSK,
                   catPRI, catSEC=2*catPRI, catIDX=3*catPRI, catREL=0,
                   catMSK=FindMask(catIDX) ^ catBASE};

      uint           nLLmt;
      uint           nULmt;
      uint           nStep;

      static uint    nCntIDX;
      static ColCfg  cfgRef1;
      static ColCfg  cfgRef2;

                     ExtColCfg(void);
      static   ColCfg *Build(uint idx);
      virtual  cchar *Parse(cchar *psz);
      virtual  char  *Print(char *psz, ColCfg &cfgRef, cchar *pszIns = NULL);
   }; // ExtColCfg

   int            iLastType;
   ColCfg         colCfgAlt;
   ExtColCfg      *pPriECC;
   ExtColCfg      *pSecECC;

            void  BuildReport(int iType);
   virtual  void  *EditColCfg(ColCfg *pCfg, EditOption eo, void *pvData);
   virtual  void  FieldList(TStringList *pStrs);
   virtual  void  GridMouse(TStringGrid *pSG, MouseBtn &mBtn);
            void  UpdForm(void);
public:		// User declarations
        __fastcall TReportForm(TComponent* Owner);
}; // TReportForm

//---------------------------------------------------------------------------
extern PACKAGE TReportForm *ReportForm;
//---------------------------------------------------------------------------
#endif
