//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MovieMain.h"

#include <IniFiles.hpp>

#include "FrmMovie.h"
#include "FrmPerson.h"
#include "FrmReport.h"
#include "FrmGraph.h"
#include "FrmQuery.h"

#include "FrmTools.h"
#include "StrTools.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

//---------------------------------------------------------------------------

void FlagChangeDB(void)
{
   Form1->Flag(Form1->ftDB_CHANGED);
}  // FlagChangeDB

//---------------------------------------------------------------------------

void Log(cchar *pszMsg)
{
   Form1->Log(pszMsg);
}  // Log

//---------------------------------------------------------------------------

int Printf(const char *pszFmt,...)
{
va_list argptr;
int cnt;
static char sz[1024];
   va_start(argptr, pszFmt);
   cnt = vsprintf(sz, pszFmt, argptr);
   va_end(argptr);

char *pszBeg, *pszEnd;
   pszBeg = sz;
   while ((pszEnd = strchr(pszBeg, '\n')) != NULL)
   {
      *pszEnd = '\0';
      Form1->Log(pszBeg);
      pszBeg = pszEnd;
      if (*pszBeg == '\r')
         ++pszBeg;
   }
   if (*pszBeg)
      Form1->Log(pszBeg);
   if (strncmp(pszFmt, "E>", 2) == 0)
   {
      err.LogErr();
   }
   return (cnt);
}  // Printf


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
ErrorStack err;

//---------------------------------------------------------------------------

ErrorStack::ErrorStack(void)
{
   idx = 0;
   Zero(apszCode);
   Zero(anCode);
}  // ErrorStack::ErrorStack

//---------------------------------------------------------------------------

void ErrorStack::LogErr(void)
{
char sz[1024], *psz = sz;
   psz += sprintf(psz, "     from");
   for (int i = 0; i < idx; ++i)
   {
      psz += sprintf(psz, " '%s'", apszCode[i]);
      if ((int)anCode[i] != -123)
      {
         psz += sprintf(psz, ":%lu", anCode[i]);
      }
      psz += sprintf(psz, ";");
   }
   Log(sz);
}  // ErrorStack::LogErr

//---------------------------------------------------------------------------

void ErrorStack::Push(cchar *psz, ulong n)
{
   if (idx < entries(apszCode))
   {
      apszCode[idx] = psz;
      anCode[idx] = n;
   }
   ++idx;
}  // ErrorStack::Push

//---------------------------------------------------------------------------

void ErrorStack::Pop(void)
{
   if (idx > 0) --idx;
}  // ErrorStack::Pop

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{

IniFile ini;
char sz[128];
   Caption = VersionAndCaption(sz);

TStringList *pStrs = new TStringList;

   err.Push(ini.strFN.c_str());
   if (ini.GetSectVals("Genre", pStrs) == 0)
   {
      Printf("E>Config file missing Genre section");
   }
   for (int i = 0; i < pStrs->Count; ++i)
   {
      new Genre(pStrs->Strings[i].c_str());
   }

   Person::Configure(ini);
   Movie::pStrRate = new TStringList;
   ini.strSect = "Film";
   Movie::pStrRate->CommaText = ini.GetStr("Ratings", "1,2,3,4,5,6,7,8,9,10");
   for (int i = 0; i < Movie::pStrRate->Count; ++i)
   {
      Movie::pStrRate->Objects[i] = (TObject*)(i+1);
   }

   Nexus::pStrRate = new TStringList;
   ini.strSect = "Nexus";
   Nexus::pStrRate->CommaText = ini.GetStr("Ratings", "1,2,3,4,5,6,7");
   for (int i = 0; i < Nexus::pStrRate->Count; ++i)
   {
      Nexus::pStrRate->Objects[i] = (TObject*)(i+1);
   }

   Rank::nCnt = ini.GetSectVals("Rank", pStrs);
   if (Rank::nCnt == 0)
   {
      Printf("E>Config file missing Rank section; using defaults");
      pStrs->Add("Year Rank=Y;Depth:10;Step:1");
      pStrs->Add("Decade Rank=D;Depth:20;Step:10");
   }
   Rank::pa = new Rank[Rank::nCnt];
   for (uint idx = 0; idx < Rank::nCnt; ++idx)
   {
   Rank &rnk = Rank::pa[idx];
   cchar *psz, *pszSrc = pStrs->Strings[idx].c_str();
      Zero(rnk);
      sscanf(pszSrc, "%[^=]=%[^:;]:%u", rnk.szName, rnk.szCode, &rnk.nWidth);
      rnk.nDepth = 10;
      rnk.nStep = 1;
      if ((psz = strstr(pszSrc, ";Depth")) != NULL)
      {
         sscanf(psz, ";Depth:%u", &rnk.nDepth);
      }
      if ((psz = strstr(pszSrc, ";Step")) != NULL)
      {
         sscanf(psz, ";Step:%u", &rnk.nStep);
      }
   }

   if (ini.GetSectVals("Role", pStrs) == 0)
   {
      Printf("E>Config file missing Role section");
   }
   for (int i = 0; i < pStrs->Count; ++i)
   {
      new Role(pStrs->Strings[i].c_str());
   }
   err.Pop();
   delete pStrs;
   UpdStats();
ControlAlignment ca;
   ca.Align(btnSave,    btnRead,    ca.mBLW | ca.mLAL);
   ca.Align(btnOutStat, btnSave,    ca.mBLW | ca.mLAL);
   ca.Align(btnReport,  btnOutStat, ca.mBLW | ca.mLAL);
   ca.Align(btnGraph,   btnReport,  ca.mBLW | ca.mLAL);

   ca.Align(btnQuery,   btnRead,    ca.mTOP | ca.mROF);
   ca.Align(btnMovie,   btnQuery,   ca.mBLW | ca.mLAL);
   ca.Align(btnPeople,  btnMovie,   ca.mBLW | ca.mLAL);
   ca.Align(Memo1,      btnQuery,   ca.mTOP | ca.mROF);
   Memo1->Width = ClientWidth - Memo1->Left - 4;

   ca.Align(btnClear,   Memo1,      ca.mBTM | ca.mLOF);
   btnClear->Anchors.Clear();
   btnClear->Anchors << akLeft << akBottom;

   ca.Align(lblStats,   btnPeople,  ca.mBLW | ca.mLAL);

}  // TForm1::TForm1

//---------------------------------------------------------------------------
// TForm1::btnBuildClick

//---------------------------------------------------------------------------

void __fastcall TForm1::btnDumpClick(TObject *Sender)
{
char sz[256], *psz;
   unused(Sender);

   for (Movie *pMovie = Movie::pHead; pMovie; pMovie = (Movie*)pMovie->pNext)
   {
      Printf(pMovie->Name());
      psz = sz;
      psz += sprintf(psz, "      %u Rate:%s ", pMovie->nYear, pMovie->GetRating());
      for (uint idxR = 0; idxR < Rank::nCnt; ++idxR)
      {
         psz += sprintf(psz, " %s:%s", Rank::pa[idxR].szCode,
                                       Rank::Prt(pMovie->rrs.panRk[idxR]));
      }
      Printf(sz);
      for (Nexus *pNexus = pMovie->pHeadNexus; pNexus; pNexus = pNexus->pNext)
      {
         psz = sz;
         psz += sprintf(psz, "      %24s", pNexus->pPerson->Name());
         psz += sprintf(psz, " as %s", pNexus->pRole->Name());
         psz += sprintf(psz, "\t%s", pNexus->GetRating());
         for (uint idxR = 0; idxR < Rank::nCnt; ++idxR)
         {
            Rank::pa[idxR].Print(pNexus->rrs.panRk[idxR], psz);
         }
         unused(psz);
         Printf(sz);
      }
   }
}  // TForm1::btnDumpClick

//---------------------------------------------------------------------------

void __fastcall TForm1::btnFormClick(TObject *Sender)
{
TControl *pCtl = dynamic_cast<TControl*>(Sender);
   if (pCtl)
   {
      switch (pCtl->Tag)
      {
      case 1:   MoviesForm->Show(Movie::pHead);         break;
      case 2:   PersonForm->Show(Person::pHead);        break;
      case 3:   GraphForm->Show(Movie::pHead);          break;
      case 4:   ReportForm->Show(Movie::pHead);         break;

      case 10:
         ReadEvalCSV("EvalAll.csv");
         ReadRateCSV("RankingsAll.csv");
         break;
      }
      UpdStats();
   }
}  // TForm1::btnFormClick

//---------------------------------------------------------------------------

