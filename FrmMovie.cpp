//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MovieMain.h"
#include "FrmTools.h"
#include "FrmMovie.h"
#include "FrmPerson.h"
#include "FrmType.h"
#include <time.h>


//---------------------------------------------------------------------------
#pragma package(smart_init)
//#pragma link "FrmQuery"
#pragma link "FrmBase"
#pragma resource "*.dfm"
TMoviesForm *MoviesForm;
void FlagChangeDB(void);

//---------------------------------------------------------------------------

__fastcall TMoviesForm::TMoviesForm(TComponent* Owner)
        : TBaseForm(Owner, "Movie")
{
   ppHeadItem = &(NamedItem*)Movie::pHead;

   lblMain->Left = 4;
   lblMain->Top = 4;
ControlAlignment ca;
   ca.Align(cbMain,    lblMain,     ca.mBLW | ca.mLAL);

   lblYear->Caption = "Year";
   lblYear->OnClick = SortLabel;
   ca.Align(lblYear,   cbMain,     ca.mBLW | ca.mLAL);
   edYear->OnChange = DetectChange;
   ca.Align(edYear,    lblYear,    ca.mBLW | ca.mLAL);

   cbRating->Style = csDropDownList;
   cbRating->OnChange = DetectChange;
   ca.Align(cbRating,   edYear,     ca.mTOP | ca.mROF);
   lblRating->Caption = "Rating";
   lblRating->OnClick = SortLabel;
   ca.Align(lblRating,  cbRating,  ca.mABV | ca.mLAL | ca.mKEEP);
   cbRating->Items->AddStrings(Movie::pStrRate);

   typedef TEdit * PTEdit;
   papedRank = new PTEdit[Rank::nCnt+1];
   memset(papedRank, 0, sizeof(PTEdit)*Rank::nCnt);
   for (uint idx = 0; idx < Rank::nCnt; ++idx)
   {
   uint nWd = Rank::pa[idx].nWidth;
   TEdit *pedRank = new TEdit(MoviesForm);
      papedRank[idx] = pedRank;
      pedRank->Parent = MoviesForm;
      pedRank->OnClick = RankQuery;
      pedRank->Enabled = true;
      pedRank->Tag = idx;             // rank index
      if (nWd == 0)
      {
         nWd = Canvas->TextWidth(Rank::pa[idx].szName) +
               Canvas->TextWidth("XXXX"); + 8;
      }
      pedRank->Width = (nWd > 0) ? nWd : 75;
      ca.Align(pedRank,  NULL,  ca.mTOP | ca.mROF);

   TLabel *pLbl = new TLabel(MoviesForm);
      pLbl->Caption = Rank::pa[idx].szName;
      pLbl->Parent = MoviesForm;
      pLbl->OnClick = SortLabel;
      ca.Align(pLbl,  pedRank, ca.mABV | ca.mLAL | ca.mKEEP);

      if (pedRank->Left + pedRank->Width > ClientWidth)
      {
         ca.Align(pLbl,    papedRank[0],    ca.mBOB | ca.mLAL);
         ca.Align(pedRank, pLbl,            ca.mBLW | ca.mLAL);
      }

      AddSortMatrix(pLbl, offsetof(Movie, rrs.panRk),
                (NamedItem::ElementType)(NamedItem::etUINT + idx));
   }

   grpGenre->Height = 44;
   ca.Align(grpGenre,  edYear,    ca.mBLW | ca.mLAL);
   grpGenre->Width = cbRating->Width + cbRating->Left - grpGenre->Left;
   lblGenre->Width = grpGenre->Width - 16;

   ca.Align(lblAlias,  grpGenre,  ca.mBOB | ca.mLAL);
   AlignFrom(memAlias);
   btnDelete->Enabled = true;
   btnOK->OnClick = AcceptOK;

   AddSortMatrix(lblYear,   offsetof(Movie, nYear),       NamedItem::etUINT);
   AddSortMatrix(lblRating, offsetof(Movie, rrs.idxRat),  NamedItem::etUINT);
   AddSortMatrix(lblMain,   offsetof(Movie, pNames),      NamedItem::etNAME);
}  // TMoviesForm::TMoviesForm

//---------------------------------------------------------------------------

