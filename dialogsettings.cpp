#include "dialogsettings.h"
#include "ui_dialogsettings.h"

#include <QFileDialog>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>
#include <QSettings>

bool DialogSettings::XP11 = false;
bool DialogSettings::XP11_includeCustomAirports = false;
QString DialogSettings::xDir;
QString DialogSettings::customAirportsDir; //TODO: Currently a dummy, either implement it or define it.
QString DialogSettings::fileAirports;
QString DialogSettings::fileNavaids;
QString DialogSettings::fileWaypoints;
QString DialogSettings::fileAirways;
QString DialogSettings::fileNavdata;
bool DialogSettings::manualSetDir = false;
QString DialogSettings::fileRSBN;
bool DialogSettings::correctionVORDME = false;
int DialogSettings::beaconDistance = 500;
//int DialogSettings::windowWidth = 1600;
//int DialogSettings::windowHeight = 1054;
QSize DialogSettings::windowSize(1600, 1054);
QPoint DialogSettings::windowPos;
QByteArray DialogSettings::tableState;
bool DialogSettings::distAlignWPS = false;
bool DialogSettings::distAlignFMS = false;
bool DialogSettings::distAlignATS = false;
bool DialogSettings::distAlignEarthNav = false;
double DialogSettings::distAlignMargin = 0.0001;
bool DialogSettings::nightMode = true;
bool DialogSettings::showFeet = false;
int DialogSettings::cruiseFormat = 0;
int DialogSettings::VSFormat = 0;
int DialogSettings::TWCFormat = 0;
bool DialogSettings::customDateIsTrue = true;
QString DialogSettings::customDate = "2020.01.01";
bool DialogSettings::showTOD_METRIC = true;

/*
bool DialogSettings::showN = true;
bool DialogSettings::showID = true;
bool DialogSettings::showType = true;
bool DialogSettings::showAlt = true;
bool DialogSettings::showLat = true;
bool DialogSettings::showLon = true;
bool DialogSettings::showS = true;
bool DialogSettings::showSpas = true;
bool DialogSettings::showSrem = true;
bool DialogSettings::showMD = true;
bool DialogSettings::showOZMPUv = true;
bool DialogSettings::showOZMPUp = true;
bool DialogSettings::showPv = true;
bool DialogSettings::showPp = true;
bool DialogSettings::showMPU = true;
bool DialogSettings::showIPU = true;
bool DialogSettings::showRSBN = true;
bool DialogSettings::showSm = true;
bool DialogSettings::showZm = true;
bool DialogSettings::showMapAngle = true;
bool DialogSettings::showAtarg = true;
bool DialogSettings::showDtarg = true;
*/

/*
int DialogSettings::showN_size = 150;
int DialogSettings::showID_size = 150;
int DialogSettings::showType_size = 150;
int DialogSettings::showAlt_size = 150;
int DialogSettings::showLat_size = 150;
int DialogSettings::showLon_size = 150;
int DialogSettings::showS_size = 150;
int DialogSettings::showSpas_size = 150;
int DialogSettings::showSrem_size = 150;
int DialogSettings::showMD_size = 150;
int DialogSettings::showOZMPUv_size = 150;
int DialogSettings::showOZMPUp_size = 150;
int DialogSettings::showPv_size = 150;
int DialogSettings::showPp_size = 150;
int DialogSettings::showMPU_size = 150;
int DialogSettings::showIPU_size = 150;
int DialogSettings::showRSBN_size = 150;
int DialogSettings::showSm_size = 150;
int DialogSettings::showZm_size = 150;
int DialogSettings::showMapAngle_size = 150;
int DialogSettings::showAtarg_size = 150;
int DialogSettings::showDtarg_size = 150;

int DialogSettings::showN_pos = 150;
int DialogSettings::showID_pos = 150;
int DialogSettings::showType_pos = 150;
int DialogSettings::showAlt_pos = 150;
int DialogSettings::showLat_pos = 150;
int DialogSettings::showLon_pos = 150;
int DialogSettings::showS_pos = 150;
int DialogSettings::showSpas_pos = 150;
int DialogSettings::showSrem_pos = 150;
int DialogSettings::showMD_pos = 150;
int DialogSettings::showOZMPUv_pos = 150;
int DialogSettings::showOZMPUp_pos = 150;
int DialogSettings::showPv_pos = 150;
int DialogSettings::showPp_pos = 150;
int DialogSettings::showMPU_pos = 150;
int DialogSettings::showIPU_pos = 150;
int DialogSettings::showRSBN_pos = 150;
int DialogSettings::showSm_pos = 150;
int DialogSettings::showZm_pos = 150;
int DialogSettings::showMapAngle_pos = 150;
int DialogSettings::showAtarg_pos = 150;
int DialogSettings::showDtarg_pos = 150;
*/

