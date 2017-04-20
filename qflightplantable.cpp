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
#include "dialogsettings.h"
//#include "mainwindow.h"




QFlightplanTable::QFlightplanTable(QWidget *&w) : QTableWidget(w)
{
    this->setFocusPolicy(Qt::NoFocus);

    QString styleSheet = "background-color: rgb(0, 30, 0);"
                         "color: rgb(107, 239, 0);";
    horizontalScrollBar()->setStyleSheet(styleSheet);
    verticalScrollBar()->setStyleSheet(styleSheet);
    //horizontalHeader()->setStyleSheet(styleSheet);
    styleSheet = "::section {background-color: rgb(0, 30, 0);"
                         "color: rgb(107, 239, 0);}";
    horizontalHeader()->setStyleSheet(styleSheet);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    //horizontalHeader()->show();
    verticalHeader()->setHidden(true);


    //Setup columns and horizontal header
    setColumnCount(COL::_SIZE);

    //setHorizontalHeaderLabels(QStringList() << "N" << "Identifier" << "Type" << "Altitude" << "Latitude" << "Longitude" << "S" << "Spas"<< "RSBN and/or VORDME");
    //setHorizontalHeaderLabels(QStringList() << "N" << "Identifier" << "Type" << "Altitude" << "Latitude" << "Longitude" << "MD" << "OZMPUv" << "OZMPUp" << "Pv" << "Pp" <<
    //                                           "MPU" << "IPU" << "S" << "Spas" << "Srem" << "RSBN and/or VORDME" << "Sm" << "Zm" << "Map Angle" << "A targ" << "D targ");
    setHorizontalHeaderLabels(COL_STR);
    setColumnWidth(COL::N, 50);
    setColumnWidth(COL::ID, 100);
    setColumnWidth(COL::TYPE, 100);
    setColumnWidth(COL::ALT, 100);
    setColumnWidth(COL::LAT, 150);
    setColumnWidth(COL::LON, 150);
    setColumnWidth(COL::S, 75);
    setColumnWidth(COL::SPAS, 75);
    setColumnWidth(COL::RSBN, 360);


    //horizontalHeader()->show() //Does not work???
    verticalHeader()->setDefaultSectionSize(20);
    horizontalHeader()->setStretchLastSection(true);
}

