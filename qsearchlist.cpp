#include "qsearchlist.h"
#include <QScrollBar>
#include <QHeaderView>
#include <waypoint.h>
#include <XFMS_DATA.h>
#include <QDebug>
#include <QMouseEvent>

#include <airway.h>
#include <airport_data.h>

QSearchList::QSearchList(QWidget *&w)
{
    setFocusPolicy(Qt::NoFocus);

    QString styleSheet = "background-color: rgb(0, 30, 0);"
                         "color: rgb(107, 239, 0);";

    horizontalScrollBar()->setStyleSheet(styleSheet);
    verticalScrollBar()->setStyleSheet(styleSheet);
}

void QSearchList::mousePressEvent(QMouseEvent* event)
{
    QModelIndex item = indexAt(event->pos());
    if (!item.isValid())
    {
        QModelIndex qI = currentIndex();
        qI = qI.child(-1, -1);
        setCurrentIndex(qI);
    }
    else if(currentIndex() == item)
    {
        QModelIndex qI = currentIndex();
        qI = qI.child(-1, -1);
        setCurrentIndex(qI);
    }
    else QListWidget::mousePressEvent(event);
}

void QSearchList::search(const QString &name, bool filter)
{
    currentSearch = name.toUpper();
    currentFilter = filter;
    std::vector<NVUPOINT*> lWP = XFMS_DATA::search(currentSearch);


    clear();
    if(currentSearch.isEmpty()) return;
    for(int i=0; i<lWP.size(); i++)
    {
        QListWidgetItemData *newItem = new QListWidgetItemData;
        QString qstr = lWP[i]->name;
        if(lWP[i]->type == WAYPOINT::TYPE_AIRWAY)
        {
            AIRWAY* awy = (AIRWAY*) lWP[i]->data;
            qstr = qstr + "  [" + awy->lATS[0]->name + "] - [" + awy->lATS.back()->name + "]";
        }
        else
        {
            if((!lWP[i]->name2.isEmpty()) && lWP[i]->type!=WAYPOINT::TYPE_FIX) qstr = qstr + " - " + lWP[i]->name2;
            if(!lWP[i]->country.isEmpty()) qstr = qstr + " [" + lWP[i]->country + "]";
        }//else
        newItem->setText(qstr);
        newItem->nvupoint = lWP[i];
        addItem(newItem, filter);

/*
        if(lWP[i]->type == WAYPOINT::TYPE_AIRPORT)
        {
            AIRPORT_DATA* ap = (AIRPORT_DATA*) lWP[i]->data;
            if(ap)
            {
                for(unsigned int j = 0; j<ap->lRunways.size(); j++)
                {
                    QString sString = "    " + ap->lRunways[j]->name + " (" + QString::number(LMATH::feetToMeter(ap->lRunways[j]->longest_runway), 'f', 1) + "m)";
                    newItem = new QListWidgetItemData;
                    newItem->setText(sString);
                    newItem->nvupoint = (NVUPOINT*) ap->lRunways[j];
                    addItem(newItem, filter);
                }
            }
        }
*/
    }

}

void QSearchList::refreshSearch()
{
    search(currentSearch, currentFilter);
}

void QSearchList::addItem(QListWidgetItemData *wd, bool filter)
{
    if(filter) switch(wd->nvupoint->type)
    {
        case WAYPOINT::TYPE_HELIPAD: if(showTYPE_AIRPORT); else return; break;
        case WAYPOINT::TYPE_RUNWAY: if(showTYPE_AIRPORT); else return; break;
        case WAYPOINT::TYPE_AIRPORT: if(showTYPE_AIRPORT); else return; break;
        case WAYPOINT::TYPE_NDB: if(showTYPE_NDB); else return; break;
        case WAYPOINT::TYPE_VORDME: if(showTYPE_VORDME); else return; break;
        case WAYPOINT::TYPE_VORTAC: if(showTYPE_VORDME); else return; break;
        case WAYPOINT::TYPE_ILS: if(showTYPE_VORDME); else return; break; //TODO currently we are combining ILS navaids with VOR/DME:s.
        case WAYPOINT::TYPE_VOR: if(showTYPE_VOR); else return; break;
        case WAYPOINT::TYPE_DME: if(showTYPE_DME); else return; break;
        case WAYPOINT::TYPE_TACAN: if(showTYPE_DME); else return; break;
        case WAYPOINT::TYPE_RSBN: if(showTYPE_RSBN); else return; break;
        case WAYPOINT::TYPE_FIX: if(showTYPE_FIX); else return; break;
        case WAYPOINT::TYPE_AIRWAY: if(showTYPE_AIRWAY); else return; break;

        default:;
    }


    if(filter) switch(wd->nvupoint->wpOrigin)
    {
        case WAYPOINT::ORIGIN_XNVU: if(showORIGIN_XNVU); else return; break;
        default:;
    }

    QListWidget::addItem(wd);
}

void QSearchList::showType(int type, bool show)
{
    switch(type)
    {
        case WAYPOINT::TYPE_AIRPORT: showTYPE_AIRPORT = show; break;
        case WAYPOINT::TYPE_NDB: showTYPE_NDB = show; break;
        case WAYPOINT::TYPE_VORDME: showTYPE_VORDME = show; break;
        case WAYPOINT::TYPE_VORTAC: showTYPE_VORDME = show; break;
        case WAYPOINT::TYPE_VOR: showTYPE_VOR = show; break;
        case WAYPOINT::TYPE_DME: showTYPE_DME = show; break;
        case WAYPOINT::TYPE_TACAN: showTYPE_DME = show; break;
        case WAYPOINT::TYPE_RSBN: showTYPE_RSBN = show; break;
        case WAYPOINT::TYPE_FIX: showTYPE_FIX = show; break;
        case WAYPOINT::TYPE_AIRWAY: showTYPE_AIRWAY = show; break;
    }
}


void QSearchList::showOrigin(int origin, bool show)
{
    switch(origin)
    {
        case WAYPOINT::ORIGIN_XNVU: showORIGIN_XNVU = show; break;
    }
}
NVUPOINT* QSearchList::getWaypoint(int _index)
{
    return getWaypoint(item(_index));
}

NVUPOINT *QSearchList::getWaypoint(QListWidgetItem *_lItem)
{
    QListWidgetItemData* lItem = (QListWidgetItemData*) _lItem;
    if(!lItem) return NULL;
    return lItem->nvupoint;
}

