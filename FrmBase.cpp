//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FrmBase.h"

#include <IniFiles.hpp>
//---------------------------------------------------------------------------
//#include "MovieMain.h"
#include "FrmTools.h"
#include "TimeStamp.h"
#include "FrmType.h"
#include "MovieMain.h"

#include "FrmMovie.h"           // for GridMouse 
#include "FrmPerson.h"

TimeStamp atms[5];


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TBaseForm *BaseForm;

#define DEAD_ZONE 4

#define SEL_CANCEL  0xFFF
#define SEL_FONT    0x00F

#define ORI_MASK    0x0F0
#define ORI_SHIFT   4

#define SEL_ONE     0x000
#define SEL_GBL     0x100
#define SEL_IDX     0x200
#define SEL_GRP     0x300
#define SEL_TYP     0x400

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

NamedItem::NamedItem(void)
{
   ZeroClass(&pNext, this);
   pNames = new TStringList;
}  // NamedItem::NamedItem

//---------------------------------------------------------------------------

NamedItem::~NamedItem()
{
   delete pNames;
}  // NamedItem::~NamedItem

//---------------------------------------------------------------------------

void NamedItem::Dump(char *&psz)
{
   psz += sprintf(psz, "'%s'", Name());
}  // NamedItem::Dump

//---------------------------------------------------------------------------

NamedItem *NamedItem::Find(cchar *psz, NamedItem *pHead)
{
NamedItem *pItem = NULL;
   if (psz != NULL)
   {
      for (pItem = pHead; pItem; pItem = pItem->pNext)
      {
         for (int i = 0; i < pItem->pNames->Count; ++i)
         {
            if (stricmp(psz, pItem->Name(i)) == 0)
            {
               return (pItem);
            }
         }
      }
   }
   return (pItem);
}  // NamedItem::Find

//---------------------------------------------------------------------------

int NamedItem::FindPartial(cchar *pszMatch, NamedItem *pHead, TStrings *pStrs,
                bool *pbExact)
{
int iLen = strlen(pszMatch);
int iRtn = 0;
   if (pStrs) pStrs->Clear();
   for (NamedItem *pItem = pHead; pItem; pItem = pItem->pNext)
   {
      for (int i = 0; i < pItem->pNames->Count; ++i)
      {
         for (cchar *psz = pItem->Name(i); *psz; ++psz)
         {
            if (strnicmp(psz, pszMatch, iLen) == 0)
            {
               if (pbExact)
               {
                  *pbExact = (stricmp(psz, pszMatch) == 0);  // exact match
               }
               if (pStrs)
               {
                  pStrs->Add(pItem->Name());
               }
               ++iRtn;
               break;
            }
         }
      }
   }
   return (iRtn);
}  // NamedItem::FindPartial

//---------------------------------------------------------------------------

bool NamedItem::Sort(NamedItem *&pHead, uint nOffset, ElementType et,
                        SortMethod sm)
{
   if ((pHead == NULL) || (sm == smNONE))
      return (false);

bool bA2Z = (sm == smA2Z);
NamedItem *pScan = pHead->pNext;
   pHead->pNext = NULL;
   while (pScan)
   {
   NamedItem *pIns = pScan;
      pScan = pScan->pNext;

   NamedItem **ppLast = &pHead;
      for (NamedItem *pItem = pHead; pItem; pItem = pItem->pNext)
      {
      bool bIns = false;
      byte *paby1 = ((byte*)pItem  + nOffset);
      byte *paby2 = ((byte*)pIns + nOffset);
         if (et > etUINT)
         {
         int idx = et - etUINT;
            bIns = (((uint*)paby1)[idx] < ((uint*)paby2)[idx]);
         }
         else switch (et)
         {
         case etNAME:
            bIns = (stricmp(pItem->Name(), pIns->Name()) < 0);
            break;

         case etSTR:
            bIns = (stricmp((cchar*)paby1, (cchar*)paby2) < 0);
            break;

         case etUINT:
            break;
         }

         if (bIns ^ bA2Z)
         {
            *ppLast = pIns;
            pIns->pNext = pItem;
            pIns = NULL;
            break;
         }
         ppLast = &pItem->pNext;
      }
      if (pIns)
      {
         pIns->pNext = NULL;
         *ppLast = pIns;
      }
   }
   return (true);
}  // NamedItem::Sort

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

__fastcall TBaseForm::TBaseForm(TComponent* Owner, cchar *pszName,
                           ColCfg *(*pfnBuildColCfg)(uint idx)) : TForm(Owner)
{
   pfnBuildCfg = pfnBuildColCfg;

   ZeroClassSkip(4, &colCfg);
   for (int idx = ColCfg::oriHDR; idx < ColCfg::oriMAX; ++idx)
   {
      colCfg.aaClr[idx][ColCfg::ostBG] = (idx == ColCfg::oriHDR) ?
                                            clrRGB(0xC0,0xE0,0xC0) :
                                            clrRGB(0xFF,0xFF,0xFF);
      colCfg.aaClr[idx][ColCfg::ostLN] = clrRGB(0xFF,0xFF,0xFF);
      colCfg.apFont[idx] = new TFont;
   }
   colCfg.nWidth = 10;
   pCurrSet = NULL;

   bOwnedColSet = false;
   bUpdOnResize = true;


TStringList *pStrs = new TStringList;

IniFile ini;
char sz[256];
   strncpy(szGridName, pszName, sizeof(szGridName)-1);

char szFmt[64];
int idxDef = 0;
   sprintf(szFmt, "%s.%%[^\n]", szGridName);
   ini.GetSectList(pStrs);
   cbGridFmt->Clear();
   for (int i = 0; i < pStrs->Count; ++i)
   {
      if (sscanf(pStrs->Strings[i].c_str(), szFmt, sz) == 1)
      {
      TStringList *pStrs2 = new TStringList;
         ini.GetSectVals(pStrs->Strings[i].c_str(), pStrs2);
      ColSet *pSet = new ColSet(pStrs2, sz, colCfg, pfnBuildCfg);
      int idx = cbGridFmt->Items->AddObject(sz, (TObject*)pSet);
         delete pStrs2;
         if (strcmp(sz, "Default") == 0)
         {
            idxDef = idx;
         }
      }
   }
   cbGridFmt->ItemIndex = idxDef;
   if (idxDef < cbGridFmt->Items->Count)
   {
      SetupGrid((ColSet*)cbGridFmt->Items->Objects[idxDef]);
   }

   lblMain->Left = 4;
   lblMain->Top = 4;
   lblAlias->Height = cbGridFmt->Height;

   AlignFrom(cbMain);

   pHeadSort = NULL;

   cbEdit->Clear();
   cbEdit->DropDownCount = 7;
   cbEdit->Items->AddObject("Cancel", (TObject*)eoCANCEL);
   cbEdit->Items->AddObject("Add Column Before", (TObject*)eoADDB);
   cbEdit->Items->AddObject("Add Column After", (TObject*)eoADDA);
   cbEdit->Items->AddObject("Delete Column", (TObject*)eoDEL);
   cbEdit->Items->AddObject("Edit Column Data", (TObject*)eoEDIT);
   cbEdit->Items->AddObject("Adjust Style", (TObject*)eoSTYLE);
   cbEdit->Items->AddObject("Save Config", (TObject*)eoSAVE);
   cbEdit->Style = csDropDownList;
   cbEdit->ItemIndex = 0;

   pCurrItem = NULL;
   ppHeadItem = &pCurrItem;

   delete pStrs;
}  // TBaseForm::TBaseForm

