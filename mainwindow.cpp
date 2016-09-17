#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "XFMS_DATA.h"
#include "LMATH.h"
#include "qlistwidgetitemdata.h"
#include "qtablewidgetitemdata.h"
#include <vector>
#include <sstream>
#include <QShortcut>
#include <coremag.h>
#include "dialogsettings.h"
#include "dialogwaypointedit.h"
#include "dialogrsbn.h"
#include "dialogoptions.h"
#include "dialogwpsedit.h"
#include "dialogcolumns.h"
#include <ctime>

#define XNVU_VERSION    "XNVU version 0.31 beta"

//XFMS_DATA xdata;
int dat;
QLabel* labelWarning;
QTimer clearFlightplan_timer;
int clearFlightplan_countdown = -1;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(XNVU_VERSION);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    //Setup shortcuts
    QShortcut* shortcutDeleteKey = new QShortcut(QKeySequence(Qt::Key_Delete), ui->tableWidget);
    QShortcut* shortcutUpKey = new QShortcut(QKeySequence(Qt::Key_Up), ui->tableWidget);
    QShortcut* shortcutDownKey = new QShortcut(QKeySequence(Qt::Key_Down), ui->tableWidget);
    connect(shortcutDeleteKey, SIGNAL(activated()), this, SLOT(deleteCurrentWaypoint()));
    connect(shortcutUpKey, SIGNAL(activated()), this, SLOT(tableGoUp()));
    connect(shortcutDownKey, SIGNAL(activated()), this, SLOT(tableGoDown()));
    connect(ui->actionExport_X_Plane_FMS, SIGNAL(triggered()), this, SLOT(exportFMS()));
    connect(ui->actionImport_X_Plane_FMS, SIGNAL(triggered()), this, SLOT(importFMS()));
    connect(ui->actionSave_XNVU_flightplan, SIGNAL(triggered()), this, SLOT(saveNVUFlightPlan()));
    connect(ui->actionLoad_XNVU_flightplan, SIGNAL(triggered()), this, SLOT(loadNVUFlightplan()));
    connect(ui->actionX_Plane_folder, SIGNAL(triggered()), this, SLOT(showXPlaneSettings()));
    connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(on_pushButtonPrint_clicked()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(quit()));
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showFlightplanContextMenu(const QPoint&)));
    connect(ui->frameDescription, SIGNAL(clicked(QPoint)), this, SLOT(on_frameDescription_clicked()));
    connect(&clearFlightplan_timer, SIGNAL(timeout()), this, SLOT(clearFlightplanTimeout()));

    //Give tablewidget a reference to labelFork and set its size
    ui->tableWidget->setColumnCount(QFlightplanTable::COL::_SIZE);
    ui->tableWidget->qFork = ui->labelFork;
    ui->tableWidget->horizontalHeader()->show();


    //Set color for status bar
    QPalette pal = ui->statusBar->palette();
    pal.setColor(ui->statusBar->foregroundRole(), QColor(255, 255, 0));
    ui->statusBar->setPalette(pal);

    //Set lineEdits to always show as capitalized (not this does not mean that the font IS capitalized)
    QFont font = ui->lineEdit->font();
    font.setCapitalization(QFont::AllUppercase);
    ui->lineEdit->setFont(font);
    ui->lineEditRoute->setFont(font);

    //Setup warning label
    labelWarning = new QLabel("");
    labelWarning->setStyleSheet("background-color: rgb(0, 30, 0);"
                                "color: rgb(255, 255, 0);");
    ui->statusBar->addPermanentWidget(labelWarning);

    //Load settings and data
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    dat = yymmdd_to_julian_days(now->tm_year, now->tm_mon+1, now->tm_mday);
    ui->tableWidget->dat = dat;
    DialogSettings::loadSettings();
    //this->resize(QSize(DialogSettings::windowWidth, DialogSettings::windowHeight));
    ui->actionShow_feet->setChecked(DialogSettings::showFeet);
    ui->tableWidget->showFeet = DialogSettings::showFeet;
    if(DialogSettings::showFeet)
    {
        ui->spinBoxFL->setSuffix(" ft");
    }//if
    else ui->spinBoxFL->setSuffix(" m");

    QString sError = XFMS_DATA::load(dat);
    if(!sError.isEmpty())
    {
        labelWarning->setText("WARNING: " + sError + "is not loaded!");
    }

    ui->label_alignATS->setText("ATS align");
    ui->label_alignEarthNav->setText("EarthNav align");
    ui->label_alignWPS->setText("WPS align");
    ui->label_alignFMS->setText("FMS align");

    ui->label_alignATS->setVisible(DialogSettings::distAlignATS);
    ui->label_alignEarthNav->setVisible(DialogSettings::distAlignEarthNav);
    ui->label_alignWPS->setVisible(DialogSettings::distAlignWPS);
    ui->label_alignFMS->setVisible(DialogSettings::distAlignFMS);

    //ui->tableWidget->updateShownColumns();

    this->resize(DialogSettings::windowSize);
    this->move(DialogSettings::windowPos);
    ui->tableWidget->horizontalHeader()->restoreState(DialogSettings::tableState);
}

