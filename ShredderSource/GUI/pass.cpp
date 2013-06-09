#include "pass.h"

QString Pass::PassTypes[PASS_TYPES_NUM] = { QString::fromUtf8("אפסים"),
                                            QString::fromUtf8("אחדות"),
                                            QString::fromUtf8("אקראי"),
                                            QString::fromUtf8("מחרוזת"),
                                            QString::fromUtf8("בית")
                                          };

QString sVerify = QString::fromUtf8(" (אימות)");

Pass::Pass()
{
}

QString Pass::toString(bool bIncVerify)
{
    QString sResult = PassTypes[this->pType];

    if(bIncVerify && this->bVerify)
        sResult += sVerify;

    if(this->pType == Pass::STRING ||
       this->pType == Pass::BYTE)
    {
        sResult += " - " + this->sData;
    }

    return sResult;
}

/*
   Converts Pass to ScrubPass and returns it
*/
ScrubPass Pass::toScrubPass()
{
    ScrubPass scrbPass;

    // Set scrub pass type, random is handled in switch
    if (this->bVerify)
        scrbPass.pType = ScrubPass::VERIFY;
    else
        scrbPass.pType = ScrubPass::NORMAL;

    // Check pass type and convert to array accordingly
    switch (this->pType)
    {
        case Pass::ZERO:
        case Pass::ONE:
        case Pass::BYTE:
        {
            bool ok;

            // Split strings
            QList<QString> lstString = this->sData.split(",", QString::SkipEmptyParts);

            // Convert strings to integers
            for(int nStr = 0; nStr < lstString.length(); nStr++)
            {
                scrbPass.arr.push_back((char)lstString[nStr].toInt(&ok));
            }

            break;
        }
        case Pass::STRING:
        {
            char* sStr = this->sData.toLocal8Bit().data();
            int nLen = strlen(sStr);

            // Convert each letter to a byte
            for(int nChar = 0; nChar < nLen; nChar++)
            {
                scrbPass.arr.push_back(sStr[nChar]);
            }

            break;
        }        
        case Pass::RAND:
        {
            scrbPass.pType = ScrubPass::RANDOM;
            scrbPass.arr.push_back(0);
            break;
        }
    }

    return scrbPass;
}