//---------------------------------------------------------------------------

int TBaseForm::Accept(int iFlags)
{
int iRtn = 0;
   if (btnOK->Tag > 0)
   {
      iRtn = Application->MessageBoxA("Changes",
                "Discard changes to form?", iFlags);
   }
   return (iRtn);
}  // TBaseForm::Accept

//---------------------------------------------------------------------------

int TBaseForm::AcceptBase(void)
{
int iRtn = 0;
char sz[256];
   if (pCurrSet && (pCurrSet->dwChanged != 0))
   {
      sprintf(sz, "Do you wish to Save changed to GridView (%lx)?",
                        pCurrSet->dwChanged);
      iRtn = Application->MessageBoxA(sz, "Save View", MB_YESNOCANCEL);
      if (iRtn == IDYES)
      {
         SaveColSet();
         iRtn = 0;
      }
      else if (iRtn == IDNO)
      {
         iRtn = 0;
      }
   }
   return (iRtn);
}  // TBaseForm::AcceptBase

//---------------------------------------------------------------------------

void TBaseForm::AddSortMatrix(TControl *pCtl, uint nOffset,
                        NamedItem::ElementType et)
{
SortMatrix *pSort = new SortMatrix;
   ZeroMem(pSort);
   pSort->pNext = pHeadSort;
   pHeadSort = pSort;
   pSort->pCtl = pCtl;
   pSort->nOff = nOffset;
   pSort->et = et;
   ((TLabel*)pCtl)->OnClick = SortLabel;
   pCtl->Tag = NamedItem::smNONE;       // no sort
}  // TBaseForm::AddSortMatrix

//---------------------------------------------------------------------------

void TBaseForm::AlignFrom(TControl *pCtl)
{
bool bRest = false;
ControlAlignment ca;
   if (bRest || (bRest |= (pCtl == cbMain)))
   {
      ca.Align(cbMain,    lblMain,   ca.mBLW | ca.mLAL);
   }
   if (bRest || (bRest |= (pCtl == lblAction)))
   {
      ca.Align(lblAction,    cbMain,   ca.mABV | ca.mRAR);
   }
   if (bRest || (bRest |= (pCtl == lblAlias)))
   {
      ca.Align(lblAlias,  cbMain,    ca.mBLW | ca.mLAL);
   }
   if (bRest || (bRest |= (pCtl == memAlias)))
   {
      ca.Align(memAlias,  lblAlias,  ca.mBLW | ca.mLAL);
   }
   if (bRest || (bRest |= (pCtl == grid)))
   {
      ca.Align(grid,   memAlias,  ca.mTOP | ca.mROF);
      grid->Height = ClientHeight - grid->Top - 4;
   }
   if (bRest || (bRest |= (pCtl == cbGridFmt)))
   {
      ca.Align(cbGridFmt, grid, ca.mABV | ca.mLAL);
   }
   if (bRest || (bRest |= (pCtl == btnOK)))
   {
      ca.Align(btnOK,     memAlias,  ca.mBLW | ca.mLAL);
   }
   if (bRest || (bRest |= (pCtl == btnCancel)))
   {
      ca.Align(btnCancel, btnOK,     ca.mTOP | ca.mROF);
   }
   if (bRest || (bRest |= (pCtl == btnDelete)))
   {
      ca.Align(btnDelete, btnOK,     ca.mBLW | ca.mLAL);
      btnDelete->Enabled = false;
      btnDelete->OnClick = CurrItemDel;
   }
   if (bRest || (bRest |= (pCtl == btnRename)))
   {
      ca.Align(btnRename, btnDelete, ca.mTOP | ca.mROF);
      btnRename->Enabled = false;
      btnRename->OnClick = CurrItemRen;
   }
   if (bRest || (bRest |= (pCtl == btnPrev)))
   {
      ca.Align(btnPrev,   btnDelete, ca.mBLW | ca.mLAL);
   }
   if (bRest || (bRest |= (pCtl == btnNext)))
   {
      ca.Align(btnNext,   btnPrev,   ca.mTOP | ca.mROF);
   }
}  // TBaseForm::AlignFrom

//---------------------------------------------------------------------------

void TBaseForm::AssignColCfg(ColCfg *pColCfg, uint idxCol)
{
   grid->Objects[idxCol][0] = (TObject*)pColCfg;
   if (pColCfg)
   {
   char szHdr[128];
      for (char *pszS = pColCfg->szHeader, *pszD = szHdr; *pszS; ++pszS)
      {
         if (strncmp(pszS, "<auto>", 6) == 0)
         {
         TStringList *pStrs = new TStringList;
            FieldList(pStrs);
         int idxStr = FindField(pStrs, pColCfg->dwFlds);
            if (idxStr >= 0)
            {
               pszD += sprintf(pszD, pStrs->Strings[idxStr].c_str());
               pszS = strchr(pszS, '>');
            }
            delete pStrs;
         }
         else
         {
            *pszD++ = *pszS;
            *pszD = '\0';
         }
      }
      grid->Cells[idxCol][0] = szHdr;
      grid->ColWidths[idxCol] = pColCfg->nWidth;
   }
}  // TBaseForm::AssignColCfg

//---------------------------------------------------------------------------

uint TBaseForm::AssignRowObj(void *pvObj, uint idxRow, bool bInc)
{
   if (idxRow >= (uint)grid->RowCount)
   {
      grid->RowCount = idxRow+1;
   }
RowParam rp;
   rp.nID = idxRow;
   Set(rp, idxRow);
   grid->Objects[1][idxRow] = (TObject*)pvObj;
   if (bInc)
   {
      if (++idxRow >= (uint)grid->RowCount)
      {
         grid->RowCount = idxRow+1;
      }
   }
   return (idxRow);
}  // TBaseForm::AssignRowObj

//---------------------------------------------------------------------------

void __fastcall TBaseForm::btnNextClick(TObject *Sender)
{
   unused(Sender);
   if (Accept(MB_YESNOCANCEL) != IDCANCEL)  // changes pending
   {
      pCurrItem = pCurrItem->pNext;
      UpdateForm();
   }
}  // TBaseForm::btnNextClick

//---------------------------------------------------------------------------

void __fastcall TBaseForm::btnPrevClick(TObject *Sender)
{
   unused(Sender);
   if (Accept(MB_YESNOCANCEL) != IDCANCEL)
   {
      for (NamedItem *pItem = *ppHeadItem; pItem; pItem = pItem->pNext)
      {
         if (pItem->pNext == pCurrItem)
         {
            pCurrItem = pItem;
            break;
         }
      }
      UpdateForm();
   }
}  // TBaseForm::btnPrevClick

//---------------------------------------------------------------------------

void __fastcall TBaseForm::cbAttribExit(TObject *Sender)
{
TComboBox *pCB = dynamic_cast<TComboBox*>(Sender);
   if (pCB)
   {
      pCB->Visible = false;
      pCB->ItemIndex = 0;
   }
}  // TBaseForm::cbAttribExit

//---------------------------------------------------------------------------