MainWindow::~MainWindow()
{
    DialogSettings::windowSize = size();
    DialogSettings::windowPos = pos();
    DialogSettings::tableState = ui->tableWidget->horizontalHeader()->saveState();
    DialogSettings::showFeet = ui->actionShow_feet->isChecked();
    DialogSettings::saveSettings();
/*
    //Save section position and size of QTableFlightplan
    DialogSettings::showN_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::N);
    DialogSettings::showID_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::ID);
    DialogSettings::showType_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::TYPE);
    DialogSettings::showAlt_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::ALT);
    DialogSettings::showLat_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::LAT);
    DialogSettings::showLon_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::LON);
    DialogSettings::showS_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::S);
    DialogSettings::showSpas_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::SPAS);
    DialogSettings::showSrem_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::SREM);
    DialogSettings::showMD_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::MD);
    DialogSettings::showOZMPUv_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::OZMPUV);
    DialogSettings::showOZMPUp_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::OZMPUP);
    DialogSettings::showPv_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::PV);
    DialogSettings::showPp_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::PP);
    DialogSettings::showMPU_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::MPU);
    DialogSettings::showIPU_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::IPU);
    DialogSettings::showRSBN_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::RSBN);
    DialogSettings::showSm_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::SM);
    DialogSettings::showZm_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::ZM);
    DialogSettings::showMapAngle_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::MAPA);
    DialogSettings::showAtarg_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::ATRG);
    DialogSettings::showDtarg_pos = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::DTRG);


    DialogSettings::showN_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::N);
    DialogSettings::showID_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::ID);
    DialogSettings::showType_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::TYPE);
    DialogSettings::showAlt_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::ALT);
    DialogSettings::showLat_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::LAT);
    DialogSettings::showLon_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::LON);
    DialogSettings::showS_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::S);
    DialogSettings::showSpas_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::SPAS);
    DialogSettings::showSrem_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::SREM);
    DialogSettings::showMD_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::MD);
    DialogSettings::showOZMPUv_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::OZMPUV);
    DialogSettings::showOZMPUp_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::OZMPUP);
    DialogSettings::showPv_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::PV);
    DialogSettings::showPp_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::PP);
    DialogSettings::showMPU_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::MPU);
    DialogSettings::showIPU_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::IPU);
    DialogSettings::showRSBN_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::RSBN);
    DialogSettings::showSm_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::SM);
    DialogSettings::showZm_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::ZM);
    DialogSettings::showMapAngle_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::MAPA);
    DialogSettings::showAtarg_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::ATRG);
    DialogSettings::showDtarg_size = ui->tableWidget->horizontalHeader()->sectionSize(ui->tableWidget->COL::DTRG);
*/

    XFMS_DATA::saveXNVUData();
    //nvu.clear();
    XFMS_DATA::clear();
    delete ui;
}

void MainWindow::quit()
{
    qApp->quit();
}

void MainWindow::showFlightplanContextMenu(const QPoint& pos) // this is a slot
{
    // for most widgets
    QPoint globalPos = ui->tableWidget->mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);



    QMenu fpMenu;
    QAction* selectedItem;
    QString styleSheet = "background-color: rgb(0, 30, 0);"
                         "color: rgb(107, 239, 0);"
                         "selected-background-color: rgb(107, 239, 0);"
                         "selected-color: rgb(0, 0, 0);";
    fpMenu.setStyleSheet(styleSheet);

    int row = ui->tableWidget->rowAt(pos.y());
    if(row>=0)
    {
        fpMenu.addAction("Move up");
        fpMenu.addAction("Move down");
        fpMenu.addAction("Edit waypoint...");
        fpMenu.addAction("Set correction beacon...");
        fpMenu.addSeparator();
        fpMenu.addAction("Delete");

        selectedItem = fpMenu.exec(globalPos);
        if(selectedItem)
        {
            NVUPOINT* wp = ui->tableWidget->getWaypoint(row);
            if(selectedItem == fpMenu.actions()[0])
            {
                if(row>0)
                {
                    ui->tableWidget->moveWaypoint(row, true);
                }
            }
            else if(selectedItem == fpMenu.actions()[1])
            {
                if(row<(ui->tableWidget->rowCount()-1))
                {
                    ui->tableWidget->moveWaypoint(row, false);
                }
            }
            else if(selectedItem == fpMenu.actions()[2])
            {
                DialogWaypointEdit dEdit(wp, true);
                int dr = dEdit.exec();
                if(dr == QDialog::Rejected || dr == DialogWaypointEdit::CANCEL) return;

                if(dr==DialogWaypointEdit::ADD_XNVU) //Create new
                {
                    XFMS_DATA::addXNVUWaypoint(new NVUPOINT(dEdit.nvupoint));
                    ui->tableWidget->replaceWaypoint(new NVUPOINT(dEdit.nvupoint), row);
                }
                else if(dr==DialogWaypointEdit::SAVE)
                {
                    WAYPOINT* rsbn = wp->rsbn;
                    wp->clone(dEdit.nvupoint);
                    wp->rsbn = rsbn;
                    ui->tableWidget->refreshFlightplan();
                }
            }
            else if(selectedItem == fpMenu.actions()[3])
            {
                DialogRSBN dRSBN(wp);
                int dr = dRSBN.exec();
                if(dr == QDialog::Rejected) return;

                wp->rsbn = dRSBN.rsbn;
                ui->tableWidget->refreshFlightplan();
            }
            else if(selectedItem == fpMenu.actions()[5])
            {
                ui->tableWidget->deleteWaypoint(row);
            }
        }
    }//if
    else
    {
        //Clicked outside
        fpMenu.addAction("Add custom waypoint...");
        selectedItem = fpMenu.exec(globalPos);
        if(selectedItem)
        {
            DialogWaypointEdit dEdit(NULL, true);
            int dr = dEdit.exec();
            if(dr == QDialog::Rejected || dr == DialogWaypointEdit::CANCEL) return;
            if(dr == DialogWaypointEdit::ADD_XNVU)
            {
                XFMS_DATA::addXNVUWaypoint(new NVUPOINT(dEdit.nvupoint));
                ui->tableWidget->insertWaypoint(new NVUPOINT(dEdit.nvupoint), ui->tableWidget->rowCount());
            }
            else if(dr==DialogWaypointEdit::SAVE)
            {
                ui->tableWidget->insertWaypoint(new NVUPOINT(dEdit.nvupoint), ui->tableWidget->rowCount());
            }
        }

        return;
    }
}