void __fastcall TForm1::btnReadClick(TObject *Sender)
{
   unused(Sender);
char sz[4096];
ParseString parse(4096, 80, parse.typCSV);
enum Fields {fldNONE, fldYEAR, fldFILM, fldRATE, fldGENRE, fldALIAS,
             fldTYPE, fldNAME, fldMAX};
Fields afld[64];
   Zero(afld);
int aidxFld[fldMAX];
   for (int i = 0; i < entries(aidxFld); ++i)
   {
      aidxFld[i] = -1;
   }

int &idxFilm = aidxFld[fldFILM];
int &idxYear = aidxFld[fldYEAR];
int &idxGenre = aidxFld[fldGENRE];
int &idxAlias = aidxFld[fldALIAS];
int &idxFRating = aidxFld[fldRATE];
uint nLn = 1;
   if (OpenDialog1->Execute())
   {
   AnsiString strFN = ChangeFileExt(OpenDialog1->FileName, ".CSV");
   FILE *fp = fopen(strFN.c_str(), "rt");

      fgets(sz, sizeof(sz), fp);
      parse.Extract(sz);
      for (uint i = 0; i < parse.nPrms; ++i)
      {
      cchar *pszHdr = parse.apszPrm[i];
      Fields fld = fldNONE;
              if (stricmp(pszHdr,  "YEAR") == 0)     {  fld = fldYEAR;   }
         else if (stricmp(pszHdr,  "FILM") == 0)     {  fld = fldFILM;   }
         else if (stricmp(pszHdr,  "*") == 0)        {  fld = fldRATE;   }
         else if (stricmp(pszHdr,  "Genre") == 0)    {  fld = fldGENRE;  }
         else if (stricmp(pszHdr,  "Alias") == 0)    {  fld = fldALIAS;  }
         else if (strnicmp(pszHdr, "Cont", 4) == 0)  {  fld = fldTYPE;   }
         else if (strnicmp(pszHdr, "Name", 4) == 0)  {  fld = fldNAME;   }
         afld[i] = fld;
         if (aidxFld[fld] < 0)
         {
            aidxFld[fld] = i;
         }
      }

      while (fgets(sz, sizeof(sz), fp))
      {
         err.Push(strFN.c_str(), ++nLn);        // file name and line

         parse.Extract(sz);
         if ((idxFilm > -1) && (parse.apszPrm[idxFilm] != NULL))
         {
         char szName[256], *pszName = szName;
            Zero(szName);
            pszName += sprintf(pszName, "%s", parse.apszPrm[idxFilm]);
            err.Push(szName);

         uint nYear;
            if (sscanf(parse.apszPrm[idxYear], "%u", &nYear) != 1)
            {
               Printf("E>Can't decode year in '%s'\n", parse.apszPrm[idxYear]);
            }

         dword dwGenre = Genre::Encode(parse.apszPrm[idxGenre]);

         Movie *pMovie;
            pMovie = Movie::Find(szName);
            if (pMovie && (pMovie->nYear != nYear))
            {
               if (stricmp(szName, pMovie->Name()) == 0)
               {
                  sprintf(pszName, " (%4u)", pMovie->nYear);
                  pMovie->pNames->Insert(0,szName);
               }
               sprintf(pszName, " (%4u)", nYear);
               pMovie = NULL;  // build it
            }
            if (pMovie == NULL)
            {
               pMovie = new Movie(szName, nYear, dwGenre);
               if (*pszName != '\0')
               {
                  pMovie->pNames->Add(parse.apszPrm[idxFilm]);
               }
               pMovie->rrs.DecodeAll(parse.apszPrm[idxFRating], Movie::pStrRate);

               if (idxAlias >= 0)
               {
               uint n;
                  for (cchar *psz = parse.apszPrm[idxAlias]; *psz; psz += n)
                  {
                     n = 0;
                     if (sscanf(psz, "%[^|]|%n", szName, &n) == 1)
                     {
                        if (stricmp(szName, parse.apszPrm[idxFilm]) != 0)
                        {
                           pMovie->pNames->Add(szName);
                        }
                     }
                     if (n == 0)
                     {
                        break;
                     }
                  }
               }
            }

         Person *pPerson;
            for (uint idx = aidxFld[fldTYPE]; idx < parse.nPrms-2; idx += 3)
            {
               if ((parse.apszPrm[idx] == NULL) || (parse.apszPrm[idx][0] == '\0'))
               {
                  break;
               }

               err.Push("Col", idx+1);
               pPerson = Person::Find(parse.apszPrm[idx+1]);
               if (pPerson == NULL)
               {
                  pPerson = new Person(parse.apszPrm[idx+1]);
               }
               err.Pop();

               err.Push("Col", idx);
            Role *pRole = Role::Find(parse.apszPrm[idx]);
               err.Pop();

               err.Push("Col", idx+2);
               pMovie->Add(pPerson, pRole, parse.apszPrm[idx+2]);
               err.Pop();
            }
            err.Pop();
         }
         err.Pop();
      }
      fclose(fp);
   }
   Printf("Database Read successful");
   UpdStats();
}  // TForm1::btnReadClick

//---------------------------------------------------------------------------

void TForm1::Flag(FlagType ft)
{
   if (ft == ftDB_CHANGED)
   {
      btnSave->Font->Style = TFontStyles() << fsBold << fsUnderline;
      ++btnSave->Tag;
   }
   else if (ft == ftDB_SAVE)
   {
      btnSave->Font->Style = TFontStyles();
      btnSave->Tag = 0;
   }
}  // TForm1::Flag

//---------------------------------------------------------------------------

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
char sz[256];
   if (btnSave->Tag > 0)
   {
      sprintf(sz, "Save changes to Movie Database (%u)?", btnSave->Tag);
      switch (Application->MessageBoxA(sz, "Save Movie DB", MB_YESNOCANCEL))
      {
      case IDYES:
         btnSave->OnClick(Sender);
      case IDNO:
         break;

      case IDCANCEL:
         Action = caNone;
         break;
      }
   }
}  // TForm1::FormClose

//---------------------------------------------------------------------------

void TForm1::Log(cchar *pszMsg)
{
   Memo1->Lines->Add(pszMsg);
}  // TForm1::Log

//---------------------------------------------------------------------------

void TForm1::ReadEvalCSV(cchar *pszFN)
{
FILE *fp = fopen(pszFN, "rt");
   if (fp == NULL)
      return;
ParseString parse(4096, ParseString::typCSV);


char sz[4096];
   Zero(sz);
Movie *pMovie = NULL;
   Movie::pHead = NULL;
   fgets(sz, sizeof(sz)-1, fp);
   parse.Extract(sz);

   while (fgets(sz, sizeof(sz)-1, fp))
   {
   char szTmp[64], szName[128], szNames[256], szRate[64];
   char *pszTgt;
      pszTgt = szTmp;
   bool bQuote = false;
   int iItem = 0;
   uint nYear;
   Role *pRole = NULL;
      parse.Extract(sz);

      for (char *psz = sz; *psz; ++psz)
      {
      char ch = *psz;
         if (ch == '\\')        // escape char
         {
            ch = *++psz;        // get next character
            switch (ch)         // convert as needed
            {
            case 'n':  ch = '\n';  break;
            case 'r':  ch = '\r';  break;
            case 't':  ch = '\t';  break;
            }
         }
         else if (ch == '\"')
         {
            bQuote ^= true;
            ch = '\0';
         }
         else if (bQuote)
         {
         }
         else if ((ch == ',') || (psz[1] == '\0'))    // separator or end
         {
         uint n = -1;
            if (ch != ',')
            {
               *pszTgt++ = ch;
            }
            ch = 0;
            *pszTgt = '\0';
            switch (++iItem)
            {
            case 1:
               sscanf(szTmp, "%u", &nYear);
               pszTgt = szName;
               break;

            case 2:
               pszTgt = szRate;
               break;

            case 3:
               pMovie = new Movie(szName, nYear, 0);
///               sscanf(szRate, "%u", &pMovie->rrs.nRat);
               for (int iR = 0; iR < Movie::pStrRate->Count; ++iR)
               {
                  if (strcmp(szRate, Movie::pStrRate->Strings[iR].c_str()) == 0)
                  {
                     pMovie->rrs.idxRat = iR+1;
                  }
               }
               pszTgt = szTmp;  // type
               break;

            case 4:
               pRole = Role::Find(szTmp);
               if ((pRole == NULL) && (strlen(szTmp) > 0))
               {
                  pRole = new Role(szTmp);
               }
               pszTgt = szNames;
               break;

            case 5:
               pszTgt = szRate;
               break;

            case 6:
               for (char *pszName = szNames; (n != 0) && *pszName; pszName += n)
               {
                  n = 0;
                  sscanf(pszName, "%[^;]; %n", szName, &n);
               Person *pPerson = Person::Find(szName);
                  if (pPerson == NULL)
                  {
                     pPerson = new Person(szName);
                  }
                  pMovie->Add(pPerson, pRole, szRate);
               }
               pszTgt = szTmp;
               iItem -=3;
               break;

            case 7:
               pszTgt = szRate;
               iItem = 4;       // repeat every 3
               break;
            }

         }
         if (ch != '\0')
         {
            *pszTgt++ = ch;
         }
      }
   }
   fclose(fp);

   Printf("Found Type:%u Person:%u Movie:%u Node:%u",
                Role::nCnt, Person::nCnt, Movie::nCnt, Nexus::nCnt);
}  // TForm1::ReadEvalCSV

//---------------------------------------------------------------------------

