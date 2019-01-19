//---------------------------------------------------------------------------

#ifndef MovieMainH
#define MovieMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#include "common.h"
#include <Buttons.hpp>
#include <Dialogs.hpp>
#include "FrmTools.h"
#include <Menus.hpp>

#include "FrmBase.h"

class Person;   // fwd ref
class Role;   // fwd ref

int Printf(cchar *pszFmt, ...);

class ErrorStack
{
public:
   int            idx;
   cchar          *apszCode[32];
   ulong          anCode[32];
                  ErrorStack(void);
            void  LogErr(void);
            void  Push(cchar *psz, ulong n = -123);
            void  Pop(void);
};
extern ErrorStack err;

//***********************************************************************
//************************************************************************

struct RatingRanks      // (rrs)
{
   uint           idxRat;       // into pStrRate
   uint           *panRk;
                  RatingRanks(void);
                  ~RatingRanks();
            RatingRanks& operator=(const RatingRanks &rrs);
            bool  DecodeAll(cchar *pszRateRank, TStringList *pStrRate = NULL);
            cchar *GetRating(TStringList *pStrRate = NULL);
            void  Reset(uint idxRatDef = 4);
}; // RatingRanks

//************************************************************************
//************************************************************************

struct Rank
{
   enum Ranks {rnkYEAR, rnkDEC,
               rnkMAX=10,       // for array allocation, nCnt is number defined
               NR=9999};        // NR used to flag non-ranked
   static uint    nCnt;
   static Rank    *pa;

   char           szCode[8];
   char           szName[64];
   uint           nWidth;
   uint           nDepth;
   uint           nStep;


            void  Print(uint nRank, char *&pszTemp);

   static   char  *Prt(uint nRank, char *pszTemp = NULL); // return is static
   static   uint  Decode(cchar *psz);
   static   uint  FindIdx(cchar *psz);          // only uses after RNKxxxx
}; // Rank


//************************************************************************
//************************************************************************

class RatingRanksAdjust
{
protected:
   TList          *pList;
   int            iAdj;
   uint           nTgt;
   int            idxRank;
public:
   uint           nLLmt;
   uint           nULmt;

                  RatingRanksAdjust(void);
                  ~RatingRanksAdjust();

   inline   void  AddAdj(RatingRanks &rrs)  {  pList->Add(&rrs);  }
   inline   void  AddSet(RatingRanks &rrs)  {  pList->Insert(0, &rrs);  }
            bool  ProcessList(void);
            bool  Setup(RatingRanks &rrsFrom, RatingRanks &rrsTo, int idxRank);
};  // RatingRanksAdjust

//************************************************************************
//************************************************************************

class Nexus
{
   friend class Movie;
   friend class TMoviesForm;
protected:
                  Nexus(void);
public: //
   enum Field {fldNONE=0x00,
               fldYEAR, fldFILM, fldFRAT, fldFRNK, fldFRNK_N=fldFRNK+0x2F,
               fldGENR, fldPERS, fldPERS_N=fldPERS+0x1f,
               fldROLE, fldRRAT, fldRRNK, fldRRNK_N=fldRRNK+0x2F,
               fldMAX,  fldMSK=FindMask(fldMAX)};
   enum FieldSets {fsetFILM=0x01, fsetROLE=0x02,
                   fsetPERSON=0x04, fsetROLES=0x08, fsetGENRE=0x10};

   enum FieldMatch {fmNONE=0, fmHEAD=0x0001,    // restart at headNexus
                    fmGRP=0x0002, fmPERSON=0x0004, fmTYPE=0x0008,
                    fmRATING=0x0010, fmRANK0=0x0020, //... fmRANK15
                    };

   static   uint  nCnt;

   Nexus          *pNext;


   Movie          *pMovie;
   Person         *pPerson;
   Role           *pRole;

   static TStringList *pStrRate;
   RatingRanks     rrs;

                  ~Nexus();
//            Nexus& operator=(const Nexus &nexus);
            void  AdjustCounts(bool bAdd);
            Nexus *NextMatch(FieldMatch fm);   // find next match Nexus in chain
   static   dword FieldDecode(cchar *&psz);
   static   void  FieldEncode(dword dwFlds, char *&psz);
            cchar *FieldGet(dword dwFlds, char *pszTemp);
   static   void  FieldList(TStringList *pStrs, FieldSets flds);

   static inline cchar *GetRating(RatingRanks &rrs)
                     {  return (rrs.GetRating(pStrRate));  }
   inline   cchar *GetRating(void)  {  return (rrs.GetRating(pStrRate));  }
}; // Nexus

//************************************************************************
//************************************************************************

class Genre : public NamedItem
{
   friend class TForm1;
public:
   static Genre   *pHead;
   static uint    nCnt;

   uint           nID;
   dword          dwMask;

                  Genre(cchar *psz);
                  ~Genre();
   static   void  Decode(char *&psz, dword dw, cchar *pszSep = ";");
   static   dword Encode(cchar *psz);
   virtual  void  Dump(char *&psz);
}; // Genre

