#ifndef DIALOGWPSEDIT_H
#define DIALOGWPSEDIT_H

#include <QDialog>
#include <NVU.h>

namespace Ui {
class DialogWPSEdit;
}

class DialogWPSEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogWPSEdit(QWidget *parent = 0);
    ~DialogWPSEdit();

private slots:
    void on_listWPS_itemSelectionChanged();
    void on_pushButton_Edit_clicked();
    void on_pushButton_Delete_clicked();
    void on_pushButton_CreateNew_clicked();

    void on_pushButton_clicked();

private:
    Ui::DialogWPSEdit *ui;

    void setWaypointDescription(const NVUPOINT* wp);
    void initializeList(NVUPOINT *select);

public:

};

#endif // DIALOGWPSEDIT_H
