#ifndef PASS_H
#define PASS_H

#include <QtGui>

#include "Scrubber/scrubpass.h"

#define PASS_TYPES_NUM 5

class Pass
{
public:

    // Enum
    enum PASS_TYPE
    {
        ZERO,
        ONE,
        RAND,
        STRING,
        BYTE
    };

    // Static Members
    static QString PassTypes[PASS_TYPES_NUM];

    // Data Members
    PASS_TYPE pType;
    QString sData;
    bool    bVerify;

    // Member functions
    Pass();
    QString toString(bool bIncVerify = false);
    ScrubPass toScrubPass();
};

#endif // PASS_H
