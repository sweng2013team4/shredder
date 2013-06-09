#include "listdialog.h"
#include "ui_listdialog.h"
#include <QtGui>

ListDialog::ListDialog(QWidget *parent, QString strTitle,
                           QString strLabel, QList<QListWidgetItem>* lstItems) :
    QDialog(parent),
    ui(new Ui::ListDialog)
{
    ui->setupUi(this);

    this->setWindowTitle(strTitle);
    ui->lblText->setText(strLabel);

    for(int i = 0; i < lstItems->count(); i++)
        ui->lstItems->addItem(&(*lstItems)[i]);
}

ListDialog::~ListDialog()
{
    delete ui;
}

void ListDialog::changeEvent(QEvent *e)
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

void ListDialog::on_btnOk_clicked()
{
    if(ui->lstItems->currentIndex().isValid())
    {
        done(QDialog::Accepted);
    }
}

void ListDialog::on_btnCancel_clicked()
{
    done(QDialog::Rejected);
}

int ListDialog::GetSelectedRow()
{
    return (ui->lstItems->currentRow());
}
