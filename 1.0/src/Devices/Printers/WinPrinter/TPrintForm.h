//---------------------------------------------------------------------------

#ifndef TPrintFormH
#define TPrintFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TPrintForm : public TForm
{
__published:	// IDE-managed Components
  TRichEdit *PrintBox;
private:	// User declarations
public:		// User declarations
  __fastcall TPrintForm(TComponent* Owner);
  void Clear();
  void Add(AnsiString line);
  void Print();
};
//---------------------------------------------------------------------------
extern PACKAGE TPrintForm *PrintForm;
//---------------------------------------------------------------------------
#endif
