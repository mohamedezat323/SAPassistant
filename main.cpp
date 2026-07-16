#include "widget.h"

#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QSharedMemory>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSharedMemory sharedMemory("SAP_Assistant_Unique_Key_55335533");

    if (!sharedMemory.create(1)) {
        // If create(1) fails, it's because the memory already exists
        QMessageBox::critical(nullptr, " ",
                              "SAP assistant is already running.");
        return 0; // Exit the app immediately
    }

    QFile styleSheetFile(":/Chatbee.qss");

    if (styleSheetFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleSheetFile.readAll());
        a.setStyleSheet(styleSheet);
        styleSheetFile.close();
    }

    Widget w;
    w.show();
    return QCoreApplication::exec();
}