int TMoviesForm::Accept(int iFlags)
{
char sz[1024], *psz = sz;

int iRtn;           // didn't ask question

uint nYr = 2000;
   sscanf(edYear->Text.c_str(), "%u", &nYr);

RatingRanks rrs;
   rrs.idxRat = 1;
   rrs.DecodeAll(cbRating->Text.c_str(), Movie::pStrRate);

   for (uint idxR = 0; idxR < Rank::nCnt; ++idxR)
   {
      rrs.panRk[idxR] = Rank::Decode(papedRank[idxR]->Text.c_str());
   }

   if (bChangeName || (pCurrItem == NULL))
   {
      sprintf(sz, "Do you wish to ADD '%s' to the movie database?",
                cbMain->Text.c_str());
      iRtn = Application->MessageBoxA(sz, "Add Movie", iFlags);
      if (iRtn == IDYES)
      {
         pCurrItem = new Movie(cbMain->Text.c_str(), nYr, dwGenre);
         ((Movie*)pCurrItem)->rrs.idxRat = rrs.idxRat;
         bChangeName = false;
         UpdateForm();
      }
   }
   else
   {
   Movie *pMovie = (Movie*)pCurrItem;
   #define szB "\n  \x95 "
      psz += sprintf(psz, "Do you wish to change %s", pMovie->Name());
   char *pszBase = psz;
      if (pMovie->nYear != nYr)
      {
         psz += sprintf(psz, szB"Year(%u=>%u)", pMovie->nYear, nYr);
      }
      if (pMovie->rrs.idxRat != rrs.idxRat)
      {
         psz += sprintf(psz, szB"Rating(%s=>%s)", pMovie->GetRating(),
                Movie::GetRating(rrs));
      }
      for (uint idxR = 0; idxR < Rank::nCnt; ++idxR)
      {
         if (pMovie->rrs.panRk[idxR] != rrs.panRk[idxR])
         {
            psz += sprintf(psz, szB"%s(%s", Rank::pa[idxR].szName,
                Rank::Prt(pMovie->rrs.panRk[idxR]));
            psz += sprintf(psz, "=>%s)", Rank::Prt(rrs.panRk[idxR]));
         }
      }
      if (pMovie->dwGenre != dwGenre)
      {
         psz += sprintf(psz, szB"Genre '", pMovie->Name());
         Genre::Decode(psz, pMovie->dwGenre);
         psz += sprintf(psz, "' =>\n\t'");
         Genre::Decode(psz, dwGenre);
         psz += sprintf(psz, "'");
      }
      sprintf(psz, "'?");
      if (psz == pszBase)       // nothing changed
      {
         btnOK->Tag = 0;
         iRtn = 0;      // no question
      }
      else if ((iRtn = Application->MessageBox(sz, "Movie Changes", iFlags)) == IDYES)
      {
         pMovie->nYear     = nYr;
         pMovie->rrs.idxRat  = rrs.idxRat;

         for (uint idxR = 0; idxR < Rank::nCnt; ++idxR)
         {
            if (pMovie->rrs.panRk[idxR] != rrs.panRk[idxR])
            {
            RatingRanksAdjust rrsa;
               rrsa.Setup(pMovie->rrs, rrs, idxR);

            uint nStep = Rank::pa[idxR].nStep;
            uint nLoYr = pMovie->nYear;
               if (nStep != 1)
               {
                  nLoYr = nLoYr - (nLoYr % nStep);
               }
            uint nHiYr = nLoYr + (nStep - 1);

               for (uint nRnk = rrsa.nLLmt; nRnk <= rrsa.nULmt; ++nRnk)
               {
               Nexus *pN = Movie::FindRank(nRnk, nLoYr, nHiYr, idxR, NULL);
                  if (pN && pN->pMovie)
                  {
                     rrsa.AddAdj(pN->pMovie->rrs);
                  }
               }
               rrsa.ProcessList();
               pMovie->rrs.panRk[idxR] = rrs.panRk[idxR];
            }
         }  // RankLmt
         pMovie->dwGenre   = dwGenre;
         btnOK->Tag = 0;
         UpdateForm();
         FlagChangeDB();
      }
   }
   return (iRtn);
}  // TMoviesForm::Accept

//---------------------------------------------------------------------------

void __fastcall TMoviesForm::AcceptOK(TObject *Sender)
{
   unused(Sender);
   if (Accept(MB_YESNO) == 0)
   {
      ModalResult = mrOk;
   }
   else
   {
      ModalResult = mrNone;
   }
}  // TMoviesForm::AcceptOK

//---------------------------------------------------------------------------

void TMoviesForm::FieldList(TStringList *pStrs)
{
   Nexus::FieldList(pStrs, Nexus::fsetPERSON | Nexus::fsetROLE);
}  // TMoviesForm::FieldList

//---------------------------------------------------------------------------

