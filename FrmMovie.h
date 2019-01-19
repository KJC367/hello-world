//---------------------------------------------------------------------------

#ifndef FrmMovieH
#define FrmMovieH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>

//#include "FrmQuery.h"
#include "FrmBase.h"

class Movie;    // fwd ref

//---------------------------------------------------------------------------
// class TMoviesForm
//---------------------------------------------------------------------------

class TMoviesForm : public TBaseForm
{
__published:	// IDE-managed Components
        TLabel *lblYear;
        TLabel *lblRating;
        TEdit *edYear;
        TComboBox *cbRating;
        TLabel *lblGenre;
        TGroupBox *grpGenre;
        void __fastcall lblGenreClick(TObject *Sender);

            void __fastcall RankQuery(TObject *Sender);
private:	// User declarations
   dword          dwGenre;
   TEdit          **papedRank;

            void __fastcall AcceptOK(TObject *Sender);
   virtual  int   Accept(int iFlags);
   virtual  void  FieldList(TStringList *pStrs);
   virtual  void  GridMouse(TStringGrid *pSG, MouseBtn &mBtn);
   virtual  void  UpdForm(void);
public:		// User declarations
        __fastcall TMoviesForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMoviesForm *MoviesForm;
//---------------------------------------------------------------------------
#endif