/*
QString DialogSettings::_xDir;
QString DialogSettings::_fileAirports;
QString DialogSettings::_fileNavaids;
QString DialogSettings::_fileWaypoints;
QString DialogSettings::_fileAirways;
QString DialogSettings::_fileNavdata;
bool DialogSettings::_manualSetDir = false;
QString DialogSettings::_fileRSBN;
bool DialogSettings::_correctionVORDME = false;
int DialogSettings::_beaconDistance = 500;
int DialogSettings::_windowWidth = 1600;
int DialogSettings::_windowHeight = 1054;
bool DialogSettings::_distAlignWPS = false;
bool DialogSettings::_distAlignFMS = false;
bool DialogSettings::_distAlignATS = false;
bool DialogSettings::_distAlignEarthNav = false;
double DialogSettings::_distAlignMargin = 0.0001;
bool DialogSettings::_showFeet = false;

bool DialogSettings::_showN = true;
bool DialogSettings::_showID = true;
bool DialogSettings::_showType = true;
bool DialogSettings::_showAlt = true;
bool DialogSettings::_showLat = true;
bool DialogSettings::_showLon = true;
bool DialogSettings::_showS = true;
bool DialogSettings::_showSpas = true;
bool DialogSettings::_showSrem = true;
bool DialogSettings::_showMD = true;
bool DialogSettings::_showOZMPUv = true;
bool DialogSettings::_showOZMPUp = true;
bool DialogSettings::_showPv = true;
bool DialogSettings::_showPp = true;
bool DialogSettings::_showMPU = true;
bool DialogSettings::_showIPU = true;
bool DialogSettings::_showRSBN = true;
bool DialogSettings::_showSm = true;
bool DialogSettings::_showZm = true;
bool DialogSettings::_showMapAngle = true;
bool DialogSettings::_showAtarg = true;
bool DialogSettings::_showDtarg = true;
*/



DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);

    ui->lineEditXPlaneDirectory->setText(xDir);
    ui->lineEditAirportsFiles->setText(fileAirports);
    ui->lineEditNavaidsFile->setText(fileNavaids);
    ui->lineEditWaypointsFile->setText(fileWaypoints);
    ui->lineEditAirwaysFile->setText(fileAirways);
    ui->lineEditEarthnavFile->setText(fileNavdata);
    ui->lineEditRSBNFile->setText(fileRSBN);

    ui->checkBoxManual->setChecked(manualSetDir);
    on_checkBoxManual_clicked(manualSetDir);
    ui->checkBoxXP11->setChecked(XP11);
    ui->checkBoxAPTOverride->setChecked(XP11_includeCustomAirports);
}

DialogSettings::~DialogSettings()
{
    delete ui;
}


