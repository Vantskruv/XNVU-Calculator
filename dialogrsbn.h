#ifndef DIALOGRSBN_H
#define DIALOGRSBN_H

#include <QDialog>
#include <NVU.h>
#include <QTableWidgetItem>

namespace Ui {
class DialogRSBN;
}

class DialogRSBN : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRSBN(NVUPOINT*, NVUPOINT *wp2, QWidget *parent = 0);
    ~DialogRSBN();

private:
    Ui::DialogRSBN *ui;

public:
    NVUPOINT* nvupoint;
    NVUPOINT* nvupoint2;
    WAYPOINT* rsbn;
    QString generateListString(double d, NVUPOINT* rsbn, NVUPOINT* dnvu);
    void setWaypointDescription(const NVUPOINT* wp);
    void initializeList();

private slots:
    void on_checkBoxVORDME_stateChanged(int arg1);
    void on_spinBox_valueChanged(int arg1);
    void on_buttonBox_accepted();
    void on_listRSBN_itemClicked(QTableWidgetItem *item);
};

#endif // DIALOGRSBN_H
