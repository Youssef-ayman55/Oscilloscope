#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QFile file(":/Darkeum.qss"); // Path to your QSS file in resources
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet); // Apply to the entire application
        w.setStyleSheet(styleSheet);
        file.close();
    }
    w.show();
    return a.exec();
}