void __fastcall TBaseForm::cbEditSelect(TObject *Sender)
{
   unused(Sender);
ControlAlignment ca;
EditOption eo = (EditOption)cbEdit->Items->Objects[cbEdit->ItemIndex];
   cbEdit->Visible = false;
ColCfg *pCfg = GetColCfg(idxCurrCol);
ColCfg **ppCfg = &pCurrSet->pHeadCfg;
   for ( ; *ppCfg && (*ppCfg != pCfg); ppCfg = &(*ppCfg)->pNext) { }

   if (eo == eoCANCEL)
   {
      return;
   }
   else if (eo == eoDEL)
   {
      *ppCfg = pCfg->pNext;
      delete pCfg;
      for (int idxCol = idxCurrCol + 1; idxCol < grid->ColCount; ++idxCol)
      {
         AssignColCfg(GetColCfg(idxCol),idxCol-1);
      }
      --pCurrSet->nCols;
      --grid->ColCount;
      UpdateSet(0x0001);
   }
   else if (eo == eoSAVE)
   {
      SaveColSet();
   }
   else if ((eo == eoADDA) || (eo == eoADDB) || (eo == eoEDIT))
   {
   TTypeForm::QueryOptions qo;
      Zero(qo);
      qo.m = ca.mBLW;
   TStringList *pStrs = new TStringList;
   bool bEd = (eo == eoEDIT);

      TypeForm->Reset(bEd ? "Edit Column" : "Add Column",
                "Please Select the following data for the column");

      qo.iWidth = -12;
   int idName = TypeForm->AddEdit("Name", qo, bEd ? pCfg->szHeader : "<auto>");

      pStrs->Clear();
      FieldList(pStrs);
   uint idx = (bEd) ? FindField(pStrs, pCfg->dwFlds) : 0;
      Zero(qo);
      qo.m = ca.mTOP | ca.mROF;
   int idFld = TypeForm->AddComboBox(pStrs, qo, idx);

   char sz[128];
      sprintf(sz, "%u", bEd ? pCfg->anGrp[1] : 0);
      Zero(qo);
      qo.iWidth = 50;
      qo.m = ca.mTOP | ca.mROF;
   int idGrp = TypeForm->AddEdit("Group", qo, sz);
   void *pvEditData = EditColCfg(pCfg, eo, NULL);
      if (pCfg && TypeForm->Query())
      {
      int iRtn;

         if ((eo == eoADDA) || (eo == eoADDB))
         {
            grid->ColCount += 1;
            if (eo == eoADDA)   // need to advance pointers
            {
               ++idxCurrCol;
               ppCfg = &pCfg->pNext;
            }
            for (int ii= grid->ColCount - 1; ii > idxCurrCol; --ii)
            {
               AssignColCfg(GetColCfg(ii-1), ii);
            }
//            pCfg = new ColCfg(colCfg);
            pCfg = pfnBuildCfg(1);
            *pCfg = colCfg;
            pCfg->nWidth = 50;
            pCfg->pNext = *ppCfg;
            *ppCfg = pCfg;
            ++pCurrSet->nCols;
         }

         TypeForm->GetValue(idName, pCfg->szHeader, iRtn);
         TypeForm->GetValue(idFld, NULL, iRtn);
         if ((0 <= iRtn) && (iRtn < pStrs->Count))
         {
            pCfg->dwFlds &= ~Nexus::fldMSK;
            pCfg->dwFlds |= (uint)pStrs->Objects[iRtn];
         }
         pCfg->anGrp[2] = pCfg->dwFlds & Nexus::fldMSK;
         TypeForm->GetValue(idGrp, NULL, iRtn);
         pCfg->anGrp[1] = iRtn;
         EditColCfg(pCfg, eo, pvEditData);
         AssignColCfg(pCfg, idxCurrCol);
         UpdateSet(0x0002);
      }
      else
      {
         EditColCfg(NULL, eo, pvEditData);      // allow cleanup
      }
   }
   else if (eo == eoSTYLE)
   {
      GetAttribs();
   }
   UpdateForm();
}  // TBaseForm::cbEditSelect

//---------------------------------------------------------------------------

void __fastcall TBaseForm::cbGridFmtSelect(TObject *Sender)
{
   unused(Sender);
TComboBox *pCB = dynamic_cast<TComboBox*>(Sender);
   if ((pCB == NULL) || (pCB->ItemIndex < 0))
      return;

int iType = pCB->ItemIndex;
ColSet *pSet = (iType >= 0) ? (ColSet*)pCB->Items->Objects[iType] : NULL;
   if ((pSet != pCurrSet) && (AcceptBase() != 0))
   {
      pSet = NULL;
   }
   if (pSet)
   {
      SetupGrid(pSet);
   }
   UpdateForm();
}  // TBaseForm::cbGridFmtSelect

//---------------------------------------------------------------------------

void __fastcall TBaseForm::cbMainChange(TObject *Sender)
{
TComboBox *pCB = dynamic_cast<TComboBox*>(Sender);
   if (!pCB)
      return;

int iSel = pCB->SelStart;
int iLen = pCB->SelLength;
   if (iSel != 0)
   {
   char sz[256], *psz;
      strncpy(sz, pCB->Text.c_str(), sizeof(sz) -1);
      sz[iSel] = '\0';
   bool bExact;
   int iFound = NamedItem::FindPartial(sz, *ppHeadItem, pCB->Items, &bExact);

      strncpy(sz, lblMain->Caption.c_str(), sizeof(sz)-1);
      if ((psz = strstr(sz, " (")) == NULL)
      {
         psz = strchr(sz, '\0');        // EOS
      }
      psz += sprintf(psz, " (match: %d)", iFound);
      if (pCurrItem)
      {
         sprintf(psz, "  displaying '%s'", pCurrItem->Name());
      }
      lblMain->Caption = sz;
      btnRename->Enabled = !bExact;
      bChangeName = true;
      UpdateForm();
   }
   pCB->SelStart = iSel;
   pCB->SelLength = iLen;
}  // TBaseForm::cbMainChange

//---------------------------------------------------------------------------

void __fastcall TBaseForm::cbMainExit(TObject *Sender)
{
TComboBox *pCB = dynamic_cast<TComboBox*>(Sender);
   if (!pCB)
      return;

char sz[256], *psz;
   strncpy(sz, pCB->Text.c_str(), sizeof(sz)-1);
int iFound = NamedItem::FindPartial(sz, *ppHeadItem);
   bChangeName = true;
   if (iFound > 0)
   {
   NamedItem *pItem = NamedItem::Find(sz, *ppHeadItem);
      if (pItem)
      {
         pCurrItem = pItem;
         bChangeName = false;
      }
   }
   UpdateForm();

   strncpy(sz, lblMain->Caption.c_str(), sizeof(sz)-1);
   if ((psz = strstr(sz, " (")) != NULL)
   {
      *psz = '\0';
   }
   lblMain->Caption = sz;
}  // TBaseForm::cbMainExit

//---------------------------------------------------------------------------

void __fastcall TBaseForm::cbMainKeyPress(TObject *Sender, char &Key)
{
TComboBox *pCB = dynamic_cast<TComboBox*>(Sender);
   if ((Key == '\r') && pCB)
   {
      pCB->OnExit(Sender);
      Key = 0;
   }
}  // TBaseForm::cbMainKeyPress

//---------------------------------------------------------------------------

void TBaseForm::CntReport(int idx)
{
char sz[128], *psz;
   if (idx >= 0) ++anCnt[idx];
   psz = sz;
   for (int i = 0; i < entries(anCnt); ++i)
   {
      if (anCnt[i] > 0)
      {
         psz += sprintf(psz, "C%u:%u ", i, anCnt[i]);
      }
   }
   lblAction->Caption = sz;
}  // TBaseForm::CntReport

