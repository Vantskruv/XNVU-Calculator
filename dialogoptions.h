#ifndef DIALOGOPTIONS_H
#define DIALOGOPTIONS_H

#include <QDialog>

namespace Ui {
class DialogOptions;
}

class DialogOptions : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptions(QWidget *parent = 0);
    ~DialogOptions();

private slots:
    void on_buttonBox_accepted();

    void on_doubleSpinBoxDistance_valueChanged(double arg1);

private:
    Ui::DialogOptions *ui;

public:
    bool alignATS;
    bool alignEarthNav;
    bool alignFMS;
    bool alignWPS;
    double distMargin;
};

#endif // DIALOGOPTIONS_H