void MainWindow::showXPlaneSettings()
{
    DialogSettings dSettings;
    if(QDialog::Rejected == dSettings.exec()) return;

    DialogSettings::saveSettings();

    XFMS_DATA::saveXNVUData();
    ui->lineEdit->clear();
    ui->listWidget->clear();
    XFMS_DATA::clear();

    QString sError = XFMS_DATA::load(dat);
    if(!sError.isEmpty())
    {
        labelWarning->setText("WARNING: " + sError + " is not loaded!");
    }//if
    else labelWarning->setText("");

    ui->label_alignATS->setVisible(DialogSettings::distAlignATS);
    ui->label_alignEarthNav->setVisible(DialogSettings::distAlignEarthNav);
    ui->label_alignWPS->setVisible(DialogSettings::distAlignWPS);
    ui->label_alignFMS->setVisible(DialogSettings::distAlignFMS);
}

void MainWindow::importFMS()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import FMS flightplan","", "FMS Files (*.fms);;All files (*.*)");
    if(fileName.isEmpty()) return;
    ui->tableWidget->clearFlightplan();
    std::vector<NVUPOINT*> lFMS = XFMS_DATA::loadFMS(fileName);
    ui->tableWidget->insertRoute(lFMS, 0);
}

void MainWindow::exportFMS()
{
    QString qstr = ".fms";
    std::vector<NVUPOINT*> lP = ui->tableWidget->getWaypoints();
    if(lP.size()>1)
    {
        qstr = lP[0]->name + "_" + lP[lP.size()-1]->name + qstr;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Export FMS flightplan", qstr, "FMS Files (*.fms);;All files (*.*)");
    if(fileName.isEmpty()) return;
    XFMS_DATA::saveFMS(fileName, ui->tableWidget->getWaypoints());
}

void MainWindow::loadNVUFlightplan()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load XNVU flightplan", "", "XWP Files (*.xwp);;All files (*.*)");
    if(fileName.isEmpty()) return;
    ui->tableWidget->clearFlightplan();
    std::vector<NVUPOINT*> lXNVU = XFMS_DATA::loadXNVUFlightplan(fileName);
    ui->tableWidget->insertRoute(lXNVU, 0);
}

void MainWindow::saveNVUFlightPlan()
{
    QString qstr = ".xwp";
    std::vector<NVUPOINT*> lP = ui->tableWidget->getWaypoints();
    if(lP.size()>1)
    {
        qstr = lP[0]->name + "_" + lP[lP.size()-1]->name + qstr;
    }


    QString fileName = QFileDialog::getSaveFileName(this, "Save XNVU flightplan", qstr, "XWP Files (*.xwp);;All files (*.*)");
    if(fileName.isEmpty()) return;
    XFMS_DATA::saveXNVUFlightplan(fileName, ui->tableWidget->getWaypoints());
}



void MainWindow::deleteCurrentWaypoint()
{
    if(ui->tableWidget->currentRow()<0) return;
    ui->tableWidget->deleteWaypoint(ui->tableWidget->currentRow());
}


void MainWindow::tableGoUp()
{
    int c = ui->tableWidget->currentRow();
    c--;
    if(c<0) return;

    ui->tableWidget->selectRow(c);
}

void MainWindow::tableGoDown()
{
    int c = ui->tableWidget->currentRow();
    c++;
    if(c>=ui->tableWidget->rowCount()) return;

    ui->tableWidget->selectRow(c);
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
    QString sSearch = ui->lineEdit->text();
    ui->listWidget->search(sSearch, true);
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *_item)
{
    QListWidgetItemData* itemData = (QListWidgetItemData*) _item;
    ui->tableWidget->insertWaypoint(new NVUPOINT(*itemData->nvupoint), ui->tableWidget->rowCount());
}


void MainWindow::printPreview(QPrinter* printer)
{
    printer->setPageSize(QPrinter::A4);
    printer->setOrientation(QPrinter::Landscape);
    printer->setPageMargins (10,10,10,10,QPrinter::Millimeter);
    printer->setFullPage(false);
    //printer->setOutputFileName("output.pdf");
    //printer->setOutputFormat(QPrinter::PdfFormat); //you can use native format of system usin QPrinter::NativeFormat

    std::vector<NVUPOINT*> lWPs = ui->tableWidget->getWaypoints();
    double fork;
    if(lWPs.size()<1) return;

    NVU::generate(lWPs, fork, dat);

    QPainter painter(printer); // create a painter which will paint 'on printer'.

#ifdef _WIN32
    painter.scale(0.5, 0.5);
#endif

    int y=0;
    drawNVUHeader(painter, lWPs[0], lWPs[lWPs.size() - 1], fork, y);
    y+=30;
    int j = 0;
    for(int i=0; i<lWPs.size(); i++, j++)
    {
        if(j==19)
        {
            printer->newPage();
            y = 0;
            drawNVUHeader(painter, lWPs[0], lWPs[lWPs.size() - 1], fork, y);
            y+=30;
            j = 0;
        }
        painterDrawNVUPoint(painter, lWPs[i], i+1, i == (lWPs.size() - 1), y);

    }

    painter.end();
}

void MainWindow::on_pushButtonInsertBefore_clicked()
{
    if(ui->listWidget->currentRow()<0) return;
    QListWidgetItemData* itemData = (QListWidgetItemData*)ui->listWidget->currentItem();

    int row = ui->tableWidget->currentRow();
    if(row<0) row = 0;
    ui->tableWidget->insertWaypoint(new NVUPOINT(*itemData->nvupoint), row);
}

void MainWindow::on_pushButtonReplace_clicked()
{

    if(ui->listWidget->currentRow()<0) return;
    QListWidgetItemData* itemData = (QListWidgetItemData*)ui->listWidget->currentItem();
    ui->tableWidget->replaceWaypoint(new NVUPOINT(*itemData->nvupoint), ui->tableWidget->currentRow());
}