//---------------------------------------------------------------------------

TBaseForm::ColCfg::ColCfg(void)
{
   ZeroClass(&pNext, this);
}  // TBaseForm::ColCfg::ColCfg

//---------------------------------------------------------------------------

TBaseForm::ColCfg::~ColCfg()
{
   for (int i = 0; i < oriMAX; ++i)
   {
      if (apFont[i])
      {
         delete apFont[i];
         apFont[i] = NULL;
      }
   }
}  // TBaseForm::ColCfg::~ColCfg

//---------------------------------------------------------------------------

TBaseForm::ColCfg& TBaseForm::ColCfg::operator=(const TBaseForm::ColCfg &cc)
{
#define VT 4
   if (this != &cc)
   {
      memcpy(&aaClr, &cc.aaClr, sizeof(cc) - ((char*)&cc.aaClr - (char*)&cc));
      for (int i = 0; i < oriMAX; ++i)
      {
         if (apFont[i] == NULL)
         {
            apFont[i] = new TFont;
         }
         apFont[i]->Assign(cc.apFont[i]);
      }
   }
   return (*this);
}  // TBaseForm::ColCfg::operator=

//---------------------------------------------------------------------------

TBaseForm::ColCfg *TBaseForm::ColCfg::Build(uint idx)
{
   unused(idx);
   return (new ColCfg());
}  // TBaseForm::ColCfg::Build

//---------------------------------------------------------------------------

cchar *TBaseForm::ColCfg::Parse(cchar *pszBase)
{
cchar *psz;
uint n;
int idx;
cchar *pszRtn = NULL;   // assume it didn't decode

   if (sscanf(pszBase, "%u:%[^=]=%u;%n", &idx, szHeader, &nWidth, &n) == 3)
   {
      pszBase += n;
      dwFlds = Nexus::FieldDecode(pszBase); // pszBase moved to end of field
      pszRtn = pszBase;
   }

   psz = pszBase;
   while ((psz = strstr(psz, ";clr")) != NULL)
   {
   char szKey[32];
      sscanf(++psz, "clr%31[^(]", szKey);
   int iClr = oriMAX;
   int ost = ostBG;

           if (strcmp(szKey, "HDR")  == 0)  {  iClr = oriHDR;  ost = ostBG;  }
      else if (strcmp(szKey, "HOUT") == 0)  {  iClr = oriHDR;  ost = ostLN;  }
      else if (strcmp(szKey, "BDY")  == 0)  {  iClr = oriBDY1; ost = ostBG;  }
      else if (strcmp(szKey, "BOUT") == 0)  {  iClr = oriBDY1; ost = ostLN;  }
      else if (sscanf(szKey, "BG%u", &iClr) == 1)  {  ost = ostBG;  }
      else if (sscanf(szKey, "LN%u", &iClr) == 1)  {  ost = ostLN;  }

      if ((oriHDR <= iClr) && (iClr < oriMAX))
      {
         ColorDecode(psz, aaClr[iClr][ost]);
      }
      else
      {
         Printf("Unknown color 'clr%s'", szKey);
      }
   }

   psz = pszBase;
   while ((psz = strstr(psz, ";fnt")) != NULL)
   {
   char szKey[32], szFont[64];
      ++psz;
      if (sscanf(psz, "fnt%15[^(](%63[^)])", szKey, szFont) == 2)
      {
      int iFnt = oriMAX;

              if (stricmp(szKey, "HDR") == 0) iFnt = oriHDR;
         else if (stricmp(szKey, "BDY") == 0) iFnt = oriBDY1;
         else if (sscanf(szKey, "%u", &iFnt) == 1);

         if ((oriHDR <= iFnt) && (iFnt < oriMAX))
         {
            FontDecode(szFont, apFont[iFnt]);
         }
         else
         {
            Printf("Unknown font 'fnt%s'", szKey);
         }
      }
   }

   Zero(anGrp);
   if ((psz = strstr((char*)pszBase, ";grp:")) != NULL)
   {
      sscanf(psz, ";grp:%u", &anGrp[1]);
   }

   return (pszRtn);
}  // TBaseForm::ColCfg::Parse

//---------------------------------------------------------------------------

char *TBaseForm::ColCfg::Print(char *psz, ColCfg &ccRef, cchar *pszIns)
{
   psz += sprintf(psz, "%u;", nWidth);

Nexus::Field fld = (Nexus::Field)(dwFlds & Nexus::fldMSK);
   Nexus::FieldEncode(fld, psz);

   if (pszIns)
   {
      psz += sprintf(psz, "%s", pszIns);
   }
   for (int i = oriHDR; i < oriMAX; ++i)
   {
   char sz[16];
      sprintf(sz, "BG%u", i);
      ColorAppend(psz, sz, aaClr[i][ostBG], ccRef.aaClr[i][ostBG]);
      sprintf(sz, "LN%u", i);
      ColorAppend(psz, sz, aaClr[i][ostLN], ccRef.aaClr[i][ostLN]);
      sprintf(sz, "%u", i);
      FontAppend(psz,  sz, apFont[i],       ccRef.apFont[i]);
   }

   if (anGrp[1] != 0)
   {
      psz += sprintf(psz, ";grp:%u", anGrp[1]);
   }
   return (psz);
}  // TBaseForm::ColCfg::Print

//---------------------------------------------------------------------------

bool TBaseForm::ColorAppend(char *&psz, cchar *pszCode,
                TColor clr, TColor clrFrom)
{
   if (clrFrom != clr)
   {
   LongWordByte lwb;
      lwb.li = clr;
      psz +=  sprintf((char*)psz, ";clr%s(%02X%02X%02X)", pszCode,
                lwb.aby[0], lwb.aby[1], lwb.aby[2]);
   }
   return (true);
}  // TBaseForm::ColorAppend

//---------------------------------------------------------------------------

bool TBaseForm::ColorDecode(cchar *pszColor, TColor &clr)
{
LongWordByte lwb;
char *psz = strchr((char*)pszColor, '(');
   if (sscanf(psz, "(%06X)", &lwb.li) == 1)
   {
      clr = clrRGB(lwb.aby[2], lwb.aby[1], lwb.aby[0]);
      return (true);
   }
   return (false);
}  // TBaseForm::ColorDecode

//---------------------------------------------------------------------------

TBaseForm::ColSet::ColSet(TStringList *pStrs, cchar *pszTitle, ColCfg &ccDef,
        ColCfg *(*pfnBuild)(uint idx))
{
   ZeroClassFrom(szTitle, this);
   strncpy(szTitle, pszTitle, sizeof(szTitle)-1);
ColCfg **ppCfg = &pHeadCfg;
   for (int idx = 0; idx < pStrs->Count; ++idx)
   {
   ColCfg *pCfg = pfnBuild(idx);
      *pCfg = ccDef;
      if (pCfg->Parse(pStrs->Strings[idx].c_str()))
      {
         ++nCols;
         *ppCfg = pCfg;
         ppCfg = &pCfg->pNext;
      }
      else
      {
         delete pCfg;
      }
   }
}  // TBaseForm::ColSet::ColSet

//---------------------------------------------------------------------------

TBaseForm::ColSet::~ColSet()
{
   while (pHeadCfg)
   {
   ColCfg *pCfg = pHeadCfg;
      pHeadCfg = pCfg->pNext;
      delete pCfg;
   }
}  // TBaseForm::ColSet::~ColSet

//---------------------------------------------------------------------------

