//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MovieMain.h"
#include "FrmGraph.h"
#include "FrmTools.h"
#include "FrmType.h"
#include <math.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "PERFGRAP"
#pragma link "FrmBase"
#pragma resource "*.dfm"
TGraphForm *GraphForm;

//---------------------------------------------------------------------------
bool bEnbUpd = false;
__fastcall TGraphForm::TGraphForm(TComponent* Owner)
        : TBaseForm(Owner, "Graph")
{
   ppHeadItem = &(NamedItem*)Person::pHead;
   lblAlias->Visible = false;
   memAlias->Visible = false;
   lblMain->Visible = false;
   btnDelete->Visible = false;
   btnRename->Visible = false;
   btnNext->Visible = false;
   btnPrev->Visible = false;
   lblAction->Visible = false;
   cbMain->Visible = false;
   grid->Visible = false;
   cbGridFmt->Visible = false;

ControlAlignment ca;
   AlignFrom(btnOK);

   graph->Anchors << akRight;
   ca.SetPanel(this);
   ca.Align(btnOK,     btnOK,     ca.mTIP | ca.mLIP);
   ca.Align(btnCancel, btnOK,     ca.mTOP | ca.mROF);
   ca.Align(graph,     btnOK,     ca.mBLW | ca.mLAL);
   ca.Align(cbGraph,   btnCancel, ca.mTOP | ca.mROF);

   AddSet("Genre", rfGENRE);
   AddSet("Roles", rfROLE);
   AddSet("Decade", rfDECADE);
   AddSet("Decade-Genre", rfDEC_GENRE);
   AddSet("Genre-Decade", rfGENRE_DEC);

   cbGraph->ItemIndex = 0;

   gi.aclr[gi.ciEVEN] = clAqua;
   gi.aclr[gi.ciODD] = clGreen;
   gi.aclr[gi.ciEVEN2] = clBlue;
   gi.aclr[gi.ciODD2] = clSkyBlue;
   gi.aclr[gi.ciZERO] = clRed;
   gi.aclr[gi.ciAXIS] = clWhite;
   gi.aclr[gi.ciTEXT] = clBlack;
   gi.aclr[gi.ciSTAT] = clWhite;

   btnOK->OnClick = AcceptOK;
   bEnbUpd = true;
}  // TGraphForm::TGraphForm

//---------------------------------------------------------------------------

void __fastcall TGraphForm::AcceptOK(TObject *Sender)
{
   unused(Sender);
}  // TGraphForm::AcceptOK

//---------------------------------------------------------------------------

void TGraphForm::AddSet(cchar *psz, RefType rf)
{
char sz[128];
   sprintf(sz, "%s: H Count", psz);
   cbGraph->Items->AddObject(sz, (TObject*)(rf));

   sprintf(sz, "%s: V Count", psz);
   cbGraph->Items->AddObject(sz, (TObject*)(rf | rfVERT));

   sprintf(sz, "%s: H Rate", psz);
   cbGraph->Items->AddObject(sz, (TObject*)(rf | rfRATE));

   sprintf(sz, "%s: V Rate", psz);
   cbGraph->Items->AddObject(sz, (TObject*)(rf | rfRATE | rfVERT));
}  // TGraphForm::AddSet

//---------------------------------------------------------------------------

void __fastcall TGraphForm::cbGraphSelect(TObject *Sender)
{
   unused(Sender);
   if (cbGraph->ItemIndex != iCurr)
   {
      iCurr = cbGraph->ItemIndex;
      UpdateForm();
   }
}  // TGraphForm::cbGraphSelect

//---------------------------------------------------------------------------

void __fastcall TGraphForm::chkStatsClick(TObject *Sender)
{
   unused(Sender);
   UpdateForm();
}  // TGraphForm::chkStatsClick

//---------------------------------------------------------------------------

void __fastcall TGraphForm::FormResize(TObject *Sender)
{
  unused(Sender);
  UpdateForm();
}  // TGraphForm::FormResize

