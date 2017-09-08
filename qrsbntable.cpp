#include "qrsbntable.h"
#include <QMouseEvent>
#include <QTreeView>
#include <QDebug>
#include <QScrollBar>
#include <QHeaderView>
#include <QAbstractButton>
#include "qtablewidgetitemdata.h"
#include "XFMS_DATA.h"
#include "LMATH.h"
#include "dialogsettings.h"

QRSBNTable::QRSBNTable(QWidget *w) : QTableWidget(w)
{
    this->setFocusPolicy(Qt::NoFocus);


    QString styleSheet = "background-color: rgb(0, 30, 0);"
                         "color: rgb(107, 239, 0);";
    horizontalScrollBar()->setStyleSheet(styleSheet);
    verticalScrollBar()->setStyleSheet(styleSheet);
    styleSheet = "::section {background-color: rgb(0, 30, 0);"
                         "color: rgb(107, 239, 0);}";
    horizontalHeader()->setStyleSheet(styleSheet);

    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    verticalHeader()->setHidden(true);

    //Setup columns and horizontal header
    setColumnCount(COL::_SIZE);

    setHorizontalHeaderLabels(COL_STR);
    setColumnWidth(COL::S, 75);
    setColumnWidth(COL::ZM, 75);
    setColumnWidth(COL::SM, 75);
    setColumnWidth(COL::ID, 100);
    setColumnWidth(COL::NAME, 250);
    verticalHeader()->setDefaultSectionSize(20);
    horizontalHeader()->setStretchLastSection(true);

    insertRow(0);
    for(unsigned int i=0; i<COL::_SIZE; i++) setItem(0, i, new QTableWidgetItem);
    item(0, 0)->setText("NONE");
    selectRow(0);
}

void QRSBNTable::refresh(NVUPOINT* _nvupoint, NVUPOINT* _nvupoint2, int _dist, bool _showVORDME, WAYPOINT *rsbn)
{
    int crow = 0;
    if(rsbn==NULL)
    {
        crow = currentRow();
        crow--;
        if(crow>=0) rsbn = lRSBN[crow].first;
        else crow = 0;
    }//if
    clearRSBNTable();

    lRSBN = XFMS_DATA::getClosestRSBN(_nvupoint, -1, _dist, _showVORDME);
    NVUPOINT dnvupoint(*_nvupoint);
    for(unsigned int row=0; row<lRSBN.size(); row++)
    {
        insertRow(row+1);
        for(unsigned int i=0; i<COL::_SIZE; i++) setItem(row+1, i, new QTableWidgetItem);
        WAYPOINT* r = (WAYPOINT*) lRSBN[row].first;
        double d = lRSBN[row].second;

        item(row+1, COL::S)->setText(QString::number(d, 'f', 0));
        if(_nvupoint2)
        {
            dnvupoint.setRSBN(r);
            dnvupoint.calc_rsbn_corr(_nvupoint2->latlon);
            item(row+1, COL::ZM)->setText(QString::number(dnvupoint.Zm, 'f', 0));
            item(row+1, COL::SM)->setText(QString::number(dnvupoint.Sm, 'f', 0));
        }//if
        item(row+1, COL::ID)->setText(r->name + (r->country.isEmpty() ? "" : " (" + r->country + ")"));
        item(row+1, COL::NAME)->setText((r->name2.isEmpty() ? "" : r->name2));

        if(rsbn == r) crow = row + 1;
    }

    selectRow(crow);
}


NVUPOINT* QRSBNTable::getRSBN(int row, bool lastOutOfIndex)
{
    if(row == 0) return NULL;

    if(row<1 || row>=rowCount())
    {
        if(lastOutOfIndex && lRSBN.size()>0) return lRSBN[lRSBN.size()-1].first;
        return NULL;
    }

    return lRSBN[row-1].first;
}


void QRSBNTable::clearRSBNTable()
{
    while(rowCount()>1) removeRow(1);
    lRSBN.clear();
    selectRow(0);
}