void DialogSettings::loadSettings()
{
    QSettings s("xnvu.conf", QSettings::IniFormat);

    XP11 = s.value("XP11", XP11).toBool();
    XP11_includeCustomAirports = s.value("XP11_includeCustomAirports", XP11_includeCustomAirports).toBool();
    xDir = s.value("XPlaneDirectory").toString();
    fileAirports = s.value("AirportFile").toString();
    fileNavaids = s.value("NavaidFile").toString();
    fileWaypoints = s.value("WaypointsFile", fileWaypoints).toString();
    fileAirways = s.value("AirwaysFile", fileAirways).toString();
    fileNavdata = s.value("EarthNavFile", fileNavdata).toString();
    fileRSBN = s.value("RSBNFile", fileRSBN).toString();
    manualSetDir = s.value("ManualSetDir", manualSetDir).toBool();
    correctionVORDME = s.value("VORDMECorrection", correctionVORDME).toBool();
    beaconDistance = s.value("BeaconDistance", beaconDistance).toInt();
    distAlignWPS = s.value("DistAlignWPS", distAlignWPS).toBool();
    distAlignFMS = s.value("DistAlignFMS", distAlignFMS).toBool();
    distAlignATS = s.value("DistAlignATS", distAlignATS).toBool();
    distAlignEarthNav = s.value("DistAlignEarthNav", distAlignEarthNav).toBool();
    distAlignMargin = s.value("DistAlignMargin", distAlignMargin).toDouble();
    nightMode = s.value("NightMode", nightMode).toBool();
    showFeet = s.value("ShowFeet", showFeet).toBool();
    cruiseFormat = s.value("CruiseFormat", cruiseFormat).toInt();
    VSFormat = s.value("VSFormat", VSFormat).toInt();
    TWCFormat = s.value("TWCFormat", TWCFormat).toInt();
    windowSize = s.value("WindowSize", windowSize).toSize();
    windowPos = s.value("WindowPos", windowPos).toPoint();
    tableState = s.value("TableState", tableState).toByteArray();
    customDateIsTrue = s.value("CustomDateIsTrue", customDateIsTrue).toBool();
    customDate = s.value("CustomDate", customDate).toString();
    showTOD_METRIC = s.value("ShowTOD_METRIC", showTOD_METRIC).toBool();
}

void DialogSettings::saveSettings()
{
    QSettings s("xnvu.conf", QSettings::IniFormat);

    s.setValue("XP11", XP11);
    s.setValue("XP11_includeCustomAirports", XP11_includeCustomAirports);
    s.setValue("XPlaneDirectory", xDir);
    s.setValue("AirportFile", fileAirports);
    s.setValue("NavaidFile", fileNavaids);
    s.setValue("WaypointsFile", fileWaypoints);
    s.setValue("AirwaysFile", fileAirways);
    s.setValue("EarthNavFile", fileNavdata);
    s.setValue("ManualSetDir", manualSetDir);
    s.setValue("RSBNFile", fileRSBN);
    s.setValue("VORDMECorrection", correctionVORDME);
    s.setValue("BeaconDistance", beaconDistance);
    s.setValue("DistAlignWPS", distAlignWPS);
    s.setValue("DistAlignFMS", distAlignFMS);
    s.setValue("DistAlignATS", distAlignATS);
    s.setValue("DistAlignEarthNav", distAlignEarthNav);
    s.setValue("DistAlignMargin", distAlignMargin);
    s.setValue("ShowFeet", showFeet);
    s.setValue("NightMode", nightMode);
    s.setValue("CruiseFormat", cruiseFormat);
    s.setValue("VSFormat", VSFormat);
    s.setValue("TWCFormat", TWCFormat);
    s.setValue("WindowSize", windowSize);
    s.setValue("WindowPos", windowPos);
    s.setValue("TableState", tableState);
    s.setValue("CustomDateIsTrue", customDateIsTrue);
    s.setValue("CustomDate", customDate);
    s.setValue("ShowTOD_METRIC", showTOD_METRIC);
}

