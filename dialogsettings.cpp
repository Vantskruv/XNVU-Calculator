#include "dialogsettings.h"
#include "ui_dialogsettings.h"

#include <QFileDialog>
#include <QTextStream>
#include <QFileInfo>

QString DialogSettings::xDir;
QString DialogSettings::fileAirports;
QString DialogSettings::fileNavaids;
QString DialogSettings::fileWaypoints;
QString DialogSettings::fileNavdata;
QString DialogSettings::fileRSBN;
bool DialogSettings::correctionVORDME = false;
int DialogSettings::beaconDistance = 500;
int DialogSettings::windowWidth = 1600;
int DialogSettings::windowHeight = 1054;

QString DialogSettings::_xDir;
QString DialogSettings::_fileAirports;
QString DialogSettings::_fileNavaids;
QString DialogSettings::_fileWaypoints;
QString DialogSettings::_fileNavdata;
QString DialogSettings::_fileRSBN;
bool DialogSettings::_correctionVORDME = false;
int DialogSettings::_beaconDistance = 500;
int DialogSettings::_windowWidth = 1600;
int DialogSettings::_windowHeight = 1054;


DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);

    ui->lineEditXPlaneDirectory->setText(xDir);
    ui->lineEditAirportsFiles->setText(fileAirports);
    ui->lineEditNavaidsFile->setText(fileNavaids);
    ui->lineEditWaypointsFile->setText(fileWaypoints);
    ui->lineEditEarthnavFile->setText(fileNavdata);
    ui->lineEditRSBNFile->setText(fileRSBN);
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

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
                _xDir = qstr;
            }
            else if(list[0].simplified().compare("AirportFile")==0)
            {
                fileAirports = qstr;
                _fileAirports = qstr;
            }
            else if(list[0].simplified().compare("NavaidFile")==0)
            {
                fileNavaids = qstr;
                _fileNavaids = qstr;
            }
            else if(list[0].simplified().compare("WaypointsFile")==0)
            {
                fileWaypoints = qstr;
                _fileWaypoints = qstr;
            }
            else if(list[0].simplified().compare("EarthNavFile")==0)
            {
                fileNavdata = qstr;
                _fileNavdata = qstr;
            }
            else if(list[0].simplified().compare("RSBNFile")==0)
            {
                fileRSBN = qstr;
                _fileRSBN = qstr;
            }
            else if(list[0].simplified().compare("VORDMECorrection")==0)
            {
                correctionVORDME = qstr.toInt();
                _correctionVORDME = qstr.toInt();
            }
            else if(list[0].simplified().compare("BeaconDistance")==0)
            {
                beaconDistance = qstr.toInt();
                _beaconDistance = qstr.toInt();
            }
            else if(list[0].simplified().compare("WindowWidth")==0)
            {
                windowWidth = qstr.toInt();
                _windowWidth = qstr.toInt();
            }
            else if(list[0].simplified().compare("WindowHeight")==0)
            {
                windowHeight = qstr.toInt();
                _windowHeight = qstr.toInt();
            }


        }

    }//while

    infile.close();

/*
    QString sError;
    QFileInfo qf(xDir);
    if(!qf.exists() || !qf.isDir()) sError = " [XPlane dir]";
    qf = QFileInfo(fileAirports);
    if(!qf.exists() || !qf.isFile()) sError = " [Airports.txt]";
    qf = QFileInfo(fileNavaids);
    if(!qf.exists() || !qf.isFile()) sError = " [Navaids.txt]";
    qf = QFileInfo(fileWaypoints);
    if(!qf.exists() || !qf.isFile()) sError = " [Waypoints.txt]";
    qf = QFileInfo(fileNavdata);
    if(!qf.exists() || !qf.isFile()) sError = " [earth_nav.txt]";
    qf = QFileInfo(fileRSBN);
    if(!qf.exists() || !qf.isFile()) sError = " [rsbn.dat]";

    return sError;
    */

    return;
}

void DialogSettings::saveSettings()
{
    if(!DialogSettings::isChanged()) return;


    QFile outfile(XNVU_SETTINGS_FILE);
    if(!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&outfile);

    out << "XPlaneDirectory = " << xDir << "\n";
    out << "AirportFile = " << fileAirports << "\n";
    out << "NavaidFile = " << fileNavaids << "\n";
    out << "WaypointsFile = " << fileWaypoints << "\n";
    out << "EarthNavFile = " << fileNavdata << "\n";
    out << "RSBNFile = " << fileRSBN << "\n";
    out << "VORDMECorrection = " << correctionVORDME << "\n";
    out << "BeaconDistance = " << beaconDistance << "\n";
    out << "WindowWidth = " << windowWidth << "\n";
    out << "WindowHeight = " << windowHeight<< "\n";

    outfile.close();
}

bool DialogSettings::isChanged()
{
    if(xDir.compare(_xDir)!=0) return true;
    if(fileAirports.compare(_fileAirports)!=0) return true;
    if(fileNavaids.compare(_fileNavaids)!=0) return true;
    if(fileWaypoints.compare(_fileWaypoints)!=0) return true;
    if(fileNavdata.compare(_fileNavdata)!=0) return true;
    if(fileRSBN.compare(_fileRSBN)!=0) return true;
    if(correctionVORDME!=_correctionVORDME) return true;
    if(beaconDistance!=_beaconDistance) return true;
    if(windowWidth!=_windowWidth) return true;
    if(windowHeight!=_windowHeight) return true;

    return false;
}

void DialogSettings::on_toolButton_6_clicked()
{
    QString _xDir = QFileDialog::getExistingDirectory(this, "Choose X-Plane directory");
    if(_xDir.isEmpty()) return;

    xDir = _xDir;
    fileAirports = xDir + "/Resources/GNS430/navdata/Airports.txt";
    fileNavaids = xDir + "/Resources/GNS430/navdata/Navaids.txt";
    fileWaypoints = xDir + "/Resources/GNS430/navdata/Waypoints.txt";
    fileNavdata = xDir + "/Resources/default data/earth_nav.dat";

    ui->lineEditXPlaneDirectory->setText(xDir);
    ui->lineEditAirportsFiles->setText(fileAirports);
    ui->lineEditNavaidsFile->setText(fileNavaids);
    ui->lineEditWaypointsFile->setText(fileWaypoints);
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
    fileAirports = xDir + "/Resources/GNS430/navdata/Airports.txt";
    fileNavaids = xDir + "/Resources/GNS430/navdata/Navaids.txt";
    fileWaypoints = xDir + "/Resources/GNS430/navdata/Waypoints.txt";
    fileNavdata = xDir + "/Resources/default data/earth_nav.dat";

    ui->lineEditXPlaneDirectory->setText(xDir);
    ui->lineEditAirportsFiles->setText(fileAirports);
    ui->lineEditNavaidsFile->setText(fileNavaids);
    ui->lineEditWaypointsFile->setText(fileWaypoints);
    ui->lineEditEarthnavFile->setText(fileNavdata);
}
