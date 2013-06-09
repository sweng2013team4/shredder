#include <QtGui/QApplication>
#include <QtCore/QTextCodec>

#include "shredder.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    // Register type for signals
    qRegisterMetaType<Logger::MSG_TYPE>("Logger::MSG_TYPE");
    Shredder::GetInstance()->show();
    return a.exec();
}