void __fastcall TBaseForm::CurrItemDel(TObject *Sender)
{
   unused(Sender);
char sz[1024];

   ModalResult = mrNone;
   if (pCurrItem != NULL)
   {
      sprintf(sz, "Do you really wish to DELETE '%s' from the movie database?",
                cbMain->Text.c_str());
      if (Application->MessageBoxA(sz, "Delete", MB_YESNO) == IDYES)
      {
      NamedItem *pItem, **ppTail = ppHeadItem;
         for (pItem = *ppHeadItem; pItem; pItem = pItem->pNext)
         {
            if (pItem == pCurrItem)
            {
               *ppTail = pItem->pNext;
               break;
            }
            ppTail = &pItem->pNext;
         }
         delete pCurrItem;
         pCurrItem = (*ppTail) ? *ppTail : *ppHeadItem;
         UpdateForm();
      }
   }
}  // TBaseForm::CurrItemDel

//---------------------------------------------------------------------------

void __fastcall TBaseForm::CurrItemRen(TObject *Sender)
{
   unused(Sender);
char sz[1024];

   if (pCurrItem != NULL)
   {
      sprintf(sz, "Do you wish to RENAME '%s' to '%s' in the movie database?",
                pCurrItem->Name(), cbMain->Text.c_str());
      if (Application->MessageBoxA(sz, "Rename", MB_YESNO) == IDYES)
      {
         pCurrItem->pNames->Strings[0] = cbMain->Text;
      }
      UpdateForm();
   }
}  // TBaseForm::CurrItemRen

//---------------------------------------------------------------------------

void __fastcall TBaseForm::DetectChange(TObject *Sender)
{
   unused(Sender);
   btnOK->Tag |= 1;
   btnOK->Caption = bChangeName ? "Add" : "Accept";
}  // TBaseForm::DetectChange

//---------------------------------------------------------------------------

void *TBaseForm::EditColCfg(ColCfg *pCfg, EditOption eo, void *pvData)
{
   unused(pCfg);
   unused(eo);
   unused(pvData);
   return (NULL);
}  // TBaseForm::EditColCfg

//---------------------------------------------------------------------------

dword TBaseForm::FieldDecode(cchar *&psz)
{
   return (Nexus::FieldDecode(psz));
}  // TBaseForm::FieldDecode

//---------------------------------------------------------------------------

void TBaseForm::FieldEncode(dword dwFlds, char *&psz)
{
   Nexus::FieldEncode(dwFlds, psz);
}  // TBaseForm::FieldEncode

//---------------------------------------------------------------------------

void TBaseForm::FieldList(TStringList *pStrs)
{
   Nexus::FieldList(pStrs, Nexus::fsetFILM | Nexus::fsetROLE);
}  // TBaseForm::FieldList

//---------------------------------------------------------------------------

int TBaseForm::FindField(TStringList *pStrs, DWord dwFld)
{
TObject *pObjFld = (TObject*)(dwFld & Nexus::fldMSK);
   for (int ii = 0; ii < pStrs->Count; ++ii)
   {
      if (pStrs->Objects[ii] == pObjFld)
      {
         return (ii);
      }
   }
   return (-1);
}  // TBaseForm::FindField

//---------------------------------------------------------------------------

bool TBaseForm::FontAppend(char *&psz, cchar *pszCode,
                TFont *pFnt, TFont *pFntRef)
{
   if (!pFntRef || (pFntRef->Name != pFnt->Name) ||
       (pFntRef->Size != pFnt->Size) ||
       (pFntRef->Style != pFnt->Style) ||
       (pFntRef->Color != pFnt->Color))
   {
      psz +=  sprintf(psz, ";fnt%s(%s:%u", pszCode, pFnt->Name.c_str(), pFnt->Size);
      if (pFnt->Style.Contains(fsBold))       {  psz += sprintf(psz, "+B");  }
      if (pFnt->Style.Contains(fsItalic))     {  psz += sprintf(psz, "+I");  }
      if (pFnt->Style.Contains(fsUnderline))  {  psz += sprintf(psz, "+U");  }
      if (pFnt->Style.Contains(fsStrikeOut))  {  psz += sprintf(psz, "+S");  }
   LongWordByte lwb;
      lwb.li = pFnt->Color;
      psz +=  sprintf(psz, ",%02x%02x%02x", lwb.aby[0], lwb.aby[1], lwb.aby[2]);

      psz += sprintf(psz, ")");
   }
   return (true);
}  // TBaseForm::FontAppend

//---------------------------------------------------------------------------

bool TBaseForm::FontDecode(cchar *pszFont, TFont *pFont)
{
char szName[64];
uint n = 0;
int iSz = 0;
   sscanf(pszFont, "%[^:]:%d%n", szName, &iSz, &n);
   pszFont += n;
   pFont->Name = szName;
   pFont->Size = iSz;
   pFont->Style.Clear();
   for ( ; *pszFont; pszFont += 2)
   {
   int iR, iB, iG;
           if (strnicmp(pszFont, "+B", 2) == 0)  pFont->Style << fsBold;
      else if (strnicmp(pszFont, "+I", 2) == 0)  pFont->Style << fsItalic;
      else if (strnicmp(pszFont, "+U", 2) == 0)  pFont->Style << fsUnderline;
      else if (strnicmp(pszFont, "+S", 2) == 0)  pFont->Style << fsStrikeOut;
      else if (sscanf(pszFont, ",%02X%02X%02X", &iR, &iB, &iG) == 3)
      {
         pszFont += 5;
         pFont->Color = clrRGB(iR, iB, iG);
      }
      else
      {
         break;
      }
   }
   return (true);
}  // TBaseForm::FontDecode

//---------------------------------------------------------------------------

void __fastcall TBaseForm::FormClose(TObject *Sender, TCloseAction &Action)
{
   unused(Sender);
   if (AcceptBase() != 0)
   {
      Action = caNone;
   }
}  // TBaseForm::FormClose

//---------------------------------------------------------------------------

void __fastcall TBaseForm::FormDestroy(TObject *Sender)
{
   unused(Sender);
   for (int i = 0; i < cbGridFmt->Items->Count; ++i)
   {
   ColSet *pSet = (ColSet*)cbGridFmt->Items->Objects[i];
      delete pSet;
   }
}  // TBaseForm::FormDestroy

//---------------------------------------------------------------------------

void __fastcall TBaseForm::FormMouseWheel(TObject *Sender,
      TShiftState Shift, int WheelDelta, TPoint &MousePos, bool &Handled)
{
   unused(Sender);
   unused(Shift);
   if (WithIn(grid, MousePos))
   {
   int iNew = grid->TopRow + ((WheelDelta > 0) ? +1 : -1);
      if ((0 < iNew) && (iNew < grid->RowCount))
         grid->TopRow = iNew;
      Handled = true;
   }
}  // TBaseForm::FormMouseWheel

//---------------------------------------------------------------------------

void __fastcall TBaseForm::FormResize(TObject *Sender)
{
   unused(Sender);
   if (bUpdOnResize)
   {
      UpdateForm();
   }
}  // TBaseForm::FormResize

//---------------------------------------------------------------------------

void __fastcall TBaseForm::FormShow(TObject *Sender)
{
   unused(Sender);
   UpdateForm();
}  // TBaseForm::FormShow

//---------------------------------------------------------------------------

