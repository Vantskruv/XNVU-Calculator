#ifndef DIALOGWAYPOINTEDIT_H
#define DIALOGWAYPOINTEDIT_H

#include <QDialog>
#include "NVU.h"

namespace Ui {
class DialogWaypointEdit;
}

class DialogWaypointEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogWaypointEdit(const NVUPOINT *, bool isSave, QWidget *parent = 0);
    ~DialogWaypointEdit();

private:
    Ui::DialogWaypointEdit *ui;

public:
    NVUPOINT* newPoint;
    constexpr static int CANCEL = 0;
    constexpr static int SAVE = 1;
    constexpr static int ADD_XNVU = 2;

private slots:
    void on_comboBox_Type_currentIndexChanged(int index);
    void on_pushButton_Cancel_clicked();
    void on_pushButton_SaveCurrent_clicked();
    void on_pushButton_CreateNew_clicked();
    //void on_pushButton_CreateTemp_clicked();
};

#endif // DIALOGWAYPOINTEDIT_H