//_FL(km), _MACH(mach or km/h), _VS(m/s), _TWC(km/h), _ISA(celsius)
NVUPOINT* QFlightplanTable::calculateTOD(NVUPOINT*& _pc, double& _KM, double __FL, double _MACH, double _VS, double _TWC, double _ISA)
{
    _pc = NULL;
    if(lNVUPoints.size()<2) return NULL;

    //FL
    //MACH
    //VS
    //ISA
    //TWC or WD/WS (Average Tail Wind Component or average WindSpeed/WindDirection)
    _VS = fabs(_VS);

    //Convert altitude to km, and correct it to earth radius
    const double EARTH_RADIUS = 6356.766; //KM
    double _FL = (EARTH_RADIUS*__FL) / (EARTH_RADIUS + __FL);

    //Const values
    const double P0 = 101325; //Sea standard pressure
    const double G = 9.80665; //Earth gravitation
    const double M = 0.0289644; //Molar mass of dry air
    const double R = 0.0083144598; //Gas constant
    const double LP = -6.5;     //Laps rate (6.5C/KM)
    const double A0 = 340.3; //Speed of sound at sea level (m/s)
    const double D0 = 1.2250; //Sea standard air density (kg/m3)
    const double K = 0.0; //Error correction for CAS

    //Temperatures at sea level and altitude
    double T0 = 273.15 + 15 + _ISA; //Sea standard temperature (15 degrees) + ISA
    double T = T0 + LP*_FL;       //OAT temp (6.5 degree change per 1000 meter);

    //Calculate pressure at altitude (Pa)
    double PA = P0 * pow(T0/T, (G*M)/(R*LP));

    //Calculate density of air (kg/m3)
    double DA = D0 * pow(T0/T,  (1.0 + (G*M)/(R*LP)));

    //Convert IAS to MACH if value given is bigger than 10, otherwise _MACH is already in the correct format.
    if(_MACH>10.0) _MACH = (_MACH/3.6)/(A0*sqrt(T/T0)*sqrt(DA/D0));

    //Calculate TAS(m/s) from MACH
    double TAS = A0*_MACH*sqrt(T/T0);

    //Calculate CAS(m/s) from TAS
    double CAS = A0*sqrt(5.0*(pow(1.0 + ((0.5*DA*TAS*TAS)/P0), 2.0/7.0) - 1.0)) + K;

    //Calculate IAS(m/s) from TAS. Equation is actually for EAS, but IAS is the same as EAS if there is a 0 error.
    double IAS = TAS*sqrt(DA/D0);

    //Convert to km/h
    IAS = IAS*3.6;
    TAS = TAS*3.6;
    CAS = CAS*3.6;

    //Calculate GS(km/h) from TAS
    double GS = TAS + _TWC;

    //Calculate the distance needed for descent
    //FL = Flightlevel in thousands feet
    //VS = Descent rate (m/s)
    //GS = Ground speed (km/h)
    __FL = LMATH::meterToFeet(__FL) - (lNVUPoints.back())->elev/1000.0;
    double d = (__FL*(GS - 5.0*__FL)) / (_VS*(15.0/1.27));


    std::vector<NVUPOINT*>::reverse_iterator iter;
    for(iter = lNVUPoints.rbegin(); iter!=lNVUPoints.rend(); iter++)
    {
        NVUPOINT* cp = (*iter);
        if(cp->Srem>d)
        {
            _KM = cp->S - (cp->Srem - d);
            if(iter!=lNVUPoints.rbegin())
            {
                iter--;
                _pc = (*iter);
            }
            return NULL; //Should be a new TOD point if decide it.
        }
    }



    /*
    std::vector<NVUPOINT*>::reverse_iterator iter;
    for(iter = lNVUPoints.rbegin(); iter!=lNVUPoints.rend(); iter++)
    {
        NVUPOINT* cp = (*iter);
        if(cp->Srem>d)
        {
            _pc = cp;
            _KM = cp->Srem - d;
            return NULL; //Should be a new TOD point if decide it.
        }
    }
    */

    /*
    //Calculate pressure at height
    int ISA = 0;   //ISA deviation
    int p0 = 101325; //Sea standard pressure
    int t0 = 273.15 + 15 + ISA; //Sea standard temperature (15 degrees) + ISA
    double g = 9.80665; //Earth gravitation
    double M = 0.0289644; //Molar mass of dry air
    double r0 = 8.31447; //Gas constant
    double pr = p0 * exp(-(g*M*h)/(r0*t0)); //Pressure at height (i.e. 101325)
    */

    return NULL;
}

void QFlightplanTable::mousePressEvent(QMouseEvent* event)
{
    QModelIndex item = indexAt(event->pos());
    if (!item.isValid())
    {
        selectNone();
    }
    else if(currentIndex() == item)
    {
        selectNone();
    }
    else QTableWidget::mousePressEvent(event);
}

void QFlightplanTable::selectNone()
{
    QModelIndex qI = currentIndex();
    qI = qI.child(-1, -1);
    setCurrentIndex(qI);
}

