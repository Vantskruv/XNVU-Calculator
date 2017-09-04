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
    styleSheet = "::section {background-color: rgb(0, 30, 0);"
                         "color: rgb(107, 239, 0);}";
    horizontalHeader()->setStyleSheet(styleSheet);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    verticalHeader()->setHidden(true);

    //Setup columns and horizontal header
    setColumnCount(COL::_SIZE);

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

//_FL(km), _MACH(mach), _VS(m/s), _TWC(km/h), _ISA(celsius)
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
    //if(_MACH>10.0) _MACH = (_MACH/3.6)/(A0*sqrt(T/T0)*sqrt(DA/D0));

    //Calculate TAS(m/s) from MACH
    double TAS = A0*_MACH*sqrt(T/T0);

    //Calculate CAS(m/s) from TAS
    double Q = 0.5*DA*TAS*TAS;	//Dynamic pressure
    double QC = Q*(1.0 + pow(_MACH, 2.0)/4.0 + pow(_MACH, 4.0)/40.0 + pow(_MACH, 6.0)/1600.0); //Impact pressure
    double CAS = A0*sqrt(5.0*(pow((QC/P0) + 1.0, 2.0/7.0) - 1.0)) + K;

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
    __FL = LMATH::meterToFeet(__FL) - (lNVUPoints.back())->alt/1000.0;
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

//Delete waypoint at row
void QFlightplanTable::deleteWaypoint(int row)
{
    if(row<0 || row>=lNVUPoints.size()) return;

    //Add adjacent waypoints to a list
    std::vector<NVUPOINT*> adjPoints;
    if(row>0) adjPoints.push_back(lNVUPoints[row-1]);
    if(row<(int(lNVUPoints.size())-2)) adjPoints.push_back(lNVUPoints[row+1]);

    //Remove the waypoint
    removeRow(row);
    delete lNVUPoints[row];
    lNVUPoints.erase(lNVUPoints.begin() + row);

    //Set the altitudes of the adjacent waypoints if they are airports and is now start- or end-points.
    for(unsigned int i=0; i<adjPoints.size(); i++)
    {
        NVUPOINT *wp = adjPoints[i];
        if(wp == (*lNVUPoints.begin()) && wp->type == WAYPOINT::TYPE_AIRPORT) wp->alt = wp->elev;
        else if(wp == lNVUPoints.back() && wp->type == WAYPOINT::TYPE_AIRPORT) wp->alt = wp->elev;
    }

    refreshFlightplan();
}

//Insert a waypoint at row (current item at row, and following items, will be placed after the inserted item)
void QFlightplanTable::insertWaypoint(NVUPOINT* wp, int row)
{
    if(row<0) return;

    //Add the adjacent waypoints to a list, and flag them if they where start- or end-points
    std::vector<std::pair<bool, NVUPOINT*>> adjPoints;
    if(row>0) adjPoints.push_back(std::make_pair(lNVUPoints[row-1] == (*lNVUPoints.begin()) || lNVUPoints[row-1] == lNVUPoints.back(), lNVUPoints[row-1]));
    if(row<(int(lNVUPoints.size())-1)) adjPoints.push_back(std::make_pair(lNVUPoints[row] == (*lNVUPoints.begin()) || lNVUPoints[row] == lNVUPoints.back(), lNVUPoints[row]));


    //Insert the waypoint
    insertRow(row);
    lNVUPoints.insert(lNVUPoints.begin() + row, wp);
    for(int i=0; i<COL::_SIZE; i++)
    {
        setItem(row, i, new QTableWidgetItem);
    }

    //Set the altitude of this waypoint
    if(wp == (*lNVUPoints.begin()) && wp->type == WAYPOINT::TYPE_AIRPORT) wp->alt = wp->elev;
    else if(wp == lNVUPoints.back() && wp->type == WAYPOINT::TYPE_AIRPORT) wp->alt = wp->elev;
    else wp->alt = (DialogSettings::showFeet ? fplData.fl : LMATH::meterToFeet(fplData.fl));

    //Set the altitude of the adjacent waypoints if they are airports, and previous was the start- or end-points in flightplan,
    //but not now.
    for(unsigned int i=0; i<adjPoints.size(); i++)
    {
        bool endP = adjPoints[i].first;
        NVUPOINT *wp = adjPoints[i].second;
        if(endP && wp->type == WAYPOINT::TYPE_AIRPORT)
        {
            if(wp == (*lNVUPoints.begin())) continue;
            else if(wp == lNVUPoints.back()) continue;
            wp->alt = (DialogSettings::showFeet ? fplData.fl : LMATH::meterToFeet(fplData.fl));
        }//if
   }

    refreshFlightplan();
}

