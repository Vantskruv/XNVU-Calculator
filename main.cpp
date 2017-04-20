#include "mainwindow.h"
#include <QApplication>
#include <QtConcurrent>
#include <QSettings>
#include "customloadingdialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setLibraryPaths(QStringList() << "");
    DialogSettings::loadSettings();

    CustomLoadingDialog loadDataDialog;
    loadDataDialog.setWindowIcon(QIcon(":/images/icon.ico"));
    loadDataDialog.exec();

    //a.processEvents();

    MainWindow w;
    w.setWindowIcon(QIcon(":/images/icon.ico"));
    w.show();

    return a.exec();
}