/*
void DialogSettings::loadSettings()
{
    QFile infile(XNVU_SETTINGS_FILE);
    if(!infile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    while(!infile.atEnd())
    {
        QString line = infile.readLine();
        QStringList list = line.split('=', QString::SkipEmptyParts);
        if(list.size()==2)
        {
            QString qstr = list[1];
            qstr = qstr.simplified();
            if(list[0].simplified().compare("XPlaneDirectory")==0)
            {
                xDir = qstr;
                //_xDir = qstr;
            }
            else if(list[0].simplified().compare("AirportFile")==0)
            {
                fileAirports = qstr;
                //_fileAirports = qstr;
            }
            else if(list[0].simplified().compare("NavaidFile")==0)
            {
                fileNavaids = qstr;
                //_fileNavaids = qstr;
            }
            else if(list[0].simplified().compare("WaypointsFile")==0)
            {
                fileWaypoints = qstr;
                //_fileWaypoints = qstr;
            }
            else if(list[0].simplified().compare("AirwaysFile")==0)
            {
                fileAirways = qstr;
                //_fileAirways = qstr;
            }
            else if(list[0].simplified().compare("EarthNavFile")==0)
            {
                fileNavdata = qstr;
                //_fileNavdata = qstr;
            }
            else if(list[0].simplified().compare("ManualSetDir")==0)
            {
                manualSetDir = qstr.toInt();
                //_manualSetDir = qstr.toInt();
            }
            else if(list[0].simplified().compare("RSBNFile")==0)
            {
                fileRSBN = qstr;
                //_fileRSBN = qstr;
            }
            else if(list[0].simplified().compare("VORDMECorrection")==0)
            {
                correctionVORDME = qstr.toInt();
                //_correctionVORDME = qstr.toInt();
            }
            else if(list[0].simplified().compare("BeaconDistance")==0)
            {
                beaconDistance = qstr.toInt();
                //_beaconDistance = qstr.toInt();
            }
            else if(list[0].simplified().compare("WindowWidth")==0)
            {
                windowWidth = qstr.toInt();
                //_windowWidth = qstr.toInt();
            }
            else if(list[0].simplified().compare("WindowHeight")==0)
            {
                windowHeight = qstr.toInt();
                //_windowHeight = qstr.toInt();
            }
            else if(list[0].simplified().compare("DistAlignWPS")==0)
            {
                distAlignWPS = qstr.toInt();
                //_distAlignWPS = qstr.toInt();
            }
            else if(list[0].simplified().compare("DistAlignFMS")==0)
            {
                distAlignFMS = qstr.toInt();
                //_distAlignFMS = qstr.toInt();
            }
            else if(list[0].simplified().compare("DistAlignATS")==0)
            {
                distAlignATS = qstr.toInt();
                //_distAlignATS = qstr.toInt();
            }
            else if(list[0].simplified().compare("DistAlignEarthNav")==0)
            {
                distAlignEarthNav = qstr.toInt();
                //_distAlignEarthNav = qstr.toInt();
            }
            else if(list[0].simplified().compare("DistAlignMargin")==0)
            {
                distAlignMargin = qstr.toDouble();
                //_distAlignMargin = qstr.toDouble();
            }
            else if(list[0].simplified().compare("ShowFeet")==0)
            {
                showFeet = qstr.toInt();
                //_showFeet = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowN")==0)
            {
                showN = qstr.toInt();
                //_showN = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowID")==0)
            {
                showID = qstr.toInt();
                //_showID = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowType")==0)
            {
                showType = qstr.toInt();
                //_showType = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowAlt")==0)
            {
                showAlt = qstr.toInt();
                //_showAlt = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowLat")==0)
            {
                showLat = qstr.toInt();
                //_showLat = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowLon")==0)
            {
                showLon = qstr.toInt();
                //_showLon = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowS")==0)
            {
                showS = qstr.toInt();
                //_showS = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowSpas")==0)
            {
                showSpas = qstr.toInt();
                //_showSpas = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowSrem")==0)
            {
                showSrem = qstr.toInt();
                //_showSrem = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowMD")==0)
            {
                showMD = qstr.toInt();
                //_showMD = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowOZMPUv")==0)
            {
                showOZMPUv = qstr.toInt();
                //_showOZMPUv = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowOZMPUp")==0)
            {
                showOZMPUp = qstr.toInt();
                //_showOZMPUp = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowPv")==0)
            {
                showPv = qstr.toInt();
                //_showPv = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowPp")==0)
            {
                showPp = qstr.toInt();
                //_showPp = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowMPU")==0)
            {
                showMPU = qstr.toInt();
                //_showMPU = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowIPU")==0)
            {
                showIPU = qstr.toInt();
                //_showIPU = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowRSBN")==0)
            {
                showRSBN = qstr.toInt();
                //_showRSBN = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowSm")==0)
            {
                showSm = qstr.toInt();
                //_showSm = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowZm")==0)
            {
                showZm = qstr.toInt();
                //_showZm = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowMapAngle")==0)
            {
                showMapAngle = qstr.toInt();
                //_showMapAngle = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowAtarg")==0)
            {
                showAtarg = qstr.toInt();
                //_showAtarg = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowDtarg")==0)
            {
                showDtarg = qstr.toInt();
                //_showDtarg = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowN_size")==0)
            {
                showN_size = qstr.toInt();
                //_showN_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowID_size")==0)
            {
                showID_size = qstr.toInt();
                //_showID_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowType_size")==0)
            {
                showType_size = qstr.toInt();
                //_showType_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowAlt_size")==0)
            {
                showAlt_size = qstr.toInt();
                //_showAlt_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowLat_size")==0)
            {
                showLat_size = qstr.toInt();
                //_showLat_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowLon_size")==0)
            {
                showLon_size = qstr.toInt();
                //_showLon_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowS_size")==0)
            {
                showS_size = qstr.toInt();
                //_showS_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowSpas_size")==0)
            {
                showSpas_size = qstr.toInt();
                //_showSpas_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowSrem_size")==0)
            {
                showSrem_size = qstr.toInt();
                //_showSrem_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowMD_size")==0)
            {
                showMD_size = qstr.toInt();
                //_showMD_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowOZMPUv_size")==0)
            {
                showOZMPUv_size = qstr.toInt();
                //_showOZMPUv_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowOZMPUp_size")==0)
            {
                showOZMPUp_size = qstr.toInt();
                //_showOZMPUp_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowPv_size")==0)
            {
                showPv_size = qstr.toInt();
                //_showPv_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowPp_size")==0)
            {
                showPp_size = qstr.toInt();
                //_showPp_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowMPU_size")==0)
            {
                showMPU_size = qstr.toInt();
                //_showMPU_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowIPU_size")==0)
            {
                showIPU_size = qstr.toInt();
                //_showIPU_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowRSBN_size")==0)
            {
                showRSBN_size = qstr.toInt();
                //_showRSBN_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowSm_size")==0)
            {
                showSm_size = qstr.toInt();
                //_showSm_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowZm_size")==0)
            {
                showZm_size = qstr.toInt();
                //_showZm_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowMapAngle_size")==0)
            {
                showMapAngle_size = qstr.toInt();
                //_showMapAngle_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowAtarg_size")==0)
            {
                showAtarg_size = qstr.toInt();
                //_showAtarg_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowDtarg_size")==0)
            {
                showDtarg_size = qstr.toInt();
                //_showDtarg_size = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowN_pos")==0)
            {
                showN_pos = qstr.toInt();
                //_showN_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowID_pos")==0)
            {
                showID_pos = qstr.toInt();
                //_showID_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowType_pos")==0)
            {
                showType_pos = qstr.toInt();
                //_showType_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowAlt_pos")==0)
            {
                showAlt_pos = qstr.toInt();
                //_showAlt_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowLat_pos")==0)
            {
                showLat_pos = qstr.toInt();
                //_showLat_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowLon_pos")==0)
            {
                showLon_pos = qstr.toInt();
                //_showLon_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowS_pos")==0)
            {
                showS_pos = qstr.toInt();
                //_showS_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowSpas_pos")==0)
            {
                showSpas_pos = qstr.toInt();
                //_showSpas_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowSrem_pos")==0)
            {
                showSrem_pos = qstr.toInt();
                //_showSrem_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowMD_pos")==0)
            {
                showMD_pos = qstr.toInt();
                //_showMD_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowOZMPUv_pos")==0)
            {
                showOZMPUv_pos = qstr.toInt();
                //_showOZMPUv_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowOZMPUp_pos")==0)
            {
                showOZMPUp_pos = qstr.toInt();
                //_showOZMPUp_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowPv_pos")==0)
            {
                showPv_pos = qstr.toInt();
                //_showPv_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowPp_pos")==0)
            {
                showPp_pos = qstr.toInt();
                //_showPp_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowMPU_pos")==0)
            {
                showMPU_pos = qstr.toInt();
                //_showMPU_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowIPU_pos")==0)
            {
                showIPU_pos = qstr.toInt();
                //_showIPU_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowRSBN_pos")==0)
            {
                showRSBN_pos = qstr.toInt();
                //_showRSBN_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowSm_pos")==0)
            {
                showSm_pos = qstr.toInt();
                //_showSm_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowZm_pos")==0)
            {
                showZm_pos = qstr.toInt();
                //_showZm_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowMapAngle_pos")==0)
            {
                showMapAngle_pos = qstr.toInt();
                //_showMapAngle_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowAtarg_pos")==0)
            {
                showAtarg_pos = qstr.toInt();
                //_showAtarg_pos = qstr.toInt();
            }
            else if(list[0].simplified().compare("ShowDtarg_pos")==0)
            {
                showDtarg_pos = qstr.toInt();
                //_showDtarg_pos = qstr.toInt();
            }
        }

    }//while

    infile.close();

    return;
}
*/

