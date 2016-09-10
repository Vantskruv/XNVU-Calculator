#include "dialogwpsedit.h"
#include "ui_dialogwpsedit.h"
#include "qlistwidgetitemdata.h"
#include "dialogwaypointedit.h"
#include "XFMS_DATA.h"

DialogWPSEdit::DialogWPSEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogWPSEdit)
{
    ui->setupUi(this);

    lXNVUTemp = XFMS_DATA::lXNVU;

    initializeList(NULL);
}

DialogWPSEdit::~DialogWPSEdit()
{
    delete ui;
}

void DialogWPSEdit::initializeList(NVUPOINT* select)
{
    QListWidgetItemData* cItem;
    NVUPOINT* p;
    int sIndex = -1;

    ui->listWPS->clear();
    for(int i=0; i<lXNVUTemp.size(); i++)
    {
        p = lXNVUTemp[i];
        cItem = new QListWidgetItemData();
        cItem->nvupoint = p;
        cItem->setText(p->name + (!p->country.isEmpty() ? (" [" + p->country + "]") : " " + (!p->name2.isEmpty() ? "  " + p->name2 : "")));
        ui->listWPS->addItem(cItem);
        if(select == p) sIndex = i;
    }

    if(sIndex>=0) ui->listWPS->setCurrentRow(sIndex);
}

void DialogWPSEdit::setWaypointDescription(const NVUPOINT* wp)
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
            wp->type == WAYPOINT::TYPE_VORDME)
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


    if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_AIRPORTS)
    {
        ui->labelWPNote->setText("Source: airports.txt (GNS430 AIRAC)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_NAVAIDS)
    {
        ui->labelWPNote->setText("Source: navaids.txt (GNS430 AIRAC)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_WAYPOINTS)
    {
        ui->labelWPNote->setText("Source: AIRAC waypoints.txt (GNS430 AIRAC)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_ATS)
    {
        ui->labelWPNote->setText("Source: AIRAC ats.txt (GNS430 AIRAC)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_FMS)
    {
        ui->labelWPNote->setText("Source: Imported from user FMS flightplan");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_EARTHNAV)
    {
        ui->labelWPNote->setText("Source: earth_nav.dat (X-Plane)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_XNVU)
    {
        ui->labelWPNote->setText("Source: xnvu_wps.txt (XNVU local library)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_XNVU_TEMP)
    {
        ui->labelWPNote->setText("Source: Custom user tempory waypoint");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_RSBN)
    {
        ui->labelWPNote->setText("Source: rsbn.dat (RSBN library)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_XNVU_FLIGHTPLAN)
    {
        ui->labelWPNote->setText("Source: Imported from XNVU flightplan");
    }
}


void DialogWPSEdit::on_listWPS_itemSelectionChanged()
{
    int c = ui->listWPS->currentRow();

    QListWidgetItemData* iD = (QListWidgetItemData*) ui->listWPS->item(c);

    if(iD) setWaypointDescription(iD->nvupoint);
    else setWaypointDescription(NULL);
}

void DialogWPSEdit::on_pushButton_Edit_clicked()
{
    int c = ui->listWPS->currentRow();

    QListWidgetItemData* iD = (QListWidgetItemData*) ui->listWPS->item(c);

    if(!iD) return;

    DialogWaypointEdit dEdit(iD->nvupoint, false);
    const int rv = dEdit.exec();

    switch(rv)
    {
        case DialogWaypointEdit::SAVE:
            initializeList(new NVUPOINT(dEdit.nvupoint));
        break;

        case DialogWaypointEdit::ADD_XNVU:
            NVUPOINT* nP = new NVUPOINT(dEdit.nvupoint);
            XFMS_DATA::addXNVUWaypoint(nP);
            lXNVUTemp.push_back(nP);
            initializeList(nP);
        break;
    }
}


void DialogWPSEdit::on_pushButton_Delete_clicked()
{
    int c = ui->listWPS->currentRow();

    QListWidgetItemData* iD = (QListWidgetItemData*) ui->listWPS->item(c);

    if(!iD) return;

    lRemove.push_back(iD->nvupoint);
    for(std::vector<NVUPOINT*>::iterator iL = lXNVUTemp.begin(); iL!=lXNVUTemp.end(); iL++)
    {
        if(*iL == iD->nvupoint)
        {
            lXNVUTemp.erase(iL);
            break;
        }
    }

    initializeList(NULL);
}

void DialogWPSEdit::on_pushButton_clicked()
{
    DialogWaypointEdit dEdit(NULL, false);
    const int rv = dEdit.exec();

    switch(rv)
    {
        case DialogWaypointEdit::ADD_XNVU:
            NVUPOINT* nP = new NVUPOINT(dEdit.nvupoint);
            XFMS_DATA::addXNVUWaypoint(nP);
            lXNVUTemp.push_back(nP);
            initializeList(nP);
        break;
    }
}

void DialogWPSEdit::on_buttonBox_accepted()
{
    done(QDialog::Accepted);
}

void DialogWPSEdit::on_buttonBox_rejected()
{
    done(QDialog::Rejected);
}
