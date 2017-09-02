#include "dialogoptions.h"
#include "ui_dialogoptions.h"
#include "dialogsettings.h"
#include <QDebug>

DialogOptions::DialogOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptions)
{
    ui->setupUi(this);

    ui->checkBoxCustomDate->setChecked(DialogSettings::customDateIsTrue);
    ui->dateEdit->setEnabled(DialogSettings::customDateIsTrue);
    ui->dateEdit->setDate(QDate::fromString(DialogSettings::customDate, "yyyy.MM.dd"));
    ui->checkBox_FMS->setChecked(DialogSettings::distAlignFMS);
    ui->checkBox_WPS->setChecked(DialogSettings::distAlignXWP);
    ui->spinBoxDistance->setValue(DialogSettings::distAlignMargin*1000);
    double km = DialogSettings::distAlignMargin;
    ui->label_Meter->setText(" meters ( " + QString::number(km, 'g', 6) + " KM )");
}

DialogOptions::~DialogOptions()
{
    delete ui;
}

void DialogOptions::on_buttonBox_accepted()
{
    DialogSettings::customDateIsTrue = ui->checkBoxCustomDate->isChecked();
    DialogSettings::customDate = ui->dateEdit->date().toString("yyyy.MM.dd");
    DialogSettings::distAlignFMS = ui->checkBox_FMS->isChecked();
    DialogSettings::distAlignXWP = ui->checkBox_WPS->isChecked();
    DialogSettings::distAlignMargin = ui->spinBoxDistance->value()/1000.0;
}

void DialogOptions::on_checkBoxCustomDate_clicked()
{
    ui->dateEdit->setEnabled(ui->checkBoxCustomDate->isChecked());
}

void DialogOptions::on_spinBoxDistance_valueChanged(int arg1)
{
    double km = arg1/1000.0;
    ui->label_Meter->setText(" meters ( " + QString::number(km, 'g', 6) + " KM )");
}