/*
void DialogSettings::saveSettings()
{
    //if(!DialogSettings::isChanged()) return;


    QFile outfile(XNVU_SETTINGS_FILE);
    if(!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&outfile);

    out << qSetRealNumberPrecision(16);
    out << "XPlaneDirectory", xDir << "\n";
    out << "AirportFile", fileAirports << "\n";
    out << "NavaidFile", fileNavaids << "\n";
    out << "WaypointsFile", fileWaypoints << "\n";
    out << "AirwaysFile", fileAirways << "\n";
    out << "EarthNavFile", fileNavdata << "\n";
    out << "ManualSetDir", manualSetDir << "\n";
    out << "RSBNFile", fileRSBN << "\n";
    out << "VORDMECorrection", correctionVORDME << "\n";
    out << "BeaconDistance", beaconDistance << "\n";
    out << "WindowWidth", windowWidth << "\n";
    out << "WindowHeight", windowHeight << "\n";
    out << "DistAlignWPS", distAlignWPS << "\n";
    out << "DistAlignFMS", distAlignFMS << "\n";
    out << "DistAlignATS", distAlignATS << "\n";
    out << "DistAlignEarthNav", distAlignEarthNav << "\n";
    out << "DistAlignMargin", distAlignMargin << "\n";
    out << "ShowFeet", showFeet << "\n";
    out << "ShowN", showN << "\n";
    out << "ShowID", showID << "\n";
    out << "ShowType", showType << "\n";
    out << "ShowAlt", showAlt << "\n";
    out << "ShowLat", showLat << "\n";
    out << "ShowLon", showLon << "\n";
    out << "ShowS", showS << "\n";
    out << "ShowSpas", showSpas << "\n";
    out << "ShowSrem", showSrem << "\n";
    out << "ShowMD", showMD << "\n";
    out << "ShowOZMPUv", showOZMPUv << "\n";
    out << "ShowOZMPUp", showOZMPUp << "\n";
    out << "ShowPv", showPv << "\n";
    out << "ShowPp", showPp << "\n";
    out << "ShowMPU", showMPU << "\n";
    out << "ShowIPU", showIPU << "\n";
    out << "ShowRSBN", showRSBN << "\n";
    out << "ShowSm", showSm << "\n";
    out << "ShowZm", showZm << "\n";
    out << "ShowMapAngle", showMapAngle << "\n";
    out << "ShowAtarg", showAtarg << "\n";
    out << "ShowDtarg", showDtarg << "\n";
    out << "ShowN_size =" << showN_size << "\n";
    out << "ShowID_size =" << showID_size << "\n";
    out << "ShowType_size =" << showType_size << "\n";
    out << "ShowAlt_size =" << showAlt_size << "\n";
    out << "ShowLat_size =" << showLat_size << "\n";
    out << "ShowLon_size =" << showLon_size << "\n";
    out << "ShowS_size =" << showS_size << "\n";
    out << "ShowSpas_size =" << showSpas_size << "\n";
    out << "ShowSrem_size =" << showSrem_size << "\n";
    out << "ShowMD_size =" << showMD_size << "\n";
    out << "ShowOZMPUv_size =" << showOZMPUv_size << "\n";
    out << "ShowOZMPUp_size =" << showOZMPUp_size << "\n";
    out << "ShowPv_size =" << showPv_size << "\n";
    out << "ShowPp_size =" << showPp_size << "\n";
    out << "ShowMPU_size =" << showMPU_size << "\n";
    out << "ShowIPU_size =" << showIPU_size << "\n";
    out << "ShowRSBN_size =" << showRSBN_size << "\n";
    out << "ShowSm_size =" << showSm_size << "\n";
    out << "ShowZm_size =" << showZm_size << "\n";
    out << "ShowMapAngle_size =" << showMapAngle_size << "\n";
    out << "ShowAtarg_size =" << showAtarg_size << "\n";
    out << "ShowDtarg_size =" << showDtarg_size << "\n";
    out << "ShowN_pos =" << showN_pos << "\n";
    out << "ShowID_pos =" << showID_pos << "\n";
    out << "ShowType_pos =" << showType_pos << "\n";
    out << "ShowAlt_pos =" << showAlt_pos << "\n";
    out << "ShowLat_pos =" << showLat_pos << "\n";
    out << "ShowLon_pos =" << showLon_pos << "\n";
    out << "ShowS_pos =" << showS_pos << "\n";
    out << "ShowSpas_pos =" << showSpas_pos << "\n";
    out << "ShowSrem_pos =" << showSrem_pos << "\n";
    out << "ShowMD_pos =" << showMD_pos << "\n";
    out << "ShowOZMPUv_pos =" << showOZMPUv_pos << "\n";
    out << "ShowOZMPUp_pos =" << showOZMPUp_pos << "\n";
    out << "ShowPv_pos =" << showPv_pos << "\n";
    out << "ShowPp_pos =" << showPp_pos << "\n";
    out << "ShowMPU_pos =" << showMPU_pos << "\n";
    out << "ShowIPU_pos =" << showIPU_pos << "\n";
    out << "ShowRSBN_pos =" << showRSBN_pos << "\n";
    out << "ShowSm_pos =" << showSm_pos << "\n";
    out << "ShowZm_pos =" << showZm_pos << "\n";
    out << "ShowMapAngle_pos =" << showMapAngle_pos << "\n";
    out << "ShowAtarg_pos =" << showAtarg_pos << "\n";
    out << "ShowDtarg_pos =" << showDtarg_pos << "\n";



    outfile.close();
}
*/
/*
bool DialogSettings::isChanged()
{
    if(xDir.compare(_xDir)!=0) return true;
    if(fileAirports.compare(_fileAirports)!=0) return true;
    if(fileNavaids.compare(_fileNavaids)!=0) return true;
    if(fileWaypoints.compare(_fileWaypoints)!=0) return true;
    if(fileAirways.compare(_fileAirways)!=0) return true;
    if(fileNavdata.compare(_fileNavdata)!=0) return true;
    if(manualSetDir!=_manualSetDir) return true;
    if(fileRSBN.compare(_fileRSBN)!=0) return true;
    if(correctionVORDME!=_correctionVORDME) return true;
    if(beaconDistance!=_beaconDistance) return true;
    if(windowWidth!=_windowWidth) return true;
    if(windowHeight!=_windowHeight) return true;
    if(distAlignWPS!=_distAlignWPS) return true;
    if(distAlignFMS!=_distAlignFMS) return true;
    if(distAlignATS!=_distAlignATS) return true;
    if(distAlignEarthNav!=_distAlignEarthNav) return true;
    if(distAlignMargin!=_distAlignMargin) return true;
    if(showFeet!=_showFeet) return true;
    if(showN!=_showN) return true;
    if(showID!=_showID) return true;
    if(showType!=_showType) return true;
    if(showAlt!=_showAlt) return true;
    if(showLat!=_showLat) return true;
    if(showLon!=_showLon) return true;
    if(showS!=_showS) return true;
    if(showSpas!=_showSpas) return true;
    if(showSrem!=_showSrem) return true;
    if(showMD!=_showMD) return true;
    if(showOZMPUv!=_showOZMPUv) return true;
    if(showOZMPUp!=_showOZMPUp) return true;
    if(showPv!=_showPv) return true;
    if(showPp!=_showPp) return true;
    if(showMPU!=_showMPU) return true;
    if(showIPU!=_showIPU) return true;
    if(showRSBN!=_showRSBN) return true;
    if(showSm!=_showSm) return true;
    if(showZm!=_showZm) return true;
    if(showMapAngle!=_showMapAngle) return true;
    if(showAtarg!=_showAtarg) return true;
    if(showDtarg!=_showDtarg) return true;

    return false;
}
*/

