#include "dialogoptions.h"
#include "ui_dialogoptions.h"
#include "dialogsettings.h"

DialogOptions::DialogOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptions)
{
    ui->setupUi(this);

    ui->checkBox_ATS->setChecked(DialogSettings::distAlignATS);
    ui->checkBox_EarthNav->setChecked(DialogSettings::distAlignEarthNav);
    ui->checkBox_FMS->setChecked(DialogSettings::distAlignFMS);
    ui->checkBox_WPS->setChecked(DialogSettings::distAlignWPS);
    ui->doubleSpinBoxDistance->setValue(DialogSettings::distAlignMargin);
    double meter = DialogSettings::distAlignMargin*1000.0;
    ui->label_Meter->setText("( " + QString::number(meter, 'f', 1) + " meters )");
}

DialogOptions::~DialogOptions()
{
    delete ui;
}

void DialogOptions::on_buttonBox_accepted()
{
    DialogSettings::distAlignATS = ui->checkBox_ATS->isChecked();
    DialogSettings::distAlignEarthNav = ui->checkBox_EarthNav->isChecked();
    DialogSettings::distAlignFMS = ui->checkBox_FMS->isChecked();
    DialogSettings::distAlignWPS = ui->checkBox_WPS->isChecked();
    DialogSettings::distAlignMargin = ui->doubleSpinBoxDistance->value();
}

void DialogOptions::on_doubleSpinBoxDistance_valueChanged(double arg1)
{
    double meter = arg1*1000.0;
    ui->label_Meter->setText("( " + QString::number(meter, 'f', 1) + " meters )");
}
