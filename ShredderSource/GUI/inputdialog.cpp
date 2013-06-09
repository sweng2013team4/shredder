#include "inputdialog.h"
#include "ui_inputdialog.h"

#include "helper.h"
#include <QtGui>

InputDialog::InputDialog(QWidget *parent, QString strTitle,
                         QString strLabel, InputType itType,
                         int nMin, int nMax) :
        QDialog(parent),
        ui(new Ui::InputDialog)
{   
    ui->setupUi(this);

    this->itType = itType;
    this->nMin = nMin;
    this->nMax = nMax;

    this->setWindowTitle(strTitle);
    ui->lblText->setText(strLabel);
    ui->lnInput->setFocus();

    if(itType == PASSWORD)
        ui->lnInput->setEchoMode(QLineEdit::Password);
}

InputDialog::~InputDialog()
{
    delete ui;
}

void InputDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QString InputDialog::GetResult()
{
    return this->strResult;
}

bool InputDialog::CheckInput()
{
    bool bIsValid = true;
    int nLen = -1;

    // Check input type
    switch(this->itType)
    {
        case PASSWORD:
        case STRING:
        {
            nLen = ui->lnInput->text().length();
            break;
        }
        case NUMBERS:
        {
            QString sText = ui->lnInput->text();

            // Remove commas from the end
            while(sText.endsWith(","))
                sText.chop(1);

            QStringList lstStrings = sText.split(",", QString::SkipEmptyParts);
            nLen = lstStrings.count();

            for(int nCurr = 0; nCurr < lstStrings.count(); nCurr++)
            {
                bool ok;
                int num;

                // Convert number
                num = lstStrings[nCurr].toInt(&ok);

                // Check if it is a number and between the range
                if(!ok || num < nMin || num > nMax)
                {
                    Helper::InfoBox(this,
                                    "שגיאה",
                                    QString("פורמט לא תקין. טווח המספרים חייב להיות ") +
                                    QString::number(nMin) + "-" + QString::number(nMax) +
                                    QString(".\nהשתמש בפורמט הבא: מספר1,מספר2,מספר3."));

                    bIsValid = false;
                    break;
                }
            }

            ui->lnInput->setText(sText);

            break;
        }
    }

    if(nLen > MAXLEN || nLen == 0)
    {
        if(nLen > MAXLEN)
        {
            Helper::InfoBox(this,
                            "שגיאה",
                            QString("אורך השדה המקסימלי הוא ") +
                            QString::number(MAXLEN) + ".");
        }
        else
        {
            Helper::InfoBox(this,
                            "שגיאה",
                            "השדה ריק, אנא הכנס ערך.");
        }

        bIsValid = false;
    }

    return (bIsValid);
}

void InputDialog::on_btnOK_clicked()
{
    if(CheckInput())
    {
        strResult = ui->lnInput->text();
        done(QDialog::Accepted);
    }
}

void InputDialog::on_btnCancel_clicked()
{
    done(QDialog::Rejected);
}