void TForm1::ReadRateCSV(cchar *pszFN)
{
FILE *fp = fopen(pszFN, "rt");
   if (fp == NULL)
      return;

char sz[4096];
   Zero(sz);
Role *apRole[10];
   Zero(apRole);
   apRole[2] = Role::Find("Leading Actor");
   apRole[3] = Role::Find("Leading Actress");
   apRole[4] = Role::Find("Supporting Actor");
   apRole[5] = Role::Find("Supporting Actress");
   apRole[6] = Role::Find("Director");
   apRole[7] = Role::Find("Writer");
   apRole[8] = Role::Find("Producer");

   fgets(sz, sizeof(sz)-1, fp);
   while (fgets(sz, sizeof(sz)-1, fp))
   {
   char szTmp[64], szMovie[128], szNames[256];
   char *pszTgt;
      pszTgt = szTmp;
   bool bQuote = false;
   int iItem = 0;
   uint nRank, nLoYr, nHiYr, idxRank;
   Movie *pMovie;
      for (char *psz = sz; *psz; ++psz)
      {
      char ch = *psz;
         if (ch == '\\')        // escape char
         {
            ch = *++psz;        // get next character
            switch (ch)         // convert as needed
            {
            case 'n':  ch = '\n';  break;
            case 'r':  ch = '\r';  break;
            case 't':  ch = '\t';  break;
            }
         }
         else if (ch == '\"')
         {
            bQuote ^= true;
            ch = '\0';
         }
         else if (bQuote)
         {
         }
         else if ((ch == ',') || (psz[1] == '\0'))    // separator
         {
            ch = 0;
            *pszTgt = '\0';
            switch (++iItem)
            {
            case 1:     // year
               ch = '\0';
               sscanf(szTmp, "%u%c", &nLoYr, &ch);
               nHiYr = nLoYr + ((ch == 's') ? 9 : 0);
               idxRank = (ch == 's') ? 1 : 0;
               pszTgt = szTmp;
               ch = '\0';
               break;

            case 2:     // rank
               sscanf(szTmp, "%u", &nRank);
               pszTgt = szMovie;
               break;

            case 3:     // picture
            case 5:     // Leading Actor
            case 7:     // Leading Actress
            case 9:     // Supporting Actor
            case 11:    // Supporting Actress
            case 13:    // Director
            case 15:    // Write

               pszTgt = szNames;        //
               pMovie = NULL;
               while (true)
               {
                  pMovie = Movie::Find(szMovie, pMovie);
                  if ((pMovie == NULL) ||
                     ((nLoYr <= pMovie->nYear) && (pMovie->nYear <= nHiYr)))
                  {
                     break;
                  }
               }
               if (pMovie == NULL)
               {
                  if (szMovie[0] != '\0')
                  {
                     Printf("Can't find '%s' in %u..%u", szMovie, nLoYr, nHiYr);
                  }
               }
               else if (iItem == 3)
               {
                  pMovie->rrs.panRk[idxRank] = nRank;
                  for (Nexus *pN = pMovie->pHeadNexus; pN; pN = pN->pNext)
                  {
                     if (pN->pRole == apRole[8]) // Producers
                     {
                        pN->rrs.panRk[idxRank] = nRank;
                     }
                  }
               }
               else
               {
               int idx = iItem / 2;
               uint n = -1;
               char szName[128];

                  for (char *pszName = szNames; (n != 0) && *pszName; pszName += n)
                  {
                     n = 0;
                     sscanf(pszName, "%[^;]; %n", szName, &n);
                  Person *pPerson = Person::Find(szName);
                     if (pPerson == NULL)
                     {
                        Printf("Can't find '%s'", szName);
                     }
                  Nexus *pNexus = pMovie->FindNexus(pPerson, apRole[idx]);
                     if (pNexus == NULL)
                     {
                        Printf("Can't find Contribution for '%s' in '%s' as %s",
                                szName, pMovie->Name(), apRole[idx]->Name());
                     }
                     else
                     {
                        pNexus->rrs.panRk[idxRank] = nRank;
                     }
                  }
               }
               break;

            case 4:     // got Leading Actor
            case 6:     // got Leading Actress
            case 8:     // got Supporting Actor
            case 10:     // got Supporting Actress
            case 12:     // got Director
            case 14:     // got Writer
               pszTgt = szMovie;
               break;
            }
         }
         if (ch != '\0')
         {
            *pszTgt++ = ch;
         }
      }
   }
   fclose(fp);
}  // TForm1::ReadRateCSV

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

Genre *Genre::pHead = NULL;
uint Genre::nCnt = 0;

//---------------------------------------------------------------------------

Genre::Genre(cchar *psz)
{
char szName[128],szAbbr[32];
   pNext = NULL;
Genre **ppG = &pHead;
   for (Genre *pG = pHead; pG; pG = (Genre*)pG->pNext)
   {
      ppG = &(Genre*)pG->pNext;
   }
   *ppG = this;

   sscanf(psz, "%127[^=]=%31[^;:]", szName, szAbbr);
   nID = ++nCnt;
   dwMask = 1L << nID;
   pNames->Add(szName);
   pNames->Add(szAbbr);
}  // Genre::Genre

//---------------------------------------------------------------------------

Genre::~Genre()
{
   --nCnt;
}  // Genre::~Genre

//---------------------------------------------------------------------------

void Genre::Decode(char *&psz, dword dw, cchar *pszSep)
{
int iCnt = 0;
   for (Genre *pG = pHead; pG; pG = (Genre*)pG->pNext)
   {
      if ((dw & pG->dwMask) != 0)
      {
         if (iCnt++ > 0)
         {
            psz += sprintf(psz, "%s", pszSep);
         }
         psz += sprintf(psz, "%s", pG->Name((pG->pNames->Count > 1) ? 1 : 0));
      }
   }
}  // Genre::Decode

//---------------------------------------------------------------------------

void Genre::Dump(char *&psz)
{
   NamedItem::Dump(psz);
}  // Genre::Dump

//---------------------------------------------------------------------------

dword Genre::Encode(cchar *psz)
{
dword dwGenre = 0;
   for ( ; *psz; )
   {
   uint n = 0;
   char ch, sz[64];
   Genre *pG;

      if (sscanf(psz, " %63[^,;]%c%n", sz, &ch, &n) < 1)
         break;
      for (pG = Genre::pHead; pG; pG = (Genre*)pG->pNext)
      {
         if ((stricmp(sz, pG->Name()) == 0) ||
             (strcmp(sz, pG->Name(1)) == 0))
         {
            dwGenre |= pG->dwMask;
            break;
         }
      }
      if (pG == NULL)   // can't find it
      {
         Printf("E>Unknown genre '%s'\n", sz);
      }
      psz += n;
      if (n == 0)
         break;
   }
   if (dwGenre == 0) dwGenre = 1;
   return (dwGenre);
}  // Genre::Encode

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

Movie *Movie::pHead = NULL;
uint Movie::nCnt = 0;
TStringList *Movie::pStrRate;

//---------------------------------------------------------------------------

Movie::Movie(cchar *pszName, uint nYearNew, dword dwGenreNew)
{
   pHeadNexus = NULL;
   pNext = pHead;
   pHead = this;

   pNames->Add(pszName);
   dwGenre = dwGenreNew;
   nYear = nYearNew;
   ++nCnt;
}  // Movie::Movie

//---------------------------------------------------------------------------

Movie::~Movie()
{
   while (pHeadNexus)
   {
   Nexus *pNexus = pHeadNexus;
      pNexus->AdjustCounts(false);
      pHeadNexus = pNexus->pNext;
      delete pNexus;    // will adjust counts
   }
   delete rrs.panRk;
   --nCnt;
}  // Movie::Movie

//---------------------------------------------------------------------------

Nexus *Movie::Add(Person *pPerson, Role *pRole, cchar *pszRating)
{
Nexus *pNexus = FindNexus(pPerson, pRole);

   if (pNexus == NULL)
   {
      pNexus = new Nexus;
      pNexus->pNext = pHeadNexus;
      pHeadNexus = pNexus;

      pNexus->pMovie = this;
      pNexus->pPerson = pPerson;
      pNexus->pRole = pRole;
   }
   else
   {
      pNexus->AdjustCounts(false);
   }

   pNexus->rrs.DecodeAll(pszRating, Nexus::pStrRate);
   pNexus->AdjustCounts(true);

   return (pNexus);
}  // Movie::Add

//---------------------------------------------------------------------------

void Movie::Dump(char *&psz)
{
   NamedItem::Dump(psz);
   psz += sprintf(psz, " %s", GetRating());
   for (uint idxRnk = 0; idxRnk < Rank::nCnt; ++idxRnk)
   {
      Rank::pa[idxRnk].Print(rrs.panRk[idxRnk], psz);
   }
}  // Movie::Dump

//---------------------------------------------------------------------------

Movie *Movie::Find(cchar *psz, Movie *pAfter)
{
NamedItem *pBeginItem = pAfter ? pAfter->pNext : pHead;
Movie *pMovie = static_cast<Movie*>(NamedItem::Find(psz, pBeginItem));
   return (pMovie);
}  // Movie::Find

//---------------------------------------------------------------------------

