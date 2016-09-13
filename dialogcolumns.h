#ifndef DIALOGCOLUMNS_H
#define DIALOGCOLUMNS_H

#include <QDialog>
#include "qflightplantable.h"
namespace Ui {
class DialogColumns;
}

class DialogColumns : public QDialog
{
    Q_OBJECT

public:
    explicit DialogColumns(QFlightplanTable* _t, QWidget *parent = 0);
    ~DialogColumns();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogColumns *ui;

    QFlightplanTable* t;
};

#endif // DIALOGCOLUMNS_H
