#include "qflightplantable.h"
#include <QMouseEvent>
#include <QTreeView>
#include <QDebug>

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
}

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