void QFlightplanTable::updateShownColumns()
{
    /*
    (DialogSettings::showN ? horizontalHeader()->showSection(COL::N) : horizontalHeader()->hideSection(COL::N));
    (DialogSettings::showID ? horizontalHeader()->showSection(COL::ID) : horizontalHeader()->hideSection(COL::ID));
    (DialogSettings::showType ? horizontalHeader()->showSection(COL::TYPE) : horizontalHeader()->hideSection(COL::TYPE));
    (DialogSettings::showAlt ? horizontalHeader()->showSection(COL::ALT) : horizontalHeader()->hideSection(COL::ALT));
    (DialogSettings::showLat ? horizontalHeader()->showSection(COL::LAT) : horizontalHeader()->hideSection(COL::LAT));
    (DialogSettings::showLon ? horizontalHeader()->showSection(COL::LON) : horizontalHeader()->hideSection(COL::LON));
    (DialogSettings::showMD ? horizontalHeader()->showSection(COL::MD) : horizontalHeader()->hideSection(COL::MD));
    (DialogSettings::showOZMPUv ? horizontalHeader()->showSection(COL::OZMPUV) : horizontalHeader()->hideSection(COL::OZMPUV));
    (DialogSettings::showOZMPUp ? horizontalHeader()->showSection(COL::OZMPUP) : horizontalHeader()->hideSection(COL::OZMPUP));
    (DialogSettings::showPv ? horizontalHeader()->showSection(COL::PV) : horizontalHeader()->hideSection(COL::PV));
    (DialogSettings::showPp ? horizontalHeader()->showSection(COL::PP) : horizontalHeader()->hideSection(COL::PP));
    (DialogSettings::showMPU ? horizontalHeader()->showSection(COL::MPU) : horizontalHeader()->hideSection(COL::MPU));
    (DialogSettings::showIPU ? horizontalHeader()->showSection(COL::IPU) : horizontalHeader()->hideSection(COL::IPU));
    (DialogSettings::showS ? horizontalHeader()->showSection(COL::S) : horizontalHeader()->hideSection(COL::S));
    (DialogSettings::showSpas ? horizontalHeader()->showSection(COL::SPAS) : horizontalHeader()->hideSection(COL::SPAS));
    (DialogSettings::showSrem ? horizontalHeader()->showSection(COL::SREM) : horizontalHeader()->hideSection(COL::SREM));
    (DialogSettings::showRSBN ? horizontalHeader()->showSection(COL::RSBN) : horizontalHeader()->hideSection(COL::RSBN));
    (DialogSettings::showSm ? horizontalHeader()->showSection(COL::SM) : horizontalHeader()->hideSection(COL::SM));
    (DialogSettings::showZm ? horizontalHeader()->showSection(COL::ZM) : horizontalHeader()->hideSection(COL::ZM));
    (DialogSettings::showMapAngle ? horizontalHeader()->showSection(COL::MAPA) : horizontalHeader()->hideSection(COL::MAPA));
    (DialogSettings::showAtarg ? horizontalHeader()->showSection(COL::ATRG) : horizontalHeader()->hideSection(COL::ATRG));
    (DialogSettings::showDtarg ? horizontalHeader()->showSection(COL::DTRG) : horizontalHeader()->hideSection(COL::DTRG));
    */
}

//Delete waypoint at row
void QFlightplanTable::deleteWaypoint(int row)
{
    if(row<0 || row>=lNVUPoints.size()) return;

    removeRow(row);
    delete lNVUPoints[row];
    lNVUPoints.erase(lNVUPoints.begin() + row);

    refreshFlightplan();
}

//Insert a waypoint at row (current item at row, and following items, will be placed after the inserted item)
void QFlightplanTable::insertWaypoint(NVUPOINT* wp, int row)
{
    if(row<0) return;
    insertRow(row);

    lNVUPoints.insert(lNVUPoints.begin() + row, wp);
    for(int i=0; i<COL::_SIZE; i++)
    {
        setItem(row, i, new QTableWidgetItem);
    }

    refreshFlightplan();
}

void QFlightplanTable::replaceWaypoint(NVUPOINT* wp, int row)
{
    if(row<0 || row>=lNVUPoints.size()) return;

    delete lNVUPoints[row];
    lNVUPoints[row] = wp;
    refreshFlightplan();
}

//Insert a list waypoints at row (current item at row, and following items, will be placed after the last inserted item)
void QFlightplanTable::insertRoute(std::vector<NVUPOINT*> route, int row)
{
    for(int i=0; i<route.size(); i++)
    {
        insertWaypoint(route[i], row);
        row++;
    }
}