Nexus *Movie::FindNexus(Person *pPerson, Role *pRole)
{
Nexus *pNexus = NULL;
   if (this)
   {
      for (pNexus = pHeadNexus; pNexus; pNexus = pNexus->pNext)
      {
      bool bFound = true;
         bFound &= (pPerson == NULL) || (pNexus->pPerson == pPerson);
         bFound &= (pRole == NULL) || (pNexus->pRole == pRole);
         if (bFound)
         {
            break;
         }
      }
   }
   return (pNexus);
}  // Movie::FindNexus

//---------------------------------------------------------------------------

Nexus *Movie::FindRank(uint nRank, uint nLoYr, uint nHiYr, uint idxRank, Role *pR)
{
static Nexus nexus;
   for (Movie *pM = Movie::pHead; pM; pM = (Movie*)pM->pNext)
   {
      if ((nLoYr <= pM->nYear) && (pM->nYear <= nHiYr))
      {
         if (pR == NULL)
         {
            if (pM->rrs.panRk[idxRank] == nRank)
            {
               if (pM->pHeadNexus)
               {
                  return (pM->pHeadNexus);     // any one will do
               }
               else
               {
                  Zero(nexus);
                  nexus.pMovie = pM;
                  return (&nexus);
               }
            }
         }
         else
         {
            for (Nexus *pN = pM->pHeadNexus; pN; pN = pN->pNext)
            {
               if ((pN->rrs.panRk[idxRank] == nRank) && (pN->pRole == pR))
               {
                  return (pN);
               }
            }
         }
      }
   }
   return (NULL);               // can't find it
}; // Movie::FindRank

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

uint Nexus::nCnt = 0;
TStringList *Nexus::pStrRate;

//--------------------------------------------------------------------------

Nexus::Nexus(void)
{
uint *pan = rrs.panRk;
   ZeroMem(this);
   rrs.panRk = pan;
   ++nCnt;
}  // Nexus::Nexus

//--------------------------------------------------------------------------

Nexus::~Nexus()
{
   --nCnt;
}  // Nexus::~Nexus

//--------------------------------------------------------------------------

/*
Nexus& Nexus::operator=(const Nexus &nexus)
{
   pNext = nexus.pNext;
   pMovie = nexus.pMovie;
   pPerson = nexus.pPerson;
   pRole = nexus.pRole;
   nRating = nexus.nRating;
   for (uint idx = 0; idx < Rank::nCnt; ++idx)
   {
      panRank[idx] = nexus.panRank[idx];
   }
}  // Nexus::operator= */

//--------------------------------------------------------------------------

void Nexus::AdjustCounts(bool bAdd)
{
   if (pPerson && pRole && (pRole->nID > 0) && (rrs.idxRat > 0))
   {
   uint &nCnt = pPerson->paanCnt[pRole->nID-1][rrs.idxRat-1];
      if (bAdd)
        ++nCnt;
      else
        --nCnt;
      if (nCnt > 10000)
      {
         Printf("Cnt error");
      }
   }
}  // Nexus::AdjustCounts

//--------------------------------------------------------------------------

dword Nexus::FieldDecode(cchar *&psz)
{
char sz[64];
Field fld = fldNONE;
   if (psz[1] == ':')
   {
      psz += 2;
   }

uint n = 0;
   if (sscanf(psz, "%[^:;]%n", sz, &n) == 1)
   {
   Role *pRole;
      psz += n;
           if (stricmp(sz, "FILM") == 0)     { fld = fldFILM; }
      else if (stricmp(sz, "YEAR") == 0)     { fld = fldYEAR; }
      else if (stricmp(sz, "FRAT") == 0)     { fld = fldFRAT; }
      else if (strnicmp(sz, "FRNK", 4) == 0) { fld = fldFRNK; }
      else if (stricmp(sz, "GENR") == 0)     { fld = fldGENR; }
      else if (stricmp(sz, "PERS") == 0)     { fld = fldPERS; }
      else if (stricmp(sz, "ROLE") == 0)     { fld = fldROLE; }
      else if (stricmp(sz, "RRAT") == 0)     { fld = fldRRAT; }
      else if (strnicmp(sz, "RRNK", 4) == 0) { fld = fldRRNK; }
      else if ((pRole = Role::Find(sz)) != NULL)
      {
         fld = (Field)(fldPERS + pRole->nID);
      }
      else
      {
         Printf("E>Unknown Field '%s'", sz);
      }

      if ((fld == fldFRNK) || (fld == fldRRNK))
      {
         for (n = 0; n < Rank::nCnt; ++n)
         {
            if (stricmp(sz+4, Rank::pa[n].szCode) == 0)
            {
               break;
            }
         }
         if (n >= Rank::nCnt)
         {
            n = 0;
            sscanf(sz+4, "%u", &n);
         }
         fld = (Field)(fld + n);
      }

   }
   return (fld);
}  // Nexus::FieldDecode

//--------------------------------------------------------------------------

void Nexus::FieldEncode(dword dwFlds, char *&psz)
{
Field fld = (Field)(dwFlds & fldMSK);
static char *pszTrack;
   pszTrack = psz;
   if ((fldFRNK <= fld) && (fld <= fldFRNK_N))
   {
      psz += sprintf(psz, "FRNK%u", (int)(fld - fldFRNK));
   }
   else if ((fldRRNK <= fld) && (fld <= fldRRNK_N))
   {
      psz += sprintf(psz, "RRNK%u", (int)(fld - fldRRNK));
   }
   else if ((fldPERS <= fld) && (fld <= fldPERS_N))
   {
   Role *pRole = Role::Find(fld - fldPERS);
      psz += sprintf(psz, "%s", pRole ? pRole->Name(1) : "PERS");
   }
   else switch (fld)
   {
   case fldFILM:  psz += sprintf(psz, "FILM");  break;
   case fldYEAR:  psz += sprintf(psz, "YEAR");  break;
   case fldFRAT:  psz += sprintf(psz, "FRAT");  break;
   case fldGENR:  psz += sprintf(psz, "GENR");  break;
   case fldROLE:  psz += sprintf(psz, "ROLE");  break;
   case fldRRAT:  psz += sprintf(psz, "RRAT");  break;
   default:       psz += sprintf(psz, "UNK%lx", fld); break;
   }
   if ((dwFlds & ~fldMSK) != 0)
   {
      psz += sprintf(psz, ";Fld:%X", dwFlds & ~fldMSK);
   }
}  // Nexus::FieldEncode

//--------------------------------------------------------------------------

cchar *Nexus::FieldGet(dword dwFlds, char *psz)
{
cchar *pszRtn = psz;
Field fld = (Field)(dwFlds & fldMSK);
   if ((fldFRNK <= fld) && (fld <= fldFRNK_N))
   {
   uint idx = fld - fldFRNK;
      if (idx < Rank::nCnt)  {  Rank::Prt(pMovie->rrs.panRk[idx], psz);  }
      else  {  sprintf(psz, "invalid idx %u", idx);  }
   }
   else if ((fldRRNK <= fld) && (fld <= fldRRNK_N))
   {
   uint idx = fld - fldRRNK;
      if (idx < Rank::nCnt)  {  Rank::Prt(rrs.panRk[idx], psz);  }
      else  {  sprintf(psz, "invalid idx %u", idx);  }
   }
   else if ((fldPERS <= fld) && (fld <= fldPERS_N))
   {
      pszRtn = pPerson->Name();
   }
   else switch (dwFlds & fldMSK)
   {
   case fldFILM:   pszRtn = pMovie->Name();                     break;
   case fldFRAT:   sprintf(psz, "%s", pMovie->GetRating());     break;
   case fldGENR:   Genre::Decode(psz, pMovie->dwGenre);         break;
   case fldYEAR:   sprintf(psz, "%u", pMovie->nYear);           break;
   case fldROLE:   pszRtn = pRole->Name();                      break;
   case fldRRAT:   sprintf(psz, "%s", GetRating());             break;
   default:        sprintf(psz, "<unk%lx>", dwFlds);            break;
   }
   return (pszRtn);
}  // Nexus::FieldGet

//---------------------------------------------------------------------------

void Nexus::FieldList(TStringList *pStrs, FieldSets fset)
{
char sz[128];
   if ((fset & fsetFILM) != 0)
   {
      pStrs->AddObject("Film",            (TObject*)fldFILM);
      pStrs->AddObject("Year",            (TObject*)fldYEAR);
      pStrs->AddObject("Rating-F",        (TObject*)fldFRAT);
      for (uint idx = 0; idx < Rank::nCnt; ++idx)
      {
         sprintf(sz, "%s-F", Rank::pa[idx].szName);
         pStrs->AddObject(sz,             (TObject*)(fldFRNK+idx));
      }
   }

   if ((fset & fsetGENRE) != 0)
   {
      pStrs->AddObject("Genre",           (TObject*)fldGENR);
   }

   if ((fset & fsetPERSON) != 0)
   {
      pStrs->AddObject("Person",          (TObject*)fldPERS);
   }

   if ((fset & fsetROLE) != 0)
   {
      pStrs->AddObject("Role",            (TObject*)fldROLE);
      pStrs->AddObject("Rating",          (TObject*)fldRRAT);
      for (uint idx = 0; idx < Rank::nCnt; ++idx)
      {
         sprintf(sz, "%s", Rank::pa[idx].szName);
         pStrs->AddObject(sz,             (TObject*)(fldRRNK+idx));
      }
   }

   if ((fset & fsetROLES) != 0)
   {
      for (Role *pRole = Role::pHead; pRole; pRole = (Role*)pRole->pNext)
      {
         pStrs->AddObject(pRole->Name(), (TObject*)(fldPERS + pRole->nID));
      }
   }
}  // Nexus::FieldList

