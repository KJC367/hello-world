//---------------------------------------------------------------------------

#ifndef FrmGraphH
#define FrmGraphH
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
class TGraphForm : public TBaseForm
{
__published:	// IDE-managed Components
        TImage *graph;
        TColorDialog *ColorDialog1;
        TCheckBox *chkStats;
        TComboBox *cbGraph;
        TCheckBox *chkLog;
        void __fastcall FormResize(TObject *Sender);
        void __fastcall graphMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall cbGraphSelect(TObject *Sender);
        void __fastcall chkStatsClick(TObject *Sender);
private:	// User declarations
   struct GraphElement
   {
      char           szCat[64];
      bool           bAll;
      ulong          nCnt;
      uint           nClrIdx;

      double         rTtl;
      double         rTtl2;
      double         rMax;
      double         rMin;
               void  Data(double r);
   }; // GraphElement
   struct GraphInfo
   {
      GraphElement   *paGE;
      uint           nCnt;

      enum ColorIndex {ciODD, ciEVEN, ciZERO, ciAXIS, ciTEXT, ciSTAT,
                       ciODD2, ciEVEN2, 
                       ciMAX};
      TColor         aclr[ciMAX];

      bool           bVert;
      bool           bPlotTtl;
      bool           bPlotStats;
                     GraphInfo(void);
                     ~GraphInfo();
               void  Setup(uint nCnt, bool bAll = true, uint nGrp = 0);
   }; // GraphInfo
   GraphInfo      gi;
   int            iCurr;

   enum RefType {rfROLE, rfGENRE, rfDEC_GENRE, rfGENRE_DEC, rfDECADE,
                 rfTYPE_MSK=FindMask(rfDECADE),
                 rfVERT, rfRATE=2*rfVERT};
   RefType        rfMaster;

        void __fastcall AcceptOK(TObject *Sender);
            void  AddSet(cchar *psz, RefType rf);
            void  DrawBar(void);
   virtual  void  FieldList(TStringList *pStrs);
            void  UpdForm(void);
public:		// User declarations
        __fastcall TGraphForm(TComponent* Owner);
        bool  Select(Person *&pPerson);
};
//---------------------------------------------------------------------------
extern PACKAGE TGraphForm *GraphForm;
//---------------------------------------------------------------------------
#endif
