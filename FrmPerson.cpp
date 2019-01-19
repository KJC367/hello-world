//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MovieMain.h"
#include "FrmPerson.h"
#include "FrmTools.h"
#include "FrmType.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "PERFGRAP"
#pragma link "FrmBase"
#pragma resource "*.dfm"
TPersonForm *PersonForm;

//---------------------------------------------------------------------------

__fastcall TPersonForm::TPersonForm(TComponent* Owner)
        : TBaseForm(Owner, "Person")
{
   ppHeadItem = &(NamedItem*)Person::pHead;

ControlAlignment ca;
   AlignFrom(lblAlias);

   graph->Anchors << akRight;
   ca.Align(graph,     grid,     ca.mTOP | ca.mROF);
   ca.Align(lblScores, graph,    ca.mBLW | ca.mLAL);
   ca.Align(lblScores, grid,     ca.mROF);
   btnOK->OnClick = AcceptOK;
}  // TPersonForm::TPersonForm

//---------------------------------------------------------------------------

void __fastcall TPersonForm::AcceptOK(TObject *Sender)
{
   unused(Sender);
   if (bChangeName || (pCurrItem == NULL))
   {
      bChangeName = false;
   cchar *pszName = cbMain->Text.c_str();
      pCurrItem = (*pszName) ? new Person(pszName) : NULL;
   }
}  // TPersonForm::AcceptOK

//---------------------------------------------------------------------------

void TPersonForm::FieldList(TStringList *pStrs)
{
   Nexus::FieldList(pStrs, Nexus::fsetFILM | Nexus::fsetROLE);
}  // TPersonForm::FieldList

//---------------------------------------------------------------------------

bool TPersonForm::Select(Person *&pPerson)
{
   pCurrItem = pPerson;
   if (pCurrItem == NULL)
   {
      cbMain->Text = "<enter name>";
   }
   btnOK->Caption = "Accept";
bool bRtn = (ShowModal() == mrOk);
   if (bRtn)
   {
      pPerson = (Person*)pCurrItem;
   }
   btnOK->Caption = "OK";
   return (bRtn);
}  // TPersonQForm::Select

//---------------------------------------------------------------------------

void TPersonForm::UpdForm(void)
{
char sz[512], *psz;
Person *pPerson = (Person*)pCurrItem;
   if (pPerson)
   {
   uint idxRow = 1;
      for (Movie *pMovie = Movie::pHead; pMovie; pMovie = (Movie*)pMovie->pNext)
      {
         for (Nexus *pNexus = pMovie->pHeadNexus; pNexus; pNexus = pNexus->pNext)
         {
            if (pNexus->pPerson == pPerson)
            {
               for (int iCol = 0; iCol < grid->ColCount; ++iCol)
               {
               ColCfg *pCfg = GetColCfg(iCol);
                  grid->Cells[iCol][idxRow] = pNexus->FieldGet(pCfg->dwFlds, sz);
               }
               idxRow = AssignRowObj(pNexus, idxRow);
            }
         }
      }
      psz = sz;
      for (Role *pR = Role::pHead; pR; pR = (Role*)pR->pNext)
      {
      char szR2[64], *pszRole2 = NULL;
      float fScore;

      cchar *pszRole = pR->Name();
         if (strncmp(pszRole, "Leading ", 8) == 0)
         {
            pszRole += 8;
            sprintf(szR2, "Supporting %s", pszRole);
            pszRole2 = szR2;
         }
         if (strncmp(pR->Name(), "Supp", 4) != 0)
         {
            fScore = pPerson->ComputeScore(pR->Name(), pszRole2, NULL);
            if (fScore)
            {
               psz += sprintf(psz, "%s = %.2f\n", pszRole, fScore);
            }
         }
      }
      lblScores->Caption = sz;
   }


TCanvas *pCnvs = graph->Canvas;
   graph->Width = ClientWidth - graph->Left - 4;

   pCnvs->Brush->Color = clBlack;
   pCnvs->Rectangle(0,0, graph->Width, graph->Height);

int iRoles = Role::GetMax();
int iWd = (graph->Width - 2) / iRoles - 2;
double rSclY = graph->Height / (double)iRoles;
int x, y;
int yMax = graph->Height;

#define BAR 3
   for (Role *pR = Role::pHead; pR; pR = (Role*)pR->pNext)
   {
   double r = 0;
   int iMaxRat = 0;
   int iMinRat = 0;
   ulong lnTtl = 0;
   uint nCnt = 0;

      x = pR->nID * (iWd+2) - iWd;
      if (pPerson)
      {
         iMinRat = 99;
         for (int iRat = 1; iRat <= 7; ++iRat)
         {
         uint n = pPerson->paanCnt[pR->nID-1][iRat-1];
           if (n > 0)
            {
               if (iRat < iMinRat) iMinRat = iRat;
               if (iRat > iMaxRat) iMaxRat = iRat;
               lnTtl += n * iRat;
               nCnt += n;
            }
         }
      }
      if (nCnt > 0)
      {
         r = rSclY * lnTtl / nCnt;
      }

      pCnvs->Pen->Color = pR->aclr[Role::ciOUT];
      pCnvs->Brush->Color = pR->aclr[Role::ciBAR];
      pCnvs->Rectangle(x,yMax, x+iWd,yMax - r);

      pCnvs->Pen->Color = pR->aclr[Role::ciLMT];
      if (iMaxRat > 0)
      {
         y = yMax - rSclY * iMaxRat;
         pCnvs->MoveTo(x+BAR,y);
         pCnvs->LineTo(x+iWd-BAR, y);
         pCnvs->MoveTo(x+iWd/2, y);

         y = yMax - rSclY * iMinRat;
         pCnvs->LineTo(x+iWd/2,y);
         pCnvs->MoveTo(x+BAR,y);
         pCnvs->LineTo(x+iWd-BAR, y);
      }

      pCnvs->Pen->Color = pR->aclr[Role::ciTXT];
      sprintf(sz, "%c", pR->Name()[0]);
      pCnvs->TextOut(x + (iWd - pCnvs->TextWidth(sz))/2,
                     yMax - pCnvs->TextHeight(sz) - 2, sz);
   }
   graph->Update();
}  // TPersonQForm::UpdForm

