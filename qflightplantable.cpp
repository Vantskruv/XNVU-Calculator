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
    //horizontalHeader()->show();
    verticalHeader()->setHidden(true);


    //Setup columns and horizontal header
    setColumnCount(9);
    setHorizontalHeaderLabels(QStringList() << "N" << "Identifier" << "Type" << "Altitude" << "Latitude" << "Longitude" << "S" << "Spas"<< "RSBN and/or VORDME");
    setColumnWidth(0, 50);
    setColumnWidth(1, 100);
    setColumnWidth(2, 100);
    setColumnWidth(3, 100);
    setColumnWidth(4, 150);
    setColumnWidth(5, 150);
    setColumnWidth(6, 75);
    setColumnWidth(7, 75);
    setColumnWidth(8, 360);
    //horizontalHeader()->show() //Does not work???
    verticalHeader()->setDefaultSectionSize(20);
    horizontalHeader()->setStretchLastSection(true);
}

void QFlightplanTable::deleteWaypoint(int row)
{
    if(row<0 || row>=rowCount()) return;

    QTableWidgetItemData* pItem;
    QTableWidgetItemData* nItem;


    if(row  == (rowCount() - 1)) //We change the distance of previous point, as that point will be the last one.
    {
        if(row>0) item(row-1, 6)->setText("0.0");
    }
    else if(row>0)                                //We set distance between previous and next waypoint, at the previous point
    {
        pItem = (QTableWidgetItemData*) item(row-1, 0);
        nItem = (QTableWidgetItemData*) item(row+1, 0);
        double d = LMATH::calc_distance(pItem->nvupoint->latlon, nItem->nvupoint->latlon);
        item(row - 1, 6)->setText(QString::number(d, 'f', 1));
    }

    pItem = (QTableWidgetItemData*) item(row, 0);
    delete pItem->nvupoint;

    removeRow(row);

    updateDistanceAndN();
}

void QFlightplanTable::insertWaypoint(NVUPOINT* wp, int row, int offset)
{
    if(wp->type == WAYPOINT::TYPE_AIRWAY) return;
    if(row<0) row = 0;

    if(offset<0)
    {
        insertWaypoint(wp, row);
        selectRow(row);
    }
    else if(offset>0)
    {
        if(currentRow()<0) row = 0;
        else row++;
        if(row > rowCount()) row = rowCount();
        insertWaypoint(wp, row);
        selectRow(row);
    }
    else
    {
        refreshRow(row, wp);
        //removeRow(row);
        //insertWaypoint(wp, row);
        selectRow(row);
    }
}

void QFlightplanTable::insertWaypoint(NVUPOINT* wp, int row)
{
    insertRow(row);
    for(int i=0; i<9; i++)
    {
        setItem(row, i, new QTableWidgetItemData);
    }

    refreshRow(row, wp);

}

void QFlightplanTable::insertRoute(std::vector<NVUPOINT*> route, int row, int offset)
{
    row = row -  (offset < 0 ? 1 : 0);
    for(int i=0; i<route.size(); i++)
    {
        if(i == 0 && offset==0) insertWaypoint(route[i], row, 0);
        else insertWaypoint(route[i], row, 1);
        row = currentRow();
    }
}

void QFlightplanTable::moveWaypoint(int row, bool up)
{
    if(row<0 || row>=rowCount()) return;

    QTableWidgetItemData* pItem;
    QTableWidgetItemData* nItem;

    pItem = (QTableWidgetItemData*) item(row, 0);
    NVUPOINT* mP = pItem->nvupoint;

    if(row  == (rowCount() - 1)) //We change the distance of previous point, as that point will be the last one.
    {
        if(row>0) item(row-1, 6)->setText("0.0");
    }
    else if(row>0)                                //We set distance between previous and next waypoint, at the previous point
    {
        pItem = (QTableWidgetItemData*) item(row-1, 0);
        nItem = (QTableWidgetItemData*) item(row+1, 0);
        double d = LMATH::calc_distance(pItem->nvupoint->latlon, nItem->nvupoint->latlon);
        item(row - 1, 6)->setText(QString::number(d, 'f', 1));
    }
    removeRow(row);

    if(up)
    {
        insertWaypoint(mP, row-1, -1);
        selectRow(row - 1);
    }//if
    else
    {
        insertWaypoint(mP, row, 1);
        selectRow(row + 1);
    }//else
}