//--------------------------------------------------------------------------

Nexus *Nexus::NextMatch(FieldMatch fm)
{
Nexus *pNexus = NULL;

   if (pRole->bGroup || ((fm & fmGRP) == 0))
   {
      pNexus = ((fm & fmHEAD) != 0) ? pMovie->pHeadNexus : pNext;
      for (; pNexus; pNexus = pNexus->pNext)
      {
      bool  bMatch = true;
         if ((fm & fmPERSON) != 0)
         {
            bMatch &= (pNexus->pPerson == pPerson);
         }
         if ((fm & fmTYPE) != 0)
         {
            bMatch &= (pNexus->pRole == pRole);
         }
         if ((fm & fmRATING) != 0)
         {
            bMatch &= (pNexus->rrs.idxRat == rrs.idxRat);
         }
         for (uint idxRnk = 0; idxRnk < Rank::nCnt; ++idxRnk)
         {
            if ((fm & (fmRANK0<<idxRnk)) != 0)
            {
               bMatch &= (pNexus->rrs.panRk[idxRnk] == rrs.panRk[idxRnk]);
            }
         }

         if (bMatch)
         {
            break;
         }
      }
   }
   return (pNexus);
}  // Nexus::NextMatch

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

Person *Person::pHead = NULL;
uint Person::nCnt = 0;
Person::Coef Person::coef;

//---------------------------------------------------------------------------

Person::Person(cchar *pszName, long liID)
{
static long idLast = 100;
   ZeroClass((byte*)this+sizeof(NamedItem), this);
   pNext = pHead;
   pHead = this;

int idx = Role::GetMax();
   paanCnt = new uint[idx][7];
   ZeroLen(paanCnt, sizeof(uint) * idx * 7);

   pNames->Add(pszName);
   id = (liID != 0) ? liID : idLast + 1;
   if (id > idLast)
   {
      idLast = id;
   }
   ++nCnt;
}  // Person::Person

//---------------------------------------------------------------------------

Person::~Person()
{
   --nCnt;
}  // Person::~Person

//---------------------------------------------------------------------------

void Person::Coef::Load(IniFile &ini)
{
   ini.SetSect("Person");

   fBlendQ = ini.GetFloat("BlendQ", 0.3);
   afRatWt[0][0] = ini.GetFloat("RatWt1",  -2.50);
   afRatWt[0][1] = ini.GetFloat("RatWt2",  -1.20);
   afRatWt[0][2] = ini.GetFloat("RatWt3",   0.25);
   afRatWt[0][3] = ini.GetFloat("RatWt4",   1.50);
   afRatWt[0][4] = ini.GetFloat("RatWt5",   6.50);
   afRatWt[0][5] = ini.GetFloat("RatWt6",  16.60);
   afRatWt[0][6] = ini.GetFloat("RatWt7",  23.50);

   afRatWt[1][0] = ini.GetFloat("RatWt1S", -2.50);
   afRatWt[1][1] = ini.GetFloat("RatWt2S", -1.20);
   afRatWt[1][2] = ini.GetFloat("RatWt3S",  0.20);
   afRatWt[1][3] = ini.GetFloat("RatWt4S",  1.50);
   afRatWt[1][4] = ini.GetFloat("RatWt5S",  6.50);
   afRatWt[1][5] = ini.GetFloat("RatWt6S", 16.60);
   afRatWt[1][6] = ini.GetFloat("RatWt7S", 23.50);

   fYrWinWt = ini.GetFloat("YrWinWt", 0.25);
   fDcTopWt = ini.GetFloat("DcTopWt", 0.35);
   fRtWinWt = ini.GetFloat("RtWinWt", 0.40);
   fYrNomWt = ini.GetFloat("YrNomWt", 0.50);
   fRtNomWt = ini.GetFloat("RtNomWt", 0.50);

   fRtWinK  = ini.GetFloat("RtWinK",  4.00);
   fRtNomK  = ini.GetFloat("RtNomK",  2.50);
   fYrWinK  = ini.GetFloat("YrWinK",  2.50);
   fYrNomK  = ini.GetFloat("YrNomK",  2.50);
   fTop10K  = ini.GetFloat("Top10K",  1.00);
   fDcTopK  = ini.GetFloat("DcTopK",  5.00);
   fDcBotK  = ini.GetFloat("DcBotK",  1.50);
}  // Person::Coef::Load

//---------------------------------------------------------------------------

void Person::Coef::Save(IniFile &ini)
{
char sz[128];
   ini.SetSect("Person");
   ini.SaveFloat(fBlendQ, "BlendQ");
   for (int i = 0; i < 2; ++i)
   {
   char ch = (i == 0) ? '\0' : 'S';
      for (int ii = 0; ii < 7; ++ii)
      {
         sprintf(sz, "RatWt%u%c", ii+1, ch);
         ini.SaveFloat(afRatWt[i][ii], sz);
      }
   }
   ini.SaveFloat(fYrWinWt, "YrWinWt");
   ini.SaveFloat(fDcTopWt, "DcTopWt");
   ini.SaveFloat(fRtWinWt, "RtWinWt");
   ini.SaveFloat(fYrNomWt, "YrNomWt");
   ini.SaveFloat(fRtNomWt, "RtNomWt");

   ini.SaveFloat(fRtWinK, "RtWinK");
   ini.SaveFloat(fRtNomK, "RtNomK");
   ini.SaveFloat(fYrWinK, "YrWinK");
   ini.SaveFloat(fYrNomK, "YrNomK");
   ini.SaveFloat(fTop10K, "Top10K");
   ini.SaveFloat(fDcTopK, "DcTopK");
   ini.SaveFloat(fDcBotK, "DcBotK");
}  // Person::Coef::Save

//---------------------------------------------------------------------------