void TBaseForm::GetAttribs(void)
{
TStringList *pStrs = new TStringList;
ControlAlignment ca;
   pCurrColCfg = GetColCfg(idxCurrCol);
   TypeForm->Reset("Adjust Style",
                "Please Select the following style data");
TTypeForm::QueryOptions qo;
   Zero(qo);

   pStrs->Clear();
   pStrs->AddObject("Column", (TObject*)SEL_ONE);
   pStrs->AddObject("Global", (TObject*)SEL_GBL);
   pStrs->AddObject("Index", (TObject*)SEL_IDX);
   pStrs->AddObject("Group", (TObject*)SEL_GRP);
   pStrs->AddObject("Type", (TObject*)SEL_TYP);
   qo.m = ca.mBLW | ca.mLAL;
   qo.lwbPrm.aby[2] = qo.tRBTN_D;
int idScope = TypeForm->AddGroup(pStrs, "Scope", qo, 0);

   pStrs->Clear();
   for (int ii = ColCfg::oriHDR; ii < ColCfg::oriMAX; ++ii)
   {
   char sz[32];
   int iSel = ii << ORI_SHIFT;;
      sprintf(sz, (ii == ColCfg::oriHDR) ? "Header Font" : "Body%u Font", ii);
      pStrs->AddObject(sz, (TObject*)(SEL_FONT | iSel));
      pStrs->AddObject("BG Color", (TObject*)(ColCfg::ostBG | iSel));
      pStrs->AddObject("Line Color", (TObject*)(ColCfg::ostLN | iSel));
   }

   qo.m = ca.mTOP | ca.mROF;
   qo.lwbPrm.aby[0] = ColCfg::ostMAX + 1;
   qo.lwbPrm.aby[2] = qo.tRBTN_D;
int idAttrib = TypeForm->AddGroup(pStrs, "Attribute", qo, 0);

   if (TypeForm->Query())
   {
   int iScope, iAttrib;
      TypeForm->GetValue(idScope, NULL, iScope);
      TypeForm->GetValue(idAttrib, NULL, iAttrib);

   ColCfg::ObjRegionIndex ori = (ColCfg::ObjRegionIndex)((iAttrib & ORI_MASK) >> ORI_SHIFT);

   uint idxClr = iAttrib & 0x0F;
   int iGrp = ((iScope & 0xF00) >> 8) - 2;
   uint nGrpVal = (iGrp < 0) ? 0 : pCurrColCfg->anGrp[iGrp];

      if (idxClr == SEL_FONT)
      {
         FontDialogQF->Font = pCurrColCfg->apFont[ori];
         if (FontDialogQF->Execute())
         {
            UpdateSet(0x0010);
            pCurrColCfg->apFont[ori]->Assign(FontDialogQF->Font);
            if (iGrp == -2)        // one only
            {
               pCurrColCfg->apFont[ori]->Assign(FontDialogQF->Font);
            }
            else
            {
               for (int i = 0; i < grid->ColCount; ++i)
               {
               ColCfg *pCfg = GetColCfg(i);
                  if ((iGrp == -1) || (pCfg->anGrp[iGrp] == nGrpVal))
                  {
                     pCfg->apFont[ori]->Assign(FontDialogQF->Font);
                  }
               }
            }
         }
      }
      else if (idxClr < ColCfg::ostMAX)
      {
         ColorDialogQF->Color = pCurrColCfg->aaClr[ori][idxClr];
         if (ColorDialogQF->Execute())
         {
         TColor clr = ColorDialogQF->Color;
            UpdateSet(0x0020);
            if (iGrp == -2)        // one only
            {
               pCurrColCfg->aaClr[ori][idxClr] = clr;
            }
            else
            {
               for (int i = 0; i < grid->ColCount; ++i)
               {
               ColCfg *pCfg = GetColCfg(i);
                  if ((iGrp == -1) || (pCfg->anGrp[iGrp] == nGrpVal))
                  {
                     pCfg->aaClr[ori][idxClr] = clr;
                  }
               }
            }
         }
      }
   }
}  // TBaseForm::GetAttribs

//---------------------------------------------------------------------------

static inline TColor ColorShift(TColor clr, bool bDim)
{
   return (bDim ? (TColor)((clr & 0xFEFEFEFEL) >> 1) : clr);
}  // ColorShift

//---------------------------------------------------------------------------

void __fastcall TBaseForm::GridDrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
atms[0].Start();

   unused(State);
char sz[256];
TStringGrid *pGrid = dynamic_cast<TStringGrid*>(Sender);
  if (!pGrid)
      return;
ColCfg *pCfg = GetColCfg(ACol);
   if (pCfg == NULL)
   {
      pCfg = &colCfg;
   }
bool bHdr = (ARow == 0);
ColCfg::ObjRegionIndex ori  = (bHdr ? ColCfg::oriHDR : ColCfg::oriBDY1);
   if (!bHdr)
   {
   RowParam rp;
      Get(rp, ARow);
      if ((ColCfg::oriHDR < rp.nGrp) && (rp.nGrp < ColCfg::oriMAX))
      {
         ori = (ColCfg::ObjRegionIndex)rp.nGrp;
      }
   }
bool bDim = (!pGrid->Enabled);

   strncpy(sz, pGrid->Cells[ACol][ARow].c_str(), sizeof(sz)-1);
TCanvas *pCnvs = pGrid->Canvas;
   pCnvs->Brush->Color = ColorShift(pCfg->aaClr[ori][ColCfg::ostBG], bDim);
   pCnvs->Pen->Color = ColorShift(pCfg->aaClr[ori][ColCfg::ostLN], bDim);
   pCnvs->Rectangle(Rect);

   if (pCfg->apFont[ori])
   {
      pCnvs->Font->Assign(pCfg->apFont[ori]);
   }
uint nColWd = pGrid->ColWidths[ACol];
   if (ARow == 0)
   {
      if (nColWd != pCfg->nWidth)
      {
         pCfg->nWidth = nColWd;
         UpdateSet(0x0004);
      }
   }

int x = Rect.left + 4;
int y = Rect.top + 4;
int yHt = pCnvs->TextHeight("XX");
int iHt = 8;
char *pszBase = sz;
char *pszLast = sz;
int iWdLmt = nColWd - pCnvs->TextWidth("X");
   for (char *psz = sz; *psz; ++psz)
   {
   char ch = *psz;
      if ((ch == ' ') || (ch == ':') || (ch == ';') || (ch == ','))
      {
         *psz = '\0';
      int iWd = pCnvs->TextWidth(pszBase);
         *psz = ch;
         if (iWd > iWdLmt)
         {
         char chLast = *pszLast;
            *pszLast = '\0';
            pCnvs->TextOut(x,y, pszBase);
            *pszLast = chLast;
            pszBase = pszLast;

            y += yHt;
            iHt += yHt;
         }
         pszLast = psz;
      }
      else if (*psz == '\n')
      {
         *psz++ = '\0';
         pCnvs->TextOut(x,y, pszBase);
         y += yHt;
         iHt += yHt;
         pszBase = psz;
         pszLast = psz;
      }
      else if (psz[1] == '\0')
      {
      int iWd = pCnvs->TextWidth(pszBase);
         if (iWd > iWdLmt)
         {
         char chLast = *pszLast;
            *pszLast = '\0';
            pCnvs->TextOut(x,y, pszBase);
            *pszLast = chLast;
            pszBase = pszLast;

            y += yHt;
            iHt += yHt;
         }

         pCnvs->TextOut(x,y, pszBase);
         y += yHt;
         iHt += yHt;
      }
   }
   pCnvs->Brush->Color = clWhite;
   pGrid->Color = clWhite;