//---------------------------------------------------------------------------

void __fastcall TGraphForm::graphMouseDown(TObject *Sender,
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
   pStrs->AddObject("Odds", (TObject*)&gi.aclr[gi.ciODD]);
   pStrs->AddObject("Evens",  (TObject*)&gi.aclr[gi.ciEVEN]);
   pStrs->AddObject("Odds2", (TObject*)&gi.aclr[gi.ciODD2]);
   pStrs->AddObject("Evens2",  (TObject*)&gi.aclr[gi.ciEVEN2]);
   pStrs->AddObject("Missing",  (TObject*)&gi.aclr[gi.ciZERO]);
   pStrs->AddObject("Axis",  (TObject*)&gi.aclr[gi.ciAXIS]);
   pStrs->AddObject("Stat",  (TObject*)&gi.aclr[gi.ciSTAT]);
   pStrs->AddObject("Text",  (TObject*)&gi.aclr[gi.ciTEXT]);
   qo.m = ca.mBLW | ca.mLAL;
   qo.lwbPrm.aby[2] = qo.tRBTN_D;
int idColor = TypeForm->AddGroup(pStrs, "Scope", qo, 0);


   qo.m = ca.mBLW | ca.mCIP;
//   TypeForm->AddButton("Save", cc, mrYes);

bool bUpd = false;
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
            bUpd = true;
         }
      }
      if (iRtn == mrYes)        // save a copy
      {
         Role::Save();
      }
   }
   delete pStrs;

   if (bUpd)
   {
      UpdateForm();
   }
}  // graphMouseDown

//---------------------------------------------------------------------------

void TGraphForm::GraphElement::Data(double r)
{
   if (bAll || (r != 0))
   {
      rTtl += r;
      rTtl2 += r*r;
      ++nCnt;
      if (rMax < rMin)
      {
         rMin = r;
         rMax = r;
      }
      else if (r > rMax) rMax = r;
      else if (r < rMin) rMin = r;
   }
}  // TGraphForm::GraphElement::Data

//---------------------------------------------------------------------------

TGraphForm::GraphInfo::GraphInfo(void)
{
   ZeroMem(this);
}  // TGraphForm::GraphInfo::GraphInfo

//---------------------------------------------------------------------------

TGraphForm::GraphInfo::~GraphInfo()
{
   delete paGE;
}  // TGraphForm::GraphInfo::~GraphInfo()

//---------------------------------------------------------------------------

void TGraphForm::GraphInfo::Setup(uint nCount, bool bAll, uint nGrp)
{
   bPlotTtl = false;
   bPlotStats = false;
   nCnt = nCount;
   if (paGE)
   {
      delete paGE;
   }
   paGE = new GraphElement[nCnt];
   ZeroLen(paGE, sizeof(GraphElement) * nCnt);
   for (uint idx = 0; idx < nCnt; ++idx)
   {
      paGE[idx].rMin = 1;
      paGE[idx].bAll = bAll;
      paGE[idx].nClrIdx = ((idx & 0x01) != 0) ? ciODD : ciEVEN;
      if ((nGrp > 1) && (((idx / nGrp) & 0x01) != 0))
      {
         paGE[idx].nClrIdx = ((idx & 0x01) != 0) ? ciODD2 : ciEVEN2;
      }
   }
}  // TGraphForm::GraphInfo::Setup

//---------------------------------------------------------------------------

double rScale;
bool bLog;
double Scale(double r)
{
   if (bLog)
   {
      if (r != 0) r = log(r);
   }
   return (rScale * r);
}  // Scale