//Move row up or down.
void QFlightplanTable::moveWaypoint(int row, bool up)
{
    if(row<0 || row>=lNVUPoints.size()) return;            //Row should be in the span of list
    if(up && row<=0) return;                              //Moving down first item does not do anything
    if(!up && row >= (lNVUPoints.size()-1)) return;         //Moving up the last item does not do anything

    NVUPOINT* wp = lNVUPoints[row];

    if(!up)
    {
        lNVUPoints.erase(lNVUPoints.begin() + row);
        lNVUPoints.insert(lNVUPoints.begin() + row + 1, wp);
    }
    else
    {
        lNVUPoints.erase(lNVUPoints.begin() + row);
        lNVUPoints.insert(lNVUPoints.begin() + row - 1, wp);
    }

    refreshFlightplan();
}

void QFlightplanTable::refreshRow(int row)
{
    if(row<0 || row>=lNVUPoints.size()) return;

    NVUPOINT *waypoint = lNVUPoints[row];
    QString qstr;

    item(row, COL::N)->setText(QString::number(row));
    item(row, COL::ID)->setText(waypoint->name);
    item(row, COL::TYPE)->setText(WAYPOINT::getTypeStr(waypoint));
    item(row, COL::ALT)->setText(QString::number((showFeet ? waypoint->alt : LMATH::feetToMeter(waypoint->alt)), 'f', 0));

    double l1, l2;
    l1 = fabs(modf(waypoint->latlon.x, &l2)*60.0);
    int i2 = (int) fabs(l2);
    item(row, COL::LAT)->setText((waypoint->latlon.x < 0 ? "S" : "N") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));

    l1 = fabs(modf(waypoint->latlon.y, &l2)*60.0);
    i2 = (int) fabs(l2);
    item(row, COL::LON)->setText((waypoint->latlon.y < 0 ? "W" : "E") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));

    item(row, COL::MD)->setText(QString::number(waypoint->MD, 'f', 1));
    if(row<(rowCount() - 1))
    {
        item(row, COL::OZMPUV)->setText(QString::number(waypoint->OZMPUv, 'f', 1));
        item(row, COL::OZMPUP)->setText(QString::number(waypoint->OZMPUp, 'f', 1));
        if(row>0) item(row, COL::PV)->setText(QString::number(waypoint->Pv, 'f', 1));
        else item(row, COL::PV)->setText("");
        item(row, COL::PP)->setText(QString::number(waypoint->Pp, 'f', 1));
        item(row, COL::MPU)->setText(QString::number(waypoint->MPU, 'f', 1));
        item(row, COL::IPU)->setText(QString::number(waypoint->IPU, 'f', 1));
        item(row, COL::S)->setText(QString::number(waypoint->S, 'f', 1));
    }//if
    else
    {
        item(row, COL::OZMPUV)->setText("");
        item(row, COL::OZMPUP)->setText("");
        item(row, COL::PV)->setText("");
        item(row, COL::PP)->setText("");
        item(row, COL::MPU)->setText("");
        item(row, COL::IPU)->setText("");
        item(row, COL::S)->setText("");
    }
    item(row, COL::SPAS)->setText(QString::number(waypoint->Spas, 'f', 1));
    item(row, COL::SREM)->setText(QString::number(waypoint->Srem, 'f', 1));

    if(waypoint->rsbn)
    {
        double d = LMATH::calc_distance(waypoint->latlon, waypoint->rsbn->latlon);
        qstr =        waypoint->rsbn->name +
                      (waypoint->rsbn->country.isEmpty() ? "" : + " (" + waypoint->rsbn->country + ")") +
                      (waypoint->rsbn->name2.isEmpty() ? "" : "  " + waypoint->rsbn->name2) +
                      " (" + QString::number(d, 'f', 0) + " KM)";
        item(row, COL::RSBN)->setText(qstr);

        if(row<(rowCount()-1))
        {
            item(row, COL::SM)->setText(QString::number(waypoint->Sm, 'f', 1));
            item(row, COL::ZM)->setText(QString::number(waypoint->Zm, 'f', 1));
            item(row, COL::MAPA)->setText(QString::number(waypoint->MapAngle, 'f', 1));
            item(row, COL::ATRG)->setText(QString::number(waypoint->Atrg, 'f', 1));
            item(row, COL::DTRG)->setText(QString::number(waypoint->Dtrg, 'f', 1));
        }//if
        else
        {
            item(row, COL::SM)->setText("");
            item(row, COL::ZM)->setText("");
            item(row, COL::MAPA)->setText("");
            item(row, COL::ATRG)->setText("");
            item(row, COL::DTRG)->setText("");
        }
    }
    else
    {
        item(row, COL::RSBN)->setText("NO CORRECTION");
        item(row, COL::SM)->setText("");
        item(row, COL::ZM)->setText("");
        item(row, COL::MAPA)->setText("");
        item(row, COL::ATRG)->setText("");
        item(row, COL::DTRG)->setText("");
    }
}

