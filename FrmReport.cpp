//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop


#include "FrmTools.h"
#include "FrmReport.h"
#include "FrmType.h"
#include "FrmMovie.h"
#include "FrmPerson.h"
#include <IniFiles.hpp>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "FrmBase"
#pragma resource "*.dfm"
TReportForm *ReportForm;

#define ECC ExtColCfg

//---------------------------------------------------------------------------

__fastcall TReportForm::TReportForm(TComponent* Owner)
        : TBaseForm(Owner, "Report", ExtColCfg::Build)
{
char sz[512];
   lblAlias->Visible = false;
   memAlias->Visible = false;
   lblMain->Visible = false;
   cbMain->Visible = false;
   btnDelete->Visible = false;
   btnRename->Visible = false;
   btnNext->Visible = false;
   btnPrev->Visible = false;
   lblAction->Visible = false;

IniFile ini;

   for (int i = 0; i < cbGridFmt->Items->Count; ++i)
   {
   int iPri = 0;
   int iSec = 0;
   ColSet *pSet = (ColSet*)cbGridFmt->Items->Objects[i];
   int iLn = 0;
      for (ColCfg *pCfg = pSet->pHeadCfg; pCfg; pCfg = pCfg->pNext)
      {
      ECC::CatFld cat = (ECC::CatFld)(pCfg->dwFlds & ECC::catMSK);
         ++iLn;
      cchar *pszErr = NULL;
         if ((cat == ECC::catPRI) && (++iPri > 1))
         {
            pszErr = "multiple primary";
         }
         if ((cat == ECC::catSEC) && (++iSec > 1))
         {
            pszErr = "multiple secondary";
         }
         if (pszErr)
         {
            sprintf(sz, "%s in 'Report.%s.%s'@%u", pszErr, pSet->szTitle, iLn);
            Log(sz);
         }
      }
   }

ControlAlignment ca;
   btnOK->Left = 4;
   btnOK->Top = 4;
   ca.Align(btnCancel, btnOK,     ca.mTOP | ca.mROF);
   ca.Align(grid,      btnOK,     ca.mBLW | ca.mLAL);
   ca.Align(cbGridFmt, btnCancel, ca.mMID | ca.mROF);

   grid->Width = ClientWidth - 8;
   grid->Width = ClientHeight - grid->Top - 4;
   grid->Anchors << akRight << akBottom;
//   cbEdit->OnSelect = cbEditSelect;

   pPriECC = NULL;
   pSecECC = NULL;
   iLastType = -99;
}  // TReportForm::TReportForm

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

uint TReportForm::ECC::nCntIDX = 0;
TReportForm::ColCfg TReportForm::ECC::cfgRef1;
TReportForm::ColCfg TReportForm::ECC::cfgRef2;

//---------------------------------------------------------------------------

TReportForm::ExtColCfg::ExtColCfg(void)
{
   ZeroClassFrom(nLLmt, this);
}  // TReportForm::ExtColCfg::ExtColCfg

//---------------------------------------------------------------------------

TBaseForm::ColCfg *TReportForm::ExtColCfg::Build(uint idx)
{
   if (idx == 0)
   {
      nCntIDX = 0;
   }
   return (new ExtColCfg());
}  // TReportForm::ExtColCfg::BuildOne

//---------------------------------------------------------------------------

cchar *TReportForm::ExtColCfg::Parse(cchar *pszCfg)
{
char ch = 'R';
uint n = 0;
   nStep = 1;
   if (sscanf(pszCfg, "%*[^=]=%*[^;];%*[^;:]:%c%n", &ch, &n) == 1)
   {
      sscanf(pszCfg+n, "(%u-%u,%u)", &nLLmt, &nULmt, &nStep);
   }

DWord dwCat;
   switch (ch)
   {
   case 'P':   dwCat = catPRI;             break;
   case 'S':   dwCat = catSEC;             break;
   case 'I':   dwCat = catIDX;  ++nCntIDX; break;
   default:
   case 'R':   dwCat = catREL;             break;
   }

bool bAlt = ((nCntIDX > 1) && ((nCntIDX & 0x01) == 0));
   if (cfgRef1.nWidth == 0)     // need to initialize the colCfg1/2
   {
      cfgRef1 = *(ColCfg*)this;
      cfgRef2 = *this;
      cfgRef1.aaClr[oriHDR][ostBG] = clrRGB(0xC0,0xE0,0xC0);
      for (int i = oriBDY1; i < oriMAX; ++i)
      {
         cfgRef1.aaClr[i][ostBG] = clrRGB(0xFF,0xFF,0xFF);
         cfgRef2.aaClr[i][ostBG] = clrRGB(0xF0,0xF0,0xF2);
      }
   }
   *(ColCfg*)this = bAlt ? cfgRef2 : cfgRef1;

   pszCfg = ColCfg::Parse(pszCfg);

   anGrp[2] = dwFlds;
   anGrp[0] = nCntIDX;

   dwFlds |= dwCat;
   return (pszCfg);
}  // TReportForm::ExtColCfg::Parse

//---------------------------------------------------------------------------