static int iMaxHt, iMaxHtRow = -1;
   if ((ARow != iMaxHtRow) || (ACol == 0))
   {
      if ((iMaxHtRow < pGrid->RowCount) &&
          (pGrid->RowHeights[iMaxHtRow] != iMaxHt))
      {
         pGrid->RowHeights[iMaxHtRow] = iMaxHt;
      }
      if (iHt < yHt+8) iHt = yHt+8;
      iMaxHt = iHt;
      iMaxHtRow = ARow;
   }
   else
   {
      if (iHt > iMaxHt) iMaxHt = iHt;
      if ((ACol == pGrid->ColCount - 1) &&  // i.e. last col
          (pGrid->RowHeights[ARow] != iMaxHt))
      {
         pGrid->RowHeights[ARow] = iMaxHt;
      }
   }
atms[0].Stop();
}  //  TBaseForm::GridDrawCell

//---------------------------------------------------------------------------

void TBaseForm::GridMouse(TStringGrid *pGrid, MouseBtn &mBtn)
{
Nexus *pNexus = (mBtn.iRow == 0) ? NULL : (Nexus *)pGrid->Objects[1][mBtn.iRow];
ColCfg *pCfg = (ColCfg*)pGrid->Objects[mBtn.iCol][0];
   if ((pNexus == NULL) || (pCfg == NULL))
      return;

   switch (pCfg->dwFlds & Nexus::fldMSK)
   {
   case Nexus::fldFILM:
      MoviesForm->Show(pNexus->pMovie);
      break;

   case Nexus::fldPERS:            // Name
      PersonForm->Show(pNexus->pPerson);
      break;
   }
}  // TBaseForm::GridMouse

//---------------------------------------------------------------------------

void __fastcall TBaseForm::GridMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   unused(Button);
   unused(Shift);
TStringGrid *pGrid = dynamic_cast<TStringGrid*>(Sender);
   if (!pGrid)
      return;
MouseBtn mBtn;
   mBtn.x = X;
   mBtn.y = Y;
int x = X;
int y = Y;
int iLnWd = pGrid->GridLineWidth;
   for (mBtn.iRow = 0; mBtn.iRow < pGrid->RowCount; ++mBtn.iRow)
   {
      if (mBtn.iRow == pGrid->FixedRows)  // adj for scrolling
      {
         mBtn.iRow = pGrid->TopRow;
      }
      y -= pGrid->RowHeights[mBtn.iRow] + iLnWd;
      if (y < 0)
      {
         break;
      }
   }

   for (mBtn.iCol = 0; mBtn.iCol < pGrid->ColCount; ++mBtn.iCol)
   {
      if (mBtn.iCol == pGrid->FixedCols)
      {
         mBtn.iCol = pGrid->LeftCol;
      }
      x -= pGrid->ColWidths[mBtn.iCol] + iLnWd;
      if (x <= 0)
      {
         break;
      }
   }

   idxCurrCol = mBtn.iCol;
   idxCurrRow = mBtn.iRow;
   mBtn.s = mBtn.sNONE;
   if (Shift.Contains(ssShift))  mBtn.s |= mBtn.sSHF;
   if (Shift.Contains(ssCtrl))   mBtn.s |= mBtn.sCTL;
   if (Shift.Contains(ssAlt))    mBtn.s |= mBtn.sALT;
   if (Shift.Contains(ssLeft))   mBtn.s |= mBtn.sLBTN;
   if (Shift.Contains(ssMiddle)) mBtn.s |= mBtn.sMBTN;
   if (Shift.Contains(ssRight))  mBtn.s |= mBtn.sRBTN;
   if (Shift.Contains(ssDouble)) mBtn.s |= mBtn.sDBL;

   if (mBtn.Is(mBtn.sRBTN | mBtn.sALT) || mBtn.Is(mBtn.sLBTN | mBtn.sCTL))
   {
      TypeForm->Left = grid->Left + X;// - cbAttrib->Width + cbAttrib->Height;
      TypeForm->Top = grid->Top + Y;// - cbAttrib->Height / 2;
      GetAttribs();
      UpdateForm();
   }
   else if (mBtn.Is(mBtn.sLBTN | mBtn.sALT))
   {
      cbEdit->Left = grid->Left + mBtn.x - cbEdit->Width + cbEdit->Height;
      cbEdit->Top = grid->Top + mBtn.y - cbEdit->Height / 2;
      cbEdit->Visible = true;
      cbEdit->ItemIndex = 0;
      pCurrColCfg = GetColCfg(mBtn.iCol);
   }
   else if (mBtn.Is(mBtn.sLBTN | mBtn.sSHF) && (mBtn.iRow == 0))
   {
      GridSort(grid, mBtn.iCol);
   }
   else
   {
      GridMouse(pGrid, mBtn);
   }
   if (mBtn.Is(mBtn.sRBTN | mBtn.sSHF))
   {
   char sz[512];
      for (int i = 0; i < entries(atms); ++i)
      {
         if (atms[i].szName[0] == '\0')
         {
            sprintf(sz, "T%u", i+1);
            atms[i].SetName(sz);
         }
         atms[i].Dump(sz, 0xFFFF);
         Log(sz);
      }
   }
   CntReport(0);
}  // TBaseForm::GridMouseDown

//---------------------------------------------------------------------------

void __fastcall TBaseForm::GridSelectCell(TObject *Sender, int ACol,
      int ARow, bool &CanSelect)
{
   unused(Sender);
   unused(ACol);
   CanSelect = (ARow != 0);
}  // TBaseForm::GridSelectCell

//---------------------------------------------------------------------------

void TBaseForm::GridSort(TStringGrid *pSG, int iColSel)
{
atms[1].Start();
char sz[128];
ColCfg::SortMethod sm = ColCfg::smNONE;
   for (int iCol = 0; iCol < pSG->ColCount; ++iCol)
   {
      sscanf(pSG->Cells[iCol][0].c_str(), "%[^:]", sz);
      if (iCol == iColSel)
      {
         iColSel = iCol;
      ColCfg *pCfg = (ColCfg*)pSG->Objects[iCol][0];
         pCfg->sm = (ColCfg::SortMethod)(pCfg->sm + 1);
         if (pCfg->sm >= ColCfg::smMAX) pCfg->sm = ColCfg::smNONE;
         sm = pCfg->sm;
         switch (sm)
         {
         case ColCfg::smA2Z:   strcat(sz, ":A-Z");  break;
         case ColCfg::smZ2A:   strcat(sz, ":Z-A");  break;
         }
      }
      pSG->Cells[iCol][0] = sz;
   }

   for (int iRow = 1; iRow < pSG->RowCount; ++iRow)
   {
   RowParam rp1, rp2;
   int iBest = iRow;    // assume it is already in right place
      for (int iRow2 = iRow + 1; iRow2 < pSG->RowCount; ++iRow2)
      {
      cchar *psz1 = pSG->Cells[iColSel][iBest].c_str();
      cchar *psz2 = pSG->Cells[iColSel][iRow2].c_str();
      bool bSwap = false;
         switch (sm)
         {
         case ColCfg::smNONE:
            Get(rp1, iBest);
            Get(rp2, iRow2);
            bSwap = rp1.nID > rp2.nID;
            break;

         case ColCfg::smA2Z:
            bSwap = (stricmp(psz1, psz2) > 0);
            break;

         case ColCfg::smZ2A:
            bSwap = (stricmp(psz1, psz2) < 0);
            break;
         }
         if (bSwap)
         {
           iBest = iRow2;
         }
      }
      if (iBest != iRow)
      {
      AnsiString str;
         for (int iCol = 0; iCol < pSG->ColCount; ++iCol)
         {
            str = pSG->Cells[iCol][iRow];
            pSG->Cells[iCol][iRow] = pSG->Cells[iCol][iBest];
            pSG->Cells[iCol][iBest] = str;

         TObject *pObj = pSG->Objects[iCol][iRow];
            pSG->Objects[iCol][iRow] = pSG->Objects[iCol][iBest];
            pSG->Objects[iCol][iBest] = pObj;
         }
      }
   }
   CntReport(4);
atms[1].Stop();
}  // TBaseForm::GridSort