void DialogSettings::on_toolButton_6_clicked()
{
    QString _xDir = QFileDialog::getExistingDirectory(this, "Choose X-Plane directory");
    if(_xDir.isEmpty()) return;

    xDir = _xDir;
    fileAirports = xDir + "/Resources/GNS430/navdata/Airports.txt";
    fileNavaids = xDir + "/Resources/GNS430/navdata/Navaids.txt";
    fileWaypoints = xDir + "/Resources/GNS430/navdata/Waypoints.txt";
    fileAirways = xDir + "/Resources/GNS430/navdata/ATS.txt";
    fileNavdata = xDir + "/Resources/default data/earth_nav.dat";

    ui->lineEditXPlaneDirectory->setText(xDir);
    if(manualSetDir) return;

    ui->lineEditAirportsFiles->setText(fileAirports);
    ui->lineEditNavaidsFile->setText(fileNavaids);
    ui->lineEditWaypointsFile->setText(fileWaypoints);
    ui->lineEditAirwaysFile->setText(fileAirways);
    ui->lineEditEarthnavFile->setText(fileNavdata);
}

void DialogSettings::on_toolButton_4_clicked()
{
    QString rsbnFile = QFileDialog::getOpenFileName(this, "Select TU154 rsbn.dat", xDir, "RSBN.dat file (*.dat)");
    if(rsbnFile.isEmpty()) return;

    fileRSBN = rsbnFile;
    ui->lineEditRSBNFile->setText(fileRSBN);
}

