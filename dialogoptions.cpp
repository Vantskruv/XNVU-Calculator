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
    DialogSettings::customDateIsTrue = ui->checkBoxCustomDate->isChecked();
    DialogSettings::customDate = ui->dateEdit->date().toString("yyyy.MM.dd");
    DialogSettings::distAlignFMS = ui->checkBox_FMS->isChecked();
    DialogSettings::distAlignXWP = ui->checkBox_WPS->isChecked();
    DialogSettings::distAlignMargin = ui->doubleSpinBoxDistance->value();
}

void DialogOptions::on_doubleSpinBoxDistance_valueChanged(double arg1)
{
    double meter = arg1*1000.0;
    ui->label_Meter->setText("( " + QString::number(meter, 'f', 1) + " meters )");
}

void DialogOptions::on_checkBoxCustomDate_clicked()
{
    ui->dateEdit->setEnabled(ui->checkBoxCustomDate->isChecked());
}
