//---------------------------------------------------------------------------

#ifndef TNoteH
#define TNoteH

#include <vector>
#include "LogClass.h"

class TNote
{
private:
    void swap(const TNote& rhs);
public:
    TNote();
    TNote(int _nominal, int _count, int _ValidatorID = 0, std::string _currencyID = ""):
        Nominal(_nominal), Count(_count), ValidatorID(_ValidatorID), CurrencyID(_currencyID) {}
    TNote(const TNote& rhs);
    TNote& operator = (const TNote& rhs);

    int ValidatorID;
    std::string CurrencyID;
    double Nominal;
    /*int Nominal;*/
    int Count;
};
typedef std::vector<TNote> TNotesVector;

//---------------------------------------------------------------------------
#endif
 