#include "qflightplantable.h"
#include <QMouseEvent>
#include <QTreeView>
#include <QDebug>
#include <QScrollBar>
#include <QHeaderView>
#include <QAbstractButton>
#include "qtablewidgetitemdata.h"
#include "XFMS_DATA.h"
#include "LMATH.h"
#include "mainwindow.h"

void QFlightplanTable::mousePressEvent(QMouseEvent* event)
{
    QModelIndex item = indexAt(event->pos());
    if (!item.isValid())
    {
        QModelIndex qI = currentIndex();
        qI = qI.child(-1, -1);
        setCurrentIndex(qI);
    }
    else QTableWidget::mousePressEvent(event);
}

QFlightplanTable::QFlightplanTable(QWidget *&w) : QTableWidget(w)
{
    this->setFocusPolicy(Qt::NoFocus);

    QString styleSheet = "background-color: rgb(0, 30, 0);"
                         "color: rgb(107, 239, 0);";
    horizontalScrollBar()->setStyleSheet(styleSheet);
    verticalScrollBar()->setStyleSheet(styleSheet);
    horizontalHeader()->setStyleSheet(styleSheet);
    horizontalHeader()->show();
    verticalHeader()->setHidden(true);
}

/*
void QFlightplanTable::insertWaypoint(NVUPOINT* wp, int row)
{
    //Insert the waypoint at the position before row (becoming the row number)
    std::list<NVUPOINT*>::iterator iNVU = lNVU.begin();
    if(row<lNVU.size())
    {
        std::advance(iNVU, row);
        lNVU.insert(iNVU, wp);
    }//if
    else lNVU.push_back(wp);

    while(rowCount()>0) removeRow(0);
    initializeFlightplan();
}

void QFlightplanTable::replaceWaypoint(NVUPOINT* wp, int row)
{
    std::list<NVUPOINT*>::iterator iNVU = lNVU.begin();
    if(row<lNVU.size())
    {
        std::advance(iNVU, row);
        lNVU.insert(iNVU, wp);
        iNVU = lNVU.begin();
        std::advance(iNVU, row);
        iNVU++;
        lNVU.erase(iNVU);
    }//if
    else lNVU.push_back(wp);

    while(rowCount()>0) removeRow(0);
    initializeFlightplan();
}

void QFlightplanTable::moveWaypoint(int row, bool down)
{
    std::list<NVUPOINT*>::iterator iNVU = lNVU.begin();

    if(down)
    {
        if(row<(lNVU.size() - 1))
        {
            std::advance(iNVU, row);
            NVUPOINT* wm = *iNVU;
            lNVU.erase(iNVU);
            iNVU = lNVU.begin();
            std::advance(iNVU, row);
            iNVU++;
            lNVU.insert(iNVU, wm);
        }//if
        else return;
    }//if
    else
    {
        if(row>0)
        {
            std::advance(iNVU, row);
            NVUPOINT* wm = *iNVU;
            lNVU.erase(iNVU);
            iNVU = lNVU.begin();
            std::advance(iNVU, row);
            iNVU--;
            lNVU.insert(iNVU, wm);
        }
        else return;
    }

    while(rowCount()>0) removeRow(0);
    initializeFlightplan();
}

void QFlightplanTable::deleteWaypoint(int row)
{
    std::list<NVUPOINT*>::iterator iNVU = lNVU.begin();
    if(row>=0 && row<lNVU.size())
    {
        std::advance(iNVU, row);
        lNVU.erase(iNVU);
    }
    else return;

    while(rowCount()>0) removeRow(0);
    initializeFlightplan();
}


void QFlightplanTable::clearFlightplan()
{
    while(rowCount()>0) removeRow(0);
    XFMS_DATA::removeFMS();
    XFMS_DATA::removeXNVUFlightplan();
}

void QFlightplanTable::initializeFlightplan()
{
    std::list<NVUPOINT*>::iterator iNVU = lNVU.begin();
    NVUPOINT* waypoint;

    double totalD = 0.0;
    double deltaD = 0.0;
    for(int row = 0; iNVU!=lNVU.end(); iNVU++, row++)
    {
        waypoint = *iNVU;
        insertRow(row);
        QString qstr;
        QFont font;

        QTableWidgetItemData* itemD = new QTableWidgetItemData;
        itemD->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        itemD->setText(QString::number(row));
        itemD->nvupoint = waypoint;
        setItem(row, 0, itemD);

        itemD = new QTableWidgetItemData;
        itemD->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        font = itemD->font();
        font.setBold(true);
        itemD->setFont(font);
        itemD->setText(waypoint->name);
        setItem(row, 1, itemD);

        QTableWidgetItemData* item = new QTableWidgetItemData;
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        qstr = WAYPOINT::getTypeStr(waypoint);
        if(waypoint->type == WAYPOINT::TYPE_NDB ||
                waypoint->type == WAYPOINT::TYPE_RSBN)
        {
            qstr = qstr + " Ch" + QString::number((int) waypoint->freq);
        }//if
        else if(waypoint->type == WAYPOINT::TYPE_VOR ||
                waypoint->type == WAYPOINT::TYPE_DME ||
                waypoint->type == WAYPOINT::TYPE_VORDME)
        {
            qstr = qstr + " " + QString::number(waypoint->freq, 'f', 3);
        }//if

        item->setText(qstr);
        setItem(row, 2, item);

        item = new QTableWidgetItemData;
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        double l1, l2;
        l1 = fabs(modf(waypoint->latlon.x, &l2)*60.0);
        int i2 = (int) fabs(l2);
        item->setText((waypoint->latlon.x < 0 ? "S" : "N") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));
        setItem(row, 3, item);

        item = new QTableWidgetItemData;
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        l1 = fabs(modf(waypoint->latlon.y, &l2)*60.0);
        i2 = (int) fabs(l2);
        item->setText((waypoint->latlon.y < 0 ? "W" : "E") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));
        setItem(row, 4, item);

        item = new QTableWidgetItemData;
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        totalD+=deltaD;
        if(row>0)   //Set leg distance between this and previous point
        {
            QTableWidgetItemData* iP = (QTableWidgetItemData*) this->item(row-1, 0);
            deltaD = LMATH::calc_distance(iP->nvupoint->latlon, waypoint->latlon);
            this->item(row-1, 5)->setText(QString::number(deltaD, 'f', 1));
        }//if

        if(row<(rowCount()-1))
        {
            QTableWidgetItemData* iN = (QTableWidgetItemData*) this->item(row+1, 0);
            d = LMATH::calc_distance(iN->nvupoint->latlon, waypoint->latlon);
            item->setText(QString::number(d, 'f', 1));
        }
        else item->setText("0.0");
        setItem(row, 5, item);


        item = new QTableWidgetItemData;
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        item->setText(QString::number(totalD));
        setItem(row, 6, item);


        item = new QTableWidgetItemData;
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        if(waypoint->rsbn)
        {
            double d = LMATH::calc_distance(waypoint->latlon, waypoint->rsbn->latlon);
            item->setText(waypoint->rsbn->name +
                          (waypoint->rsbn->country.isEmpty() ? "" : + " (" + waypoint->rsbn->country + ")") +
                          (waypoint->rsbn->name2.isEmpty() ? "" : "  " + waypoint->rsbn->name2) +
                          " (" + QString::number(d, 'f', 0) + " KM)");
        }
        else item->setText("NO CORRECTION");
        setItem(row, 7, item);


        itemD = (QTableWidgetItemData*) this->item(0, 0);
        NVUPOINT* dep = itemD->nvupoint;
        itemD = (QTableWidgetItemData*) this->item(row, 0);
        NVUPOINT* arr = itemD->nvupoint;

        fork = LMATH::calc_fork(dep->latlon.x, dep->latlon.y, arr->latlon.x, arr->latlon.y, XFMS_DATA::dat);


        //ui->labelFork->setText("Fork   " + QString::number(f, 'f', 1));

    }//for

    totalDistance = totalD;
}
*/

/*
void QFlightplanTable::move(bool up)
{
    Q_ASSERT(selectedItems().count() > 0);
    const int sourceRow = row(selectedItems().at(0));
    const int destRow = (up ? sourceRow-1 : sourceRow+1);
    Q_ASSERT(destRow >= 0 && destRow < rowCount());

    // take whole rows
    QList<QTableWidgetItem*> sourceItems = takeRow(sourceRow);
    QList<QTableWidgetItem*> destItems = takeRow(destRow);

    // set back in reverse order
    setRow(sourceRow, destItems);
    setRow(destRow, sourceItems);
}

// takes and returns the whole row
QList<QTableWidgetItem*> QFlightplanTable::takeRow(int row)
{
    QList<QTableWidgetItem*> rowItems;
    qDebug() << "Row items: " << rowItems.size() << "\n";
    for (int col = 0; col < columnCount(); ++col)
    {
        rowItems << takeItem(row, col);
    }
    return rowItems;
}

// sets the whole row
void QFlightplanTable::setRow(int row, const QList<QTableWidgetItem*>& rowItems)
{
    for (int col = 0; col < columnCount(); ++col)
    {
        setItem(row, col, rowItems.at(col));
    }
}
*/
