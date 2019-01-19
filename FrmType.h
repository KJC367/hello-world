//---------------------------------------------------------------------------

#ifndef FrmTypeH
#define FrmTypeH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#include "Common.h"
#include "FrmTools.h"

//---------------------------------------------------------------------------

class TTypeForm : public TForm
{
__published:	// IDE-managed Components
        TLabel *lblDesc;
        TButton *btnCancel;
        TButton *btnOK;
public:		// User declarations
   struct QueryOptions
   {
      enum Type {tNONE=0, tCHK, tRADBTN, tACROSS=0x20,
                 tCHK_A=tCHK|tACROSS,     tCHK_D=tCHK,
                 tRBTN_A=tRADBTN|tACROSS, tRBTN_D=tRADBTN};     // aby[2]

      ControlAlignment::Method m;
      int            idAlign;    // 0=last, N=link
      LongWordByte   lwbPrm;
      int            iWidth;
   };
protected:	// User declarations
   struct QueryLink
   {
      QueryLink     *pNext;
      int           id;
      TControl      *pCtl;
      TLabel        *pLbl;
   }; // QueryLink
   QueryLink      *pHeadQL;
   int            idMaster;
   TPoint         ptInsert;
   ControlAlignment ca;
   ControlGroup   ctlGrp;

            int   AddLink(TControl *pCtl, TLabel *pLbl, QueryOptions &qo);
   inline   int   GetWidest(TStrings *pStrs)
                     {  return (ca.GetWidest(pStrs, Canvas));  }

public:		// User declarations
        __fastcall TTypeForm(TComponent* Owner);
           int    AddButton(cchar *pszCaption, QueryOptions &qo,
                     TModalResult mr=mrOk);
           int    AddComboBox(TStrings *pStrs, QueryOptions &qo, int iDefIdx=0,
                     bool bObjAsInt=false);
           int    AddEdit(cchar *pszLabel, QueryOptions &qo, cchar *pszDef="");
                        // b0=Rows, b1=Cols, b2=type b3=flags
           int    AddGroup(TStrings *pStrs, cchar *pszGroup, QueryOptions &qo,
                        int iDefChks=0);
           int    AddRange(TStrings *pStrs, cchar *pszRange, QueryOptions &qo,
                                int iDefLLmt=0, int iDefULmt=-1);

           void   Align(int idCtl, int idRef, ControlAlignment::Method m);
           bool   GetValue(int idCtl, char *pszStr, int &iRtn);
           int    Query(void);          // 0=Cancel, 1=OK
           int    Query(TStrings *pStrs, cchar *pszCaption, cchar *pszDesc,
                        int iDefIndex=0, bool bChkBoxes=false);
           void   Reset(cchar *pszCaption, cchar *pszDesc,
                        int iWd=300, int iHt=100);
};
//---------------------------------------------------------------------------
extern PACKAGE TTypeForm *TypeForm;
//---------------------------------------------------------------------------
#endif