void TGraphForm::DrawBar(void)
{
char sz[128];

TCanvas *pCnvs = graph->Canvas;
int iGraphWd = ClientWidth - graph->Left - 8;
int iGraphHt = ClientHeight - graph->Top - 8;
double rMax = 1;
double rMin = 2;
int iMaxTxtHrz = 0;
int iMaxTxtVrt = 0;
int iTxtHt = pCnvs->TextHeight("XX");
   for (uint idx = 0; idx < gi.nCnt; ++idx)
   {
   GraphElement &ge = gi.paGE[idx];
      if (gi.bPlotTtl)
      {
         if (rMax < rMin)
         {
            rMin = ge.rTtl;
            rMax = ge.rTtl;
         }
         else if (ge.rTtl > rMax) rMax = ge.rTtl;
         else if (ge.rTtl < rMin) rMin = ge.rTtl;
      }
      else
      {
         if (rMax < rMin)
         {
            rMin = (ge.nCnt > 0) ? ge.rTtl / ge.nCnt : ge.rTtl;
            rMax = rMin;
         }
         if (ge.rMax > rMax) rMax = ge.rMax;
         if (ge.rMin < rMin) rMin = ge.rMin;
      }

   int ii = pCnvs->TextWidth(ge.szCat);
      if (ii > iMaxTxtHrz) iMaxTxtHrz = ii;
      ii = strlen(ge.szCat) * iTxtHt;
      if (ii > iMaxTxtVrt) iMaxTxtVrt = ii;
   }


bool bVertTxt = false;
   if (gi.bVert)
   {
      bVertTxt = (iGraphWd / gi.nCnt < iMaxTxtHrz);
      iMaxTxtHrz = pCnvs->TextWidth("1000");
   }
int xAxis = iMaxTxtHrz + 10;
int xMax = iGraphWd;
int yAxis = iGraphHt - (bVertTxt ? iMaxTxtVrt : iTxtHt) - 12;
int yMax = yAxis;

   bLog = chkLog->Checked;
double rSclMax = rMax;
   if (bLog)
   {
      rSclMax = log(rMax);
   }
   if (rSclMax == 0.0) rSclMax = 1.0;
   if (gi.bVert)
   {
      rScale = (yMax - 8) / rSclMax;
   }
   else
   {
      rScale = (xMax - xAxis - 8) / rSclMax;
   }
//double rSclX = (xMax - xAxis - 8) / rMax;
//double rSclY = (yMax - 8) / rMax;

int xGap = 4;
int yGap = 4;
   if (bVertTxt)
   {
      xGap = 1;
   }

int iHt = yMax / (double)gi.nCnt - yGap;
int iWd = (xMax - xAxis) / (double)gi.nCnt - xGap;
   if (iHt < 2 * yGap)
   {
      yGap = 1;
      iHt = yMax / (double)gi.nCnt - yGap;
   }

TPoint ptMax, ptMin, ptOff;
TPoint ptAvg, ptStd;

int x, y;
int xPos = xAxis + (gi.bVert ? 4 : 1);
int yPos = yAxis - (gi.bVert ? 1 : 4);
   for(uint idx = 0; idx < gi.nCnt; ++idx)
   {
   double r;
   TColor clr;
   GraphElement &ge = gi.paGE[idx];
   double rVal = ge.rTtl;
      if (!gi.bPlotTtl && (ge.nCnt > 0))
      {
         rVal /= ge.nCnt;
      }
      if (rVal == 0)
      {
         clr = gi.aclr[gi.ciZERO];
      }
      else
      {
         clr = gi.aclr[ge.nClrIdx];
      }
      pCnvs->Pen->Color = clr;
      pCnvs->Brush->Color = clr;
      pCnvs->Font->Color = gi.aclr[gi.ciTEXT];
      sprintf(sz, " %s ", ge.szCat);
   int iTxtWd = pCnvs->TextWidth(sz);
      if (gi.bVert)
      {
         r = Scale(rVal);
         pCnvs->Rectangle(xPos,yPos, xPos+iWd,yPos-r);

         if (bVertTxt)
         {
         int xTxt = xPos + (iWd) /2;
         int yTxt = yPos;
         int yHt = pCnvs->TextHeight("XX") - 2;
         char szTxt[2];
            szTxt[1] = '\0';

            pCnvs->Rectangle(xPos,yPos+1, xPos+iWd,yPos+iMaxTxtVrt);
            for (char *psz = sz; *psz; ++psz)
            {
               szTxt[0] = *psz;
               pCnvs->TextOut(xTxt - pCnvs->TextWidth(szTxt)/2, yTxt, szTxt);
               yTxt += yHt;
            }
         }
         else
         {
            pCnvs->TextOut(xPos + (iWd - iTxtWd)/2, yPos, sz);
         }

         ptAvg.x = xPos + iWd / 2; ptAvg.y = yPos - r;
         ptOff.x = iWd / 2 - 3;    ptOff.y = 0;
         ptMin.x = xPos + iWd/2;   ptMin.y = yPos - Scale(ge.rMin);
         ptMax.x = ptMin.x;        ptMax.y = yPos - Scale(ge.rMax);
         r = (ge.nCnt > 0) ? sqrt(ge.rTtl2/ge.nCnt - ge.rTtl*ge.rTtl / ge.nCnt / ge.nCnt) : 0;
         ptStd.x = 0;              ptStd.y = Scale(r);

         xPos += iWd + xGap;
      }
      else
      {
         r = Scale(rVal);
         pCnvs->Rectangle(xPos,yPos, xPos+r, yPos-iHt);

         pCnvs->TextOut(xPos - iTxtWd - 8, yPos - (iHt+iTxtHt)/2, sz);

         ptAvg.x = xPos + r;                ptAvg.y = yPos - iHt/2;
         ptOff.x = 0;                       ptOff.y = iHt / 2 -  3;
         ptMin.x = xPos + Scale(ge.rMin);   ptMin.y = yPos - iHt/2;
         ptMax.x = xPos + Scale(ge.rMax);   ptMax.y = ptMin.y;
         r = (ge.nCnt > 0) ? sqrt(ge.rTtl2/ge.nCnt - ge.rTtl*ge.rTtl/ge.nCnt/ge.nCnt) : 0;
         ptStd.x = Scale(r);                ptStd.y = 0;

         yPos -= iHt + yGap;
      }
      if (gi.bPlotStats && (ge.rMin < ge.rMax))
      {
         pCnvs->Pen->Color = gi.aclr[gi.ciSTAT];
         pCnvs->MoveTo(ptMin.x+ptOff.x, ptMin.y+ptOff.y);
         pCnvs->LineTo(ptMin.x-ptOff.x, ptMin.y-ptOff.y);
         pCnvs->MoveTo(ptMin.x, ptMin.y);
         pCnvs->LineTo(ptMax.x, ptMax.y);
         pCnvs->MoveTo(ptMax.x+ptOff.x, ptMax.y+ptOff.y);
         pCnvs->LineTo(ptMax.x-ptOff.x, ptMax.y-ptOff.y);

         pCnvs->MoveTo(ptAvg.x+ptOff.x, ptAvg.y+ptOff.y);
         pCnvs->LineTo(ptAvg.x-ptOff.x, ptAvg.y-ptOff.y);

         pCnvs->MoveTo(ptAvg.x+ptOff.x+ptStd.x, ptAvg.y+ptOff.y+ptStd.y);
         pCnvs->LineTo(ptAvg.x-ptOff.x+ptStd.x, ptAvg.y-ptOff.y+ptStd.y);
         pCnvs->LineTo(ptAvg.x+ptOff.x-ptStd.x, ptAvg.y+ptOff.y-ptStd.y);
         pCnvs->LineTo(ptAvg.x-ptOff.x-ptStd.x, ptAvg.y-ptOff.y-ptStd.y);
         pCnvs->LineTo(ptAvg.x+ptOff.x+ptStd.x, ptAvg.y+ptOff.y+ptStd.y);
      }


   }
   pCnvs->Pen->Color = gi.aclr[gi.ciAXIS];
   pCnvs->MoveTo(xAxis, 0);
   pCnvs->LineTo(xAxis, yAxis);
   pCnvs->LineTo(xMax, yAxis);

   pCnvs->Brush->Color = clBlack;
   pCnvs->Font->Color = gi.aclr[gi.ciAXIS];
int iDiv = 10;
   if (gi.bVert)
   {
      iDiv = (yAxis) / (3 * pCnvs->TextHeight("X"));
   }
   else
   {
      iDiv = (iGraphWd - xAxis) / (9 * pCnvs->TextWidth("X"));
   }
double rStep = rSclMax / iDiv;

   if (rStep > 1E8)
   {
      rStep = 1E6;
   }
   else if (rStep > 100)
   {
      rStep = (int)rStep - ((int)rStep % 100);
   }
   else if (rStep > 10)
   {
      rStep = (int)rStep - ((int)rStep % 10);
   }
   else if (rStep > 0.8)
   {
      rStep = (int)(rStep + 0.5);
   }
   else rStep = 0.5;

bool bText = ((rfMaster & rfTYPE_MSK) != rfROLE) &&
                ((rfMaster & rfRATE) != 0);
   for (double rV = 0; rV <= rMax; rV += rStep)
   {
      if (bText && (rV <= Movie::pStrRate->Count))
      {
         sz[0] = '\0';
         if ((rV > 0) && ((int)rV == rV))
         {
            sprintf(sz, "%s", Movie::pStrRate->Strings[rV-1].c_str());
         }
      }
      else
      {
         sprintf(sz, "%lg", rV);
      }
      if (gi.bVert)
      {
         y = yAxis - Scale(rV);
         pCnvs->MoveTo(xAxis, y);
         pCnvs->LineTo(xAxis - 4, y);

         pCnvs->TextOut(xAxis - 4 - pCnvs->TextWidth(sz), y-5, sz);
      }
      else
      {
         x = xAxis + Scale(rV);
         pCnvs->MoveTo(x, yAxis);
         pCnvs->LineTo(x, yAxis + 4);

         pCnvs->TextOut(x - pCnvs->TextWidth(sz)/2, yAxis+5, sz);
      }
      if (bLog)
      {
         rStep *= 2;
      }
   }

   graph->Update();
}  // TGraphForm::DrawBar

