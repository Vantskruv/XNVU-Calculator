#ifndef QLISTWIDGETITEMDATA_H
#define QLISTWIDGETITEMDATA_H

#include <QListWidgetItem>
#include <NVU.h>

class QListWidgetItemData : public QListWidgetItem
{
    public:
        NVUPOINT* nvupoint;
        //WAYPOINT* waypoint;


        QListWidgetItemData();

};

#endif // QLISTWIDGETITEMDATA_H
