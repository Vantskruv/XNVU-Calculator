#include "dialogrsbn.h"
#include "ui_dialogrsbn.h"
#include "XFMS_DATA.h"
#include "qlistwidgetitemdata.h"
#include "LMATH.h"
#include <dialogsettings.h>

#include <QDebug>

DialogRSBN::DialogRSBN(NVUPOINT* wp, NVUPOINT* wp2, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRSBN)
{
    if(!wp) return;
    ui->setupUi(this);

    ui->listRSBN->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    nvupoint = wp;
    nvupoint2 = wp2;
    rsbn = (NVUPOINT*) nvupoint->rsbn;

    ui->spinBox->setValue(DialogSettings::beaconDistance);
    ui->checkBoxVORDME->setChecked(DialogSettings::correctionVORDME);

    if(rsbn)    //If rsbn is already set for this waypoint, and distance is greater then default max-distance shown, set new distance + 10 km so the rsbn can be shown in list.
    {
        double d = LMATH::calc_distance(wp->latlon, rsbn->latlon);
        if(d>ui->spinBox->value()) ui->spinBox->setValue((int)d+10);
        if(rsbn->type!=WAYPOINT::TYPE_RSBN) ui->checkBoxVORDME->setChecked(true);
    }//

    ui->listRSBN->refresh(nvupoint, nvupoint2, ui->spinBox->value(), ui->checkBoxVORDME->isChecked(), rsbn);
    setWaypointDescription(rsbn);
}

DialogRSBN::~DialogRSBN()
{
    delete ui;
}

void DialogRSBN::setWaypointDescription(const NVUPOINT* wp)
{
    ui->labelIWPName2->setText("");
    ui->labelWPType->setText("");
    ui->labelWPType2->setText("");
    ui->labelWPMagVar->setText("");
    ui->labelWPLatlon->setText("");
    ui->labelWPNote->setText("");

    if(!wp) return;

    QString qstr;
    qstr = WAYPOINT::getTypeStr(wp);
    ui->labelWPType2->setText(qstr);

    qstr = wp->name;
    if(!wp->country.isEmpty()) qstr = qstr + " [" + wp->country + "]";

    if(wp->type == WAYPOINT::TYPE_NDB ||
       wp->type == WAYPOINT::TYPE_RSBN)
    {
        qstr = qstr + "  Ch" + QString::number((int) wp->freq);
    }//if
    else if(wp->type == WAYPOINT::TYPE_VOR ||
            wp->type == WAYPOINT::TYPE_DME ||
            wp->type == WAYPOINT::TYPE_VORDME ||
            wp->type == WAYPOINT::TYPE_VORTAC ||
            wp->type == WAYPOINT::TYPE_TACAN)
    {
        qstr = qstr + "  " + QString::number(wp->freq, 'f', 3);
    }//if
    ui->labelWPType->setText(qstr);

    if(wp->type == WAYPOINT::TYPE_AIRWAY)
    {
        AIRWAY* ats = (AIRWAY*) wp->data;
        ui->labelIWPName2->setText("[" + ats->lATS[0]->name + "] ---> [" + ats->lATS[ats->lATS.size()-1]->name + "]");
    }
    else if(!wp->name2.isEmpty()) ui->labelIWPName2->setText(wp->name2);

    if(wp->type == WAYPOINT::TYPE_AIRWAY)
    {
        AIRWAY* ats = (AIRWAY*) wp->data;
        ui->labelWPLatlon->setText("Fixes: " + QString::number(ats->lATS.size()) + "    Dist: " + QString::number(ats->distance, 'f', 1) + " KM");
    }
    else
    {
        double l1, l2;
        l1 = fabs(modf(wp->latlon.x, &l2)*60.0);
        int i2 = (int) fabs(l2);
        qstr = "";
        qstr = qstr + "Lat:   " + (wp->latlon.x < 0 ? "S" : "N") + (i2<10 ? "0" : "") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2) + "       ";
        l1 = fabs(modf(wp->latlon.y, &l2)*60.0);
        i2 = (int) fabs(l2);
        qstr = qstr + "Lon:  " + (wp->latlon.y < 0 ? "W" : "E") + (i2<100 ? (i2<10 ? "00" : "0") : "") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2);
        ui->labelWPLatlon->setText(qstr);
    }//else


    if(wp->type==WAYPOINT::TYPE_AIRWAY)
    {
        ui->labelWPMagVar->setText("CLICK TO SHOW WAYPOINTS");
    }
    else ui->labelWPMagVar->setText("Magnetic Declination: " + QString::number(wp->MD, 'f', 1));

    ui->labelWPNote->setText("Source: " + WAYPOINT::getOriginStr(wp->wpOrigin));
}

void DialogRSBN::on_checkBoxVORDME_stateChanged(int arg1)
{
    ui->listRSBN->refresh(nvupoint, nvupoint2, ui->spinBox->value(), ui->checkBoxVORDME->isChecked(), rsbn);
    setWaypointDescription(rsbn);
}

void DialogRSBN::on_spinBox_valueChanged(int arg1)
{
    ui->listRSBN->refresh(nvupoint, nvupoint2, ui->spinBox->value(), ui->checkBoxVORDME->isChecked(), rsbn);
    setWaypointDescription(rsbn);
}

void DialogRSBN::on_buttonBox_accepted()
{
    DialogSettings::beaconDistance = ui->spinBox->value();
    DialogSettings::correctionVORDME = ui->checkBoxVORDME->isChecked();
}

void DialogRSBN::on_listRSBN_itemClicked(QTableWidgetItem *item)
{
    int row = ui->listRSBN->row(item);
    rsbn = ui->listRSBN->getRSBN(row);
    setWaypointDescription(rsbn);
}
