//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FrmType.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTypeForm *TypeForm;

//---------------------------------------------------------------------------

__fastcall TTypeForm::TTypeForm(TComponent* Owner)
        : TForm(Owner), ctlGrp(this)
{
   pHeadQL = NULL;
   idMaster = 0;
   ptInsert.x = 4;
   ptInsert.y = 40;

   ca.SetPanel(this, ca.mPWD_GRO);
}  // TTypeForm::TTypeForm

//---------------------------------------------------------------------------

int TTypeForm::AddButton(cchar *pszCaption, QueryOptions &qo, TModalResult mr)
{
TButton *pBtn = new TButton(TypeForm);
   pBtn->Caption = pszCaption;
   pBtn->ModalResult = mr;
   pBtn->Parent = TypeForm;

   return (AddLink(pBtn, NULL, qo));
}  // TTypeForm::AddButton

//---------------------------------------------------------------------------

int TTypeForm::AddComboBox(TStrings *pStrs, QueryOptions &qo, int iDefIndex,
        bool bObjAsInt)
{
TComboBox *pCB = new TComboBox(TypeForm);
   pCB->Parent = TypeForm;
   pCB->Width = GetWidest(pStrs) + pCB->Height + 8;    // allow for arrow btn
   pCB->Items->Clear();
   pCB->Items->AddStrings(pStrs);
   pCB->ItemIndex = iDefIndex;
   pCB->Tag = bObjAsInt;

   return (AddLink(pCB, NULL, qo));
}  // TTypeForm::AddComboBox

//---------------------------------------------------------------------------

int TTypeForm::AddEdit(cchar *pszLabel, QueryOptions &qo, cchar *pszDefault)
{
TLabel *pLbl = new TLabel(TypeForm);
   pLbl->Caption = pszLabel;

TEdit *pEdit = new TEdit(TypeForm);
   pEdit->Width = qo.iWidth + ((qo.iWidth < 0) ? ClientWidth : 0);
   pEdit->Text = pszDefault;

   return (AddLink(pEdit, pLbl, qo));
}  // TTypeForm::AddEdit

//---------------------------------------------------------------------------

int TTypeForm::AddGroup(TStrings *pStrs, cchar *pszGroup, QueryOptions &qo,
                        int iDefChks)
{
ControlGroup ctlGrp(this);
   ctlGrp.iCols = qo.lwbPrm.aby[1];
   ctlGrp.iRows = qo.lwbPrm.aby[0];
   ctlGrp.type = ctlGrp.tNONE;
   switch (qo.lwbPrm.aby[2])
   {
   case qo.tCHK_A:      ctlGrp.type = ctlGrp.tACROSS;
   case qo.tCHK_D:      ctlGrp.type |= ctlGrp.tCHECK;   break;

   case qo.tRBTN_A:     ctlGrp.type = ctlGrp.tACROSS;
   case qo.tRBTN_D:     ctlGrp.type |= ctlGrp.tRADIO;  break;
   }
   ctlGrp.Build(pStrs, pszGroup, iDefChks);
   ctlGrp.pGrpBox->Tag = ctlGrp.type;

   return (AddLink(ctlGrp.pGrpBox, NULL, qo));
}  // TTypeForm::AddGroup

//---------------------------------------------------------------------------

int TTypeForm::AddRange(TStrings *pStrs, cchar *pszRange, QueryOptions &qo,
                                int iDefLLmt, int iDefULmt)
{
TLabel *pLbl;
TComboBox *pCB;

QueryOptions qoLcl = qo;
   qoLcl.m = ControlAlignment::mTOP | ControlAlignment::mROF;

   pLbl = new TLabel(TypeForm);
   pLbl->Parent = this;
   pLbl->Caption = "";
   ca.Align(pLbl, qo.m);

   pLbl->Caption = pszRange;
   pCB = new TComboBox(TypeForm);
   pCB->Parent = TypeForm;
   pCB->Width = GetWidest(pStrs) + pCB->Height + 8;    // allow for arrow btn
   pCB->Items->Clear();
   pCB->Items->AddStrings(pStrs);
   pCB->ItemIndex = iDefLLmt;
   AddLink(pCB, pLbl, qoLcl);

   pLbl = new TLabel(TypeForm);
   pLbl->Parent = this;
   pLbl->Caption = "-";
   pCB = new TComboBox(TypeForm);
   pCB->Parent = TypeForm;
   pCB->Width = GetWidest(pStrs) + pCB->Height + 8;    // allow for arrow btn
   pCB->Items->Clear();
   pCB->Items->AddStrings(pStrs);
   if (iDefULmt < 0) iDefULmt = pStrs->Count - 1;
   pCB->ItemIndex = iDefULmt;
   return (AddLink(pCB, pLbl, qoLcl));
}  // TTypeForm::AddRange

//---------------------------------------------------------------------------

int TTypeForm::AddLink(TControl *pCtl, TLabel *pLbl, QueryOptions &qo)
{
QueryLink *pLink = new QueryLink;
   pLink->id = ++idMaster;
   pLink->pLbl = pLbl;
   pLink->pCtl = pCtl;

   if (qo.m == 0) qo.m = ControlAlignment::mBLW;
   if (qo.idAlign > 0)
   {
      for (QueryLink *pQL = pHeadQL; pQL; pQL = pQL->pNext)
      {
         if (pQL->id == qo.idAlign)
         {
            ca.Align(pQL->pCtl, NULL, ca.mNONE);
            break;
         }
      }
   }
   if (pLbl)
   {
      pLbl->Parent = TypeForm;
      pLbl->Visible = true;
      ca.Align(pLbl, NULL, qo.m);
   }
   pCtl->Parent = TypeForm;
   pCtl->Visible = true;
   ca.Align(pCtl, NULL, qo.m);

int iWd = pCtl->Left + pCtl->Width + 8;
   if (ClientWidth < iWd)
   {
      ClientWidth = iWd;
   }
   if (ptInsert.y > ClientHeight)
   {
      ClientHeight = ptInsert.y;
   }

   pLink->pNext = pHeadQL;
   pHeadQL = pLink;
   return (pLink->id);
}  // TTypeForm::AddLink