void MainWindow::on_pushButtonInsertAfter_clicked()
{
    if(ui->listWidget->currentRow()<0) return;
    QListWidgetItemData* itemData = (QListWidgetItemData*)ui->listWidget->currentItem();

    int row = ui->tableWidget->currentRow();
    if(row<0) row = ui->tableWidget->rowCount();
    else row++;
    ui->tableWidget->insertWaypoint(new NVUPOINT(*itemData->nvupoint), row);
}

void MainWindow::on_pushButtonRouteInsertAfter_clicked()
{
    std::vector<NVUPOINT*> route;
    QString sError = XFMS_DATA::getRoute(ui->lineEditRoute->text().toUpper(), route);
    if(!sError.isEmpty())
    {
        ui->statusBar->showMessage(sError, 10000);
        return;
    }

    int row = ui->tableWidget->currentRow();
    if(row<0) row = ui->tableWidget->rowCount();
    else row++;

    ui->tableWidget->insertRoute(route, row);
}


void MainWindow::on_pushButtonRouteInsertBefore_clicked()
{
    std::vector<NVUPOINT*> route;
    QString sError = XFMS_DATA::getRoute(ui->lineEditRoute->text().toUpper(), route);
    if(!sError.isEmpty())
    {
        ui->statusBar->showMessage(sError, 10000);
        return;
    }

    int row = ui->tableWidget->currentRow();
    if(row<0) row = 0;

    ui->tableWidget->insertRoute(route, row);
}

void MainWindow::on_pushButtonRouteReplace_clicked()
{
    std::vector<NVUPOINT*> route;
    QString sError = XFMS_DATA::getRoute(ui->lineEditRoute->text().toUpper(), route);
    if(!sError.isEmpty())
    {
        ui->statusBar->showMessage(sError, 10000);
        return;
    }

    int r = ui->tableWidget->currentRow();
    if(r>=0) ui->tableWidget->deleteWaypoint(r);
    else r = 0;
    ui->tableWidget->insertRoute(route, r);
}


void MainWindow::on_tableWidget_clicked(const QModelIndex &index)
{
    ui->tableWidget->setFocus();
    if(ui->tableWidget->currentRow()<0) return;

    NVUPOINT* wp = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow());
    ui->lineEdit->setText(wp->name);
    setWaypointDescription(wp);
}

void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    if(currentRow<0) return;
    QListWidgetItemData* itemD = (QListWidgetItemData*) ui->listWidget->item(currentRow);
    setWaypointDescription(itemD->nvupoint);
}

