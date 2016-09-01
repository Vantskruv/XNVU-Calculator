#include "dialogrsbn.h"
#include "ui_dialogrsbn.h"
#include "XFMS_DATA.h"
#include "qlistwidgetitemdata.h"
#include "LMATH.h"

#include <QDebug>

DialogRSBN::DialogRSBN(NVUPOINT* wp, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRSBN)
{
    if(!wp) return;
    ui->setupUi(this);

    ui->listRSBN->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);



    nvupoint = wp;
    rsbn = nvupoint->rsbn;

    if(rsbn)
    {
        double d= LMATH::calc_distance(wp->latlon, rsbn->latlon);
        if(d>ui->spinBox->value()) ui->spinBox->setValue((int)d+10);
    }//

    initializeList();
}

DialogRSBN::~DialogRSBN()
{
    delete ui;
}


void DialogRSBN::initializeList()
{
    std::vector< std::pair<NVUPOINT*, double> > lRSBN = XFMS_DATA::getClosestRSBN(nvupoint, -1, ui->spinBox->value(), ui->checkBoxVORDME->isChecked());

    ui->listRSBN->clear();
    int iSelected = 0;
    ui->listRSBN->addItem("NONE");

    for(unsigned int i=0; i<lRSBN.size(); i++)
    {
        NVUPOINT* p = lRSBN[i].first;
        double d = lRSBN[i].second;
        QListWidgetItemData* iD = new QListWidgetItemData();
        iD->nvupoint = p;
        iD->setText(QString::number(d, 'f', 0) + " KM    " + p->name + (p->country.isEmpty() ? "" : " (" + p->country + ")") + (p->name2.isEmpty() ? "" : "  " + p->name2));
        ui->listRSBN->addItem(iD);
        if(nvupoint->rsbn == p) iSelected = i+1;
    }

    ui->listRSBN->setCurrentRow(iSelected);
}

void DialogRSBN::setWaypointDescription(const NVUPOINT* wp)
{
    ui->labelIWPName->setText("");
    ui->labelIWPName2->setText("");
    ui->labelWPType->setText("");
    ui->labelWPMagVar->setText("");
    ui->labelWPLatlon->setText("");
    ui->labelWPNote->setText("");

    if(wp == NULL) return;

    QString qstr = wp->name;

    if(!wp->country.isEmpty()) qstr = qstr + " [" + wp->country + "]";
    ui->labelIWPName->setText(qstr);

    if(!wp->name2.isEmpty()) ui->labelIWPName->setText(wp->name2);

    qstr = WAYPOINT::getTypeStr(wp);
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

    double l1, l2;
    l1 = fabs(modf(wp->latlon.x, &l2)*60.0);
    int i2 = (int) fabs(l2);
    qstr = "";
    qstr = qstr + "Lat:  " + (wp->latlon.x < 0 ? "S" : "N") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2) + "\n";
    l1 = fabs(modf(wp->latlon.y, &l2)*60.0);
    i2 = (int) fabs(l2);
    qstr = qstr + "Lon:  " + (wp->latlon.y < 0 ? "W" : "E") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2);
    ui->labelWPLatlon->setText(qstr);


    ui->labelWPMagVar->setText("Magnetic Declination: " + QString::number(wp->MD, 'f', 1));


    if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_AIRPORTS)
    {
        ui->labelWPNote->setText("Source: airports.txt (GNS430 AIRAC)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_NAVAIDS)
    {
        ui->labelWPNote->setText("Source: navaids.txt (GNS430 AIRAC");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_WAYPOINTS)
    {
        ui->labelWPNote->setText("Source: AIRAC waypoints.txt (GNS430 AIRAC)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_FMS)
    {
        ui->labelWPNote->setText("Source: Imported from user FMS flightplan");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_EARTHNAV)
    {
        ui->labelWPNote->setText("Source: X-Plane earth_nav.dat (for VOR/DME correction)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_XNVU)
    {
        ui->labelWPNote->setText("Source: xnvu_wps.txt (XNVU local library)");
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

void DialogRSBN::on_listRSBN_itemSelectionChanged()
{
    int c = ui->listRSBN->currentRow();
    /*
    if(c<0 || c>=ui->listRSBN->rowCount())
    {
        return;
    }
    */

    //qDebug() << "CurrentRow selected: " + QString::number(c);

    if(c == 0)
    {
        rsbn = NULL;
    }
    else
    {
        QListWidgetItemData* iD = (QListWidgetItemData*) ui->listRSBN->item(c);
        rsbn = iD->nvupoint;
    }//else

    setWaypointDescription((NVUPOINT*) rsbn);
}

void DialogRSBN::on_checkBoxVORDME_stateChanged(int arg1)
{
    initializeList();
}

void DialogRSBN::on_spinBox_valueChanged(int arg1)
{
    initializeList();
}