//---------------------------------------------------------------------------

void TTypeForm::Align(int idCtl, int idRel, ControlAlignment::Method m)
{
TControl *pCtlAdj = NULL, *pCtlRel = NULL;
   for (QueryLink *pLink = pHeadQL; pLink; pLink = pLink->pNext)
   {
      if (pLink->id == idCtl)
      {
         pCtlAdj = pLink->pLbl ? pLink->pLbl : pLink->pCtl;
      }
      if (pLink->id == idRel)
      {
         pCtlRel = pLink->pLbl ? pLink->pLbl : pLink->pCtl;
      }
   }
   ca.Align(pCtlAdj, pCtlRel, m);
}  // TTypeForm::Align

//---------------------------------------------------------------------------

bool TTypeForm::GetValue(int idCtl, char *pszRtn, int &iRtn)
{
   for (QueryLink *pLink = pHeadQL; pLink; pLink = pLink->pNext)
   {
      if (pLink->id == idCtl)
      {

      TComboBox *pCB = dynamic_cast<TComboBox*>(pLink->pCtl);
         if (pCB)
         {
            if (pszRtn) strcpy(pszRtn, pCB->Text.c_str());
            iRtn = pCB->ItemIndex;
            return (true);
         }

      TEdit *pEdit = dynamic_cast<TEdit*>(pLink->pCtl);
         if (pEdit)
         {
            if (pszRtn) strcpy(pszRtn, pEdit->Text.c_str());
            sscanf(pEdit->Text.c_str(), "%d", &iRtn);
            return (true);
         }

      ctlGrp.pGrpBox = dynamic_cast<TGroupBox*>(pLink->pCtl);
         if (ctlGrp.pGrpBox)
         {
            ctlGrp.type = (ControlGroup::Type)ctlGrp.pGrpBox->Tag;
            iRtn = ctlGrp.GetMask();
            return (true);
         }
      }
   }
   return (false);
}  // TTypeForm::GetValue

//---------------------------------------------------------------------------

int TTypeForm::Query(void)
{
int iWd = (ClientWidth - btnOK->Width - btnCancel->Width) / 5;
int iTop = 0;
   for (QueryLink *pLink = pHeadQL; pLink; pLink = pLink->pNext)
   {
      if (pLink->pCtl)
      {
      int iBlw = pLink->pCtl->Top + pLink->pCtl->Height + 4;

         if (iBlw > iTop ) iTop = iBlw;
      }
   }
   btnOK->Top = iTop;
   btnOK->Left = 2 * iWd;
   btnCancel->Top = btnOK->Top;
   btnCancel->Left = btnOK->Left + btnOK->Width + iWd;

   ClientHeight = btnOK->Top + btnOK->Height + 8;
int iRtn = ShowModal();
   if (iRtn == mrCancel) iRtn = 0;
   else if (iRtn == mrOk) iRtn = 1;
   return (iRtn);
}  // TTypeForm::Query

//---------------------------------------------------------------------------

int TTypeForm::Query(TStrings *pStrs, cchar *pszCaption, cchar *pszDesc,
        int iDefIndex, bool bChkBoxes)
{
QueryOptions qo;
   Zero(qo);
   Reset(pszCaption, pszDesc);

int id;
   ptInsert.x = 4;//(ClientWidth - pCtl->Width) / 2;
   ptInsert.y = lblDesc->Top + lblDesc->Height + 8;
   if (bChkBoxes)
   {
      qo.m = ca.mBLW | ca.mCIP;
      qo.lwbPrm.aby[2] = qo.tCHK_A;
      id = AddGroup(pStrs, pszCaption, qo, iDefIndex);
   }
   else
   {
      qo.m = ca.mBLW | ca.mCIP | ca.mPWD_GRO;
      id = AddComboBox(pStrs, qo, iDefIndex);
   }

int iRtn = 0;
   if (Query())
   {
   char szRtn[128];
      GetValue(id, szRtn, iRtn);
   }
   else
   {
      iRtn = -1;
   }
   return (iRtn);
}  // TTypeForm::Query

//---------------------------------------------------------------------------

void TTypeForm::Reset(cchar *pszCaption, cchar *pszDesc, int iWd, int iHt)
{
   while (pHeadQL)
   {
   QueryLink *pLink = pHeadQL;
      pHeadQL = pLink->pNext;
      if (pLink->pCtl)
      {
         delete pLink->pCtl;
      }
      if (pLink->pLbl)
      {
         delete pLink->pLbl;
      }
      delete pLink;
   }
   idMaster = 100;
   ClientWidth = iWd;
   ClientHeight = iHt;

   Caption = pszCaption;
   lblDesc->Width = ClientWidth - 2 * lblDesc->Left;
   lblDesc->Caption = pszDesc;
   ca.Align(lblDesc, lblDesc, ca.mNONE);   // sets lblDesc as last control
}  // TTypeForm::Reset