char *TReportForm::ExtColCfg::Print(char *psz, ColCfg &cfgRef, cchar *pszExt)
{
   unused(cfgRef);
char szIns[128], *pszIns = szIns;

bool bLmts = false;
   *pszIns = '\0';
   switch (dwFlds & catMSK)
   {
   case catPRI:  pszIns += sprintf(pszIns, ":P");  bLmts = true;   break;
   case catSEC:  pszIns += sprintf(pszIns, ":S");  bLmts = true;   break;
   case catIDX:  pszIns += sprintf(pszIns, ":I");                  break;
   }
   if (bLmts)
   {
      pszIns += sprintf(pszIns, "(%u-%u,%u)", nLLmt, nULmt, nStep);
   }
   if (pszExt)
   {
      sprintf(pszIns, "%s", pszExt);
   }

bool bAlt = ((anGrp[0] > 1) && ((anGrp[0] & 0x01) == 0));
   psz = ColCfg::Print(psz, bAlt ? cfgRef2 : cfgRef1, szIns);
   return (psz);
}  // TReportForm::ExtColCfg::Print

//---------------------------------------------------------------------------

void TReportForm::FieldList(TStringList *pStrs)
{
   Nexus::FieldList(pStrs, Nexus::fsetFILM | Nexus::fsetROLE |
                           Nexus::fsetROLES);
}  // TReportForm::FieldList

//---------------------------------------------------------------------------

void TReportForm::GridMouse(TStringGrid *pGrid, MouseBtn &mBtn)
{
   unused(pGrid);
   if (mBtn.Is(mBtn.sRBTN | mBtn.sCTL) || mBtn.Is(mBtn.sLBTN | mBtn.sALT))
   {
      cbEdit->Left = grid->Left + mBtn.x - cbEdit->Width + cbEdit->Height;
      cbEdit->Top = grid->Top + mBtn.y - cbEdit->Height / 2;
      cbEdit->Visible = true;
      cbEdit->ItemIndex = 0;
   }
   else if ((mBtn.iRow != 0) && (mBtn.iCol != 0))
   {
   ColCfg *pCfg = (ColCfg*)pGrid->Objects[mBtn.iCol][0];
   Nexus *pNexus = (Nexus *)pGrid->Objects[mBtn.iCol][mBtn.iRow];
      if (pNexus == NULL)
      {
         pNexus = (Nexus *)pGrid->Objects[1][mBtn.iRow];
      }
      if ((pNexus == NULL) || (pCfg == NULL))
         return;

   Nexus::Field fld = (Nexus::Field)(pCfg->dwFlds & Nexus::fldMSK);
      if (fld == Nexus::fldFILM)
      {
         MoviesForm->Show(pNexus->pMovie);
      }
      else if ((Nexus::fldPERS <= fld) && (fld < Nexus::fldPERS_N))
      {
         PersonForm->Show(pNexus->pPerson);
      }
   }
}  // TReportForm::GridMouse

//---------------------------------------------------------------------------