//---------------------------------------------------------------------------

void __fastcall TBaseForm::memAliasExit(TObject *Sender)
{
char sz[512];

   unused(Sender);
bool bMatch = (memAlias->Lines->Count == pCurrItem->pNames->Count - 1);
   for (int i = 0; bMatch && (i < memAlias->Lines->Count); ++i)
   {
      bMatch &= (memAlias->Lines->Strings[i] == pCurrItem->pNames->Strings[i+1]);
   }
   sprintf(sz, "Do you wish to change the aliases for '%s'?",
                cbMain->Text.c_str());
   if (!bMatch &&
       (Application->MessageBoxA(sz, "Change Alias", MB_YESNO) == IDYES))
   {
      pCurrItem->pNames->Clear();
      pCurrItem->pNames->Add(cbMain->Text);
      for (int i = 0; i < memAlias->Lines->Count; ++i)
      {
         pCurrItem->pNames->Add(memAlias->Lines->Strings[i]);
      }
   }
   else
   {
      memAlias->Lines->Clear();
      for (int i = 1; i < pCurrItem->pNames->Count; ++i)
      {
         memAlias->Lines->Add(pCurrItem->pNames->Strings[i]);
      }
   }
   UpdateForm();
}  //  TBaseForm::memNamesExit

//---------------------------------------------------------------------------

void TBaseForm::SaveColSet(void)
{
IniFile ini;
char sz[256], szKey[128];
   sprintf(sz, "%s.%s", szGridName, cbGridFmt->Text.c_str());
   ini.SetSect(sz);
   ini.EraseSect();
   for (int idx = 0; idx < grid->ColCount; ++idx)
   {
   ColCfg *pCfg = GetColCfg(idx);
      pCfg->nWidth = grid->ColWidths[idx];
      sprintf(szKey, "%02u:%s", idx+1, pCfg->szHeader);
      pCfg->Print(sz, colCfg);
      ini.SaveStr(sz, szKey);
   }
   pCurrSet->dwChanged = 0;
}  //  TBaseForm::SaveColSet

//---------------------------------------------------------------------------

void TBaseForm::SetupGrid(ColSet *pSet)
{
   pCurrSet = pSet;
   grid->DefaultDrawing = false;
//   grid->DoubleBuffered = true;
   grid->FixedCols = 0;
   grid->FixedRows = 1;
   grid->ColCount = pCurrSet->nCols;

int iCol = 0;
   for (ColCfg *pCfg = pSet->pHeadCfg; pCfg; pCfg = pCfg->pNext)
   {
      AssignColCfg(pCfg, iCol++);
   }
}  // TBaseForm::SetupGrid

//---------------------------------------------------------------------------

void TBaseForm::Show(NamedItem *pItem)
{
   btnOK->Tag = 0;
   pCurrItem = pItem;
   ShowModal();
}  // TBaseForm::Show

//---------------------------------------------------------------------------

void __fastcall TBaseForm::SortLabel(TObject *Sender)
{
TControl *pCtl = dynamic_cast<TControl*>(Sender);
   if (pCtl == NULL)
      return;

char sz[128], szSort[64], szRest[128];
bool bUpd = false;
   szSort[0] = '\0';
   for (SortMatrix *pSort = pHeadSort; pSort; pSort = pSort->pNext)
   {
      if (pSort->pCtl)
      {
      TLabel *pLbl = (TLabel*)(pSort->pCtl);
         szRest[0] = '\0';
         sscanf(pLbl->Caption.c_str(), "%[^:]%s %s", sz, szSort, szRest);
         if (pSort->pCtl == pCtl)
         {
         NamedItem::SortMethod sm;
            switch ((NamedItem::SortMethod)pCtl->Tag)
            {
            case NamedItem::smNONE:
               sm = NamedItem::smA2Z;
               strcat(sz, ":A-Z");
               break;

            case NamedItem::smA2Z:
               sm = NamedItem::smZ2A;
               strcat(sz, ":Z-A");
               break;

            case NamedItem::smZ2A:
            default:
               sm = NamedItem::smNONE;
               break;
            }
            pCtl->Tag = sm;
            bUpd = NamedItem::Sort(*ppHeadItem, pSort->nOff, pSort->et, sm);
         }
         else
         {
            pCtl->Tag = NamedItem::smNONE;
         }
         strcat(sz, szRest);
         pLbl->Caption = sz;
      }
   }
   if (bUpd)
   {
      UpdateForm();
   }
}  // TBaseForm::SortLabel

//---------------------------------------------------------------------------

void TBaseForm::UpdateForm(void)
{
   grid->RowCount = 2;
   grid->Enabled = (pCurrItem && !bChangeName);
   if (grid->Enabled)
   {
      cbMain->Text = pCurrItem->Name();
   }
   memAlias->Lines->Clear();
   UpdateSet(0);

int iWd = 4;
   for (int iCol = 0; iCol < grid->ColCount; ++iCol)
   {
   ColCfg *pColCfg = GetColCfg(iCol);
      if (pColCfg)
      {
         pColCfg->nWidth = grid->ColWidths[iCol];
//         grid->Cells[iCol][0] = pColCfg->szHeader;
      }
      else
      {
         grid->ColWidths[iCol] = 50;
         grid->Cells[iCol][0] = "<none>";
      }
      iWd += grid->ColWidths[iCol] + grid->GridLineWidth;
   }
   if (grid->Left + iWd > ClientWidth - 4)
   {
      iWd = ClientWidth - grid->Left - 4;
   }
   grid->Width = iWd;

bool bVis = grid->Visible;
   grid->Visible = false;
   UpdForm();
   grid->Visible = bVis;

   if (bChangeName)
      btnOK->Caption = "Add";
   else if (btnOK->Tag > 1)
      btnOK->Caption = "Accept";
   else
      btnOK->Caption = "OK";
   btnRename->Enabled = bChangeName;

int iRow = grid->RowCount - 1;
   grid->Rows[iRow]->Clear();
   AssignRowObj(NULL, iRow, false);

   btnPrev->Enabled = pCurrItem && (pCurrItem != *ppHeadItem);
   btnNext->Enabled = pCurrItem && (pCurrItem->pNext != NULL);
}  // TBaseForm::UpdateForm

//---------------------------------------------------------------------------

void TBaseForm::UpdateSet(DWord dwChange)
{
   if (pCurrSet)
   {
      pCurrSet->dwChanged |= dwChange;
      cbGridFmt->Color = (pCurrSet->dwChanged) ? clYellow : clWindow;
   }
}  // TBaseForm::UpdateSet

//---------------------------------------------------------------------------

void TBaseForm::UpdForm(void)
{
   //
}  // TBaseForm::UpdForm

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