float Person::ComputeScore(cchar *pszRole1, cchar *pszRole2, FILE *fp)
{
char sz[512], *psz;
Role *apRole[2];
   apRole[0] = Role::Find(pszRole1);
   apRole[1] = Role::Find(pszRole2);

bool bSupp = (apRole[1] != NULL);
bool bHeader = fp && (stricmp(pszRole1, "Header") == 0);
   if (bHeader)
   {
      fprintf(fp, "Name,# Film,1,2,3,4,5,6,7,");
      if (bSupp)
      {
         fprintf(fp, "1,2,3,4,5,6,7,L Roles,S Roles,");
      }
      fprintf(fp, "+,-,Wins,Noms,");
      if (bSupp)
      {
         fprintf(fp, "AVG L,AVG S,");
      }
      fprintf(fp, "AVG,Q,");
        // From Ranks
      if (bSupp)
      {
         fprintf(fp, "L Win,L Nom,S Win,S Nom,");
      }
      fprintf(fp, "Wins,Noms,Top Ten,");
      if (bSupp)
      {
         fprintf(fp, "D L Win,D S Win,D L R-U,D S R-U,");
      }
      fprintf(fp, "D Wins,D R-U,WAvg,NAvg,Score");
      fprintf(fp, "\n");
      return (0.0);
   }

int iLmt = bSupp ? 2 : 1;


Person *pPerson = this;

ulong anTtl[2] = {0,0};
uint anCnt[2] = {0,0};

uint nRtPlus = 0;
uint nRtMinus = 0;
uint nRtWin = 0;
uint nRtNom = 0;
float fQ = 0;

uint nRoles = 0;
   psz = sz;
   for (int i = 0; i < iLmt; ++i)
   {
      for (uint nRating = 1; nRating <= 7; ++nRating)
      {
      uint n = pPerson->paanCnt[apRole[i]->nID-1][nRating-1];
         psz += sprintf(psz, "%u,", n);
         anCnt[i] += n;
         anTtl[i] += n * nRating;
         if ((1 <= nRating) && (nRating <= 2)) nRtMinus += n;
         if ((4 <= nRating) && (nRating <= 7)) nRtPlus += n;
         if ((6 <= nRating) && (nRating <= 7)) nRtWin += n;
         if ((5 <= nRating) && (nRating <= 7)) nRtNom += n;

         fQ += coef.afRatWt[i][nRating-1] * n;
      }
      nRoles += anCnt[i];
   }
   psz[-1] = '\0';   // kill trailing comma

float fScore = 0.0;
   if (nRoles > 0)
   {
   float afAvg[2];
      afAvg[0] = (anCnt[0] > 0) ? (float)anTtl[0] / anCnt[0] : 0;
      afAvg[1] = (anCnt[1] > 0) ? (float)anTtl[1] / anCnt[1] : 0;
   float fAvg = (float)(anTtl[0] + anTtl[1]) / nRoles;

      if (fp)
      {
         fprintf(fp, "%s,%d,%s,", pPerson->Name(), nRoles, sz);
         if (bSupp)
         {
            fprintf(fp, "%d,%d,", anCnt[0], anCnt[1]);
         }
         fprintf(fp, "%d,%d,%d,%d,", nRtPlus, nRtMinus, nRtWin, nRtNom);
         if (bSupp)
         {
            fprintf(fp, "%.2f,%.2f,", afAvg[0], afAvg[1]);
         }
         fprintf(fp, "%.2f,%.1f,", fAvg, fQ);
      }

   uint nTop10 = 0;
   uint anYrWin[2] = {0,0};     // Year Wins
   uint anYrNom[2] = {0,0};     // Year Nom
   uint anDcTop[2] = {0,0};     // Decade Top Half
   uint anDcBot[2] = {0,0};     // Decade Bottom Half
      for (Movie *pMovie = Movie::pHead; pMovie; pMovie = (Movie*)pMovie->pNext)
      {
         for (Nexus *pN = pMovie->pHeadNexus; pN; pN = pN->pNext)
         {
            for (int i = 0; i < iLmt; ++i)
            {
               if ((pN->pRole == apRole[i]) && (pN->pPerson == pPerson))
               {
               int nRank;
                  nRank = pN->rrs.panRk[0];
                  if (nRank > 0)
                  {
                     ++nTop10;
                     if (nRank == 1) ++anYrWin[i];
                     if ((1 <= nRank) && (nRank <= 5)) ++anYrNom[i];
                     if (apRole[i]->bBPNom && (6 <= nRank) && (nRank <= 10) &&
                         (5 <= pN->rrs.idxRat) && (pN->rrs.idxRat <= 7))
                     {
                        ++anYrNom[i];
                     }
                  }

                  nRank = pN->rrs.panRk[1];
                  if (nRank > 0)
                  {
                     if (nRank <= 10) ++anDcTop[i];
                     else ++anDcBot[i];
                  }
               }
            }
         }
      }
   uint nYrWin = anYrWin[0] + anYrWin[1];
   uint nYrNom = anYrNom[0] + anYrNom[1];
   uint nDcTop = anDcTop[0] + anDcTop[1];
   uint nDcBot = anDcBot[0] + anDcBot[1];

   float fWAvg = coef.fYrWinWt*nYrWin + coef.fDcTopWt*nDcTop + coef.fRtWinWt*nRtWin;
   float fNAvg = coef.fYrNomWt*nYrNom + coef.fRtNomWt*nRtNom;
   float fRnkQ = coef.fYrWinK*nYrWin + coef.fYrNomK*nYrNom +
                 coef.fRtWinK*nRtWin + coef.fRtNomK*nRtNom +
                 coef.fTop10K*nTop10 +
                 coef.fDcTopK*nDcTop + coef.fDcBotK*nDcBot;
      fScore = coef.fBlendQ * fQ + (1 - coef.fBlendQ) * fRnkQ;

      if (fp)
      {
         if (bSupp)
         {
            fprintf(fp, "%d,%d,", anYrWin[0], anYrNom[0]);
            fprintf(fp, "%d,%d,", anYrWin[1], anYrNom[1]);
         }
         fprintf(fp, "%d,%d,%d,", nYrWin, nYrNom, nTop10);
         if (bSupp)
         {
            fprintf(fp, "%d,%d,", anDcTop[0], anDcTop[1]);
            fprintf(fp, "%d,%d,", anDcBot[0], anDcBot[1]);
         }
         fprintf(fp, "%d,%d,%.1f,%.1f,", nDcTop, nDcBot, fWAvg, fNAvg);

         fprintf(fp, "%.2f\n", fScore);
      }
   }
   return (fScore);
}  // Person::ComputeScore

//---------------------------------------------------------------------------

void Person::Configure(IniFile &ini)
{
   coef.Load(ini);
}  // Person::Configure

//---------------------------------------------------------------------------

void Person::Dump(char *&psz)
{
   NamedItem::Dump(psz);
   psz += sprintf(psz, " = %u", id);
}  // Person::Dump

//---------------------------------------------------------------------------

Person *Person::Find(cchar *psz, Person *pAfter)
{
NamedItem *pBeginItem = pAfter ? pAfter->pNext : pHead;
Person *pPerson = static_cast<Person*>(NamedItem::Find(psz, pBeginItem));
   return (pPerson);
}  // Person::Find

//---------------------------------------------------------------------------

Role *Person::LikelyRole(void)
{
Role *pRole = NULL;
uint nCnt = 0;

   for (Role *pR = Role::pHead; pR; pR = (Role*)pR->pNext)
   {
   uint n = 0;
      for (int i = 0; i < 7; ++i)
      {
         n += paanCnt[pR->nID-1][i];
      }
      if (n > nCnt)
      {
         nCnt = n;
         pRole = pR;
      }
   }
   return (pRole);
}  // Person::LikelyRole

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

uint Rank::nCnt = 0;
Rank *Rank::pa = NULL;

//--------------------------------------------------------------------------

uint Rank::Decode(cchar *psz)
{
uint nRtn = Rank::NR;
   if ((sscanf(psz, "%u", &nRtn) != 1) && (strcmp(psz, "N/R") != 0))
   {
      Printf("E>Can't decode Rank '%s'\n", psz);
   }
   return (nRtn);
}  // Rank::Decode

//--------------------------------------------------------------------------

uint Rank::FindIdx(cchar *psz)
{
uint nRtn = 0;
   if (strncmp(psz+1, "RNK", 3) == 0)       // skip over first four
   {
      psz += 4;
   }
   if (*psz == '_') ++psz;

   if (sscanf(psz, "%u", &nRtn) != 1)
   {
      for (nRtn = 0; nRtn < nCnt; ++nRtn)
      {
         if ((stricmp(psz, pa[nRtn].szCode) == 0) ||
             (stricmp(psz, pa[nRtn].szName) == 0) )
         {
            break;
         }
      }
   }
   return (nRtn);
}  // Rank::FindIdx

//--------------------------------------------------------------------------

void Rank::Print(uint nVal, char *&psz)
{
   psz += sprintf(psz, ";%s:", szCode);
   psz += sprintf(psz, ((0 < nVal) && (nVal < Rank::NR)) ? "%u" : "N/R", nVal);
}  // Rank::Print

//--------------------------------------------------------------------------

char *Rank::Prt(uint nVal, char *psz)
{
static char sz[16];
   if (psz == NULL) psz = sz;
   sprintf(psz, ((0 < nVal) && (nVal < Rank::NR)) ? "%2u" : "N/R", nVal);
   return (psz);
}  // Rank::Prt

//--------------------------------------------------------------------------
// struct RatingRanks
//--------------------------------------------------------------------------

RatingRanks::RatingRanks(void)
{
   panRk = new uint[Rank::nCnt];
   Reset();
}  // RatingRanks::RatingRanks

//--------------------------------------------------------------------------

RatingRanks::~RatingRanks()
{
   delete panRk;
}  // RatingRanks::~RatingRanks

//--------------------------------------------------------------------------

RatingRanks& RatingRanks::operator=(const RatingRanks &rrs)
{
   idxRat = rrs.idxRat;
   for (uint idx = 0; idx < Rank::nCnt; ++idx)
   {
      panRk[idx] = rrs.panRk[idx];
   }
   return (*this);
}  // RatingRanks::operator= */

//--------------------------------------------------------------------------

bool RatingRanks::DecodeAll(cchar *pszRR, TStringList *pStrRate)
{
char szCode[8], szVal[8];
uint n = 0;
bool bFnd = false;
   Reset();

   if (pStrRate)
   {
      if (sscanf(pszRR, "%7[^ ;\n]%n", szCode, &n) != 1)
         ;
      else for (int iR = 0; iR < pStrRate->Count; ++iR)
      {
         if (strcmp(szCode, pStrRate->Strings[iR].c_str()) == 0)
         {
            idxRat = iR + 1;
            bFnd = true;
         }
      }
   }
   else
   {
      bFnd = (sscanf(pszRR, "%u%n", &idxRat, &n) == 1);
   }
   if (!bFnd) {  Printf("E>Can't decode rating '%s'\n", pszRR);  }
   pszRR += n;

   n = 0;
   while (sscanf(pszRR, "%*[ ;]%[^:]:%[^ ;\n]%n", szCode, szVal, &n) == 2)
   {
      bFnd = false;
      err.Push(szCode);
      for (uint idxR = 0; idxR < Rank::nCnt; ++idxR)
      {
         if (strcmp(szCode, Rank::pa[idxR].szCode) == 0)
         {
             panRk[idxR] = Rank::Decode(szVal);
             bFnd = true;
             break;
         }
      }
      err.Pop();
      if (!bFnd)  {  Printf("E>Unknown rank '%s'\n", szCode);  }

      if (n == 0)
         break;
      pszRR += n;
      n = 0;
   }
   return (true);
}  // RatingRanks::DecodeAll

//--------------------------------------------------------------------------

cchar *RatingRanks::GetRating(TStringList *pStrRate)
{
   if ((0 < idxRat) &&(idxRat <= (uint)pStrRate->Count))
   {
      return (pStrRate->Strings[idxRat - 1].c_str());
   }
   else
   {
      return ("Unk");
   }
}; // RatingRanks::GetRating

