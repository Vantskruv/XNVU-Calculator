#ifndef DIALOGRSBN_H
#define DIALOGRSBN_H

#include <QDialog>
#include <NVU.h>

namespace Ui {
class DialogRSBN;
}

class DialogRSBN : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRSBN(NVUPOINT*, QWidget *parent = 0);
    ~DialogRSBN();

private:
    Ui::DialogRSBN *ui;

public:
    NVUPOINT* nvupoint;
    WAYPOINT* rsbn;
    void setWaypointDescription(const NVUPOINT* wp);
    void initializeList();

private slots:
    void on_listRSBN_itemSelectionChanged();
    void on_checkBoxVORDME_stateChanged(int arg1);
    void on_spinBox_valueChanged(int arg1);
    void on_buttonBox_accepted();
};

#endif // DIALOGRSBN_H
