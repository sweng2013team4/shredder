#include "pattern.h"

Pattern::Pattern(QString sName, bool bIsEditable = true)
{
    this->sName = sName;
    this->bIsEditable = bIsEditable;
}

// Add a pass to a pattern
// NOTE: random cannot be verified and if specified true, it is ignored.
// ZERO and ONE passes value is enforced.
void Pattern::AddPass(Pass::PASS_TYPE pType, QString sVal, bool bVerify)
{
    Pass* psNew = new Pass();

    // Use correct values for ZERO/ONE pass
    if(pType == Pass::ZERO)
        sVal = "0";
    else if(pType == Pass::ONE)
        sVal = "255";

    psNew->pType = pType;
    psNew->sData = sVal;
    psNew->bVerify = bVerify;

    lstPasses.append(*psNew);
}

void Pattern::RemovePass(int nIndex)
{
    lstPasses.removeAt(nIndex);
}

void Pattern::MovePassUp(int nIndex)
{
    lstPasses.swap(nIndex, nIndex-1);
}

void Pattern::MovePassDown(int nIndex)
{
    lstPasses.swap(nIndex, nIndex+1);
}

/*
   Converts Pattern to ScrubPattern, allocates memory for object and must be freed
   Returns pointer to ScrubPattern
*/
ScrubPattern* Pattern::toScrubPattern()
{
    ScrubPattern* scrbPattern = new ScrubPattern(this->sName.toStdString());

    //scrbPattern->lstPasses.resize(lstPasses.count());

    // Loop through list and add passes to struct
    for(int nCurr = 0; nCurr < lstPasses.count(); nCurr++)
    {
       scrbPattern->lstPasses.push_back(lstPasses[nCurr].toScrubPass());
    }

    return scrbPattern;
}

bool Pattern::operator== ( const Pattern & other )
{
    return (this->sName == other.sName);
}
