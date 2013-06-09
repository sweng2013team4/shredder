#ifndef PATTERN_H
#define PATTERN_H

#include <QtGui>
#include "Scrubber/scrubpattern.h"
#include "pass.h"


class Pattern
{
public:
    QString    sName;
    bool       bIsEditable;
    QList<Pass> lstPasses;

    Pattern(QString sName, bool bIsEditable);
    void AddPass(Pass::PASS_TYPE pType, QString sVal = "0", bool bVerify = false);
    void RemovePass(int nIndex);
    void MovePassUp(int nIndex);
    void MovePassDown(int nIndex);
    ScrubPattern* toScrubPattern();
    bool operator== ( const Pattern &other );
    // QString toString() const;
};

#endif // PATTERN_H
