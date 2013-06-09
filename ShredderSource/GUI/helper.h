#ifndef HELPER_H
#define HELPER_H

#include <QtGui>

class Helper
{
public:
    static void InfoBox(QWidget* parent, QString strTitle, QString strMsg, QMessageBox::Icon icon = QMessageBox::Information);
    static bool QuestionBox(QWidget *parent, QString strTitle, QString strQuestion);
    static void CenterWindow(QWidget* window);
};

#endif // HELPER_H