//---------------------------------------------------------------------------

void __fastcall TPersonForm::FormResize(TObject *Sender)
{
  unused(Sender);
  UpdateForm();
}  // TPersonForm::FormResize

//---------------------------------------------------------------------------

void __fastcall TPersonForm::graphMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   unused(Sender);
   unused(Button);
   unused(Shift);
   unused(Y);
int iColWd = graph->Width / Role::nMasterID;
int iCol = 1+ X / iColWd;
Role *pRole = Role::Find(iCol);


TStringList *pStrs = new TStringList;
ControlAlignment ca;
char sz[256];
   sprintf(sz, "Please select the following graph features for '%s' column",
        pRole->Name());
   TypeForm->Reset("Adjust Grpah", sz, 250);
TTypeForm::QueryOptions qo;
   Zero(qo);

   pStrs->Clear();
   pStrs->AddObject("Role", (TObject*)1);
   pStrs->AddObject("All Roles",  (TObject*)2);
   qo.m = ca.mBLW | ca.mLAL;
   qo.lwbPrm.aby[2] = qo.tRBTN_D;
int idScope = TypeForm->AddGroup(pStrs, "Scope", qo, 0);
   unused(idScope);


   pStrs->Clear();
   for (int i = 0; i < Role::ciMAX; ++i)
   {
      pStrs->AddObject(Role::aClrCfg[i].szKey, (TObject*)&pRole->aclr[i]);
   }

   qo.m = ca.mTOP | ca.mROF;
   qo.lwbPrm.aby[1] = 2;
   qo.lwbPrm.aby[2] = qo.tRBTN_D;
int idColor = TypeForm->AddGroup(pStrs, "Color", qo, 0);

   qo.m = ca.mBLW | ca.mCIP;
   TypeForm->AddButton("Save", qo, mrYes);

int iRtn = TypeForm->Query();
   if (iRtn > 0)
   {
   int iColor;
      TypeForm->GetValue(idColor, NULL, iColor);
   TColor *pClr = (TColor*)iColor;
      if (pClr)
      {
         ColorDialog1->Color = *pClr;
         if (ColorDialog1->Execute())
         {
            *pClr = ColorDialog1->Color;
         }
      }
      if (iRtn == mrYes)        // save a copy
      {
         Role::Save();
      }
   }
   delete pStrs;
}  // graphMouseDown

//---------------------------------------------------------------------------