//---------------------------------------------------------------------------

void TGraphForm::FieldList(TStringList *pStrs)
{
   Nexus::FieldList(pStrs, Nexus::fsetFILM | Nexus::fsetROLE);
}  // TGraphForm::FieldList

//---------------------------------------------------------------------------

bool TGraphForm::Select(Person *&pPerson)
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
}  // TGraphForm::Select

//---------------------------------------------------------------------------

void TGraphForm::UpdForm(void)
{
   if (!bEnbUpd) return;

//   graph->Width = ClientWidth - graph->Left - 4;
//   graph->Height = ClientHeight - graph->Top - 4;

   graph->Canvas->Brush->Color = clBlack;
   graph->Canvas->Rectangle(0,0, graph->Width, graph->Height);

   rfMaster = (RefType)cbGraph->Items->Objects[iCurr];

RefType rf = rfMaster & rfTYPE_MSK;
bool bRate = ((rfMaster & rfRATE) != 0);

   if (rf == rfDECADE)
   {
   uint nYrMin=-1, nYrMax=0;

      for (Movie *pM = Movie::pHead; pM; pM = (Movie*)pM->pNext)
      {
         if (pM->nYear < nYrMin) nYrMin = pM->nYear;
         if (pM->nYear > nYrMax) nYrMax = pM->nYear;
      }
      nYrMin -= nYrMin % 10;
      nYrMax += 9 - (nYrMax % 10);
   uint nCnt = (nYrMax + 1 - nYrMin) / 10;
      gi.Setup(nCnt, !bRate);
      for (uint idx = 0; idx < nCnt; ++idx)
      {
         sprintf(gi.paGE[idx].szCat, "%u", nYrMin + 10 * idx);
      }

      for (Movie *pM = Movie::pHead; pM; pM = (Movie*)pM->pNext)
      {
      int idx = (pM->nYear - nYrMin) / 10;
      GraphElement &ge = gi.paGE[idx];
         ge.Data(bRate ? pM->rrs.idxRat : 1);
      }
   }
   else if ((rf == rfDEC_GENRE) || (rf == rfGENRE_DEC))
   {
   uint nYrMin=-1, nYrMax=0;

      for (Movie *pM = Movie::pHead; pM; pM = (Movie*)pM->pNext)
      {
         if (pM->nYear < nYrMin) nYrMin = pM->nYear;
         if (pM->nYear > nYrMax) nYrMax = pM->nYear;
      }
      nYrMin -= nYrMin % 10;
      nYrMax += 9 - (nYrMax % 10);
   uint nCnt = (nYrMax + 1 - nYrMin) / 10;
   uint nYrStep, nGenreStep, nGrp;

      if (rf == rfDEC_GENRE)
      {
         nYrStep = 1;
         nGenreStep = nCnt;
         nGrp = nCnt;
      }
      else
      {
         nYrStep = Genre::nCnt;
         nGenreStep = 1;
         nGrp = Genre::nCnt;
      }

      gi.Setup(nCnt * Genre::nCnt, !bRate, nGrp);
   uint idxGenre = 0;
      for (Genre *pG = Genre::pHead; pG; pG = (Genre*)pG->pNext)
      {
         for (uint nYr = 0; nYr < nCnt; ++nYr)
         {
         uint idx = idxGenre + nYr * nYrStep;
            sprintf(gi.paGE[idx].szCat, "%s:%u", pG->Name(1), nYrMin + 10*nYr);
         }

         for (Movie *pM = Movie::pHead; pM; pM = (Movie*)pM->pNext)
         {
         uint nYr = (pM->nYear - nYrMin) / 10;
            if ((pM->dwGenre & pG->dwMask) != 0)
            {
            GraphElement &ge = gi.paGE[idxGenre + nYr * nYrStep];
               ge.Data(bRate ? pM->rrs.idxRat : 1);
            }
         }
         idxGenre += nGenreStep;
      }
   }
   else if (rf == rfGENRE)
   {
      gi.Setup(Genre::nCnt, !bRate);

   int idx = 0;
      for (Genre *pG = Genre::pHead; pG; pG = (Genre*)pG->pNext)
      {
      GraphElement &ge = gi.paGE[idx++];
         strcpy(ge.szCat, pG->Name());
         for (Movie *pM = Movie::pHead; pM; pM = (Movie*)pM->pNext)
         {
            if ((pM->dwGenre & pG->dwMask) != 0)
            {
               ge.Data(bRate ? pM->rrs.idxRat : 1);
            }
         }
      }
   }
   else
   {
      gi.Setup(Role::nCnt, !bRate);

   int idx = 0;
      for (Role *pR = Role::pHead; pR; pR = (Role*)pR->pNext)
      {
      GraphElement &ge = gi.paGE[idx++];
         strcpy(ge.szCat, pR->Name());
         for (Movie *pM = Movie::pHead; pM; pM = (Movie*)pM->pNext)
         {
            for (Nexus *pN = pM->pHeadNexus; pN; pN = pN->pNext)
            {
               if (pN->pRole == pR)
               {
                  ge.Data(bRate ? pN->rrs.idxRat : 1);
               }
            }
         }
      }
   }
   gi.bPlotStats = bRate && chkStats->Checked;
   gi.bPlotTtl = !bRate;

   if (gi.paGE)
   {
      gi.bVert = ((rfMaster & rfVERT) != 0);
      DrawBar();
   }

   graph->Update();
}  // TGraphForm::UpdForm

//---------------------------------------------------------------------------


