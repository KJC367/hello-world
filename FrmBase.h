//---------------------------------------------------------------------------

#ifndef FrmBaseH
#define FrmBaseH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <stdint.h>

#include <Forms.hpp>
#include <Grids.hpp>

#include "Common.h"
///#include "MovieMain.h"
#include <Dialogs.hpp>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class NamedItem
{
public: //protected:
   enum ElementType {etNAME, etSTR, etUINT};
   enum SortMethod {smNONE=0, smA2Z=1, smZ2A=-1};
   NamedItem      *pNext;
   TStringList    *pNames;   //[0]=main, [1...N]=aliases

                  NamedItem(void);
   virtual        ~NamedItem();
   static   NamedItem *Find(cchar *psz, NamedItem *pHead);
   static   int   FindPartial(cchar *psz, NamedItem *pHead,
                        TStrings *pStrs=NULL, bool *pbExact = NULL);
   inline   cchar *Name(uint idx = 0)
                     {  return ((idx >= (uint)pNames->Count) ? "" :
                                pNames->Strings[idx].c_str());  }
   static   bool  Sort(NamedItem *&pHead, uint nOffset, ElementType et,
                       SortMethod sm);
   virtual  void  Dump(char *&psz);
}; // NamedItem


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class TBaseForm : public TForm
{
__published:	// IDE-managed Components
        TLabel *lblMain;
        TStringGrid *grid;
        TButton *btnOK;
        TButton *btnCancel;
        TButton *btnNext;
        TButton *btnPrev;
        TComboBox *cbMain;
        TButton *btnDelete;
        TMemo *memAlias;
        TLabel *lblAlias;
        TButton *btnRename;
        TLabel *lblAction;
        TFontDialog *FontDialogQF;
        TColorDialog *ColorDialogQF;
        TComboBox *cbEdit;
        TComboBox *cbGridFmt;
        void __fastcall btnPrevClick(TObject *Sender);
        void __fastcall btnNextClick(TObject *Sender);

        void __fastcall FormShow(TObject *Sender);
        void __fastcall FormMouseWheel(TObject *Sender, TShiftState Shift,
          int WheelDelta, TPoint &MousePos, bool &Handled);

        void __fastcall cbMainChange(TObject *Sender);
        void __fastcall cbMainExit(TObject *Sender);
        void __fastcall cbMainKeyPress(TObject *Sender, char &Key);

        void __fastcall GridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
        void __fastcall GridMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
        void __fastcall GridSelectCell(TObject *Sender, int ACol,
          int ARow, bool &CanSelect);

        void __fastcall memAliasExit(TObject *Sender);
        void __fastcall SortLabel(TObject *Sender);
        void __fastcall cbAttribExit(TObject *Sender);
            void __fastcall DetectChange(TObject *Sender);
        void __fastcall cbEditSelect(TObject *Sender);
        void __fastcall FormResize(TObject *Sender);
        void __fastcall cbGridFmtSelect(TObject *Sender);
        void __fastcall FormDestroy(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
   struct SortMatrix
   {
      SortMatrix     *pNext;
      TControl       *pCtl;
      uint           nOff;
      NamedItem::ElementType et;
   }              *pHeadSort;

public:
   struct ColCfg
   {
      enum ObjRegionIndex {oriHDR, oriBDY1, oriBDY2, oriBDY3, oriMAX};
      enum ObjStyleType {ostBG, ostLN, ostMAX};

      ColCfg         *pNext;
      TFont          *apFont[oriMAX];

        // aaClr needs to be top of copy block
      TColor         aaClr[oriMAX][ostMAX];
      int            aaiBorder[oriMAX][4];    // [os][0=L,1=B,2=R,3=T]

      enum SortMethod {smNONE, smA2Z, smZ2A, smMAX};
      SortMethod     sm;

      uint           anGrp[3];          // [0]=Index, [1]=assigned, [2]=type

      uint           nWidth;
      dword          dwFlds;
      char           szHeader[64];

                     ColCfg(void);
      virtual        ~ColCfg();
      static   ColCfg *Build(uint idx);
               ColCfg& operator=(const ColCfg &cfg);
      virtual  cchar *Parse(cchar *psz);        // return psz just past field
      virtual  char  *Print(char *pszOut, ColCfg &ccRef, cchar *pszIns = NULL);
   }; // ColCfg

   struct ColSet
   {
      char           szTitle[64];
      uint           nCols;
      ColCfg         *pHeadCfg;
      DWord          dwChanged;
      
                     ColSet(TStringList *pStrs, cchar *pszTitle, ColCfg &cfgDef,
                                ColCfg *(*pfnBldCfg)(uint idx));
                     ~ColSet();
   }; // ColSet
   ColSet         *pCurrSet;
   ColCfg         *(*pfnBuildCfg)(uint);
            void  SaveColSet(void);

   #pragma pack(1)
   union RowParam
   {
      TObject        *pObj;
      struct
      {
         int16_t        nID;
         int8_t         nGrp;
      };

   }; // RowParam
   #pragma pack()
   inline    void  Get(RowParam &rp, int idxRow)
                      {  rp.pObj = grid->Objects[0][idxRow];  }
   inline    void  Set(RowParam &rp, int idxRow)
                      {  grid->Objects[0][idxRow] = rp.pObj;  }

   enum EditOption {eoCANCEL, eoDEL, eoADDB, eoADDA, eoEDIT, eoSAVE, eoSTYLE};

   static   bool  ColorAppend(char *&psz, cchar *pszCode,
                        TColor clr, TColor clrFrom);
   static   bool  ColorDecode(cchar *pszColor, TColor &clr);
   virtual  dword FieldDecode(cchar *&psz);
   virtual  void  FieldEncode(dword dwFlds, char *&psz);
   virtual  void  FieldList(TStringList *pStrs);
   virtual  int   FindField(TStringList *pStrs, DWord dwFld);
   static   bool  FontAppend(char *&psz, cchar *pszCode,
                                TFont *pFnt, TFont *pFntFrom);
   static   bool  FontDecode(cchar *pszFont, TFont *pFont);
   static inline TColor clrRGB(int iR, int iG, int iB)
                        { return ((TColor)RGB(iR,iG,iB));  }
private:
   bool           bOwnedColSet;
protected:
   bool           bUpdOnResize;

   struct MouseBtn
   {
      enum State {sNONE=0, sSHF=1L<<ssShift, sCTL=1L<<ssCtrl, sALT=1L<<ssAlt,
                  sDBL=1L<<ssDouble,
                  sLBTN=1L<<ssLeft, sMBTN=1L<<ssMiddle, sRBTN=1L<<ssRight,

                  sMSK=sALT|sCTL|sSHF|sLBTN|sMBTN|sRBTN};
      State          s;
      int            x;
      int            y;
      int            iCol;
      int            iRow;

      inline   bool  Is(State sSet) {  return ((s & sMSK) == sSet);  }
   };

   ColCfg         colCfg;                // Global colors and fonts
   char           szGridName[64];

   ColCfg         *pCurrColCfg;
   int            idxCurrCol;
   int            idxCurrRow;

   NamedItem      **ppHeadItem;
   NamedItem      *pCurrItem;
   bool           bChangeName;

   uint           anCnt[10];

   virtual  int   Accept(int iFlags = MB_YESNO);
            int   AcceptBase(void);
            void  AddSortMatrix(TControl *pCtl, uint nOffset,
                        NamedItem::ElementType et);

            void  AlignFrom(TControl *pCtl);
            void  AssignColCfg(ColCfg *pColCfg, uint idxCol);
            uint  AssignRowObj(void *pvObj, uint idxRow, bool bInc = true);
            void  CntReport(int idx = -1);
            void __fastcall CurrItemDel(TObject *Sender);
            void __fastcall CurrItemRen(TObject *Sender);

   virtual  void  *EditColCfg(ColCfg *pCfg, EditOption eo, void *pvData);

            void  GetAttribs(void);
   inline   ColCfg *GetColCfg(uint idxCol)
                        {  return ((ColCfg*)grid->Objects[idxCol][0]);  }

            void  GridSort(TStringGrid *pSG, int iColSel);
   virtual  void  GridMouse(TStringGrid *pSG, MouseBtn &mBtn);
            void  UpdateForm(void); // calls UpdForm
   virtual  void  UpdForm(void);
            void  UpdateSet(DWord dwChange);
public:		// User declarations
        __fastcall TBaseForm(TComponent* Owner, cchar *pszName,
                        ColCfg *(*pfnBuildCfg)(uint idx) = ColCfg::Build);
        void  Show(NamedItem *pItem);
        void  SetupGrid(ColSet *pSet);
}; // TBaseForm

//---------------------------------------------------------------------------
extern PACKAGE TBaseForm *BaseForm;
//---------------------------------------------------------------------------
#endif
