//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MovieMain.h"
#include "FrmQuery.h"
#include "FrmTools.h"
#include "FrmType.h"
#include "FrmMovie.h"
#include "FrmPerson.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "PERFGRAP"
#pragma link "FrmBase"
#pragma resource "*.dfm"
TQueryForm *QueryForm;

#define TGlyphBtn TSpeedButton

//---------------------------------------------------------------------------

__fastcall TQueryForm::TQueryForm(TComponent* Owner)
        : TBaseForm(Owner, "Query"), ctlGrpGenre(this), ctlGrpRole(this)
{
   ppHeadItem = &(NamedItem*)Person::pHead;
   bUpdOnResize = false;

   AlignFrom(NULL);
   btnOK->Visible = false;
   btnCancel->Visible = false;
   btnDelete->Visible = false;
   btnNext->Visible = false;
   btnPrev->Visible = false;
   btnRename->Visible = false;
   cbEdit->Visible = false;
   cbMain->Visible = false;
   lblAction->Visible = false;
   lblAlias->Visible = false;
   lblMain->Visible = false;
   memAlias->Visible = false;

   grid->Visible = true;

   ctlGrpGenre.AddState(0, gbtnDontCare->Glyph, 2);
   ctlGrpGenre.AddState(1, gbtnInclude->Glyph, 0);
   ctlGrpGenre.AddState(2, gbtnExclude->Glyph, 1);

   gbtnDontCare->Tag = 0;
   gbtnInclude->Tag = 0x100;
   gbtnExclude->Tag = 0x200;

   pRngMovie = NULL;
   pRngNexus = NULL;
   ppRngAdd = &pRngMovie;
}  // TQueryForm::TQueryForm

//---------------------------------------------------------------------------

void __fastcall TQueryForm::AcceptOK(TObject *Sender)
{
   unused(Sender);
   if (bChangeName || (pCurrItem == NULL))
   {
      bChangeName = false;
      pCurrItem = new Person(cbMain->Text.c_str());
   }
}  // TQueryForm::AcceptOK

//---------------------------------------------------------------------------

TComboBox *TQueryForm::BldComboBox(TStrings *pStrs, int idxDef)
{
TComboBox *pCB = new TComboBox(this);
   pCB->Parent = this;
   pCB->Width = ca.GetWidest(pStrs, Canvas) + pCB->Height + 8;    // allow for arrow btn
   pCB->Items->Clear();
   pCB->Items->AddStrings(pStrs);
   pCB->ItemIndex = (idxDef < 0) ? pStrs->Count - 1 : idxDef;

   return (pCB);
}  // TQueryForm::BldComboBox

//---------------------------------------------------------------------------

TQueryForm::RangeCtl *TQueryForm::BldRange(TStrings *pStrs, int idxRnk,
                cchar *pszName, int idxLwr, int idxUpr)
{
TLabel *pLbl;
RangeCtl *pRng = new RangeCtl;
   pRng->pNext = NULL;
   *ppRngAdd = pRng;
   ppRngAdd = &pRng->pNext;

   pRng->idx = idxRnk;

   pRng->pLwrCB = BldComboBox(pStrs, idxLwr);
   ca.Align(pRng->pLwrCB, ca.mBOB | ca.mCIB);

   pLbl = new TLabel(this);
   pLbl->Parent = this;
   pLbl->Caption = "...";
   ca.Align(pLbl, ca.mTOP | ca.mROF);

   pRng->pUprCB = BldComboBox(pStrs, idxUpr);
   ca.Align(pRng->pUprCB, ca.mTOP | ca.mROF);

   pLbl = new TLabel(this);
   pLbl->Parent = this;
   pLbl->Caption = pszName;
   ca.Align(pLbl, pRng->pLwrCB, ca.mTOP | ca.mLOF | ca.mKEEP);

   return (pRng);
}  // TQueryForm::BldRange

//---------------------------------------------------------------------------