void MainWindow::setWaypointDescription(NVUPOINT* wp)
{
    //ui->labelIWPName->setText("");
    ui->labelIWPName2->setText("");
    ui->labelWPType->setText("");
    ui->labelWPMagVar->setText("");
    ui->labelWPLatlon->setText("");
    ui->labelWPNote->setText("");

    QString qstr;
    qstr = WAYPOINT::getTypeStr(wp);
    ui->labelWPType2->setText(qstr);

    qstr = wp->name;
    if(!wp->country.isEmpty()) qstr = qstr + " [" + wp->country + "]";

    if(wp->type == WAYPOINT::TYPE_NDB ||
       wp->type == WAYPOINT::TYPE_RSBN)
    {
        qstr = qstr + "  Ch" + QString::number((int) wp->freq);
    }//if
    else if(wp->type == WAYPOINT::TYPE_VOR ||
            wp->type == WAYPOINT::TYPE_DME ||
            wp->type == WAYPOINT::TYPE_VORDME)
    {
        qstr = qstr + "  " + QString::number(wp->freq, 'f', 3);
    }//if
    ui->labelWPType->setText(qstr);

    if(wp->type == WAYPOINT::TYPE_AIRWAY)
    {
        AIRWAY* ats = (AIRWAY*) wp->data;
        ui->labelIWPName2->setText("[" + ats->lATS[0]->name + "] ---> [" + ats->lATS[ats->lATS.size()-1]->name + "]");
    }
    else if(!wp->name2.isEmpty()) ui->labelIWPName2->setText(wp->name2);

    if(wp->type == WAYPOINT::TYPE_AIRWAY)
    {
        AIRWAY* ats = (AIRWAY*) wp->data;
        ui->labelWPLatlon->setText("Fixes: " + QString::number(ats->lATS.size()) + "    Dist: " + QString::number(ats->distance, 'f', 1) + " KM");
    }
    else
    {
        double l1, l2;
        l1 = fabs(modf(wp->latlon.x, &l2)*60.0);
        int i2 = (int) fabs(l2);
        qstr = "";
        qstr = qstr + "Lat:   " + (wp->latlon.x < 0 ? "S" : "N") + (i2<10 ? "0" : "") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2) + "       ";
        l1 = fabs(modf(wp->latlon.y, &l2)*60.0);
        i2 = (int) fabs(l2);
        qstr = qstr + "Lon:  " + (wp->latlon.y < 0 ? "W" : "E") + (i2<100 ? (i2<10 ? "00" : "0") : "") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2);
        ui->labelWPLatlon->setText(qstr);
    }//else


    if(wp->type==WAYPOINT::TYPE_AIRWAY)
    {
        ui->labelWPMagVar->setText("CLICK TO SHOW WAYPOINTS");
    }
    else
    {
        qstr = "MD: " + QString::number(wp->MD, 'f', 1);
        if(wp->type == WAYPOINT::TYPE_AIRPORT ||
           wp->type == WAYPOINT::TYPE_DME ||
           wp->type == WAYPOINT::TYPE_NDB ||
           wp->type == WAYPOINT::TYPE_VOR ||
           wp->type == WAYPOINT::TYPE_VORDME
           ) qstr = qstr + "        Elev: " + (ui->actionShow_feet->isChecked() ? QString::number(wp->elev, 'f', 0) + " ft": QString::number(LMATH::feetToMeter(wp->elev), 'f', 0) + " m");

        ui->labelWPMagVar->setText(qstr);
    }

    if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_AIRPORTS)
    {
        ui->labelWPNote->setText("Source: airports.txt (GNS430 AIRAC)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_NAVAIDS)
    {
        ui->labelWPNote->setText("Source: navaids.txt (GNS430 AIRAC)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_WAYPOINTS)
    {
        ui->labelWPNote->setText("Source: AIRAC waypoints.txt (GNS430 AIRAC)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_ATS)
    {
        ui->labelWPNote->setText("Source: AIRAC ats.txt (GNS430 AIRAC)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_FMS)
    {
        ui->labelWPNote->setText("Source: Imported from user FMS flightplan");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_EARTHNAV)
    {
        ui->labelWPNote->setText("Source: earth_nav.dat (X-Plane)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_XNVU)
    {
        ui->labelWPNote->setText("Source: xnvu_wps.txt (XNVU local library)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_RSBN)
    {
        ui->labelWPNote->setText("Source: rsbn.dat (RSBN library)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_WPS)
    {
        ui->labelWPNote->setText("Source: Imported from XNVU flightplan");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_FLIGHTPLAN)
    {
        ui->labelWPNote->setText("Source: Current flightplan waypoint");
    }
}

void MainWindow::drawNVUHeader(QPainter& painter, NVUPOINT* dep, NVUPOINT* arr, double fork, int& y)
{
    int fSize = 150;
    int fHOffset = 250;
    //int fWOffset = 100;
    double rectW = 330;
    int rectH = 400;
    int xBegin = 0;
    //int yBegin = y;
    int x = xBegin;
    int dx, dy;
    double xscale = 1;
    QString qstr;
    QPen gridPen(Qt::black, 0, Qt::SolidLine);
    QFont font = QFont("FreeSans");
    font.setBold(true);
    font.setPixelSize(fSize);
    QFontMetrics fM(font);
    painter.setFont(font);
    painter.setPen(gridPen);


    //Draw route name
    QString routeName = dep->name + " - " + arr->name;
    if(dep->name2.length()>0 && arr->name2.length()>0)
    {
        routeName = routeName + "   ( " + dep->name2 + " - " + arr->name2 + " )";
    }
    painter.drawText(x*xscale, y, routeName);

    font.setBold(false);
    painter.setFont(font);

    //Draw distance and fork
    xscale = 1;
    y+=260;
    painter.drawText(10*xscale, y, "Distance, km.");
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(2000*xscale, y, QString::number(dep->Srem, 'f', 1));
    font.setBold(false);
    painter.setFont(font);
    painter.drawText(4000*xscale, y, "Fork, deg.");
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(6000*xscale, y, QString::number(fork, 'f', 1));
    y+=40;
    painter.drawRect(2000*xscale, y, 1000, 5);
    painter.drawRect(6000, y, 1000, 5);

    /*
    //Draw version
    painter.drawText(11500*xscale, y, XNVU_VERSION);
    //Draw time and date
    painter.drawText(10900*xscale, y, QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd    hh:mm:ss") + "   UTC");
    */


    y+=90;
    painter.setBrush(QColor(207, 207, 207));
    //Draw waypoint N
    xscale = 1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "N";
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, qstr);
    x+=rectW*xscale;

    //Draw waypoint ID column
    x+=30;
    xscale = 4.6;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "Identifier";
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, qstr);
    x+=rectW*xscale;

    //Draw waypoint type column
    xscale = 2.6;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "Type";
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, qstr);
    x+=rectW*xscale;

    //Draw coordinate column
    xscale = 3.5;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "Latitude";
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x + 50, y + dy + 10, qstr);
    qstr = "Longitude";
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw magentic declination column
    xscale = 1.4;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "MD";
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, qstr);
    x+=rectW*xscale;

    //Draw OZMPUv and OZMPUp
    x+=30;
    xscale = 2.7;
    painter.drawRect(x, y, rectW*xscale, rectH);
    //painter.drawLine(x + rectW*xscale, y, x, y + rectH);
    qstr = "OZMPUv";
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x+50, y + dy + 10, qstr);
    qstr = "OZMPUp";
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw leg distance
    xscale = 2.1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "S";
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, qstr);
    x+=rectW*xscale;

    //Draw Pv/Pp
    xscale = 1.9;
    painter.drawRect(x, y, rectW*xscale, rectH);
    painter.drawLine(x + rectW*xscale, y, x, y + rectH);
    qstr = "Pv";
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x+50, y + dy + 10, qstr);
    qstr = "Pp";
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw MPU and IPU
    painter.setPen(gridPen);
    xscale = 2.1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "MPU";
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x + 50, y + dy + 10, qstr);
    qstr = "IPU";
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw RSBN column
    x+=30;
    xscale = 7;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "VORDME / RSBN";
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, qstr);
    x+=rectW*xscale;


    //Draw Sm and Zm
    xscale = 2.1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "Sm";
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x + 50, y + dy + 10, qstr);
    qstr = "Zm";
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw Map angle
    xscale = 1.7;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "Map";
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x + dx, y + dy + 10, qstr);
    qstr = "Angle";
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw Atrg and Dtrg
    xscale = 2.4;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "A targ";
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x + 50, y + dy + 10, qstr);
    qstr = "D targ";
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw Spas and Srem
    xscale = 2.4;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "Spas";
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x + 50, y + dy + 10, qstr);
    qstr = "Srem";
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw waypoint N right side
    x+=30;
    xscale = 1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = "N";
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, qstr);
    x+= rectW*xscale;


    //Draw version
    qstr = XNVU_VERSION;
    dx = fM.boundingRect(XNVU_VERSION).width();

    painter.drawText(x-dx, 0, XNVU_VERSION);
    //Draw time and date
    qstr = QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd    hh:mm:ss") + "   UTC";
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x - dx, 260, qstr);


    y+=rectH;
}