void TMoviesForm::GridMouse(TStringGrid *pSG, MouseBtn &mBtn)
{
char sz[512], *psz, szCol[64], szTitle[128], *pszTitle = NULL;
Nexus *pNexus = (mBtn.iRow == 0) ? NULL : (Nexus *)pSG->Objects[1][mBtn.iRow];
   sscanf(pSG->Cells[mBtn.iCol][0].c_str(), "%63[^:]", szCol);
Nexus nexus, *pN;
TStringList *pStrs = new TStringList;
int idxRank = -1;

   if (pNexus)
   {
      nexus = *pNexus;
   }
   else
   {
      nexus.pMovie = (Movie*)pCurrItem;
      nexus.pNext = nexus.pMovie->pHeadNexus;   // prep link
      nexus.pRole = Role::pHead;
      nexus.rrs.Reset(4);
      if (nexus.pRole->bGroup)
      {
         pN = nexus.pMovie->FindNexus(NULL, nexus.pRole);
         if (pN)
         {
            nexus.rrs = pN->rrs;
         }
      }
   }

ColCfg *pCfg = GetColCfg(mBtn.iCol);
Nexus::Field fld = pCfg ? (Nexus::Field)(pCfg->dwFlds & Nexus::fldMSK) : Nexus::fldNONE;
   if (mBtn.iRow == 0)
   {
   }
   else if (fld == Nexus::fldPERS)            // Name
   {
   Person *pPerson = pNexus ? pNexus->pPerson : NULL;
      if (PersonForm->Select(pPerson))
      {
         nexus.pPerson = pPerson;
         if (pPerson == NULL)
         {
            sprintf(sz, "Do you want to remove '%s' as a '%s'",
                pNexus->pPerson->Name(), pNexus->pRole->Name());
            sprintf(szTitle, "Remove %s", szCol);
         }
         else if (pNexus)
         {
            sprintf(sz, "Do you want to change '%s' to '%s'",
                        pNexus->pPerson->Name(), pPerson->Name());
            sprintf(szTitle, "Change %s", szCol);
         }
         else
         {
            sprintf(sz, "Do you want to add '%s'", pPerson->Name());
            sprintf(szTitle, "Add %s", szCol);
            nexus.pRole = pPerson->LikelyRole();
            if (nexus.pRole == NULL)
            {
               nexus.pRole = Role::Find("Pro");
            }
            nexus.rrs.Reset(4);
            if (nexus.pRole->bGroup)
            {
            Nexus *pN = nexus.pMovie->FindNexus(NULL, nexus.pRole);
               if (pN)
               {
                  nexus.rrs = pN->rrs;
               }
            }
         }
         pszTitle = szTitle;
      }
   }
   else if ((fld == Nexus::fldROLE) && pNexus)   // Type
   {
   int idx;

      for (Role *pRole = Role::pHead; pRole; pRole = (Role*)pRole->pNext)
      {
         sprintf(sz, "%s (%s)", pRole->Name(), pRole->Name(1));
      int i = pStrs->AddObject(sz, (TObject*)pRole);
         if (pRole == nexus.pRole) idx = i;
      }
      sprintf(sz, "Please select %s for %s from the following list:", szCol,
                nexus.pPerson->Name());
      idx = TypeForm->Query(pStrs , szCol, sz, idx);
      if (idx >= 0)
      {
         nexus.pRole = (Role*)pStrs->Objects[idx];
         nexus.rrs.Reset(4);
         if (nexus.pRole->bGroup)
         {
         Nexus *pN = nexus.pMovie->FindNexus(NULL, nexus.pRole);
            if (pN)
            {
               nexus.rrs = pN->rrs;
            }
         }
         sprintf(sz, "Do you want to change %s's %s from '%s' to '%s'",
            nexus.pPerson->Name(), szCol, pNexus->pRole->Name(), nexus.pRole->Name());
         sprintf(szTitle, "Change %s", szCol);
         pszTitle = szTitle;
      }
   }
   else if ((fld == Nexus::fldRRAT) && pNexus)
   {
      for (int i = 1; i <= 7; ++i)
      {
         sprintf(sz, "%u", i);
         pStrs->Add(sz);
      }
      sprintf(sz, "Please select %s for %s from the following list:", szCol,
                pNexus->pPerson->Name());

  int idx = TypeForm->Query(pStrs, szCol, sz, nexus.rrs.idxRat-1);
      if (idx >= 0)
      {
         nexus.rrs.idxRat = idx+1;
         sprintf(sz, "Do you want to change %s's %s from %s to %s",
                 nexus.pPerson->Name(), szCol, pNexus->GetRating(), nexus.GetRating());
         sprintf(szTitle, "Change %s", szCol);
         pszTitle = szTitle;
      }
   }
   else if ((Nexus::fldRRNK <= fld) && (fld <= Nexus::fldRRNK_N) && pNexus)
   {
   uint nYear = nexus.pMovie->nYear;
      idxRank = fld - Nexus::fldRRNK;
   uint nLoYr = nYear - (nYear % Rank::pa[idxRank].nStep);
   uint nHiYr = nLoYr + Rank::pa[idxRank].nStep - 1;

      pStrs->AddObject("N/R", NULL);
      for (uint idx = 1; idx <= Rank::pa[idxRank].nDepth; ++idx)
      {
         pN = Movie::FindRank(idx, nLoYr, nHiYr, idxRank, nexus.pRole);
         psz = sz;
         psz += sprintf(psz, "%2u", idx);
         if (pN)
         {
            sprintf(psz, " - [%s] %s in %s", pN->GetRating(),
                        pN->pPerson->Name(), pN->pMovie->Name());
         }
         pStrs->AddObject(sz, (TObject*)pN);
      }

      sprintf(sz, "Please select %s for %s from the following list:", szCol,
                pNexus->pPerson->Name());
   uint nRnk = nexus.rrs.panRk[idxRank];
   int idx = TypeForm->Query(pStrs, szCol, sz, (nRnk == Rank::NR) ? 0 : nRnk);
      nRnk = (idx == 0) ? Rank::NR : idx;
      if ((idx >= 0) && (nRnk != pNexus->rrs.panRk[idxRank]))
      {
         nexus.rrs.panRk[idxRank] = nRnk;
         psz = sz;
         psz += sprintf(psz, "Do you want to change %s's %s",
                 nexus.pPerson->Name(), szCol);
         psz += sprintf(psz, " from %s", Rank::Prt(pNexus->rrs.panRk[idxRank]));
         sprintf(psz, " to %s", Rank::Prt(nRnk));

         sprintf(szTitle, "Change %s", szCol);
         pszTitle = szTitle;
      }
   }
   if (pszTitle)        // try update
   {
      if (Application->MessageBox(sz, pszTitle, MB_YESNO) == IDYES)
      {
         if (!pNexus)
         {
            pNexus = new Nexus;
            ((Movie*)pCurrItem)->pHeadNexus = pNexus;
         }

         if (idxRank >= 0)
         {
         RatingRanksAdjust rrsa;
            rrsa.Setup(pNexus->rrs, nexus.rrs, idxRank);

         Nexus::FieldMatch fm = Nexus::fmGRP | Nexus::fmTYPE;
            fm |= (Nexus::fmRANK0 << idxRank);
            for (uint n = rrsa.nLLmt; n <= rrsa.nULmt; ++n)
            {
               for (pN = (Nexus*)pStrs->Objects[n]; pN; pN = pN->NextMatch(fm))
               {
                  rrsa.AddAdj(pN->rrs);
               }
            }
            rrsa.AddSet(pNexus->rrs);
            for (pN = pNexus->NextMatch(fm | Nexus::fmHEAD); pN;
                        pN = pN->NextMatch(fm))
            {
               rrsa.AddSet(pN->rrs);
            }
            rrsa.ProcessList();
         }  // RankLmt

         pNexus->AdjustCounts(false);
         if (nexus.pPerson)
         {
            *pNexus = nexus;
            pNexus->AdjustCounts(true);
         }
         else // removing person
         {
            for (Nexus **ppN = &pNexus->pMovie->pHeadNexus; *ppN; ppN = &(*ppN)->pNext)
            {
               if (*ppN == pNexus)
               {
                  *ppN = pNexus->pNext;
                  delete pNexus;
                  pNexus = NULL;
                  break;
               }
            }
         }

         UpdateForm();
         FlagChangeDB();
      }
   }
///   Zero(nexus);
   delete pStrs;
}  // TMoviesForm::GridMouse