void __fastcall TQueryForm::btnRunClick(TObject *Sender)
{
   unused(Sender);
   pCurrItem = Movie::pHead;
   UpdateForm();
}  // TQueryForm::btnRunClick

//---------------------------------------------------------------------------

void TQueryForm::BuildForm(void)
{
TStringList *pStrs = new TStringList;
char sz[64];

   if (ctlGrpGenre.pGrpBox == NULL)
   {
      ca.SetRelative(lblScores);
      ca.Align(lblScores, ca.mNONE);

      ppRngAdd = &pRngMovie;
      pStrs->Clear();
      pStrs->Duplicates = dupIgnore;
      pStrs->Sorted = true;
      for (Movie *pMovie = Movie::pHead; pMovie; pMovie = (Movie*)pMovie->pNext)
      {
         sprintf(sz, "%4u", pMovie->nYear);
         pStrs->AddObject(sz, (TObject*)pMovie->nYear);
      }
      pStrs->Sorted = false;
      BldRange(pStrs, -2, "Year: ", 0, -1);

      pStrs->Clear();
      pStrs->AddObject("None", (TObject*)0x00);
      for (NamedItem *pNI = Genre::pHead; pNI; pNI = pNI->pNext)
      {
         pStrs->AddObject(pNI->Name(), (TObject*)((Genre*)pNI)->nID);
      }
      ctlGrpGenre.iCols = 3;
      ctlGrpGenre.type = ctlGrpGenre.tSTATE | ctlGrpGenre.tACROSS;
      ctlGrpGenre.Build(pStrs, "Genre", -1);
      ca.Align(ctlGrpGenre.pGrpBox, ca.mBOB | ca.mLIW);

      BldRange(Movie::pStrRate, -1, "Movie Rating: ", 0, -1);
      for (uint iRnk = 0; iRnk < Rank::nCnt; ++iRnk)
      {
      Rank &rnk = Rank::pa[iRnk];
         pStrs->Clear();
         for (uint n = 1; n <= rnk.nDepth; ++n)
         {
            sprintf(sz, "%u", n);
            pStrs->AddObject(sz, (TObject*)n);
         }
         pStrs->AddObject("N/R", (TObject*)Rank::NR);
         BldRange(pStrs, iRnk, rnk.szName, 0, -1);
      }


      ca.ResetBoundary();
      pStrs->Clear();
      for (NamedItem *pNI = Role::pHead; pNI; pNI = pNI->pNext)
      {
         pStrs->AddObject(pNI->Name(), (TObject*)((Role*)pNI)->nID);
      }
      ctlGrpRole.iCols = 2;
      ctlGrpRole.type = ctlGrpRole.tCHECK;
      ctlGrpRole.Build(pStrs, "Contributors", 0);
      ca.Align(ctlGrpRole.pGrpBox, ctlGrpGenre.pGrpBox, ca.mTOP | ca.mROF);
      ca.Align(chkFirstOnly, ctlGrpRole.pGrpBox, ca.mABV | ca.mLAL);

      ppRngAdd = &pRngNexus;
      pStrs->Clear();
      for (int i = 1; i < 8; ++i)
      {
         sprintf(sz, "%u", i);
         pStrs->AddObject(sz, (TObject*)i);
      }
      BldRange(pStrs, -1, "Role Rating: ", 0, -1);
      for (uint nRnk = 0; nRnk < Rank::nCnt; ++nRnk)
      {
      Rank &rnk = Rank::pa[nRnk];
         pStrs->Clear();
         for (uint n = 1; n <= rnk.nDepth; ++n)
         {
            sprintf(sz, "%u", n);
            pStrs->AddObject(sz, (TObject*)n);
         }
         pStrs->AddObject("N/R", (TObject*)Rank::NR);
         BldRange(pStrs, nRnk, rnk.szName, 0, -1);
      }

      ca.Align(grid, ctlGrpRole.pGrpBox, ca.mTOP | ca.mROF);
      grid->Visible = true;
      grid->Anchors << akTop << akRight << akBottom;

      grid->Width = ClientWidth - grid->Left - 14;
      grid->Height = ClientHeight - grid->Top - 4;

      ca.Align(cbGridFmt, grid, ca.mABV | ca.mLAL);
   }
   pCurrItem = Movie::pHead;
   ca.Align(btnRun, ctlGrpRole.pGrpBox, ca.mABV | ca.mRAR);
   delete pStrs;
}  // TQueryForm::BuildForm

