#ifndef QSEARCHLIST_H
#define QSEARCHLIST_H

#include <QListWidget>
#include <qlistwidgetitemdata.h>
class QSearchList : public QListWidget
{
private:
    bool showORIGIN_XNVU = true;
    bool showTYPE_AIRPORT = true;
    bool showTYPE_NDB = true;
    bool showTYPE_VORDME = true;
    bool showTYPE_VOR = true;
    bool showTYPE_DME = true;
    bool showTYPE_RSBN = true;
    bool showTYPE_FIX = true;
    bool showTYPE_AIRWAY = true;

    QString currentSearch;
    bool currentFilter = true;

public:
    QSearchList(QWidget*&w);
    void mousePressEvent(QMouseEvent* event);
    void search(const QString& name, bool filter);
    void refreshSearch();
    void addItem(QListWidgetItemData* wd, bool filter);
    void showType(int type, bool show);
    void showOrigin(int origin, bool show);
};

#endif // QSEARCHLIST_H