//--------------------------------------------------------------------------

void RatingRanks::Reset(uint idxRatDef)
{
   idxRat = idxRatDef;
   for (uint n = 0; n < Rank::nCnt; ++n)
   {
      panRk[n] = Rank::NR;
   }
}  // RatingRanks::Reset

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

RatingRanksAdjust::RatingRanksAdjust(void)
{
   idxRank = 0;
   pList = new TList;
   pList->Add(NULL);    // marker for Adj (after) and Set (before)
}  // RatingRanksAdjust::RatingRanksAdjust

//---------------------------------------------------------------------------

RatingRanksAdjust::~RatingRanksAdjust()
{
   delete pList;
}  // RatingRanksAdjust::~RatingRanksAdjust

//---------------------------------------------------------------------------

bool RatingRanksAdjust::ProcessList(void)
{
bool bAdj = false;
   for (int i = 0; i < pList->Count; ++i)
   {
      if (pList->Items[i] == NULL)
      {
         bAdj = true;
      }
      else
      {
      uint &nRank = ((RatingRanks*)pList->Items[i])->panRk[idxRank];
         nRank = (bAdj) ? nRank + iAdj : nTgt;
         if ((nRank > Rank::pa[idxRank].nDepth) || (nRank == 0))
         {
            nRank = Rank::NR;
         }
      }
   }
   return (true);
}  // RatingRanksAdjust::ProcessList

//---------------------------------------------------------------------------

bool RatingRanksAdjust::Setup(RatingRanks &rrsFrom, RatingRanks &rrsTo,
                int idxNewRank)
{
   idxRank = idxNewRank;
   pList->Clear();
   pList->Add(NULL);    // 0..NULL=set, NULL...N = adjust
   AddSet(rrsFrom);

   nTgt = rrsTo.panRk[idxRank];
   if (rrsFrom.panRk[idxRank] == Rank::NR)       // n/r ==> N
   {
      nLLmt = rrsTo.panRk[idxRank];
      nULmt = Rank::pa[idxRank].nDepth;
      iAdj = +1;
   }
   else if (rrsTo.panRk[idxRank] == Rank::NR)    // N ==> n/r
   {
      nLLmt = rrsFrom.panRk[idxRank] + 1;
      nULmt = Rank::pa[idxRank].nDepth;
      iAdj = -1;
   }
   else if (rrsFrom.panRk[idxRank] > rrsTo.panRk[idxRank])
   {
      nLLmt = rrsTo.panRk[idxRank];
      nULmt = rrsFrom.panRk[idxRank] - 1;
      iAdj = +1;
   }
   else // pNexus->aiRank[idxRank] < nexus.aiRank[idxRank]
   {
      nLLmt = rrsFrom.panRk[idxRank] +1;
      nULmt = rrsTo.panRk[idxRank];
      iAdj = -1;
   }
   return (rrsFrom.panRk[idxRank] != rrsTo.panRk[idxRank]);
}  // RatingRanksAdjust::Setup

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

Role *Role::pHead = NULL;
uint Role::nMasterID = 0;
Role::ColorConfig Role::aClrCfg[Role::ciMAX];
uint Role::nCnt = 0;

//---------------------------------------------------------------------------

Role::Role(cchar *pszBase)
{
   if (nCnt == 0)
   {
      strcpy(aClrCfg[ciBAR].szKey, "Bar");
      aClrCfg[ciBAR].aclrDef[0] = clRed;
      aClrCfg[ciBAR].aclrDef[1] = clGreen;

      strcpy(aClrCfg[ciLMT].szKey, "Limits");
      aClrCfg[ciLMT].aclrDef[0] = clWhite;
      aClrCfg[ciLMT].aclrDef[1] = clWhite;

      strcpy(aClrCfg[ciOUT].szKey, "Outline");
      aClrCfg[ciOUT].aclrDef[0] = clAqua;
      aClrCfg[ciOUT].aclrDef[1] = clAqua;

      strcpy(aClrCfg[ciTXT].szKey, "Text");
      aClrCfg[ciTXT].aclrDef[0] = clWhite;
      aClrCfg[ciTXT].aclrDef[1] = clWhite;

   }
   ZeroClass((byte*)this+sizeof(NamedItem), this);
NamedItem **pp = &(NamedItem*)pHead;
   while (*pp)
   {
      pp = &(*pp)->pNext;
   }
   pNext = NULL;
   *pp = this;

   nID = ++nMasterID;
char szName[256], szAbbr[256], sz[256];
   sz[0] = '\0';
   if (sscanf(pszBase, "%[^=]=%[^;];%[^\n]", szName, szAbbr, sz) < 2)
   {
      strncpy(szAbbr, szName, 3);
      szAbbr[4] = 0;
   }
   pNames->Add(szName);
   pNames->Add(szAbbr);
   bGroup = (strstr(sz, "Group") != NULL);
   bBPNom = (strstr(sz, "BPNom") != NULL);
   ++nCnt;
int idxAltClr = ((nID & 0x01) != 0);
cchar *psz;
   for (int i = 0; i < ciMAX; ++i)
   {
      aclr[i] = aClrCfg[i].aclrDef[idxAltClr];
      sprintf(sz, ";clr%s(", aClrCfg[i].szKey);
      if ((psz = strstr(pszBase, sz)) != NULL)
      {
         TBaseForm::ColorDecode(psz, aclr[i]);
      }
   }
}  // Role::Role

//---------------------------------------------------------------------------

Role::~Role()
{
   --nCnt;
}  // Role::~Role

//---------------------------------------------------------------------------

void Role::Dump(char *&psz)
{
   NamedItem::Dump(psz);
   psz += sprintf(psz, " = %u/%u", nID, nMasterID);
}  // Role::Dump

//---------------------------------------------------------------------------

Role *Role::Find(cchar *psz)
{
Role *pRole = static_cast<Role*>(NamedItem::Find(psz, pHead));
   if ((pRole == NULL) && (psz != NULL))
   {
      Printf("E>Can't find role '%s'\n", psz);
   }
   return (pRole);
}  // Role::Find

//---------------------------------------------------------------------------

Role *Role::Find(uint nID)
{
Role *pRole;
   for (pRole = pHead; pRole; pRole = (Role*)pRole->pNext)
   {
      if (pRole->nID == nID)
      {
         break;
      }
   }
   return (pRole);
}  // Role::Find

//---------------------------------------------------------------------------

void Role::Save(void)
{
IniFile ini;
   ini.SetSect("Role");
   for (Role *pRole = pHead; pRole; pRole = (Role*)pRole->pNext)
   {
   char sz[1024], *psz = sz;
      psz += sprintf(psz, "%s", pRole->Name(1));
      if (pRole->bGroup)
      {
         psz += sprintf(psz, ";Group");
      }
      if (pRole->bBPNom)
      {
         psz += sprintf(psz, ";BPNom");
      }

   int idxAlt = ((pRole->nID & 0x01) != 0);
      for (int i = 0; i < ciMAX; ++i)
      {
         TBaseForm::ColorAppend(psz, aClrCfg[i].szKey, pRole->aclr[i],
                aClrCfg[i].aclrDef[idxAlt]);
      }
      ini.SaveStr(sz, pRole->Name());
   }
}  // Role::Save

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void TForm1::UpdStats(void)
{
char sz[128], *psz;
   sscanf(btnMovie->Caption.c_str(), "%[^\n]", sz);
   psz = strchr(sz, '\0');
   sprintf(psz, "\n(%u)", Movie::nCnt);
   btnMovie->Caption = sz;

   sscanf(btnPeople->Caption.c_str(), "%[^\n]", sz);
   psz = strchr(sz, '\0');
   sprintf(psz, "\n(%u)", Person::nCnt);
   btnPeople->Caption = sz;

   sprintf(sz, "G:%u R:%u\nNode:%u", Genre::nCnt, Role::nCnt, Nexus::nCnt);
   lblStats->Caption = sz;
}  // TForm1::UpdStats

//---------------------------------------------------------------------------

