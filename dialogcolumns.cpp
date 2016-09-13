#include "dialogcolumns.h"
#include "ui_dialogcolumns.h"
#include <dialogsettings.h>

DialogColumns::DialogColumns(QFlightplanTable* _t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogColumns)
{
    ui->setupUi(this);

    t = _t;

    ui->checkBoxN->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::N));
    ui->checkBoxID->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::ID));
    ui->checkBoxType->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::TYPE));
    ui->checkBoxAlt->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::ALT));
    ui->checkBoxLat->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::LAT));
    ui->checkBoxLon->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::LON));
    ui->checkBoxMD->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::MD));
    ui->checkBoxOZMPUv->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::OZMPUV));
    ui->checkBoxOZMPUp->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::OZMPUP));
    ui->checkBoxPv->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::PV));
    ui->checkBoxPp->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::PP));
    ui->checkBoxMPU->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::MPU));
    ui->checkBoxIPU->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::IPU));
    ui->checkBoxS->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::S));
    ui->checkBoxSpas->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::SPAS));
    ui->checkBoxSrem->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::SREM));
    ui->checkBoxRSBN->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::RSBN));
    ui->checkBoxSm->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::SM));
    ui->checkBoxZm->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::ZM));
    ui->checkBoxMapAngle->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::MAPA));
    ui->checkBoxAtarg->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::ATRG));
    ui->checkBoxDtarg->setChecked(!t->horizontalHeader()->isSectionHidden(t->COL::DTRG));
}

DialogColumns::~DialogColumns()
{
    delete ui;
}

void DialogColumns::on_buttonBox_accepted()
{
/*
    DialogSettings::showN = ui->checkBoxN->isChecked();
    DialogSettings::showID = ui->checkBoxID->isChecked();
    DialogSettings::showType = ui->checkBoxType->isChecked();
    DialogSettings::showAlt = ui->checkBoxAlt->isChecked();
    DialogSettings::showLat = ui->checkBoxLat->isChecked();
    DialogSettings::showLon = ui->checkBoxLon->isChecked();
    DialogSettings::showMD = ui->checkBoxMD->isChecked();
    DialogSettings::showOZMPUv = ui->checkBoxOZMPUv->isChecked();
    DialogSettings::showOZMPUp = ui->checkBoxOZMPUp->isChecked();
    DialogSettings::showPv = ui->checkBoxPv->isChecked();
    DialogSettings::showPp = ui->checkBoxPp->isChecked();
    DialogSettings::showMPU = ui->checkBoxMPU->isChecked();
    DialogSettings::showIPU = ui->checkBoxIPU->isChecked();
    DialogSettings::showS = ui->checkBoxS->isChecked();
    DialogSettings::showSpas = ui->checkBoxSpas->isChecked();
    DialogSettings::showSrem = ui->checkBoxSrem->isChecked();
    DialogSettings::showRSBN = ui->checkBoxRSBN->isChecked();
    DialogSettings::showSm = ui->checkBoxSm->isChecked();
    DialogSettings::showZm = ui->checkBoxZm->isChecked();
    DialogSettings::showMapAngle = ui->checkBoxMapAngle->isChecked();
    DialogSettings::showAtarg = ui->checkBoxAtarg->isChecked();
    DialogSettings::showDtarg = ui->checkBoxDtarg->isChecked();
*/
    (ui->checkBoxN->isChecked() ? t->horizontalHeader()->showSection(t->COL::N) : t->horizontalHeader()->hideSection(t->COL::N));
    (ui->checkBoxID->isChecked() ? t->horizontalHeader()->showSection(t->COL::ID) : t->horizontalHeader()->hideSection(t->COL::ID));
    (ui->checkBoxType->isChecked() ? t->horizontalHeader()->showSection(t->COL::TYPE) : t->horizontalHeader()->hideSection(t->COL::TYPE));
    (ui->checkBoxAlt->isChecked() ? t->horizontalHeader()->showSection(t->COL::ALT) : t->horizontalHeader()->hideSection(t->COL::ALT));
    (ui->checkBoxLat->isChecked() ? t->horizontalHeader()->showSection(t->COL::LAT) : t->horizontalHeader()->hideSection(t->COL::LAT));
    (ui->checkBoxLon->isChecked() ? t->horizontalHeader()->showSection(t->COL::LON) : t->horizontalHeader()->hideSection(t->COL::LON));
    (ui->checkBoxMD->isChecked() ? t->horizontalHeader()->showSection(t->COL::MD) : t->horizontalHeader()->hideSection(t->COL::MD));
    (ui->checkBoxOZMPUv->isChecked() ? t->horizontalHeader()->showSection(t->COL::OZMPUV) : t->horizontalHeader()->hideSection(t->COL::OZMPUV));
    (ui->checkBoxOZMPUp->isChecked() ? t->horizontalHeader()->showSection(t->COL::OZMPUP) : t->horizontalHeader()->hideSection(t->COL::OZMPUP));
    (ui->checkBoxPv->isChecked() ? t->horizontalHeader()->showSection(t->COL::PV) : t->horizontalHeader()->hideSection(t->COL::PV));
    (ui->checkBoxPp->isChecked() ? t->horizontalHeader()->showSection(t->COL::PP) : t->horizontalHeader()->hideSection(t->COL::PP));
    (ui->checkBoxMPU->isChecked() ? t->horizontalHeader()->showSection(t->COL::MPU) : t->horizontalHeader()->hideSection(t->COL::MPU));
    (ui->checkBoxIPU->isChecked() ? t->horizontalHeader()->showSection(t->COL::IPU) : t->horizontalHeader()->hideSection(t->COL::IPU));
    (ui->checkBoxS->isChecked() ? t->horizontalHeader()->showSection(t->COL::S) : t->horizontalHeader()->hideSection(t->COL::S));
    (ui->checkBoxSpas->isChecked() ? t->horizontalHeader()->showSection(t->COL::SPAS) : t->horizontalHeader()->hideSection(t->COL::SPAS));
    (ui->checkBoxSrem->isChecked() ? t->horizontalHeader()->showSection(t->COL::SREM) : t->horizontalHeader()->hideSection(t->COL::SREM));
    (ui->checkBoxRSBN->isChecked() ? t->horizontalHeader()->showSection(t->COL::RSBN) : t->horizontalHeader()->hideSection(t->COL::RSBN));
    (ui->checkBoxSm->isChecked() ? t->horizontalHeader()->showSection(t->COL::SM) : t->horizontalHeader()->hideSection(t->COL::SM));
    (ui->checkBoxZm->isChecked() ? t->horizontalHeader()->showSection(t->COL::ZM) : t->horizontalHeader()->hideSection(t->COL::ZM));
    (ui->checkBoxMapAngle->isChecked() ? t->horizontalHeader()->showSection(t->COL::MAPA) : t->horizontalHeader()->hideSection(t->COL::MAPA));
    (ui->checkBoxAtarg->isChecked() ? t->horizontalHeader()->showSection(t->COL::ATRG) : t->horizontalHeader()->hideSection(t->COL::ATRG));
    (ui->checkBoxDtarg->isChecked() ? t->horizontalHeader()->showSection(t->COL::DTRG) : t->horizontalHeader()->hideSection(t->COL::DTRG));
}
