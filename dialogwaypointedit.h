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
    explicit DialogWaypointEdit(NVUPOINT*, bool enableTemp, QWidget *parent = 0);
    ~DialogWaypointEdit();

private:
    Ui::DialogWaypointEdit *ui;

public:
    NVUPOINT* nvupoint = NULL;
    constexpr static int CANCEL = 0;
    constexpr static int SAVE_CURRENT = 1;
    constexpr static int CREATE_NEW = 2;
    constexpr static int CREATE_TEMP = 3;

private slots:
    void on_comboBox_Type_currentIndexChanged(int index);
    void on_pushButton_Cancel_clicked();
    void on_pushButton_SaveCurrent_clicked();
    void on_pushButton_CreateNew_clicked();
    void on_pushButton_CreateTemp_clicked();
};

#endif // DIALOGWAYPOINTEDIT_H