//---------------------------------------------------------------------------

void TQueryForm::FieldList(TStringList *pStrs)
{
   Nexus::FieldList(pStrs, Nexus::fsetFILM | Nexus::fsetPERSON |
                           Nexus::fsetROLE | Nexus::fsetGENRE);
}  // TQueryForm::FieldList

//---------------------------------------------------------------------------

int TQueryForm::RangeCtl::GetLimit(bool bUpper)
{
TComboBox *pCB = bUpper ? pUprCB : pLwrCB;
   return ((int)pCB->Items->Objects[pCB->ItemIndex]);
}  // TQueryForm::RangeCtl::GetLimit

//---------------------------------------------------------------------------

void TQueryForm::UpdForm(void)
{
char sz[128];
DWord dwGenresInc = ctlGrpGenre.GetMask(0x100);
DWord dwGenresExc = ctlGrpGenre.GetMask(0x200);
DWord dwRoles = ctlGrpRole.GetMask();
   pCurrItem = Movie::pHead;

   grid->RowCount = 2;
uint idxRow = 1;
ulong ulFound = 0;
   for (Movie *pMovie = Movie::pHead; pMovie; pMovie = (Movie*)pMovie->pNext)
   {
   RangeCtl *pRng;
   bool bInc = ((pMovie->dwGenre & dwGenresInc) != 0);
   bool bExc = ((pMovie->dwGenre & dwGenresExc) != 0);
      if ((bInc && !bExc) || (pMovie->dwGenre == 0))
      {
      bool bMovieOK = true;
         for (pRng = pRngMovie; bMovieOK && pRng; pRng = pRng->pNext)
         {
         int iVal;
            switch (pRng->idx)
            {
            case -2: iVal = pMovie->nYear;                 break;
            case -1: iVal = pMovie->rrs.idxRat;            break;
            default: iVal = pMovie->rrs.panRk[pRng->idx];  break;
            }
         int iLLmt = pRng->GetLimit(false);
         int iULmt = pRng->GetLimit(true);
            bMovieOK &= (iLLmt <= iVal) && (iVal <= iULmt);
         }
         if (!bMovieOK)
         {
            continue;
         }
         for (Nexus *pNexus = pMovie->pHeadNexus; pNexus; pNexus = pNexus->pNext)
         {
         DWord dwMsk = 1L << pNexus->pRole->nID;
         bool bOK = bMovieOK & ((dwRoles & dwMsk) != 0);

            for (pRng = pRngNexus; bOK && pRng; pRng = pRng->pNext)
            {
            int iVal;
               switch (pRng->idx)
               {
               case -1:  iVal = pNexus->rrs.idxRat;            break;
               default:  iVal = pNexus->rrs.panRk[pRng->idx];  break;
               }
            int iLLmt = pRng->GetLimit(false);
            int iULmt = pRng->GetLimit(true);
               bOK &= (iLLmt <= iVal) && (iVal <= iULmt);
            }
            if (bOK)
            {
               for (int iCol = 0; iCol < grid->ColCount; ++iCol)
               {
               ColCfg *pCfg = GetColCfg(iCol);
                  grid->Cells[iCol][idxRow] = pNexus->FieldGet(pCfg->dwFlds, sz);
               }
               idxRow = AssignRowObj(pNexus, idxRow);
               ++ulFound;
               if (chkFirstOnly->Checked)
               {
                  break;
               }
            }
         }
      }
   }
   sprintf(sz, "Query: %lu records", ulFound);
   lblScores->Caption = sz;
}  // TQueryForm::UpdForm

//---------------------------------------------------------------------------