void QFlightplanTable::refreshFlightplan()
{
    //Generate values for the NVU flightplan.
    double fork;
    NVU::generate(lNVUPoints, fork, dat);

    //Update visual values in table
    for(int i=0; i<rowCount(); i++) refreshRow(i);

    qFork->setText("Fork   " + QString::number(fork, 'f', 1));

    NVUPOINT *cp = NULL;
    double d;
    double fl = fplData.fl;

    if(showFeet) fl = LMATH::feetToMeter(fl);
    fl = fl/1000.0;
    calculateTOD(cp, d, fl, fplData.speed, fplData.vs, fplData.twc, fplData.isa);

    if(cp) qTOD->setText("TOD: " + (int(d) == 0 ? "at " : QString::number(d, 'f', 1) + " km before ") + cp->name);
    else qTOD->setText("TOD: ");
}

const std::vector<NVUPOINT*>& QFlightplanTable::getWaypoints()
{
    return lNVUPoints;
}

NVUPOINT* QFlightplanTable::getWaypoint(int row, bool lastOutOfIndex)
{
    if(row<0 || row>=rowCount())
    {
        if(lastOutOfIndex && lNVUPoints.size()>0) return lNVUPoints[lNVUPoints.size()-1];
        return NULL;
    }

    return lNVUPoints[row];
}

/*
void QFlightplanTable::updateDistanceAndN()
{
    QTableWidgetItemData* iN;
    QTableWidgetItemData* iC;
    double currentDistance = 0.0;
    double legDistance;

    if(rowCount()>0)
    {
        item(0, COL::SPAS)->setText("0.0");
        item(rowCount()-1, COL::SPAS)->setText(QString::number(currentDistance, 'f', 1));
        item(rowCount()-1, COL::N)->setText(QString::number(rowCount()));

        iN = (QTableWidgetItemData*) item(0, COL::N);
        iC = (QTableWidgetItemData*) item(rowCount()-1, COL::N);
        double fork = LMATH::calc_fork(iN->nvupoint->latlon.x, iN->nvupoint->latlon.y, iN->nvupoint->alt, iC->nvupoint->latlon.x, iC->nvupoint->latlon.y, iC->nvupoint->alt, dat);
        qFork->setText("Fork   " + QString::number(fork, 'f', 1));
    }
    else qFork->setText("Fork   0.0");

    for(int i=0; i<rowCount() - 1; i++)
    {
        iC = (QTableWidgetItemData*) item(i, COL::N);
        iN = (QTableWidgetItemData*) item(i+1, COL::N);

        legDistance = LMATH::calc_distance(iN->nvupoint->latlon, iC->nvupoint->latlon);
        item(i, COL::S)->setText(QString::number(legDistance, 'f', 1));
        item(i, COL::SPAS)->setText(QString::number(currentDistance, 'f', 1));
        item(i, COL::N)->setText(QString::number(i+1));
        currentDistance+=legDistance;
    }

    if(rowCount()>0) item(rowCount()-1, COL::SPAS)->setText(QString::number(currentDistance, 'f', 1));
}
*/

void QFlightplanTable::clearFlightplan()
{
    while(rowCount()>0) removeRow(0);
    while(lNVUPoints.size()>0)
    {
        delete *(lNVUPoints.end() - 1);
        lNVUPoints.erase(lNVUPoints.end()-1);
    }

    qFork->setText("Fork   0.0");
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
