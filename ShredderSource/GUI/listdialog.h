#ifndef LISTDIALOG_H
#define LISTDIALOG_H

#include <QtGui>

#include "DiskManager/diskmanager.h"

namespace Ui {
    class ListDialog;
}

class ListDialog : public QDialog {
    Q_OBJECT
public:
    ListDialog(QWidget *parent, QString strTitle,
                 QString strLabel, QList<QListWidgetItem>* lstItems);

    ~ListDialog();
    Ui::ListDialog *ui;
    int GetSelectedRow();

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_btnCancel_clicked();
    void on_btnOk_clicked();
};

#endif // LISTDIALOG_H
