#include "helper.h"

/*
  This class contains general utility functions.

*/

// Hebrew information box
void Helper::InfoBox(QWidget *parent, QString strTitle, QString strMsg, QMessageBox::Icon icon)
{
    QMessageBox* msgBox = new QMessageBox(icon,
                                          strTitle,
                                          strMsg,
                                          QMessageBox::NoButton,
                                          parent);

    msgBox->addButton("אישור", QMessageBox::AcceptRole);
    msgBox->setLayoutDirection(Qt::RightToLeft);

    msgBox->exec();
}

bool Helper::QuestionBox(QWidget *parent, QString strTitle, QString strQuestion)
// Hebrew yes/no box
{
    QMessageBox* msgBox = new QMessageBox(QMessageBox::Question,
                                          strTitle,
                                          strQuestion,
                                          QMessageBox::NoButton,
                                          parent);

    msgBox->addButton("כן", QMessageBox::YesRole);
    msgBox->addButton("לא", QMessageBox::NoRole);

    msgBox->setLayoutDirection(Qt::RightToLeft);

    int result = msgBox->exec();

    return (result == QMessageBox::AcceptRole);
}

void Helper::CenterWindow(QWidget *window)
{
    int screenWidth, width;
    int screenHeight, height;
    int x, y;
    QSize windowSize;

    QDesktopWidget *desktop = QApplication::desktop();
    QRect rect = desktop->screenGeometry();

    screenWidth = rect.width(); // get width of screen
    screenHeight = rect.height(); // get height of screen

    windowSize = window->size(); // size of our application window
    width = windowSize.width();
    height = windowSize.height();

    // little computations
    x = (screenWidth - width) / 2;
    y = (screenHeight - height) / 2;
    y -= 50;

    // move window to desired coordinates
    window->move(x, y);
}
