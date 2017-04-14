#ifndef CUSTOMLOADINGDIALOG_H
#define CUSTOMLOADINGDIALOG_H

#include <QDialog>
#include <QPainter>
#include "mainwindow.h"

class CustomLoadingDialog : public QDialog
{
    Q_OBJECT

    private:
        QString message;
        //int alignment;
        QColor color;
        QRect rect;
        QMutex mutex;

    signals:
        void loadingFinished();
        void loadingStatus(const QString&);

    private slots:
        void loadingIsFinished();
        void loadingDataStatus(const QString&);


    public:
        virtual void paintEvent(QPaintEvent* painEvent);
        CustomLoadingDialog(QWidget* parent = 0);
        ~CustomLoadingDialog(){}

        void showStatusMessage(const QString& _message, const QColor& _color = Qt::black);
        void runClean(int dat);
};

#endif // CUSTOMLOADINGDIALOG_H