void QFlightplanTable::replaceWaypoint(NVUPOINT* wp, int row)
{
    if(row<0 || row>=lNVUPoints.size()) return;

    double preAlt = lNVUPoints[row]->alt;   //We save the altitude of the old waypoint
    bool preAirport = (lNVUPoints[row]->type == WAYPOINT::TYPE_AIRPORT && (row == 0 || row == (int(lNVUPoints.size()) - 1)) ? true : false);
    delete lNVUPoints[row];
    lNVUPoints[row] = wp;

    //Set the altitude of the new waypoint
    if(wp == (*lNVUPoints.begin()) && wp->type == WAYPOINT::TYPE_AIRPORT) wp->alt = wp->elev;
    else if(wp == lNVUPoints.back() && wp->type == WAYPOINT::TYPE_AIRPORT) wp->alt = wp->elev;
    else if(preAirport) wp->alt = (DialogSettings::showFeet ? fplData.fl : LMATH::meterToFeet(fplData.fl));
    else wp->alt = preAlt; //Copy the altitude from the old waypoint if it was not an airport



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
    //TODO: We are not updating the altitudes as we have been done with insertWaypoint, deleteWaypoint and replaceWaypoint
    if(row<0 || row>=lNVUPoints.size()) return;            //Row should be in the span of list
    if(up && row<=0) return;                              //Moving down first item does not do anything
    if(!up && row >= (int(lNVUPoints.size())-1)) return;         //Moving up the last item does not do anything

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
    item(row, COL::ALT)->setText(QString::number((DialogSettings::showFeet ? waypoint->alt : LMATH::feetToMeter(waypoint->alt)), 'f', 0));

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

    if(waypoint->getRSBN())
    {
        double d = LMATH::calc_distance(waypoint->latlon, waypoint->getRSBN()->latlon);
        qstr =        waypoint->getRSBN()->name +
                      (waypoint->getRSBN()->country.isEmpty() ? "" : + " (" + waypoint->getRSBN()->country + ")") +
                      (waypoint->getRSBN()->name2.isEmpty() ? "" : "  " + waypoint->getRSBN()->name2) +
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

    qFork->setText("Fork   " + QString::number(fork, 'f', 1));

    NVUPOINT *cp = NULL;
    double d;
    double fl = fplData.fl;
    double speed = fplData.speed;
    double vs = (DialogSettings::VSFormat == 1 ? LMATH::feetToMeter(fplData.vs)/60.0 : fplData.vs);
    double twc = (DialogSettings::TWCFormat == 1 ? fplData.twc*1.852 : fplData.twc);

    if(DialogSettings::showFeet) fl = LMATH::feetToMeter(fl);
    fl = fl/1000.0;

    /*
    if(DialogSettings::cruiseFormat == 1) speed = LMATH::IAS_to_MACH(speed, fl, fplData.isa);
    else if(DialogSettings::cruiseFormat == 2) speed = LMATH::IAS_to_MACH(speed*1.852, fl, fplData.isa);
    */

    calculateTOD(cp, d, fl, speed, vs, twc, fplData.isa);
    pTOD = cp;
    dTOD = d;

    if(cp)
    {
        if(DialogSettings::showTOD_METRIC) qTOD->setText("TOD: " + (d < 1.0 ? "at " : QString::number(d, 'f', 1) + " km before ") + cp->name);
        else qTOD->setText("TOD: " + (d < 1.0 ? "at " : QString::number(d/1.852, 'f', 1) + " nm before ") + cp->name);
    }
    else qTOD->setText("TOD: UNABLE ");// + QString((DialogSettings::showTOD_METRIC == true ? " km" : " nm")));

    //Update visual values in table
    for(int i=0; i<rowCount(); i++) refreshRow(i);
}

void QFlightplanTable::autoGenerateCorrectionBeacons()
{
    double weight = 2.00;

    for(int i=0; i<(int(lNVUPoints.size()) - 1); i++)
    {
        std::vector< std::pair<NVUPOINT*, double> > lRSBN;
        lRSBN = XFMS_DATA::getClosestRSBN(lNVUPoints[i], -1, DialogSettings::beaconDistance, DialogSettings::correctionVORDME);
        NVUPOINT* rsbn = NULL;
        double minZm = 0;
        lNVUPoints[i]->setRSBN(NULL);

        //Get leg distance for second next waypoint
        //Hence this S value for that leg will replace the Sm value which user eventually put in here, so it is good to have
        //the value Sm here close to the second next S value.
        double sN = (i<(int(lNVUPoints.size()) - 2) ? lNVUPoints[i+2]->S : 0);

        if(lRSBN.size()>0)
        {
            rsbn = lRSBN[0].first;
            lNVUPoints[i]->setRSBN(lRSBN[0].first);
            lNVUPoints[i]->calc_rsbn_corr(lNVUPoints[i+1]->latlon);
            minZm = fabs(lNVUPoints[i]->Zm)*weight + fabs(lNVUPoints[i]->Sm + sN);
        }

        for(unsigned int j=0; j<lRSBN.size(); j++)
        {
            lNVUPoints[i]->setRSBN(lRSBN[j].first);
            lNVUPoints[i]->calc_rsbn_corr(lNVUPoints[i+1]->latlon);
            double sum =  fabs(lNVUPoints[i]->Zm)*weight + fabs(lNVUPoints[i]->Sm + sN);
            if(sum<minZm)
            {
                rsbn = lRSBN[j].first;
                minZm = sum;
            }//if
        }//for

        lNVUPoints[i]->setRSBN(rsbn);
    }
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