void TReportForm::BuildReport(int iType)
{
char sz[1280];
ExtColCfg *pECC;
   if ((iLastType != iType) || (iType < 0))
   {
      if (iType < 0) iType = 0;


      grid->Width = ClientWidth - 8;
      grid->Height = ClientHeight - grid->Top - 4;
      grid->Anchors << akRight << akBottom;

   ColSet *pSet = (ColSet*)cbGridFmt->Items->Objects[iType];
      SetupGrid(pSet);

      pPriECC = NULL;
      pSecECC = NULL;
      for (ColCfg *pCfg = pCurrSet->pHeadCfg; pCfg; pCfg = pCfg->pNext)
      {
         switch (pCfg->dwFlds & ECC::catMSK)
         {
         case ECC::catPRI:  pPriECC = (ECC*)pCfg;  break;
         case ECC::catSEC:  pSecECC = (ECC*)pCfg;  break;
         }
      }
      iLastType = iType;
   }



AnsiString str;
char *pszFmt = (pPriECC->nStep > 1) ? "%us" : "%u";

int iRow = 1;
Nexus::FieldMatch fmMatch = Nexus::fmNONE;        // from Pri or Sec
Nexus::Field fldPri = (Nexus::Field)(pPriECC->dwFlds & Nexus::fldMSK);
Nexus::Field fldSec = (Nexus::Field)(pSecECC->dwFlds & Nexus::fldMSK);

   if ((Nexus::fldFRNK <= fldPri) && (fldPri < Nexus::fldFRNK_N))
   {
      fmMatch = Nexus::fmTYPE | (Nexus::fmRANK0 << (fldPri - Nexus::fldFRNK));
   }
   else if ((Nexus::fldRRNK <= fldPri) && (fldPri < Nexus::fldRRNK_N))
   {
      fmMatch = Nexus::fmTYPE | (Nexus::fmRANK0 << (fldPri - Nexus::fldRRNK));
   }
   else if ((Nexus::fldFRNK <= fldSec) && (fldSec < Nexus::fldFRNK_N))
   {
      fmMatch = Nexus::fmTYPE | (Nexus::fmRANK0 << (fldSec - Nexus::fldFRNK));
   }
   else if ((Nexus::fldRRNK <= fldSec) && (fldSec < Nexus::fldRRNK_N))
   {
      fmMatch = Nexus::fmTYPE | (Nexus::fmRANK0 << (fldSec - Nexus::fldRRNK));
   }

uint idxRnk = Rank::rnkYEAR;
   if (fldPri == Nexus::fldYEAR)
   {
      idxRnk = (pPriECC->nStep > 1) ? Rank::rnkDEC : Rank::rnkYEAR;
   }

   grid->RowCount = 2;
int iRowGrp = 0;
   for (uint nPri = pPriECC->nLLmt; nPri <= pPriECC->nULmt; nPri += pPriECC->nStep)
   {
   uint nPriHi = nPri + pPriECC->nStep - 1;
   uint nPriLo = nPri;
   Nexus *pN, *pNexus = NULL;

      for (uint nSec = pSecECC->nLLmt; nSec <= pSecECC->nULmt; nSec += pSecECC->nStep)
      {
      bool bActualData = false;
         AssignRowObj(pNexus, iRow, false);

         pECC = (ECC*)pCurrSet->pHeadCfg;
         for (uint idx = 0; idx < pCurrSet->nCols; ++idx)
         {
         cchar *pszValue = sz;
         ExtColCfg &ecc = *pECC;
         Nexus::Field fld = (Nexus::Field)(ecc.dwFlds & Nexus::fldMSK);
            switch ((ecc.dwFlds & ECC::catMSK))
            {
            case ECC::catPRI:
               sprintf(sz, pszFmt, nPri);
               break;

            case ECC::catSEC:
               sprintf(sz, "%2u", nSec);
               break;

            case ECC::catIDX:
               if ((fld == Nexus::fldFILM) ||
                   ((Nexus::fldPERS <= fld) && (fld <= Nexus::fldPERS_N)))
               {
               Role *pRole = Role::Find(fld - Nexus::fldPERS);
                  pNexus = Movie::FindRank(nSec, nPriLo, nPriHi, idxRnk, pRole);
               }
               else
               {
                  pNexus = NULL;
               }
               // fall into catREL

            case ECC::catREL:
               if (!pNexus)
                  pszValue = "<none>";
               else
               {
                  bActualData = true;
                  if ((Nexus::fldPERS <= fld) && (fld <= Nexus::fldPERS_N))
                  {
                     str = pNexus->pPerson->Name();
                     pN = pNexus;
                     while ((pN = pN->NextMatch(fmMatch)) != NULL)
                     {
                        str += "; ";
                        str += pN->pPerson->Name();
                     }
                     pszValue = str.c_str();
                  }
                  else
                  {
                     pszValue = pNexus->FieldGet(fld, sz);
                  }
               }
               break;   // catREL
            }
            grid->Cells[idx][iRow] = pszValue;
            grid->Objects[idx][iRow] = (TObject*)pNexus;

            pECC = (ECC*)pECC->pNext;
         }
         if (bActualData)
         {
         RowParam rp;
            Get(rp, iRow);
            rp.nGrp = iRowGrp;
            Set(rp, iRow);
            ++iRow;
         }
         //grid->FixedCols = 2;
      }
      if (++iRowGrp > 2) iRowGrp = 0;
   }
}  // TReportForm::BuildReport

//---------------------------------------------------------------------------

void *TReportForm::EditColCfg(ColCfg *pCfg, EditOption eo, void *pvData)
{
struct EditData
{
   TStringList    *pStrs;
   int            idCat;
   uint           nLastIDX;
} *pED = (EditData*)pvData; // EditData

ECC::CatFld cat = (ECC::CatFld)((pCfg ? pCfg->dwFlds : 0) & ECC::catMSK);
   if (pED == NULL)          // setup phase
   {
      pED = new EditData;
   TTypeForm::QueryOptions qo;
      Zero(qo);

      pED->pStrs = new TStringList;
      pED->pStrs->AddObject("Relative", (TObject*)ECC::catREL);
      pED->pStrs->AddObject("Index", (TObject*)ECC::catIDX);
   uint idx = ((eo == eoEDIT) && (cat == ECC::catIDX)) ? 1 : 0;
      pED->idCat = TypeForm->AddComboBox(pED->pStrs, qo, idx);
      pED->nLastIDX = pCfg->anGrp[0];
      return (pED);
   }

   if (pCfg)         // update phase
   {
   int iRtn;
      TypeForm->GetValue(pED->idCat, NULL, iRtn);
      pCfg->dwFlds &= ~ECC::catMSK;
      pCfg->dwFlds |= (ECC::CatFld)pED->pStrs->Objects[iRtn];

      pCfg->anGrp[0] = (cat == ECC::catIDX) ? ++ECC::nCntIDX : pED->nLastIDX;
   }
   delete pED->pStrs;
   delete pED;
   return (NULL);
}  // TReportForm::EditColCfg

//---------------------------------------------------------------------------

void TReportForm::UpdForm(void)
{
   BuildReport(iLastType);
}  // TReportForm::UpdForm

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