//************************************************************************
//************************************************************************

class Movie : public NamedItem
{
   friend class TForm1;
public: //protected:
   static Movie   *pHead;
   static uint    nCnt;

   uint           nYear;
   dword          dwGenre;

   RatingRanks    rrs;
   static TStringList *pStrRate;

   Nexus          *pHeadNexus;
public:
                  Movie(cchar *pszName, uint nYear, dword deGenre);
                  ~Movie();
            Nexus *Add(Person *pPerson, Role *pRole, cchar *pszRating);
   virtual  void  Dump(char *&psz);
            Nexus *FindNexus(Person *pPerson, Role *pRole);
   static   Movie *Find(cchar *psz, Movie *pAfter = NULL);
   static   Nexus *FindRank(uint nRank, uint nLoYr, uint nHiYr, uint idxRank,
                        Role *pRole);   // pRole=NULL for movie lookup
   static inline cchar *GetRating(RatingRanks &rrs)
                     {  return (rrs.GetRating(pStrRate));  }
   inline   cchar *GetRating(void)  {  return (rrs.GetRating(pStrRate));  }
}; // Movie

//************************************************************************
//************************************************************************

class Person : public NamedItem
{
   friend class TForm1;
   friend class Movie;
public: //protected:
   static Person  *pHead;
   static uint    nCnt;
   struct Coef
   {
      float          afRatWt[2][7];

      float          fYrWinWt;
      float          fDcTopWt;
      float          fRtWinWt;
      float          fYrNomWt;
      float          fRtNomWt;

      float          fRtWinK;
      float          fRtNomK;
      float          fYrWinK;
      float          fYrNomK;
      float          fTop10K;
      float          fDcTopK;
      float          fDcBotK;


      float          fBlendQ;
               void  Load(IniFile &ini);
               void  Save(IniFile &ini);
   };
   static Coef    coef;
   long           id;

   uint           (*paanCnt)[7];
public:
                  Person(cchar *pszName, long liID = 0);
                  ~Person();
            float ComputeScore(cchar *pszRole1, cchar *pszRole2, FILE *fp);
   static   void  Configure(IniFile &ini);
   virtual  void  Dump(char *&psz);
   static   Person *Find(cchar *psz, Person *pAfter = NULL);
            Role  *LikelyRole(void);
}; // Person

//************************************************************************
//************************************************************************

class Role : public NamedItem
{
   friend class TForm1;
   friend class Movie;
public: //protected:
   uint           nID;
   bool           bGroup;
   bool           bBPNom;
   static uint    nMasterID;
   enum ColorIndex {ciBAR, ciLMT, ciOUT, ciTXT, ciMAX};
   TColor         aclr[ciMAX];
   struct ColorConfig
   {
      char           szKey[32];
      TColor         aclrDef[2];
   };
   static ColorConfig aClrCfg[ciMAX];
public:
   static Role    *pHead;
   static uint    nCnt;

                  Role(cchar *psz);
                  ~Role();
   virtual  void  Dump(char *&psz);
   static   Role  *Find(cchar *psz);
   static   Role  *Find(uint nID);
   static inline uint GetMax(void)  {  return (nMasterID);  }
   static   void  Save(void);
}; // Role

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class TForm1 : public TForm
{

__published:	// IDE-managed Components
        TMemo *Memo1;
        TButton *btnSave;
        TButton *btnRead;
        TSaveDialog *SaveDialog1;
        TOpenDialog *OpenDialog1;
        TButton *btnOutStat;
        TLabel *lblStats;
        TSpeedButton *btnMovie;
        TSpeedButton *btnPeople;
        TButton *btnClear;
        TMainMenu *MainMenu1;
        TMenuItem *File1;
        TMenuItem *mnuFileRead;
        TMenuItem *mnuFileSave;
        TMenuItem *mnuExit;
        TMenuItem *N1;
        TMenuItem *Actions1;
        TMenuItem *mnuActBuild;
        TMenuItem *mnuActDump;
        TPrintDialog *PrintDialog1;
        TButton *btnReport;
        TMenuItem *mnuActSave;
        TButton *btnGraph;
        TButton *btnQuery;
        void __fastcall btnDumpClick(TObject *Sender);
        void __fastcall btnSaveClick(TObject *Sender);
        void __fastcall btnReadClick(TObject *Sender);
        void __fastcall btnOutStatClick(TObject *Sender);
        void __fastcall btnClearClick(TObject *Sender);
        void __fastcall mnuExitClick(TObject *Sender);
        void __fastcall mnuRptRanksClick(TObject *Sender);
        void __fastcall mnuActSaveClick(TObject *Sender);
        void __fastcall btnFormClick(TObject *Sender);
        void __fastcall btnQueryClick(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
            void  ReadEvalCSV(cchar *pszFN);
            void  ReadRateCSV(cchar *pszFN);
            void  UpdStats(void);
public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
            void  Log(cchar *pszMsg);
   enum FlagType {ftDB_CHANGED, ftDB_SAVE};
            void  Flag(FlagType ft);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