//---------------------------------------------------------------------------

void __fastcall TMoviesForm::lblGenreClick(TObject *Sender)
{
char sz[2048], *psz;
TStringList *pStrs = new TStringList;
Movie *pMovie = (Movie*)pCurrItem;
   unused(Sender);
   edYear->SetFocus();
   for (Genre *pGenre = Genre::pHead; pGenre; pGenre = (Genre*)pGenre->pNext)
   {
      pStrs->AddObject(pGenre->Name(), (TObject*)pGenre->nID);
   }
   sprintf(sz, "Please select Genre for '%s' from the following list:",
                pMovie->Name());
long liRtn = TypeForm->Query(pStrs , "Genre", sz, dwGenre, true);
   if ((liRtn != -1) && (dwGenre != (dword)liRtn))
   {
      DetectChange(Sender);
      dwGenre = liRtn;
   }

   psz = sz;
   psz += sprintf(psz, "");
   Genre::Decode(psz,  dwGenre, "; ");
   lblGenre->Caption = sz;
   delete pStrs;
}  // TMoviesForm::lblGenreClick

//---------------------------------------------------------------------------

void __fastcall TMoviesForm::RankQuery(TObject *Sender)
{
TEdit *pEdit = dynamic_cast<TEdit*>(Sender);
   if (!pEdit)
      return;

uint idxRnk = pEdit->Tag;
char sz[256], *psz;
Movie *pMovie = (Movie*)pCurrItem;
TStringList *pStrs = new TStringList;

   pStrs->AddObject("N/R", (TObject*)Rank::NR);
uint nStep = Rank::pa[idxRnk].nStep;
uint nLoYr = pMovie->nYear;
   if (nStep != 1)
   {
      nLoYr = nLoYr - (nLoYr % nStep);
   }
uint nHiYr = nLoYr + (nStep - 1);
   for (uint nRank = 1; nRank <= Rank::pa[idxRnk].nDepth; ++nRank)
   {
   Nexus *pN = Movie::FindRank(nRank, nLoYr, nHiYr, idxRnk, NULL);
      psz = sz;
      psz += sprintf(psz, "%2u", nRank);
      if (pN)
      {
         sprintf(psz, " - %s", pN->pMovie->Name());
      }
      pStrs->AddObject(sz, (TObject*)nRank);
   }

   sprintf(sz, "Please select %s for %s from the following list:",
                Rank::pa[idxRnk].szName, pMovie->Name());

uint nRankFrom = pMovie->rrs.panRk[idxRnk];
int idx = TypeForm->Query(pStrs, Rank::pa[idxRnk].szName, sz,
                           (nRankFrom == Rank::NR) ? 0 : nRankFrom);
uint nRankTo = (idx >= 0) ? (uint)pStrs->Objects[idx] : nRankFrom;
   if (nRankTo != nRankFrom)
   {
      Rank::Prt(nRankTo, sz);
      papedRank[idxRnk]->Text = sz;
   }
   delete pStrs;
}  // TMoviesForm::RankQuery