void DialogSettings::on_lineEditXPlaneDirectory_textChanged(const QString &arg1)
{
    xDir = arg1;

    if(manualSetDir) return;

    fileAirports = xDir + "/Resources/GNS430/navdata/Airports.txt";
    fileNavaids = xDir + "/Resources/GNS430/navdata/Navaids.txt";
    fileWaypoints = xDir + "/Resources/GNS430/navdata/Waypoints.txt";
    fileAirways = xDir + "/Resources/GNS430/navdata/ATS.txt";
    fileNavdata = xDir + "/Resources/default data/earth_nav.dat";

    ui->lineEditXPlaneDirectory->setText(xDir);
    ui->lineEditAirportsFiles->setText(fileAirports);
    ui->lineEditNavaidsFile->setText(fileNavaids);
    ui->lineEditWaypointsFile->setText(fileWaypoints);
    ui->lineEditAirwaysFile->setText(fileAirways);
    ui->lineEditEarthnavFile->setText(fileNavdata);
}

void DialogSettings::on_buttonBox_accepted()
{
    fileRSBN = ui->lineEditRSBNFile->text();

    xDir = ui->lineEditXPlaneDirectory->text();
    fileAirports = ui->lineEditAirportsFiles->text();
    fileNavaids = ui->lineEditNavaidsFile->text();
    fileWaypoints = ui->lineEditWaypointsFile->text();
    fileAirways = ui->lineEditAirwaysFile->text();
    fileNavdata = ui->lineEditEarthnavFile->text();
    manualSetDir = ui->checkBoxManual->isChecked();
    XP11 = ui->checkBoxXP11->isChecked();
    XP11_includeCustomAirports = ui->checkBoxAPTOverride->isChecked();
}

void DialogSettings::on_checkBoxManual_clicked(bool checked)
{
    manualSetDir = checked;
    if(checked)
    {
        ui->lineEditAirportsFiles->setEnabled(true);
        ui->lineEditNavaidsFile->setEnabled(true);
        ui->lineEditWaypointsFile->setEnabled(true);
        ui->lineEditAirwaysFile->setEnabled(true);
        ui->lineEditEarthnavFile->setEnabled(true);
    }
    else
    {
        ui->lineEditAirportsFiles->setEnabled(false);
        ui->lineEditNavaidsFile->setEnabled(false);
        ui->lineEditWaypointsFile->setEnabled(false);
        ui->lineEditAirwaysFile->setEnabled(false);
        ui->lineEditEarthnavFile->setEnabled(false);
    }
}
