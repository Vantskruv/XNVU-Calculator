#ifndef QLISTWIDGETITEMDATA_H
#define QLISTWIDGETITEMDATA_H

#include <QListWidgetItem>
#include <NVU.h>

class QListWidgetItemData : public QListWidgetItem
{
    public:
        NVUPOINT* nvupoint = NULL;

        QListWidgetItemData();

};

#endif // QLISTWIDGETITEMDATA_H