void __fastcall TForm1::btnSaveClick(TObject *Sender)
{
char sz[1024], *psz;
   unused(Sender);
   if (SaveDialog1->Execute())
   {
   AnsiString strFN = ChangeFileExt(SaveDialog1->FileName, ".CSV");
   FILE *fp = fopen(strFN.c_str(), "wt");

      Flag(ftDB_SAVE);
      fprintf(fp, "Year,Film,*,Genre,Alias");
      for (int i = 0; i < 20; ++i)
      {
         fprintf(fp, ",Contribution,Name,*");
      }
      fprintf(fp, "\n");
      for (Movie *pMovie = Movie::pHead; pMovie; pMovie = (Movie*)pMovie->pNext)
      {
         psz = sz;
         psz += sprintf(psz, "%s", pMovie->GetRating());
         for (uint idxRnk = 0; idxRnk < Rank::nCnt; ++idxRnk)
         {
            if (pMovie->rrs.panRk[idxRnk] != Rank::NR)
            {
               Rank::pa[idxRnk].Print(pMovie->rrs.panRk[idxRnk], psz);
            }
         }
         fprintf(fp,"%4u,\"%s\",%s", pMovie->nYear, pMovie->Name(), sz);
         psz = sz;
         Genre::Decode(psz, pMovie->dwGenre);
         fprintf(fp, ",%s", sz);

         psz = sz;
         *psz = '\0';
         for (int i = 1; i < pMovie->pNames->Count; ++i)
         {
            psz += sprintf(psz, "%s|", pMovie->pNames->Strings[i].c_str());
         }
         fprintf(fp, ",\"%s\"", sz);

         for (Role *pRole = Role::pHead; pRole; pRole = (Role*)pRole->pNext)
         {
            for (Nexus *pN = pMovie->pHeadNexus; pN; pN = pN->pNext)
            {
               if (pN->pRole != pRole)
                  continue;

               psz = sz;
               psz += sprintf(psz, "%s", pN->GetRating());
               for (uint idxRnk = 0; idxRnk < Rank::nCnt; ++idxRnk)
               {
                  if (pN->rrs.panRk[idxRnk] != Rank::NR)
                  {
                     Rank::pa[idxRnk].Print(pN->rrs.panRk[idxRnk], psz);
                  }
               }
               fprintf(fp, ",%s,%s,%s", pN->pRole->Name(), pN->pPerson->Name(), sz);
            }
         }
         fprintf(fp, "\n");
      }
      fclose(fp);

      fp = fopen(ChangeFileExt(strFN, "_Rank.csv").c_str(), "wt");
      fprintf(fp, "Year,Rank,Best Picture");
      for (Role *pRole = Role::pHead; pRole; pRole = (Role*)pRole->pNext)
      {
         fprintf(fp, ",Best %s,Film", pRole->Name());
      }
      fprintf(fp, "\n");
      for (uint idxRnk = 0; idxRnk < 2; ++idxRnk)
      {
      Rank &rnk = Rank::pa[idxRnk];
      cchar *pszFmt = (idxRnk == 0) ? "%d,%d" : "%ds,%d";

         for (uint nYear = 1910; nYear < 2100; nYear += rnk.nStep)
         {
         uint nYrLo = nYear;
         uint nYrHi = nYear + (rnk.nStep - 1);

            for (uint nRank = 1; nRank <= rnk.nDepth; ++nRank)
            {
            long liPos = ftell(fp);

               fprintf(fp, pszFmt, nYear, nRank);
            Nexus *pNexus = Movie::FindRank(nRank, nYrLo, nYrHi, idxRnk, NULL);
               if (pNexus)
               {
                  fprintf(fp, ",\"%s\"", pNexus->pMovie->Name());
                  liPos = 0;       // don't rewind
               }
               else
               {
                  fprintf(fp, ",");
               }
               for (Role *pR = Role::pHead; pR; pR = (Role*)pR->pNext)
               {
               Nexus *pN = Movie::FindRank(nRank, nYrLo, nYrHi, idxRnk, pR);
                  if (pN)
                  {
                     fprintf(fp, ",%s,\"%s\"", pN->pPerson->Name(), pN->pMovie->Name());
                     liPos = 0;       // don't rewind
                  }
                  else
                  {
                     fprintf(fp, ",,");
                  }
               }
               fprintf(fp, "\n");
               if (liPos)
               {
                  fseek(fp, liPos, SEEK_SET);
               }
            }
         }
      }
      fclose(fp);
   }
   UpdStats();
}  // TForm1::btnSaveClick

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void __fastcall TForm1::btnOutStatClick(TObject *Sender)
{
FILE *fp;
char sz[512];
   unused(Sender);
   if (SaveDialog1->Execute())
   {
      for (Role *pRole = Role::pHead; pRole; pRole = (Role*)pRole->pNext)
      {
      char szRole[64];
         fp = NULL;
         if (sscanf(pRole->Name(), "Leading %s", sz) != 0)
         {
            sprintf(szRole, "Supporting %s", sz);
            strcat(sz, (strcmp(sz, "Actress") == 0) ? "es.csv" : "s.csv");
            fp = fopen(sz, "wt");
         }
         else if (strncmp(pRole->Name(), "Supp", 4) != 0)
         {
            szRole[0] = '\0';
            sprintf(sz, "%ss.csv", pRole->Name());
            fp = fopen(sz, "wt");
         }
         if (fp)
         {
            Person::pHead->ComputeScore("Header", szRole, fp);
            for (Person *pP = Person::pHead; pP; pP = (Person*)pP->pNext)
            {
               pP->ComputeScore(pRole->Name(), szRole, fp);
            }
            fclose(fp);
         }
      }

      fp = fopen("Merged.csv", "wt");
      fprintf(fp, "Name");
      for (Role *pRole = Role::pHead; pRole; pRole = (Role*)pRole->pNext)
      {
         if (sscanf(pRole->Name(), "Leading %s", sz) != 0)
         {
            fprintf(fp, ",%s", sz);
         }
         else if (strncmp(pRole->Name(), "Supp", 4) != 0)
         {
            fprintf(fp, ",%s", pRole->Name());
         }
      }
      fprintf(fp, ",Total\n");

      for (Person *pP = Person::pHead; pP; pP = (Person*)pP->pNext)
      {
      float fVal, fTtl = 0;
         fprintf(fp, "%s", pP->Name());
         for (Role *pRole = Role::pHead; pRole; pRole = (Role*)pRole->pNext)
         {
         char szRole[64];

            szRole[0] = '\0';
            if (sscanf(pRole->Name(), "Leading %s", sz) != 0)
            {
               sprintf(szRole, "Supporting %s", sz);
            }
            if (strncmp(pRole->Name(), "Supp", 4) != 0)
            {
               fVal = pP->ComputeScore(pRole->Name(), szRole, NULL);
               fprintf(fp,",%f", fVal);
               fTtl += fVal;
            }
         }
         fprintf(fp, ",%f\n", fTtl);
      }
      fclose(fp);
   }
   UpdStats();
}  // TForm1::btnOutStatClick

//---------------------------------------------------------------------------

void __fastcall TForm1::btnClearClick(TObject *Sender)
{
   unused(Sender);
   Memo1->Lines->Clear();
}  // TForm1::btnClearClick

//---------------------------------------------------------------------------

void __fastcall TForm1::mnuExitClick(TObject *Sender)
{
   unused(Sender);
   Application->Terminate();
}  // TForm1::mnuExitClick

//---------------------------------------------------------------------------
#include "PrtPage.h"
void __fastcall TForm1::mnuRptRanksClick(TObject *Sender)
{
   unused(Sender);
double dt = Date() + Time();
PrinterPageControl page;

   strcpy(page.szTitle, FormatDateTime("'Movie DB Rank Report 'yyyy-mm-dd hhnnss", dt).c_str());
//   if (bPrint)
   {
      if (!PrintDialog1->Execute())
      {
         return;           // cancelled
      }
      page.Setup(Printer());
   }
//   else //*/
/*
   {
   char szFN[256];
      sprintf(szFN, "%s.rpt", page.szTitle);
      page.Setup(szFN);
   }  //*/

   page.PrintBar('*');
   page.Print("\n");

char sz[1280], *psz;
cchar *pszItem;

   for (uint nYear = 1920; nYear < 2100; ++nYear)
   {
   Nexus *pNexus = Movie::FindRank(1, nYear, nYear, Rank::rnkYEAR, NULL);

      if (pNexus == NULL)
         continue;

      page.PrintBar('-');
      sprintf(sz,"Rank report for %u\n", nYear);
      page.Print(sz);

      for (uint nRank = 1; nRank <= Rank::pa[0].nDepth; ++nRank)
      {
         psz = sz;
         psz += sprintf(psz, "%2u\t", nRank);

         pNexus = Movie::FindRank(nRank, nYear, nYear, Rank::rnkYEAR, NULL);
         pszItem = (pNexus) ? pNexus->pMovie->Name() : "<none>";
         psz += sprintf(psz, "%s\t", pszItem);

         for (Role *pRole = Role::pHead; pRole; pRole = (Role*)pRole->pNext)
         {
            pNexus = Movie::FindRank(nRank, nYear, nYear, Rank::rnkYEAR, pRole);
            if (pNexus)
               psz += sprintf(psz, "%s in %s\t", pNexus->pPerson->Name(), pNexus->pMovie->Name());
            else
               psz += sprintf(psz, "<none>\t");
            break;
         }
         sprintf(psz, "\n\n");
         page.Print(sz);
      }
   }

}  // TForm1::mnuRptRanksClick

//---------------------------------------------------------------------------

void __fastcall TForm1::mnuActSaveClick(TObject *Sender)
{
   unused(Sender);
IniFile ini;
   Person::coef.Save(ini);
}  // TForm1::mnuActSaveClick

//---------------------------------------------------------------------------

#include "FrmType.h"

void __fastcall TForm1::btnQueryClick(TObject *Sender)
{
   unused(Sender);
   QueryForm->BuildForm();
   QueryForm->Show(NULL);
   UpdStats();
}  // TForm1::btnQueryClick


//---------------------------------------------------------------------------