void MainWindow::painterDrawNVUPoint(QPainter& painter, NVUPOINT *wp, int wpNumber, bool isArr, int &y)
{
    int fSize = 150;
    int fHOffset = 250;
    int fWOffset = 100;
    double rectW = 330;
    int rectH = 400;
    int xBegin = 0;
    int yBegin = y;
    int x = xBegin;
    int dx, dy;
    double xscale = 1;
    QString qstr;
    QColor red(143, 30, 30, 255);
    QColor blue(6, 6, 131, 255);


    QPen gridPen(Qt::black, 0, Qt::SolidLine);
    painter.setPen(gridPen);

    //QFont font = QFont("Liberation Serif");
    QFont font = QFont("FreeSans");
    font.setPixelSize(fSize);
    QFontMetrics fM(font);
    painter.setFont(font);
    painter.setBrush(QColor(255, 255, 255));



    //Draw waypoint N
    xscale = 1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = QString::number(wpNumber);
    dx = fM.boundingRect(qstr).width();
    //int dy = fM.boundingRect(qstr).height();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, qstr);
    x = xBegin + rectW*xscale;


    //Draw waypoint ID column
    x+=30;
    xscale = 4.6;
    /*font.setBold(true);
    fM = QFontMetrics(font);
    painter.setFont(font);
    painter.drawRect(x, y, rectW*xscale, rectH);
    dx = fM.boundingRect(wp->name).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, wp->name);
    x+=rectW*xscale;*/
    xscale = 4.6;
    font.setBold(true);
    fM = QFontMetrics(font);
    painter.setFont(font);
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = wp->name;
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    dy = fM.boundingRect(qstr).height() + 10;
    painter.drawText(x + dx, y + dy, qstr);
    font.setPixelSize(fSize*0.75);
    font.setBold(false);
    fM = QFontMetrics(font);
    painter.setFont(font);
    qstr = (ui->actionShow_feet->isChecked() ? QString::number(wp->alt, 'f', 0) + " ft" : QString::number(LMATH::feetToMeter(wp->alt), 'f', 0) + " m");
    //dx = fM.boundingRect(qstr).width();
    //dx = (rectW*xscale/2 - dx/2);
    painter.drawText(x + 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw waypoint type column
    xscale = 2.6;
    font.setBold(false);
    fM = QFontMetrics(font);
    painter.setFont(font);
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = WAYPOINT::getTypeStr(wp);
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    if(WAYPOINT::isNavaid(wp->type)) dy = fM.boundingRect(qstr).height() + 10;
    else dy = fHOffset;
    painter.drawText(x + dx, y + dy, qstr);
    font.setPixelSize(fSize*0.75);
    fM = QFontMetrics(font);
    painter.setFont(font);

    if(wp->type == WAYPOINT::TYPE_NDB ||
       wp->type == WAYPOINT::TYPE_RSBN)
    {
        qstr = "Ch" + QString::number((int) wp->freq);
        dx = fM.boundingRect(qstr).width();
        dx = (rectW*xscale)/2 - dx/2;
        painter.drawText(x + dx, y + rectH - 50, qstr);
    }
    else if(wp->type == WAYPOINT::TYPE_VOR ||
            wp->type == WAYPOINT::TYPE_DME ||
            wp->type == WAYPOINT::TYPE_VORDME)
    {
        qstr = QString::number(wp->freq, 'f', 3);
        dx = fM.boundingRect(qstr).width();
        dx = (rectW*xscale)/2 - dx/2;
        painter.drawText(x + dx, y + rectH - 50, qstr);
    }
    x+=rectW*xscale;

    //Draw coordinate column
    font.setPixelSize(fSize);
    fM = QFontMetrics(font);
    painter.setFont(font);
    xscale = 3.5;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = WAYPOINT::latToStr(wp->latlon.x);
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x + 50, y + dy + 10, qstr);
    qstr = WAYPOINT::lonToStr(wp->latlon.y);
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw magentic declination column
    xscale = 1.4;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = QString::number(wp->MD, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, qstr);
    x+=rectW*xscale;

    //Draw OZMPUv and OZMPUp
    font.setBold(true);
    font.setPixelSize(fSize*1.05);
    fM = QFontMetrics(font);
    painter.setFont(font);
    x+=30;
    xscale = 2.7;
    painter.drawRect(x, y, rectW*xscale, rectH);
    painter.drawLine(x + rectW*xscale, y, x, y + rectH);
    qstr = QString::number(wp->OZMPUv, 'f', 1);
    dy = fM.boundingRect(qstr).height();
    painter.setPen(red);
    if(!isArr) painter.drawText(x+50, y + dy + 10, qstr);
    qstr = QString::number(wp->OZMPUp, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    painter.setPen(blue);
    if(!isArr) painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw leg distance
    font.setPixelSize(fSize);
    fM = QFontMetrics(font);
    painter.setFont(font);
    //painter.setPen(QColor(0, 0, 0, 255));
    painter.setPen(gridPen);
    xscale = 2.1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = QString::number(wp->S, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    if(!isArr) painter.drawText(x + dx, y + fHOffset, qstr);
    x+=rectW*xscale;

    //Draw Pv/Pp
    font.setBold(false);
    font.setPixelSize(fSize);
    fM = QFontMetrics(font);
    painter.setFont(font);
    xscale = 1.9;
    painter.drawRect(x, y, rectW*xscale, rectH);
    painter.drawLine(x + rectW*xscale, y, x, y + rectH);
    qstr = QString::number(wp->Pv, 'f', 1);
    dy = fM.boundingRect(qstr).height();
    painter.setPen(red);
    if(!isArr && wpNumber>1) painter.drawText(x+50, y + dy + 10, qstr);
    qstr = QString::number(wp->Pp, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    painter.setPen(blue);
    if(!isArr) painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw MPU and IPU
    painter.setPen(gridPen);
    xscale = 2.1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = QString::number(wp->MPU, 'f', 1);
    dy = fM.boundingRect(qstr).height();
    if(!isArr) painter.drawText(x + 50, y + dy + 10, qstr);
    qstr = QString::number(wp->IPU, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    if(!isArr) painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw RSBN column
    qstr = "";
    x+=30;
    xscale = 7;
    painter.drawRect(x, y, rectW*xscale, rectH);
    font.setPixelSize(fSize*0.75);
    fM = QFontMetrics(font);
    painter.setFont(font);
    if(wp->rsbn) qstr = WAYPOINT::getTypeStr(wp->rsbn);
    dy = fM.boundingRect(qstr).height();
    dx = fM.boundingRect("VOR/DME").width();
    painter.drawText(x + 50, y + dy + 10, qstr);
    font.setPixelSize(fSize);
    font.setBold(true);
    fM = QFontMetrics(font);
    painter.setFont(font);
    if(wp->rsbn) qstr = (wp->rsbn->country.isEmpty() ? "" : wp->rsbn->country) +  wp->rsbn->name;
    painter.drawText(x + 50 + dx + 100, y + dy + 10, qstr);
    font.setBold(false);
    font.setPixelSize(fSize*0.75);
    fM = QFontMetrics(font);
    painter.setFont(font);
    if(wp->rsbn)
    {
        if(wp->rsbn->type==WAYPOINT::TYPE_RSBN) qstr = "Ch" + QString::number((int)wp->rsbn->freq) + (wp->rsbn->name2.isEmpty() ? "" : "   " + wp->rsbn->name2);
        else qstr = QString::number(wp->rsbn->freq, 'f', 3) + (wp->rsbn->name2.isEmpty() ? "" : "   " + wp->rsbn->name2);
    }
    painter.drawText(x + 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw Sm and Zm
    qstr = "";
    font.setPixelSize(fSize);
    fM = QFontMetrics(font);
    painter.setFont(font);
    xscale = 2.1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = QString::number(wp->Sm, 'f', 1);
    dy = fM.boundingRect(qstr).height();
    if(!isArr && wp->rsbn) painter.drawText(x + 50, y + dy + 10, qstr);
    qstr = QString::number(wp->Zm, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    if(!isArr && wp->rsbn) painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw Map angle
    qstr = "";
    xscale = 1.7;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = QString::number(wp->MapAngle, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    if(!isArr && wp->rsbn) painter.drawText(x + dx, y + fHOffset, qstr);
    x+=rectW*xscale;

    //Draw Atrg and Dtrg
    qstr = "";
    xscale = 2.4;
    painter.drawRect(x, y, rectW*xscale, rectH);
    if(!isArr && wp->rsbn) qstr = QString::number(wp->Atrg, 'f', 1);
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x + 50, y + dy + 10, qstr);
    if(!isArr && wp->rsbn) qstr = QString::number(wp->Dtrg, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw Spas and Srem
    qstr = "";
    xscale = 2.4;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = QString::number(wp->Spas, 'f', 1);
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x + 50, y + dy + 10, qstr);
    qstr = QString::number(wp->Srem, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw waypoint N right side
    qstr = "";
    x+=30;
    xscale = 1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = QString::number(wpNumber);
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, qstr);
    x+= rectW*xscale;

    y+=rectH;
}

void MainWindow::on_pushButtonPrint_clicked()
{
    QPrinter printer(QPrinter::HighResolution); //create your QPrinter (don't need to be high resolution, anyway)
    QPrintPreviewDialog dialog(&printer);
    dialog.setWindowFlags(Qt::Window);
    dialog.setWindowTitle("Print NVU-plan");
    connect(&dialog, SIGNAL(paintRequested(QPrinter *)), SLOT(printPreview(QPrinter *)));
    dialog.exec();
}

void MainWindow::on_tableWidget_itemSelectionChanged()
{
    //int c = ui->tableWidget->currentRow();
    //setWaypointDescription(ui->tableWidget->getWaypoint(c));
}

void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    NVUPOINT* wp = ui->tableWidget->getWaypoint(row);
    DialogWaypointEdit dEdit(wp, true);
    int dr = dEdit.exec();
    if(dr == QDialog::Rejected || dr == DialogWaypointEdit::CANCEL) return;

    if(dr==DialogWaypointEdit::ADD_XNVU) //Create new
    {
        XFMS_DATA::addXNVUWaypoint(new NVUPOINT(dEdit.nvupoint));
        WAYPOINT* rsbn = wp->rsbn;
        wp->clone(dEdit.nvupoint);
        wp->rsbn = rsbn;
        ui->tableWidget->refreshFlightplan();
    }
    else if(dr==DialogWaypointEdit::SAVE)
    {
        WAYPOINT* rsbn = wp->rsbn;
        wp->clone(dEdit.nvupoint);
        wp->rsbn = rsbn;
        ui->tableWidget->refreshFlightplan();
    }
}

void MainWindow::on_pushButton_ClearFlightplan_clicked()
{
    /*
    QString styleSheet = "background-color: rgb(73, 163, 0);"
                          "color: rgb(0, 0, 0);";
    */

    if(clearFlightplan_countdown < 0)
    {
        QString styleSheet = "background-color: rgb(255, 0, 0);"
                             "color: rgb(255, 255, 255);";

        ui->pushButton_ClearFlightplan->setText("CONFIRM: 10");
        ui->pushButton_ClearFlightplan->setStyleSheet(styleSheet);

        clearFlightplan_countdown = 10;
        clearFlightplan_timer.start(1000);
        return;
    }

    clearFlightplan_countdown = -1;
    QString styleSheet = "background-color: rgb(73, 163, 0);"
                          "color: rgb(0, 0, 0);";
    ui->pushButton_ClearFlightplan->setText("Clear flightplan");
    ui->pushButton_ClearFlightplan->setStyleSheet(styleSheet);

    ui->tableWidget->clearFlightplan();
}

void MainWindow::clearFlightplanTimeout()
{
    if(clearFlightplan_countdown <= 0)
    {
        clearFlightplan_timer.stop();

        QString styleSheet = "background-color: rgb(73, 163, 0);"
                              "color: rgb(0, 0, 0);";
        ui->pushButton_ClearFlightplan->setText("Clear flightplan");
        ui->pushButton_ClearFlightplan->setStyleSheet(styleSheet);
        clearFlightplan_countdown = -1;
        return;
    }

    clearFlightplan_countdown--;
    ui->pushButton_ClearFlightplan->setText("CONFIRM: " + QString::number(clearFlightplan_countdown));
    clearFlightplan_timer.start(1000);
}



void MainWindow::on_actionOptions_triggered()
{
    DialogOptions().exec();

    ui->label_alignATS->setVisible(DialogSettings::distAlignATS);
    ui->label_alignEarthNav->setVisible(DialogSettings::distAlignEarthNav);
    ui->label_alignWPS->setVisible(DialogSettings::distAlignWPS);
    ui->label_alignFMS->setVisible(DialogSettings::distAlignFMS);

}

void MainWindow::on_frameDescription_clicked()
{
    QListWidgetItemData* iD = (QListWidgetItemData*) ui->listWidget->currentItem();
    if(iD)
    {
        if(iD->nvupoint->type == WAYPOINT::TYPE_AIRWAY)
        {
            AIRWAY* ats = (AIRWAY*) iD->nvupoint->data;
            ui->listWidget->clear();
            for(int i=0; i<ats->lATS.size(); i++)
            {
                NVUPOINT* wp = (NVUPOINT*) ats->lATS[i];
                QListWidgetItemData *newItem = new QListWidgetItemData;
                QString qstr = wp->name;
                if(!wp->name2.isEmpty()) qstr = qstr + " - " + wp->name2;
                if(!wp->country.isEmpty()) qstr = qstr + " [" + wp->country + "]";
                newItem->setText(qstr);
                newItem->nvupoint = wp;
                ui->listWidget->addItem(newItem, false);
            }//for
        }//if
    }//if
}

void MainWindow::on_actionXNVU_library_triggered()
{
    DialogWPSEdit dEdit;
    int rv = dEdit.exec();

    if(rv == QDialog::Accepted)
    {
        if(dEdit.lRemove.size())
        {
           XFMS_DATA::removeWPSPoints(dEdit.lRemove);
        }
    }
}

void MainWindow::on_pushButton_showAIRAC_Airports_clicked()
{
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_AIRAC_AIRPORTS, ui->pushButton_showAIRAC_Airports->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showAIRAC_Navaids_clicked()
{
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_AIRAC_NAVAIDS, ui->pushButton_showAIRAC_Navaids->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showAIRAC_Fixes_clicked()
{
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_AIRAC_WAYPOINTS, ui->pushButton_showAIRAC_Fixes->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showAIRAC_Airways_clicked()
{
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_AIRAC_ATS, ui->pushButton_showAIRAC_Airways->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showEarthNav_clicked()
{
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_EARTHNAV, ui->pushButton_showEarthNav->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showRSBN_clicked()
{
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_RSBN, ui->pushButton_showRSBN->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showXNVU_clicked()
{
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_XNVU, ui->pushButton_showXNVU->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showNDB_clicked()
{
    ui->listWidget->showType(WAYPOINT::TYPE_NDB, ui->pushButton_showNDB->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showVOR_clicked()
{
    ui->listWidget->showType(WAYPOINT::TYPE_VOR, ui->pushButton_showVOR->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showVORDME_clicked()
{
    ui->listWidget->showType(WAYPOINT::TYPE_VORDME, ui->pushButton_showVORDME->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showDME_clicked()
{
    ui->listWidget->showType(WAYPOINT::TYPE_DME, ui->pushButton_showDME->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showFix_clicked()
{
    ui->listWidget->showType(WAYPOINT::TYPE_FIX, ui->pushButton_showFix->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showAirports_clicked()
{
    ui->listWidget->showType(WAYPOINT::TYPE_AIRPORT, ui->pushButton_showAirports->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButtonSetFL_clicked()
{
    std::vector<NVUPOINT*> lP = ui->tableWidget->getWaypoints();
    for(int i=0; i<lP.size(); i++)
    {
        NVUPOINT* p = lP[i];
        if(i==0 || i == (lP.size()-1)) p->alt = p->elev;
        else p->alt = (ui->actionShow_feet->isChecked() ? ui->spinBoxFL->value() : LMATH::meterToFeet(ui->spinBoxFL->value()));
    }

    ui->tableWidget->refreshFlightplan();
}

void MainWindow::on_actionShow_feet_triggered()
{
    ui->tableWidget->showFeet = ui->actionShow_feet->isChecked();
    if(ui->actionShow_feet->isChecked())
    {
        ui->spinBoxFL->setValue(LMATH::meterToFeet(ui->spinBoxFL->value()));
        ui->spinBoxFL->setSuffix(" ft");
    }
    else
    {
        ui->spinBoxFL->setValue(LMATH::feetToMeter(ui->spinBoxFL->value()));
        ui->spinBoxFL->setSuffix(" m");
    }//else

    ui->tableWidget->refreshFlightplan();
}

void MainWindow::on_pushButtonDeleteWaypoint_clicked()
{
    if(ui->tableWidget->currentRow()>=0) ui->tableWidget->deleteWaypoint(ui->tableWidget->currentRow());
}

void MainWindow::on_actionColumns_2_triggered()
{
    DialogColumns dCol(ui->tableWidget);
    int rv = dCol.exec();

    if(rv!=QDialog::Accepted) return;

    //ui->tableWidget->updateShownColumns();
}
