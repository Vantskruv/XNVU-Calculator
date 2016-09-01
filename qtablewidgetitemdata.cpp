#include "qtablewidgetitemdata.h"

QTableWidgetItemData::QTableWidgetItemData()
{
    //this->setBackground(QColor(255, 255, 255));
    //this->setBackground();

    QFont font = this->font();
    font.setPointSize(9);
    //font.setBold(true);
    //font.setItalic(true);
    this->setFont(font);
}
