#include "qtablewidgetitemdata.h"

QTableWidgetItemData::QTableWidgetItemData(NVUPOINT *wp)
{
    QFont font = this->font();
    font.setPointSize(9);
    setFont(font);

    nvupoint = wp;
}
