#include "customloadingdialog.h"
#include <QFuture>
#include <QtConcurrent>
#ifndef _WIN32
    #include <unistd.h>
#endif

/*
void CustomLoadingDialog::drawContents(QPainter* painter)
{

    painter->setPen(color);
    QFont font = QFont(painter->font());
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(QRect(0, 40, 320, 240), Qt::AlignHCenter, QString(XNVU_VERSION));

    font.setBold(false);
    painter->setFont(font);
    painter->drawText(QRect(40, 80, 280, 220), message);
}
*/

CustomLoadingDialog::CustomLoadingDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    resize(320, 240);
    setStyleSheet("background-image: url(:/images/splash.png);");

    connect(this, SIGNAL(loadingFinished()), this, SLOT(loadingIsFinished()));
    connect(this, SIGNAL(loadingStatus(const QString&)), this, SLOT(loadingDataStatus(const QString&)));
    int dat = QDate::currentDate().toJulianDay();
    if(DialogSettings::customDateIsTrue)
    {
        QDate date = QDate::fromString(DialogSettings::customDate, "yyyy.MM.dd");
        dat = date.toJulianDay();
    }
    QtConcurrent::run(this, &CustomLoadingDialog::runClean, dat);

}

void CustomLoadingDialog::paintEvent(QPaintEvent* painEvent)
{
    QDialog::paintEvent(painEvent);

    QPainter painter(this);
    painter.setPen(Qt::white);
    QFont font = QFont(painter.font());
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(0, 40, 320, 240), Qt::AlignHCenter, QString(XNVU_VERSION));

    font.setBold(false);
    painter.setFont(font);
    painter.drawText(QRect(40, 80, 280, 220), message);
}



void CustomLoadingDialog::loadingIsFinished()
{
    close();
}

void CustomLoadingDialog::loadingDataStatus(const QString& qstr)
{
    message = qstr;
    repaint();
}

void CustomLoadingDialog::showStatusMessage(const QString& _message, const QColor& _color)
{
    message = _message;
    color = _color;
    //showMessage(message, Qt::AlignCenter, color);
}

void CustomLoadingDialog::runClean(int dat)
{
    QString qStr;
    QFuture<void> fData = QtConcurrent::run(XFMS_DATA::load, dat);

    while(!fData.isStarted());  //We wait until fData is started
    if(DialogSettings::XP11)
    {
        QStringList sIDS = (QStringList() << "Airports" << "Fixes" << "Navaids" << "Airways" << "RSBN" << "XNVU");
        while(fData.isRunning())    //We updated status while fData is running
        {
            qStr = "Please wait while loading...\n";
            for(int i=0; i<6; i++)
            {
                qStr = qStr + sIDS[i] + "   [" + QString::number(XFMS_DATA::__DATA_LOADED[i]) + "]\n";
            }
            emit loadingStatus(qStr);
            #ifdef _WIN32
                Sleep(1);
            #else
                sleep(1);
            #endif
        }//while
    }//if
    else
    {
        QStringList sIDS = (QStringList() << "Airports" << "Navaids" << "Fixes" << "Earthnav" << "Airways" << "RSBN" << "XNVU");
        while(fData.isRunning())    //We updated status while fData is running
        {
            qStr = "Please wait while loading...\n";
            for(int i=0; i<7; i++)
            {
                qStr = qStr + sIDS[i] + "   [" + QString::number(XFMS_DATA::__DATA_LOADED[i]) + "]\n";
            }
            emit loadingStatus(qStr);
            #ifdef _WIN32
                Sleep(1);
            #else
                sleep(1);
            #endif
        }//while
    }
    fData.waitForFinished();
    emit loadingFinished();
}
