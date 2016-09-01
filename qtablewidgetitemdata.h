#ifndef QTABLEWIDGETITEMDATA_H
#define QTABLEWIDGETITEMDATA_H


#include <QTableWidgetItem>
#include <NVU.h>

class QTableWidgetItemData : public QTableWidgetItem
{
public:
    NVUPOINT* nvupoint;
    //WAYPOINT* waypoint;
    std::vector< std::pair<NVUPOINT*, double> > lRSBN_Dist;

    QTableWidgetItemData();
};

#endif // QTABLEWIDGETITEMDATA_H