void QFlightplanTable::refreshRow(int row, NVUPOINT* waypoint)
{
    if(row>=rowCount()) return;

    QString qstr;
    QTableWidgetItemData* itemD;

    itemD = (QTableWidgetItemData*) item(row, 0);
    itemD->setText(QString::number(row));
    if(waypoint==NULL) waypoint = itemD->nvupoint;
    else itemD->nvupoint = waypoint;

    item(row, 1)->setText(waypoint->name);

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
    item(row, 2)->setText(qstr);

    item(row, 3)->setText(QString::number((showMeters ? LMATH::feetToMeter(waypoint->alt) : waypoint->alt)));

    double l1, l2;
    l1 = fabs(modf(waypoint->latlon.x, &l2)*60.0);
    int i2 = (int) fabs(l2);
    item(row, 4)->setText((waypoint->latlon.x < 0 ? "S" : "N") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));

    l1 = fabs(modf(waypoint->latlon.y, &l2)*60.0);
    i2 = (int) fabs(l2);
    item(row, 5)->setText((waypoint->latlon.y < 0 ? "W" : "E") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));

    item(row, 6)->setText("");
    item(row, 7)->setText("");

    if(waypoint->rsbn)
    {
        double d = LMATH::calc_distance(waypoint->latlon, waypoint->rsbn->latlon);
        qstr =        waypoint->rsbn->name +
                      (waypoint->rsbn->country.isEmpty() ? "" : + " (" + waypoint->rsbn->country + ")") +
                      (waypoint->rsbn->name2.isEmpty() ? "" : "  " + waypoint->rsbn->name2) +
                      " (" + QString::number(d, 'f', 0) + " KM)";
    }
    else qstr = "NO CORRECTION";
    item(row, 8)->setText(qstr);

    updateDistanceAndN();
}

void QFlightplanTable::refreshFlightplan()
{
    for(int i=0; i<rowCount(); i++) refreshRow(i);
}


void QFlightplanTable::updateDistanceAndN()
{
    QTableWidgetItemData* iN;
    QTableWidgetItemData* iC;
    double currentDistance = 0.0;
    double legDistance;

    if(rowCount()>0)
    {
        item(0, 7)->setText("0.0");
        item(rowCount()-1, 7)->setText(QString::number(currentDistance, 'f', 1));
        item(rowCount()-1, 0)->setText(QString::number(rowCount()));

        iN = (QTableWidgetItemData*) item(0, 0);
        iC = (QTableWidgetItemData*) item(rowCount()-1, 0);
        fork = LMATH::calc_fork(iN->nvupoint->latlon.x, iN->nvupoint->latlon.y, iN->nvupoint->alt, iC->nvupoint->latlon.x, iC->nvupoint->latlon.y, iC->nvupoint->alt, dat);
        qFork->setText("Fork   " + QString::number(fork, 'f', 1));
    }
    else qFork->setText("Fork   0.0");

    for(int i=0; i<rowCount() - 1; i++)
    {
        iC = (QTableWidgetItemData*) item(i, 0);
        iN = (QTableWidgetItemData*) item(i+1, 0);

        legDistance = LMATH::calc_distance(iN->nvupoint->latlon, iC->nvupoint->latlon);
        item(i, 6)->setText(QString::number(legDistance, 'f', 1));
        item(i, 7)->setText(QString::number(currentDistance, 'f', 1));
        item(i, 0)->setText(QString::number(i+1));
        currentDistance+=legDistance;
    }

    if(rowCount()>0) item(rowCount()-1, 7)->setText(QString::number(currentDistance, 'f', 1));
}

void QFlightplanTable::clearFlightplan()
{
    while(rowCount()>0)
    {
        QTableWidgetItemData* cI = (QTableWidgetItemData*) item(0, 0);
        delete cI->nvupoint;
        removeRow(0);
    }
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