//---------------------------------------------------------------------------

void TMoviesForm::UpdForm(void)
{
char sz[1024], *psz;
Movie *pMovie = (Movie*)pCurrItem;
   if (pMovie)
   {
      edYear->Text = pMovie->nYear;
      cbRating->ItemIndex = pMovie->rrs.idxRat - 1;    //
      if (cbRating->ItemIndex < 0)
      {
         cbRating->ItemIndex = 3;
         btnOK->Tag |= 3;
      }

      psz = sz;
      psz += sprintf(psz, "");
      dwGenre = pMovie->dwGenre;
      Genre::Decode(psz,  dwGenre, "; ");
      lblGenre->Caption = sz;

      for (uint idxR = 0; idxR < Rank::nCnt; ++idxR)
      {
         Rank::Prt(pMovie->rrs.panRk[idxR], sz);
         papedRank[idxR]->Text = sz;
         papedRank[idxR]->Enabled = !bChangeName;
      }

   uint idxRow = 1;
      for (Nexus *pNexus = pMovie->pHeadNexus; pNexus; pNexus = pNexus->pNext)
      {
         for (int iCol = 0; iCol < grid->ColCount; ++iCol)
         {
         ColCfg *pCfg = GetColCfg(iCol);
            grid->Cells[iCol][idxRow] = pNexus->FieldGet(pCfg->dwFlds, sz);
         }
         idxRow = AssignRowObj(pNexus, idxRow);
      }
      for (int i = 1; i < pMovie->pNames->Count; ++i)
      {
         memAlias->Lines->Add(pMovie->pNames->Strings[i]);
      }
   }
   else
   {
   time_t timer = time(NULL);
   struct tm *pDT = localtime(&timer);

      edYear->Text = 1900+pDT->tm_year;
      cbRating->ItemIndex = 5;    //
      dwGenre = 0;

      for (uint n = 0; n < Rank::nCnt; ++n)
      {
         if (papedRank && papedRank[n])
         {
            papedRank[n]->Text = "N//R";
         }
      }
   }
}  // TMoviesForm::UpdForm

//---------------------------------------------------------------------------



