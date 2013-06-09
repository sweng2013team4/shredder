#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>

#define MAXLEN 30

namespace Ui {
    class InputDialog;
}

class InputDialog : public QDialog {
    Q_OBJECT

public:
    enum InputType
    {
        STRING,
        NUMBERS,
        PASSWORD
    };

    InputDialog(QWidget *parent, QString strTitle,
                QString strLabel, InputType itType, int nMin = 0, int nMax = 0);
    ~InputDialog();
    QString GetResult();

private:
    Ui::InputDialog *ui;
    QString strResult;
    int nMin;
    int nMax;
    InputType itType;

    bool CheckInput();

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_btnCancel_clicked();
    void on_btnOK_clicked();
};

#endif // INPUTDIALOG_H
