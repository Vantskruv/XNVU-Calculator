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
QString DialogSettings::defaultLoadSaveDir="";
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
QSize DialogSettings::windowSize(1600, 1054);
QPoint DialogSettings::windowPos;
QByteArray DialogSettings::tableState;
bool DialogSettings::distAlignXWP = false;
bool DialogSettings::distAlignFMS = false;
double DialogSettings::distAlignMargin = 0.0001;
bool DialogSettings::nightMode = true;
int DialogSettings::showSummary = 1;
bool DialogSettings::showFeet = false;
//bool DialogSettings::showFeet_PRINT = false;
int DialogSettings::cruiseFormat = 0;
int DialogSettings::VSFormat = 0;
int DialogSettings::TWCFormat = 0;
bool DialogSettings::customDateIsTrue = true;
QString DialogSettings::customDate = "2020.01.01";
bool DialogSettings::showTOD_METRIC = true;
//bool DialogSettings::showTOD_METRIC_PRINT = true;



DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);

    ui->lineEditXPlaneDirectory->setText(xDir);
    ui->lineEditDefaultLoadSaveDir->setText(defaultLoadSaveDir);
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
    defaultLoadSaveDir = s.value("DefaultLoadSaveDir").toString();
    fileAirports = s.value("AirportFile").toString();
    fileNavaids = s.value("NavaidFile").toString();
    fileWaypoints = s.value("WaypointsFile", fileWaypoints).toString();
    fileAirways = s.value("AirwaysFile", fileAirways).toString();
    fileNavdata = s.value("EarthNavFile", fileNavdata).toString();
    fileRSBN = s.value("RSBNFile", fileRSBN).toString();
    manualSetDir = s.value("ManualSetDir", manualSetDir).toBool();
    correctionVORDME = s.value("VORDMECorrection", correctionVORDME).toBool();
    beaconDistance = s.value("BeaconDistance", beaconDistance).toInt();
    distAlignXWP = s.value("DistAlignWPS", distAlignXWP).toBool();
    distAlignFMS = s.value("DistAlignFMS", distAlignFMS).toBool();
    distAlignMargin = s.value("DistAlignMargin", distAlignMargin).toDouble();
    nightMode = s.value("NightMode", nightMode).toBool();
    showSummary = s.value("showSummary", showSummary).toInt();
    showFeet = s.value("ShowFeet", showFeet).toBool();
    //showFeet_PRINT = s.value("ShowFeet_PRINT", showFeet_PRINT).toBool();
    cruiseFormat = s.value("CruiseFormat", cruiseFormat).toInt();
    VSFormat = s.value("VSFormat", VSFormat).toInt();
    TWCFormat = s.value("TWCFormat", TWCFormat).toInt();
    windowSize = s.value("WindowSize", windowSize).toSize();
    windowPos = s.value("WindowPos", windowPos).toPoint();
    tableState = s.value("TableState", tableState).toByteArray();
    customDateIsTrue = s.value("CustomDateIsTrue", customDateIsTrue).toBool();
    customDate = s.value("CustomDate", customDate).toString();
    showTOD_METRIC = s.value("ShowTOD_METRIC", showTOD_METRIC).toBool();
    //showTOD_METRIC_PRINT = s.value("ShowTOD_METRIC_PRINT", showTOD_METRIC_PRINT).toBool();
}

void DialogSettings::saveSettings()
{
    QSettings s("xnvu.conf", QSettings::IniFormat);

    s.setValue("XP11", XP11);
    s.setValue("XP11_includeCustomAirports", XP11_includeCustomAirports);
    s.setValue("XPlaneDirectory", xDir);
    s.setValue("DefaultLoadSaveDir", defaultLoadSaveDir);
    s.setValue("AirportFile", fileAirports);
    s.setValue("NavaidFile", fileNavaids);
    s.setValue("WaypointsFile", fileWaypoints);
    s.setValue("AirwaysFile", fileAirways);
    s.setValue("EarthNavFile", fileNavdata);
    s.setValue("ManualSetDir", manualSetDir);
    s.setValue("RSBNFile", fileRSBN);
    s.setValue("VORDMECorrection", correctionVORDME);
    s.setValue("BeaconDistance", beaconDistance);
    s.setValue("DistAlignWPS", distAlignXWP);
    s.setValue("DistAlignFMS", distAlignFMS);
    s.setValue("DistAlignMargin", distAlignMargin);
    s.setValue("ShowFeet", showFeet);
    s.setValue("showSummary", showSummary);
    //s.setValue("ShowFeet_PRINT", showFeet_PRINT);
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
    //s.setValue("ShowTOD_METRIC_PRINT", showTOD_METRIC_PRINT);
}


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

void DialogSettings::on_toolButton_5_clicked()
{
    QString _cDir = QFileDialog::getExistingDirectory(this, "Choose X-Plane directory", DialogSettings::defaultLoadSaveDir);
    if(_cDir.isEmpty()) return;

    ui->lineEditDefaultLoadSaveDir->setText(_cDir);
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
    defaultLoadSaveDir = ui->lineEditDefaultLoadSaveDir->text();
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


