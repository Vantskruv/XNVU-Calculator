#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "LMATH.h"
#include "qlistwidgetitemdata.h"
#include "qtablewidgetitemdata.h"
#include <vector>
#include <sstream>
#include <QShortcut>
#include <coremag.h>
#include "dialogwaypointedit.h"
#include "dialogrsbn.h"
#include "dialogoptions.h"
#include "dialogwpsedit.h"
#include "dialogcolumns.h"
#include <ctime>
#include <QtConcurrent>
#include "customloadingdialog.h"
#include <QFileDialog>
#include <QPrintPreviewDialog>
#include <qlabelclick.h>

#include <nvupoint.h>
#include <airway.h>


int dat; //TODO
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
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    //Setup shortcuts
    QShortcut* shortcutDeleteKey = new QShortcut(QKeySequence(Qt::Key_Delete), ui->tableWidget);
    QShortcut* shortcutUpKey = new QShortcut(QKeySequence(Qt::Key_Up), ui->tableWidget);
    QShortcut* shortcutDownKey = new QShortcut(QKeySequence(Qt::Key_Down), ui->tableWidget);

    //Setup signals
    connect(shortcutDeleteKey, SIGNAL(activated()), this, SLOT(deleteCurrentWaypoint()));
    connect(shortcutUpKey, SIGNAL(activated()), this, SLOT(tableGoUp()));
    connect(shortcutDownKey, SIGNAL(activated()), this, SLOT(tableGoDown()));
    //connect(actionDirectTo, SIGNAL(triggered()), this, SLOT(goDirectTo()));
    //connect(ui->actionExport_X_Plane_FMS, SIGNAL(triggered()), this, SLOT(exportFMS()));
    //connect(ui->actionImport_X_Plane_FMS, SIGNAL(triggered()), this, SLOT(importFMS()));
    connect(ui->actionSave_XNVU_flightplan, SIGNAL(triggered()), this, SLOT(saveNVUFlightPlan()));
    connect(ui->actionLoad_XNVU_flightplan, SIGNAL(triggered()), this, SLOT(loadNVUFlightplan()));
    connect(ui->actionX_Plane_folder, SIGNAL(triggered()), this, SLOT(showXPlaneSettings()));
    //connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(on_pushButtonPrint_clicked()));
    //connect(ui->actionExport_to_PDF, SIGNAL(triggered()), this, SLOT(printOnPDF()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(quit()));
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showFlightplanContextMenu(const QPoint&)));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showSearchListContextMenu(const QPoint&)));
    connect(ui->frameDescription, SIGNAL(clicked(QPoint)), this, SLOT(on_frameDescription_clicked()), Qt::UniqueConnection);
    connect(&clearFlightplan_timer, SIGNAL(timeout()), this, SLOT(clearFlightplanTimeout()));
    connect(ui->lineEdit_DTCourseFrom, SIGNAL(clicked(QLineEditWP*)), this, SLOT(goDirectToFieldClicked(QLineEditWP*)));
    connect(ui->lineEdit_DTCourseTo, SIGNAL(clicked(QLineEditWP*)), this, SLOT(goDirectToFieldClicked(QLineEditWP*)));
    connect(ui->lineEdit_DTTo, SIGNAL(clicked(QLineEditWP*)), this, SLOT(goDirectToFieldClicked(QLineEditWP*)));

    connect(ui->actionPrint_2, SIGNAL(triggered()), this, SLOT(on_pushButtonPrint_clicked()));
    connect(ui->actionPrint_to_PDF, SIGNAL(triggered()), this, SLOT(printOnPDF()));
    connect(ui->actionExport_to_FMS, SIGNAL(triggered()), this, SLOT(exportFMS()));
    connect(ui->actionExport_to_KLN90B, SIGNAL(triggered()), this, SLOT(exportFMS_KLN90B()));
    connect(ui->actionImport_X_Plane_FMS_2, SIGNAL(triggered()), this, SLOT(importFMS()));
    connect(ui->actionKLN_90B, SIGNAL(triggered()), this, SLOT(importFMS_KLN90B()));
    connect(ui->actionAutogenerate_correction_beacons, SIGNAL(triggered()), this, SLOT(autoGenerateCorrectionBeacons()));

    //Connect clickable labels for TOD calculations
    //connect(ui->labelCruise, SIGNAL(clicked(QLabelClick*)), this, SLOT(clickedDataLabels(QLabelClick*)));
    connect(ui->labelVS, SIGNAL(clicked(QLabelClick*)), this, SLOT(clickedDataLabels(QLabelClick*)));
    connect(ui->labelTWC, SIGNAL(clicked(QLabelClick*)), this, SLOT(clickedDataLabels(QLabelClick*)));
    connect(ui->labelFlightLevel, SIGNAL(clicked(QLabelClick*)), this, SLOT(clickedDataLabels(QLabelClick*)));
    connect(ui->labelTOD, SIGNAL(clicked(QLabelClick*)), this, SLOT(clickedDataLabels(QLabelClick*)));

    //Give tablewidget a reference to labelFork and labelTOD and set its size
    ui->tableWidget->setColumnCount(QFlightplanTable::COL::_SIZE);
    ui->tableWidget->qFork = ui->labelFork;
    ui->tableWidget->qTOD = ui->labelTOD;
    ui->tableWidget->horizontalHeader()->show();

    //Set color for status bar
    QPalette pal = ui->statusBar->palette();
    pal.setColor(ui->statusBar->foregroundRole(), QColor(255, 255, 0));
    ui->statusBar->setPalette(pal);

    //Set lineEdits to always show as capitalized (NOTE: this does not mean that the font IS capitalized)
    QFont font = ui->lineEdit->font();
    font.setCapitalization(QFont::AllUppercase);
    ui->lineEdit->setFont(font);
    ui->lineEditRoute->setFont(font);


    //Restore menu checkboxes (note the states of these is on application termination saved to DialogSettings)
    ui->actionNightmode->setChecked(DialogSettings::nightMode);
    //ui->actionDistance_in_NM->setChecked(DialogSettings::showTOD_METRIC_PRINT);
    //ui->actionAltitude_in_feet->setChecked(DialogSettings::showFeet_PRINT);

    //Set current or custom date
    /*
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    dat = yymmdd_to_julian_days(now->tm_year, now->tm_mon+1, now->tm_mday);
    */
    dat = QDate::currentDate().toJulianDay();
    if(DialogSettings::customDateIsTrue)
    {
        QDate date = QDate::fromString(DialogSettings::customDate, "yyyy.MM.dd");
        dat = date.toJulianDay();
        ui->dateEdit->setDate(date);
    }
    else ui->dateEdit->setDate(QDate::currentDate());
    ui->tableWidget->dat = dat;


    //Show saved align settings TODO: We should scrap this feature later on.
    ui->label_alignWPS->setText("XWP align");
    ui->label_alignFMS->setText("FMS align");
    ui->label_alignWPS->setVisible(DialogSettings::distAlignXWP);
    ui->label_alignFMS->setVisible(DialogSettings::distAlignFMS);

    //Set flightplan data values to default (in formats saved by user)
    //if(DialogSettings::cruiseFormat == 1){ ui->doubleSpinBox_MACH->setSuffix(" km/h"); ui->doubleSpinBox_MACH->setValue(476);}
    //else if(DialogSettings::cruiseFormat == 2) {ui->doubleSpinBox_MACH->setSuffix(" kn"); ui->doubleSpinBox_MACH->setValue(257);}
    //else {
        ui->doubleSpinBox_MACH->setSuffix(" M"); ui->doubleSpinBox_MACH->setValue(0.8);//}
    if(DialogSettings::VSFormat == 1){ ui->doubleSpinBox_VS->setSuffix(" ft/m"); ui->doubleSpinBox_VS->setValue(1575);}
    else{ui->doubleSpinBox_VS->setSuffix(" m/s"); ui->doubleSpinBox_VS->setValue(8.0);}
    if(DialogSettings::TWCFormat == 1) ui->doubleSpinBox_TWC->setSuffix(" kn");
    else ui->doubleSpinBox_TWC->setSuffix(" km/h");
    if(DialogSettings::showFeet){ui->doubleSpinBoxFL->setSuffix(" ft"); ui->doubleSpinBoxFL->setValue(35000);}
    else{ui->doubleSpinBoxFL->setSuffix(" m"); ui->doubleSpinBoxFL->setValue(10668);}
    on_pushButtonSetDate_clicked(); //Set tableWidget->fplData values


    //Restore saved windows state
    this->resize(DialogSettings::windowSize);
    this->move(DialogSettings::windowPos);
    ui->tableWidget->horizontalHeader()->restoreState(DialogSettings::tableState);

    //Show warnings if errors occured when loading data
    labelWarning = new QLabel("");
    labelWarning->setStyleSheet("background-color: rgb(0, 30, 0);"
                                "color: rgb(255, 255, 0);");
    ui->statusBar->addPermanentWidget(labelWarning);
    if(XFMS_DATA::__ERROR_LOADING.isEmpty()) labelWarning->setText("");
    else labelWarning->setText("WARNING: " + XFMS_DATA::__ERROR_LOADING + " is not loaded!");
}

MainWindow::~MainWindow()
{
    DialogSettings::windowSize = size();
    DialogSettings::windowPos = pos();
    DialogSettings::tableState = ui->tableWidget->horizontalHeader()->saveState();

    DialogSettings::nightMode = ui->actionNightmode->isChecked();
    //DialogSettings::showTOD_METRIC_PRINT = ui->actionDistance_in_NM->isChecked();
    //DialogSettings::showFeet_PRINT = ui->actionAltitude_in_feet->isChecked();

    DialogSettings::saveSettings();

    XFMS_DATA::saveXNVUData();
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

    QString styleSheet = "QMenu"
                         "{"
                         "  background-color: rgb(0, 30, 0);"
                         "  color: rgb(107, 239, 0);"
                         "}"
                         "QMenu::item:selected"
                         "{"
                         "  background-color: rgb(107, 239, 0);"
                         "  color: rgb(0, 0, 0);"
                         "}";

    fpMenu.setStyleSheet(styleSheet);

    int row = ui->tableWidget->rowAt(pos.y());
    if(row>=0)
    {
        /*
        fpMenu.addAction("Move up");
        fpMenu.addAction("Move down");
        fpMenu.addAction("Set direct to");
        fpMenu.addAction("Edit waypoint...");
        fpMenu.addAction("Set correction beacon...");
        fpMenu.addSeparator();
        fpMenu.addAction("Delete");
        */

        fpMenu.addAction("Set correction beacon...");
        fpMenu.addAction("Set direct to");
        fpMenu.addAction("Edit waypoint...");
        fpMenu.addSeparator();
        fpMenu.addAction("Delete");
        fpMenu.addSeparator();
        fpMenu.addAction("Move up");
        fpMenu.addAction("Move down");

        selectedItem = fpMenu.exec(globalPos);
        if(selectedItem)
        {
            NVUPOINT* wp = ui->tableWidget->getWaypoint(row);
            if(selectedItem == fpMenu.actions()[6]) //Move up
            {
                if(row>0)
                {
                    ui->tableWidget->moveWaypoint(row, true);
                }
            }
            else if(selectedItem == fpMenu.actions()[7]) //Move down
            {
                if(row<(ui->tableWidget->rowCount()-1))
                {
                    ui->tableWidget->moveWaypoint(row, false);
                }
            }
            else if(selectedItem == fpMenu.actions()[1]) //Direct to
            {
                ui->lineEdit_DTTo->setWaypoint(wp);
            }
            else if(selectedItem == fpMenu.actions()[2]) //Edit waypoint
            {
                DialogWaypointEdit dEdit(wp, true);
                int dr = dEdit.exec();
                if(dr == QDialog::Rejected || dr == DialogWaypointEdit::CANCEL) return;

                if(dr==DialogWaypointEdit::ADD_XNVU) //Create new
                {

                    NVUPOINT* nvup = new NVUPOINT(dEdit.newPoint);
                    nvup->wpOrigin = WAYPOINT::ORIGIN_FLIGHTPLAN;
                    ui->tableWidget->replaceWaypoint(nvup, row);
                    nvup = new NVUPOINT(dEdit.newPoint);
                    nvup->setRSBN(NULL);
                    nvup->wpOrigin = WAYPOINT::ORIGIN_XNVU;
                    XFMS_DATA::addXNVUWaypoint(nvup);
                    ui->tableWidget->refreshFlightplan();
                }
                else if(dr==DialogWaypointEdit::SAVE)
                {
                    NVUPOINT* nvup = new NVUPOINT(dEdit.newPoint);
                    nvup->wpOrigin = WAYPOINT::ORIGIN_FLIGHTPLAN;
                    ui->tableWidget->replaceWaypoint(nvup, row);
                    ui->tableWidget->refreshFlightplan();
                }
            }
            else if(selectedItem == fpMenu.actions()[0])    //Set correction beacon
            {
                NVUPOINT* wp2 = ui->tableWidget->getWaypoint(row+1);
                DialogRSBN dRSBN(wp, wp2);
                int dr = dRSBN.exec();
                if(dr == QDialog::Rejected) return;

                wp->setRSBN(dRSBN.rsbn);
                ui->tableWidget->refreshFlightplan();
            }
            else if(selectedItem == fpMenu.actions()[4]) //Delete waypoint
            {
                QMessageBox msgBox;
                QString sStyleSheet = "QMessageBox, QLabel{"
                                      " background-color: rgb(0, 30, 0);"
                                      " color: rgb(107, 239, 0);"
                                      "}"
                                      "QPushButton{"
                                      " background-color: rgb(73, 163, 0);"
                                      " color: rgb(0, 0, 0);"
                                      "}";
                msgBox.setStyleSheet(sStyleSheet);
                msgBox.setText("Delete waypoint");
                msgBox.setInformativeText("Are you sure you want to delete the waypoint?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                int ret = msgBox.exec();
                if(ret == QMessageBox::Yes) ui->tableWidget->deleteWaypoint(row);
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
                NVUPOINT* np = new NVUPOINT(dEdit.newPoint);
                np->wpOrigin = WAYPOINT::ORIGIN_FLIGHTPLAN;
                ui->tableWidget->insertWaypoint(np, ui->tableWidget->rowCount());

                np = new NVUPOINT(dEdit.newPoint);
                np->wpOrigin = WAYPOINT::ORIGIN_XNVU;
                XFMS_DATA::addXNVUWaypoint(np);
            }
            else if(dr==DialogWaypointEdit::SAVE)
            {
                NVUPOINT* np = new NVUPOINT(dEdit.newPoint);
                np->wpOrigin = WAYPOINT::ORIGIN_FLIGHTPLAN;
                ui->tableWidget->insertWaypoint(np, ui->tableWidget->rowCount());
            }
        }

        return;
    }
}

void MainWindow::showSearchListContextMenu(const QPoint& pos) // this is a slot
{
    // for most widgets
    QPoint globalPos = ui->listWidget->mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);



    QMenu fpMenu;
    QAction* selectedItem;
    QString styleSheet = "QMenu"
                         "{"
                         "  background-color: rgb(0, 30, 0);"
                         "  color: rgb(107, 239, 0);"
                         "}"
                         "QMenu::item:selected"
                         "{"
                         "  background-color: rgb(107, 239, 0);"
                         "  color: rgb(0, 0, 0);"
                         "}";
    fpMenu.setStyleSheet(styleSheet);

    QListWidgetItemData* iItem = (QListWidgetItemData*) ui->listWidget->itemAt(pos);
    if(iItem)
    {
        fpMenu.addAction("Set direct to");

        selectedItem = fpMenu.exec(globalPos);
        if(selectedItem)
        {
            if(selectedItem == fpMenu.actions()[0])
            {
                ui->lineEdit_DTTo->setWaypoint(iItem->nvupoint);
            }//if
        }//if
    }//if
}

void MainWindow::showXPlaneSettings()
{
    DialogSettings dSettings;
    if(QDialog::Rejected == dSettings.exec()) return;

    XFMS_DATA::saveXNVUData();
    ui->lineEdit->clear();
    ui->listWidget->clear();
    XFMS_DATA::clear();

    CustomLoadingDialog dialogLoadData;
    dialogLoadData.exec();
    if(XFMS_DATA::__ERROR_LOADING.isEmpty()) labelWarning->setText("");
    else labelWarning->setText("WARNING: " + XFMS_DATA::__ERROR_LOADING + " is not loaded!");

    ui->label_alignWPS->setVisible(DialogSettings::distAlignXWP);
    ui->label_alignFMS->setVisible(DialogSettings::distAlignFMS);
}

void MainWindow::importFMS()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import FMS flightplan", DialogSettings::defaultLoadSaveDir, "FMS Files (*.fms);;All files (*.*)");


    if(fileName.isEmpty()) return;
    ui->tableWidget->clearFlightplan();
    std::vector<NVUPOINT*> lFMS = XFMS_DATA::loadFMS(fileName);
    ui->tableWidget->insertRoute(lFMS, 0);
}

void MainWindow::importFMS_KLN90B()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import KLN90B flightplan", DialogSettings::xDir + "//Output//FMS plans//KLN 90B//", "FMS Files (*.fms);;All files (*.*)");

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

    if(DialogSettings::defaultLoadSaveDir.length()>0) qstr = DialogSettings::defaultLoadSaveDir + "//" + qstr;

    QString fileName = QFileDialog::getSaveFileName(this, "Export FMS flightplan", qstr, "FMS Files (*.fms);;All files (*.*)");
    if(fileName.isEmpty()) return;
    XFMS_DATA::saveFMS(fileName, ui->tableWidget->getWaypoints());
}

void MainWindow::exportFMS_KLN90B()
{
    std::vector<NVUPOINT*> lP = ui->tableWidget->getWaypoints();
    if(lP.size()>30)
    {
        QMessageBox box;
        box.setText("Flightplan with more than 30 waypoints cannot be exported to KLN90B.");
        box.setIcon(QMessageBox::Information);
        box.exec();
        return;
    }


    QString fileName = QFileDialog::getSaveFileName(this, "Export flightplan to KLN90B", DialogSettings::xDir + "//Output//FMS plans//KLN 90B//.fms", "FMS Files (*.fms);;All files (*.*)");
    if(fileName.isEmpty()) return;
    XFMS_DATA::saveFMS(fileName, ui->tableWidget->getWaypoints());
}

void MainWindow::loadNVUFlightplan()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load XNVU flightplan", DialogSettings::defaultLoadSaveDir, "XWP Files (*.xwp);;All files (*.*)");
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

    if(DialogSettings::defaultLoadSaveDir.length()>0) qstr = DialogSettings::defaultLoadSaveDir + "//" + qstr;
    QString fileName = QFileDialog::getSaveFileName(this, "Save XNVU flightplan", qstr, "XWP Files (*.xwp);;All files (*.*)");
    if(fileName.isEmpty()) return;
    XFMS_DATA::saveXNVUFlightplan(fileName, ui->tableWidget->getWaypoints());
}

void MainWindow::autoGenerateCorrectionBeacons()
{
    ui->tableWidget->autoGenerateCorrectionBeacons();
    ui->tableWidget->refreshFlightplan();
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
    NVUPOINT* wp = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow());
    ui->lineEdit->setText(wp->name);
    setWaypointDescription(wp);
}

void MainWindow::tableGoDown()
{
    int c = ui->tableWidget->currentRow();
    c++;
    if(c>=ui->tableWidget->rowCount()) return;

    ui->tableWidget->selectRow(c);
    NVUPOINT* wp = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow());
    ui->lineEdit->setText(wp->name);
    setWaypointDescription(wp);
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
    QString sSearch = ui->lineEdit->text();
    ui->listWidget->search(sSearch, true);
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *_item)
{
    QListWidgetItemData* itemData = (QListWidgetItemData*) _item;

    NVUPOINT* p = ui->listWidget->getWaypoint(_item);
    if(!p) return;  //Should not happen

    if(itemData->nvupoint->type == WAYPOINT::TYPE_AIRWAY && itemData->nvupoint->data)
    {
        AIRWAY* awy = (AIRWAY*) itemData->nvupoint->data;
        for(int i=0; i<awy->lATS.size(); i++)
        {
            ui->tableWidget->insertWaypoint(new NVUPOINT(*awy->lATS[i]), ui->tableWidget->rowCount());
        }//for
    }
    else ui->tableWidget->insertWaypoint(new NVUPOINT(*itemData->nvupoint), ui->tableWidget->rowCount());
}


void MainWindow::printPreview(QPrinter* printer)
{
    printer->setPageSize(QPrinter::A4);
    printer->setOrientation(QPrinter::Landscape);
    //printer->setPageMargins (0,0,0,0,QPrinter::Millimeter);
    printer->setFullPage(true);

    std::vector<NVUPOINT*> lWPs = ui->tableWidget->getWaypoints();
    double fork;
    if(lWPs.size()<1) return;

    NVU::generate(lWPs, fork, dat);
    QPainter painter(printer); // create a painter which will paint 'on printer'.

    QRect prect = printer->pageRect();
    QRect defvp = painter.viewport();
    if(ui->actionNightmode->isChecked()) painter.fillRect(prect, Qt::black);
    prect.setY(1000);
    prect.setX(1000);
    painter.setViewport(prect);


//#ifdef _WIN32
//    painter.scale(0.5, 0.5);
//#endif

    //qDebug() << "Width: " << painter.device()->width() << "; Height: " << painter.device()->height();
    //Width is 14033 and height is 9917

    int y = 0;
    drawNVUHeader(painter, lWPs[0], lWPs[lWPs.size() - 1], fork, y);
    y+=30;
    int j = 0;
    for(int i=0; i<lWPs.size(); i++, j++)
    {
        if(j==19)
        {
            //painterDrawSummary(painter, lWPs, y);
            printer->newPage();
            painter.setViewport(defvp);
            prect = printer->pageRect();
            if(ui->actionNightmode->isChecked()) painter.fillRect(prect, Qt::black);
            prect.setY(1000);
            prect.setX(1000);
            painter.setViewport(prect);
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
    if(itemData->nvupoint->type == WAYPOINT::TYPE_AIRWAY)
    {
        if(!itemData->nvupoint->data) return;
        AIRWAY* awy = (AIRWAY*) itemData->nvupoint->data;
        for(int i=0; i<awy->lATS.size(); i++)
        {
            ui->tableWidget->insertWaypoint(new NVUPOINT(*awy->lATS[i]), row);
            row++;
        }//for
    }
    else ui->tableWidget->insertWaypoint(new NVUPOINT(*itemData->nvupoint), row);
}

void MainWindow::on_pushButtonReplace_clicked()
{

    if(ui->listWidget->currentRow()<0 || ui->tableWidget->currentRow()<0) return;
    QListWidgetItemData* itemData = (QListWidgetItemData*)ui->listWidget->currentItem();
    if(itemData->nvupoint->type == WAYPOINT::TYPE_AIRWAY)
    {
        if(!itemData->nvupoint->data) return;
        AIRWAY* awy = (AIRWAY*) itemData->nvupoint->data;
        ui->tableWidget->replaceWaypoint(new NVUPOINT(*awy->lATS[0]), ui->tableWidget->currentRow());
        int row = ui->tableWidget->currentRow();
        row++;
        for(int i=1; i<awy->lATS.size(); i++)
        {
            ui->tableWidget->insertWaypoint(new NVUPOINT(*awy->lATS[i]), row);
            row++;
        }//for
    }
    else ui->tableWidget->replaceWaypoint(new NVUPOINT(*itemData->nvupoint), ui->tableWidget->currentRow());
}


void MainWindow::on_pushButtonInsertAfter_clicked()
{
    if(ui->listWidget->currentRow()<0) return;
    QListWidgetItemData* itemData = (QListWidgetItemData*)ui->listWidget->currentItem();

    int row = ui->tableWidget->currentRow();
    if(row<0) row = ui->tableWidget->rowCount();
    else row++;

    if(itemData->nvupoint->type == WAYPOINT::TYPE_AIRWAY)
    {
        if(!itemData->nvupoint->data) return;
        AIRWAY* awy = (AIRWAY*) itemData->nvupoint->data;
        for(int i=0; i<awy->lATS.size(); i++)
        {
            ui->tableWidget->insertWaypoint(new NVUPOINT(*awy->lATS[i]), row);
            row++;
        }//for
    }
    else ui->tableWidget->insertWaypoint(new NVUPOINT(*itemData->nvupoint), row);
}

void MainWindow::on_pushButtonRouteInsertAfter_clicked()
{
    std::vector<NVUPOINT*> route;
    NVUPOINT* wpRef = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow(), true);
    QString sError = XFMS_DATA::getRoute(ui->lineEditRoute->text().toUpper(), route, wpRef);
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
    NVUPOINT* wpRef = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow(), true);
    QString sError = XFMS_DATA::getRoute(ui->lineEditRoute->text().toUpper(), route, wpRef);
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
    NVUPOINT* wpRef = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow(), true);
    QString sError = XFMS_DATA::getRoute(ui->lineEditRoute->text().toUpper(), route, wpRef);
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
    if(ui->tableWidget->currentRow()<0) return;
    NVUPOINT* wp = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow());
    ui->lineEdit->setText(wp->name);
    setWaypointDescription(wp);
    ui->tableWidget->setFocus();
}

void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    if(currentRow<0) return;
    QListWidgetItemData* itemD = (QListWidgetItemData*) ui->listWidget->item(currentRow);
    setWaypointDescription(itemD->nvupoint);
}

void MainWindow::setWaypointDescription(const NVUPOINT* wp)
{
    //ui->labelIWPName->setText("");
    ui->labelIWPName2->setText("");
    ui->labelWPType->setText("");
    ui->labelWPMagVar->setText("");
    ui->labelWPLatlon->setText("");
    ui->labelWPNote->setText("");
    ui->labelWPType2->setText("");

    if(wp == NULL) return;

    QString qstr;
    qstr = WAYPOINT::getTypeStr(wp);
    if(wp->type == WAYPOINT::TYPE_RUNWAY || wp->type == WAYPOINT::TYPE_HELIPAD) qstr = qstr + " " + wp->name;
    ui->labelWPType2->setText(qstr);

    if(wp->type == WAYPOINT::TYPE_RUNWAY || wp->type == WAYPOINT::TYPE_HELIPAD) qstr = wp->name2;
    else qstr = wp->name;
    if(!wp->country.isEmpty()) qstr = qstr + " [" + wp->country + "]";

    if(wp->type == WAYPOINT::TYPE_NDB ||
       wp->type == WAYPOINT::TYPE_RSBN)
    {
        qstr = qstr + "  Ch " + QString::number((int) wp->freq);
    }//if
    else if(wp->type == WAYPOINT::TYPE_VOR ||
            wp->type == WAYPOINT::TYPE_DME ||
            wp->type == WAYPOINT::TYPE_VORDME ||
            wp->type == WAYPOINT::TYPE_ILS ||
            wp->type == WAYPOINT::TYPE_LOC ||
            wp->type == WAYPOINT::TYPE_TACAN ||
            wp->type == WAYPOINT::TYPE_VORTAC)
    {
        qstr = qstr + "  " + QString::number(wp->freq, 'f', 3);
    }//if

    ui->labelWPType->setText(qstr);

    if(wp->type == WAYPOINT::TYPE_AIRWAY)
    {
        AIRWAY* ats = (AIRWAY*) wp->data;
        ui->labelIWPName2->setText("[" + ats->lATS[0]->name + "] ---> [" + ats->lATS[ats->lATS.size()-1]->name + "]");
    }
    else if(wp->type == WAYPOINT::TYPE_RUNWAY) ui->labelIWPName2->setText("Length: " + QString::number(wp->length, 'f', 1) + "m");
    else if(wp->type == WAYPOINT::TYPE_HELIPAD) ui->labelIWPName2->setText("Size: " + QString::number(wp->length, 'f', 1) + "x" + QString::number(wp->width, 'f', 1) + "m");
    else if(!wp->name2.isEmpty()) ui->labelIWPName2->setText(wp->name2);

    if(wp->type == WAYPOINT::TYPE_AIRWAY)
    {
        AIRWAY* ats = (AIRWAY*) wp->data;
        if(wp->data) ui->labelWPLatlon->setText("Fixes: " + QString::number(ats->lATS.size()) + "    Dist: " + QString::number(ats->distance, 'f', 1) + " KM");
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
           wp->type == WAYPOINT::TYPE_VORDME ||
           wp->type == WAYPOINT::TYPE_ILS ||
           wp->type == WAYPOINT::TYPE_LOC ||
           wp->type == WAYPOINT::TYPE_TACAN ||
           wp->type == WAYPOINT::TYPE_VORTAC
           ) qstr = qstr + "        Elev: " + (DialogSettings::showFeet ? QString::number(wp->elev, 'f', 0) + " ft": QString::number(LMATH::feetToMeter(wp->elev), 'f', 0) + " m");
        else if(wp->type == WAYPOINT::TYPE_RUNWAY  || wp->type == WAYPOINT::TYPE_HELIPAD) qstr = qstr  + "        Surface: " + WAYPOINT::getRunwaySurfaceStr(wp);

        ui->labelWPMagVar->setText(qstr);
    }

    ui->labelWPNote->setText("Source: " + WAYPOINT::getOriginStr(wp->wpOrigin));

}


void MainWindow::painterDrawSummary(QPainter& painter, std::vector<NVUPOINT*>& lWP, int y)
{
    int width = 14033 - 1000;
    int height = 9917 - 1000;
    int fSize = 150;                    //Font size
    double xScale = 1;
    int xBegin = 0;
    int x = xBegin;
    QColor red(143, 30, 30, 255);
    QColor blue(6, 6, 131, 255);
    QColor white(255, 255, 255);
    QColor black(0, 0, 0);
    QColor orange(186, 105, 0, 255);

    if(ui->actionNightmode->isChecked())
    {
        white = QColor(0, 0, 0);
        black = QColor(0, 154, 0);
        red = QColor(242, 30, 30, 255);
        blue = QColor(0, 242, 242, 255);
        orange = QColor(224, 126, 0, 255);
    }//if

    QPen gridPen(black, 0, Qt::SolidLine);
    QFont font = QFont("FreeSans");
    font.setPixelSize(fSize);
    QFontMetrics fM(font);
    painter.setFont(font);
    painter.setBrush(white);

    QRect depRect(x, y, width/2.0, height - y);
    x+=width/2.0 - 25;
    x+=50;
    QRect arrRect(x, y, width/2.0, height - y);

    painter.drawRect(depRect);

    painter.drawRect(arrRect);

    /*
    //Drawing the runways, works and is nice, but not necessary and takes up much space.
    NVUPOINT* dep = lWP[0];
    if(dep->type == WAYPOINT::TYPE_AIRPORT && dep->data!=NULL)
    {
        AIRPORT_DATA* ad = (AIRPORT_DATA*) dep->data;
        //Set top left corner and bottom right corner of airport
        CPoint pTopLeft(181, 91, 0);
        CPoint pBottomRight(-181, -91, 0);

        //Get topleft and bottomright extreme corners fitting all the runways in the airport
        if(ad->lRunways.size()>0)       //Set first point, as we have nothing to compare it to.
        {
            RUNWAY* r = ad->lRunways[0];
            CPoint rs = CPoint(r->start.x, r->start.y, 0.0);
            CPoint re = CPoint(r->end.x, r->end.y, 0.0);
            LMATH::latlonToScreen(rs);
            LMATH::latlonToScreen(re);

            pTopLeft.x = (rs.x<re.x ? rs.x : re.x);
            pTopLeft.y = (rs.y<re.y ? rs.y : re.y);
            pBottomRight.x = (rs.x>re.x ? rs.x : re.x);
            pBottomRight.y = (rs.y>re.y ? rs.y : re.y);
        }
        for(unsigned int i=1; i<ad->lRunways.size(); i++)   //Continue comparing
        {
            RUNWAY* r = ad->lRunways[i];
            CPoint rs = CPoint(r->start.x, r->start.y, 0.0);
            CPoint re = CPoint(r->end.x, r->end.y, 0.0);
            LMATH::latlonToScreen(rs);
            LMATH::latlonToScreen(re);

            if(rs.x<pTopLeft.x) pTopLeft.x = rs.x;
            if(re.x<pTopLeft.x) pTopLeft.x = re.x;
            if(rs.y<pTopLeft.y) pTopLeft.y = rs.y;
            if(re.y<pTopLeft.y) pTopLeft.y = re.y;

            if(rs.x>pBottomRight.x) pBottomRight.x = rs.x;
            if(re.x>pBottomRight.x) pBottomRight.x = re.x;
            if(rs.y>pBottomRight.y) pBottomRight.y = rs.y;
            if(re.y>pBottomRight.y) pBottomRight.y = re.y;
        }

        //Set scale factor of airport
        CPoint pSize(pBottomRight - pTopLeft);
        pSize.x = fabs(pSize.x);
        pSize.y = fabs(pSize.y);
        double aScale = std::min(depRect.width()/pSize.x, depRect.height()/pSize.y); //std::min(depRect.width()/pSize.x, depRect.height()/pSize.y);
        pTopLeft = pTopLeft*aScale;

        //Draw the runways
        gridPen.setWidth(5);
        painter.setPen(gridPen);
        for(unsigned int i=0; i<ad->lRunways.size(); i++)
        {
            RUNWAY* r = ad->lRunways[i];
            CPoint rs = CPoint(r->start.x, r->start.y, 0.0);
            CPoint re = CPoint(r->end.x, r->end.y, 0.0);
            LMATH::latlonToScreen(rs);
            LMATH::latlonToScreen(re);

            CPoint rsl = (rs + (re - rs).getNormal(false).getNormalized()*0.000005)*aScale - pTopLeft;       //15 meters left
            CPoint rsr = (rs + (re - rs).getNormal(true).getNormalized()*0.000005)*aScale - pTopLeft;       //15 meters right
            CPoint rel = (re + (rs - re).getNormal(false).getNormalized()*0.000005)*aScale - pTopLeft;       //15 meters left
            CPoint rer = (re + (rs - re).getNormal(true).getNormalized()*0.000005)*aScale - pTopLeft;       //15 meters right
            QPointF points[4] =
            {
                QPointF(rsl.x+10, rsl.y+y+10),
                QPointF(rer.x+10, rer.y+y+10),
                QPointF(rel.x+10, rel.y+y+10),
                QPointF(rsr.x+10, rsr.y+y+10)
            };

            painter.setBrush(black);
            painter.drawPolygon(points, 4);

            rs = rs*aScale;
            re = re*aScale;
            rs = (rs - pTopLeft);
            re = (re - pTopLeft);

            rs.x+=10;
            re.x+=10;
            rs.y+=y+10;
            re.y+=y+10;
            //painter.drawLine(rs.x, rs.y, re.x, re.y);
        }//for
    }
    */
}

void MainWindow::drawNVUHeader(QPainter& painter, NVUPOINT* dep, NVUPOINT* arr, double fork, int& y)
{
    QColor black(0, 0, 0);
    QColor gray(207, 207, 207);

    if(ui->actionNightmode->isChecked())
    {
        black = QColor(0, 200, 0);
        gray = QColor(0, 32, 0);
    }

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
    QPen gridPen(black, 0, Qt::SolidLine);
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
    painter.drawText(1500*xscale, y, QString::number(dep->Srem, 'f', 1));
    font.setBold(false);
    painter.setFont(font);
    painter.drawText(3000*xscale, y, "Fork, deg.");
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(4500*xscale, y, QString::number(fork, 'f', 1));
    font.setBold(false);
    painter.setFont(font);
    painter.drawText(6000*xscale, y, "TOD, km/nm.");
    font.setBold(true);
    painter.setFont(font);

    //Draw TOD
    //QString str = ui->labelTOD->text();
    QString str;
    if(ui->tableWidget->pTOD)
    {
        str = QString::number(ui->tableWidget->dTOD,'f', 1) + " / " + QString::number(ui->tableWidget->dTOD/1.852,'f', 1);
        if(ui->tableWidget->dTOD<1.0) str = "at " + ui->tableWidget->pTOD->name;
        else str = str + " before " + ui->tableWidget->pTOD->name;
    }
    else str = "UNABLE";
    //str.remove(0, 4);

    painter.drawText(7500*xscale, y, str);
    y+=40;
    painter.drawRect(1500*xscale, y, 500, 5);
    painter.drawRect(4500*xscale, y, 500, 5);
    painter.drawRect(7500*xscale, y, 2250, 5);

    /*
    //Draw version
    painter.drawText(11500*xscale, y, XNVU_VERSION);
    //Draw time and date
    painter.drawText(10900*xscale, y, QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd    hh:mm:ss") + "   UTC");
    */


    y+=90;
    painter.setBrush(gray);
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
    QColor white(255, 255, 255);
    QColor black(0, 0, 0);
    QColor orange(186, 105, 0, 255);

    if(ui->actionNightmode->isChecked())
    {
        white = QColor(0, 0, 0);
        black = QColor(0, 154, 0);
        red = QColor(242, 30, 30, 255);
        blue = QColor(0, 242, 242, 255);
        orange = QColor(224, 126, 0, 255);
    }//if


    QPen gridPen(black, 0, Qt::SolidLine);
    painter.setPen(gridPen);

    //QFont font = QFont("Liberation Serif");
    QFont font = QFont("FreeSans");
    font.setPixelSize(fSize);
    QFontMetrics fM(font);
    painter.setFont(font);
    painter.setBrush(white);


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
    if(wp->type == WAYPOINT::TYPE_RUNWAY || wp->type == WAYPOINT::TYPE_HELIPAD) qstr = wp->name2 + " " + qstr;
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    dy = fM.boundingRect(qstr).height() + 10;
    painter.drawText(x + dx, y + dy, qstr);
    font.setPixelSize(fSize*0.75);
    font.setBold(false);
    fM = QFontMetrics(font);
    painter.setFont(font);
    //qstr = (ui->actionAltitude_in_feet->isChecked() ? QString::number(wp->alt, 'f', 0) + " ft" : QString::number(LMATH::feetToMeter(wp->alt), 'f', 0) + " m");
    qstr = QString::number(LMATH::feetToMeter(wp->alt), 'f', 0) + " m";
    //dx = fM.boundingRect(qstr).width();
    //dx = (rectW*xscale/2 - dx/2);
    painter.drawText(x + 50, y + rectH - 50, qstr);
    qstr = QString::number(wp->alt, 'f', 0) + " ft";
    dx = fM.boundingRect(qstr).width();
    painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
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
        font.setBold(true);
        painter.setFont(font);
        fM = QFontMetrics(font);
        qstr = "Ch " + QString::number((int) wp->freq);
        dx = fM.boundingRect(qstr).width();
        dx = (rectW*xscale)/2 - dx/2;
        painter.drawText(x + dx, y + rectH - 50, qstr);
    }
    else if(wp->type == WAYPOINT::TYPE_VOR ||
            wp->type == WAYPOINT::TYPE_DME ||
            wp->type == WAYPOINT::TYPE_VORDME ||
            wp->type == WAYPOINT::TYPE_ILS ||
            wp->type == WAYPOINT::TYPE_LOC ||
            wp->type == WAYPOINT::TYPE_TACAN ||
            wp->type == WAYPOINT::TYPE_VORTAC)
    {
        font.setBold(true);
        painter.setFont(font);
        fM = QFontMetrics(font);
        qstr = QString::number(wp->freq, 'f', 3);
        dx = fM.boundingRect(qstr).width();
        dx = (rectW*xscale)/2 - dx/2;
        painter.drawText(x + dx, y + rectH - 50, qstr);
    }
    x+=rectW*xscale;

    //Draw coordinate column
    font.setPixelSize(fSize);
    font.setBold(false);
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
    WAYPOINT* rsbn = wp->getRSBN();
    if(rsbn)
    {
        font.setPixelSize(fSize*0.75);
        fM = QFontMetrics(font);
        painter.setFont(font);
        qstr = WAYPOINT::getTypeStr(rsbn);
        dy = fM.boundingRect(qstr).height();
        dx = fM.boundingRect("VOR/DME").width();
        painter.drawText(x + 50, y + dy + 10, qstr);

        font.setPixelSize(fSize);
        font.setBold(true);
        fM = QFontMetrics(font);
        painter.setFont(font);
        qstr = rsbn->name + (rsbn->country.isEmpty() ? "" : " [" + rsbn->country + "]");
        painter.drawText(x + 50 + dx + 100, y + dy + 10, qstr);

        font.setPixelSize(fSize*0.75);
        fM = QFontMetrics(font);
        painter.setFont(font);
        if(rsbn->type==WAYPOINT::TYPE_RSBN) qstr = "Ch " + QString::number((int)rsbn->freq);// + (wp->rsbn->name2.isEmpty() ? "" : "   " + wp->rsbn->name2);
        else qstr = QString::number(rsbn->freq, 'f', 3);// + (wp->rsbn->name2.isEmpty() ? "" : "   " + wp->rsbn->name2);
        painter.drawText(x + 50, y + rectH - 50, qstr);
        int tWidth = painter.fontMetrics().width(qstr);

        font.setBold(false);
        painter.setFont(font);
        qstr = (rsbn->name2.isEmpty() ? "" : "   " + rsbn->name2);
        if(!qstr.isEmpty()) painter.drawText(x + 50 + tWidth, y + rectH - 50, qstr);
    }
    x+=rectW*xscale;

    //Draw Sm and Zm
    qstr = "";
    font.setPixelSize(fSize);
    font.setBold(false);
    fM = QFontMetrics(font);
    painter.setFont(font);
    xscale = 2.1;
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = QString::number(wp->Sm, 'f', 1);
    dy = fM.boundingRect(qstr).height();
    painter.setPen(red);
    if(!isArr && wp->getRSBN()) painter.drawText(x + 50, y + dy + 10, qstr);
    qstr = QString::number(wp->Zm, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    painter.setPen(blue);
    if(!isArr && wp->getRSBN()) painter.drawText(x + rectW*xscale - dx - 50, y + rectH - 50, qstr);
    x+=rectW*xscale;

    //Draw Map angle
    qstr = "";
    xscale = 1.7;
    painter.setPen(gridPen);
    painter.drawRect(x, y, rectW*xscale, rectH);
    qstr = QString::number(wp->MapAngle, 'f', 1);
    dx = fM.boundingRect(qstr).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.setPen(orange);
    if(!isArr && wp->getRSBN()) painter.drawText(x + dx, y + fHOffset, qstr);
    x+=rectW*xscale;

    //Draw Atrg and Dtrg
    qstr = "";
    xscale = 2.4;
    painter.setPen(gridPen);
    painter.drawRect(x, y, rectW*xscale, rectH);
    if(!isArr && wp->getRSBN()) qstr = QString::number(wp->Atrg, 'f', 1);
    dy = fM.boundingRect(qstr).height();
    painter.drawText(x + 50, y + dy + 10, qstr);
    if(!isArr && wp->getRSBN()) qstr = QString::number(wp->Dtrg, 'f', 1);
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
    const std::vector<NVUPOINT*>& lWP = ui->tableWidget->getWaypoints();
    if(lWP.size()<2)
    {
        QMessageBox box;
        box.setText("Flightplan cannot be printed with less than 2 waypoints.");
        box.setIcon(QMessageBox::Information);
        box.exec();
        return;
    }

    QPrinter printer(QPrinter::HighResolution); //create your QPrinter (don't need to be high resolution, anyway)
    QPrintPreviewDialog dialog(&printer);
    dialog.setWindowFlags(Qt::Window);
    dialog.setWindowTitle("Print NVU-plan");
    connect(&dialog, SIGNAL(paintRequested(QPrinter *)), SLOT(printPreview(QPrinter *)));
    dialog.exec();
}

void MainWindow::printOnPDF()
{
    const std::vector<NVUPOINT*>& lWP = ui->tableWidget->getWaypoints();
    if(lWP.size()<2)
    {
        QMessageBox box;
        box.setText("Flightplan cannot be exported with less than 2 waypoints.");
        box.setIcon(QMessageBox::Information);
        box.exec();
        return;
    }

    QString fileName = (*lWP.begin())->name + "_" + (lWP.back())->name + ".pdf";
    if(DialogSettings::defaultLoadSaveDir.length()>0) fileName = DialogSettings::defaultLoadSaveDir + "//" + fileName;
    fileName = QFileDialog::getSaveFileName((QWidget* )0, "Export PDF", fileName, "*.pdf");
    if(fileName.isEmpty()) return;
    if(QFileInfo(fileName).suffix().isEmpty()) fileName.append(".pdf");

    QPrinter printer(QPrinter::HighResolution); //create your QPrinter (don't need to be high resolution, anyway)
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printPreview(&printer);

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
        NVUPOINT* np = new NVUPOINT(dEdit.newPoint);
        np->wpOrigin = WAYPOINT::ORIGIN_FLIGHTPLAN;
        ui->tableWidget->replaceWaypoint(np, row);

        np = new NVUPOINT(dEdit.newPoint);
        np->setRSBN(NULL);
        np->wpOrigin = WAYPOINT::ORIGIN_XNVU;
        XFMS_DATA::addXNVUWaypoint(np);

        ui->tableWidget->refreshFlightplan();
    }
    else if(dr==DialogWaypointEdit::SAVE)
    {
        NVUPOINT* np = new NVUPOINT(dEdit.newPoint);
        np->wpOrigin = WAYPOINT::ORIGIN_FLIGHTPLAN;
        ui->tableWidget->replaceWaypoint(np, row);
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

    ui->label_alignWPS->setVisible(DialogSettings::distAlignXWP);
    ui->label_alignFMS->setVisible(DialogSettings::distAlignFMS);

}

void MainWindow::on_frameDescription_clicked()
{
    QListWidgetItemData* iD = (QListWidgetItemData*) ui->listWidget->currentItem();
    if(iD)
    {
        if(iD->nvupoint->type == WAYPOINT::TYPE_AIRWAY && iD->nvupoint->data)
        {
            //ui->labelWPMagVar->setText("CLICK TO SHOW WAYPOINTS");
            //if(ui->labelWPMagVar->text().compare("CLICK TO SHOW WAYPOINTS") == 0)
            //{
                ui->labelWPMagVar->setText("<---- CLICK TO GO BACK");
                AIRWAY* ats = (AIRWAY*) iD->nvupoint->data;
                ui->listWidget->clear();
                for(int i=0; i<ats->lATS.size(); i++)
                {
                    NVUPOINT* wp = (NVUPOINT*) ats->lATS[i];
                    QListWidgetItemData *newItem = new QListWidgetItemData;
                    QString qstr = wp->name;
                    if(!wp->name2.isEmpty() && wp->type!=WAYPOINT::TYPE_FIX) qstr = qstr + " - " + wp->name2;
                    if(!wp->country.isEmpty()) qstr = qstr + " [" + wp->country + "]";
                    newItem->setText(qstr);
                    newItem->nvupoint = wp;
                    ui->listWidget->addItem(newItem, false);
                }//for
            /*
            }//if
            else
            {
                ui->labelWPMagVar->setText("SELECT AIRWAY IN LIST");
                ui->listWidget->search(ui->labelWPType->text(), 0);
            }
            */

            return;
        }//if
    }//if

    if(ui->labelWPType2->text().compare("AIRWAY") == 0)
    {
        if(ui->labelWPMagVar->text().compare("<---- CLICK TO GO BACK") == 0)
        {
            ui->labelWPMagVar->setText("SELECT AIRWAY IN LIST");
            ui->listWidget->search(ui->labelWPType->text(), 0);
        }
    }

}


void MainWindow::on_actionXNVU_library_triggered()
{
    ui->lineEdit->clear();
    DialogWPSEdit dEdit;
    int rv = dEdit.exec();
}


/*
void MainWindow::on_pushButton_showAIRAC_Airports_clicked()
{
    //TODO: XP11 Custom, default and gateway airports
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_AIRAC_AIRPORTS, ui->pushButton_showAIRAC_Airports->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showAIRAC_Navaids_clicked()
{
    //TODO: XP11 AIRAC NAVAIDS?
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_AIRAC_NAVAIDS, ui->pushButton_showAIRAC_Navaids->isChecked());
    ui->listWidget->refreshSearch();
}

void MainWindow::on_pushButton_showAIRAC_Fixes_clicked()
{
    //TODO: XP11 AIRAC FIXES?
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_AIRAC_WAYPOINTS, ui->pushButton_showAIRAC_Fixes->isChecked());
    ui->listWidget->refreshSearch();
}
*/

void MainWindow::on_pushButton_showAIRAC_Airways_clicked()
{
    //TODO: XP11 AIRAC AIRWAYS?
    ui->listWidget->showType(WAYPOINT::TYPE_AIRWAY, ui->pushButton_showAIRAC_Airways->isChecked());
    ui->listWidget->refreshSearch();
}

/*
void MainWindow::on_pushButton_showEarthNav_clicked()
{
    //TODO: XP11 EARTHNAV?
    ui->listWidget->showOrigin(WAYPOINT::ORIGIN_EARTHNAV, ui->pushButton_showEarthNav->isChecked());
    ui->listWidget->refreshSearch();
}
*/

void MainWindow::on_pushButton_showRSBN_clicked()
{
    ui->listWidget->showType(WAYPOINT::TYPE_RSBN, ui->pushButton_showRSBN->isChecked());
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

/*
void MainWindow::calculateTOD()
{
    NVUPOINT *cp = NULL;
    double d;
    double fl = ui->doubleSpinBoxFL->value();
    double mach = ui->doubleSpinBox_MACH->value();
    double vs = ui->doubleSpinBox_VS->value();
    double twc = ui->doubleSpinBox_TWC->value();
    double isa = ui->doubleSpinBox_ISA->value();

    if(!ui->actionShow_feet->isChecked()) fl = LMATH::feetToMeter(fl);
    ui->tableWidget->calculateTOD(cp, d, fl, mach, vs, twc, isa);

    if(cp) ui->labelTOD->setText("TOD " + (int(d) == 0 ? "at " : QString::number(d, 'f', 0) + " km before ") + cp->name);
    else ui->labelTOD->setText("TOD [UNABLE]");
}
*/

void MainWindow::on_actionShow_feet_triggered()
{
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


void MainWindow::goDirectToFieldClicked(QLineEditWP *wp)
{
    NVUPOINT* fr;
    NVUPOINT* to;
    if(wp == ui->lineEdit_DTTo)
    {
        to = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow());
        if(!to && ui->listWidget->currentItem()) to = ((QListWidgetItemData*) ui->listWidget->currentItem())->nvupoint;
        if(to) wp->setWaypoint(to);
    }
    else if(wp == ui->lineEdit_DTCourseFrom)
    {
        fr = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow());
        to = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow()+1);
        if(fr && to)
        {
            wp->setWaypoint(fr);
            ui->lineEdit_DTCourseTo->setWaypoint(to);
        }
    }//else
    else if(wp == ui->lineEdit_DTCourseTo)
    {
        fr = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow()-1);
        to = ui->tableWidget->getWaypoint(ui->tableWidget->currentRow());
        if(fr && to)
        {
            wp->setWaypoint(to);
            ui->lineEdit_DTCourseFrom->setWaypoint(fr);
        }
    }//else

    setWaypointDescription(wp->getWaypoint());
}

void MainWindow::on_listWidget_clicked(const QModelIndex &index)
{
    //ui->tableWidget->selectNone();
    //QString tst = "QTableWidget::item{selection-background-color: rgb(82, 140, 35)}";
    //ui->tableWidget->setStyleSheet(tst);
}

void MainWindow::on_pushButtonDTInsert_clicked()
{
    NVUPOINT* a = ui->lineEdit_DTCourseFrom->getWaypoint();
    NVUPOINT* b = ui->lineEdit_DTCourseTo->getWaypoint();
    NVUPOINT* c = ui->lineEdit_DTTo->getWaypoint();

    if(a == NULL || b == NULL || c == NULL) return;

    double brng = LMATH::calc_bearing(a->latlon, b->latlon);
    CPoint cp;
    LMATH::calc_destination_orthodromic(a->latlon, brng, a->S + ui->doubleSpinBox_DTCourseDistRem->value(), cp);

    NVUPOINT* n = new NVUPOINT();
    //n->name = a->name + "_DTO";
    n->name = "DTO_" + c->name;
    n->latlon = cp;
    n->type = WAYPOINT::TYPE_LATLON;
    n->MD = calc_magvar(n->latlon.x, n->latlon.y, dat, LMATH::feetToMeter(a->alt));
    n->wpOrigin = WAYPOINT::ORIGIN_FLIGHTPLAN;

    ui->tableWidget->insertWaypoint(n, ui->tableWidget->currentRow()+1);
    ui->tableWidget->insertWaypoint(new NVUPOINT(*c), ui->tableWidget->currentRow()+2);
}


void MainWindow::fplDataChangeCheck()
{
    bool changed = false;

    if(ui->doubleSpinBox_MACH->value() != ui->tableWidget->fplData.speed)
    {
        changed = true;
        //TODO: Set color to yellow
    }

    if(ui->doubleSpinBox_VS->value() != ui->tableWidget->fplData.vs)
    {
        changed = true;
        //TODO: Set color to yellow
    }

    if(ui->doubleSpinBox_TWC->value() != ui->tableWidget->fplData.twc)
    {
        changed = true;
        //TODO: Set color to yellow
    }

    if(ui->doubleSpinBoxFL->value() != ui->tableWidget->fplData.fl)
    {
        changed = true;
        //TODO: Set color to yellow
    }

    if(ui->doubleSpinBox_ISA->value() != ui->tableWidget->fplData.isa)
    {
        changed = true;
        //TODO: Set color to yellow
    }

    if(ui->dateEdit->date().toJulianDay() != dat)
    {
        changed = true;
        //TODO: Set color to yellow
    }

    if(!changed)
    {
        ui->pushButtonSetDate->setEnabled(false);
        QString styleSheet = "background-color: rgb(0, 30, 0);"
                              "color: rgb(107, 239, 0);";
        ui->pushButtonSetDate->setText("IS SET");
        ui->pushButtonSetDate->setStyleSheet(styleSheet);
        return;
    }

    ui->pushButtonSetDate->setEnabled(true);
    QString styleSheet = "background-color: rgb(255, 0, 0);"
                         "color: rgb(255, 255, 255);";

    ui->pushButtonSetDate->setText("SET");
    ui->pushButtonSetDate->setStyleSheet(styleSheet);
}

void MainWindow::on_doubleSpinBox_MACH_valueChanged(double _speed)
{
    if(_speed>10.0) ui->doubleSpinBox_MACH->setSuffix(" km/h");
    else ui->doubleSpinBox_MACH->setSuffix(" M");
    fplDataChangeCheck();
}


void MainWindow::on_doubleSpinBox_VS_valueChanged(double arg1)
{
    fplDataChangeCheck();
}

void MainWindow::on_doubleSpinBox_TWC_valueChanged(double arg1)
{
    fplDataChangeCheck();
}

void MainWindow::on_doubleSpinBoxFL_valueChanged(double arg1)
{
    fplDataChangeCheck();
}

void MainWindow::on_doubleSpinBox_ISA_valueChanged(double arg1)
{
    fplDataChangeCheck();
}

void MainWindow::on_dateEdit_userDateChanged(const QDate &date)
{
    fplDataChangeCheck();

/*
    if(date.toJulianDay() == dat)
    {
        ui->pushButtonSetDate->setEnabled(false);
        QString styleSheet = "background-color: rgb(0, 30, 0);"
                              "color: rgb(107, 239, 0);";
        ui->pushButtonSetDate->setText("IS SET");
        ui->pushButtonSetDate->setStyleSheet(styleSheet);
        return;
    }

    ui->pushButtonSetDate->setEnabled(true);
    QString styleSheet = "background-color: rgb(255, 0, 0);"
                         "color: rgb(255, 255, 255);";

    ui->pushButtonSetDate->setText("SET");
    ui->pushButtonSetDate->setStyleSheet(styleSheet);
*/
}

void MainWindow::on_pushButtonSetDate_clicked()
{
    if(dat!=ui->dateEdit->date().toJulianDay())
    {
        dat = ui->dateEdit->date().toJulianDay();
        ui->tableWidget->dat = dat;
        XFMS_DATA::setDate(dat);
    }

    //ERROR: To tired now, what have I done. Recheck this later on (note, not an actual error)
    if(ui->tableWidget->fplData.fl!=ui->doubleSpinBoxFL->value())
    {
        ui->tableWidget->fplData.fl = ui->doubleSpinBoxFL->value();

        std::vector<NVUPOINT*> lP = ui->tableWidget->getWaypoints();
        for(int i=0; i<lP.size(); i++)
        {
            NVUPOINT* p = lP[i];
            if(i==0 || i == (int(lP.size())-1)) p->alt = p->elev;
            else p->alt = (DialogSettings::showFeet ? ui->tableWidget->fplData.fl : LMATH::meterToFeet(ui->tableWidget->fplData.fl));
        }
    }

    ui->tableWidget->fplData.speed = ui->doubleSpinBox_MACH->value();
    ui->tableWidget->fplData.vs = ui->doubleSpinBox_VS->value();
    ui->tableWidget->fplData.twc = ui->doubleSpinBox_TWC->value();
    ui->tableWidget->fplData.isa = ui->doubleSpinBox_ISA->value();
    ui->tableWidget->refreshFlightplan();


    ui->pushButtonSetDate->setEnabled(false);
    QString styleSheet = "background-color: rgb(0, 30, 0);"
                          "color: rgb(107, 239, 0);";
    ui->pushButtonSetDate->setText("IS SET");
    ui->pushButtonSetDate->setStyleSheet(styleSheet);


}

void MainWindow::clickedDataLabels(QLabelClick* _label)
{
    if(_label == ui->labelCruise)
    {
        /*
        if(DialogSettings::cruiseFormat == 0)
        {
            double FL = ui->doubleSpinBoxFL->value();
            if(DialogSettings::showFeet) FL = LMATH::feetToMeter(FL);
            FL = FL/1000.0;
            ui->doubleSpinBox_MACH->setValue(LMATH::MACH_to_IAS(ui->doubleSpinBox_MACH->value(), FL, ui->doubleSpinBox_ISA->value()));
            ui->doubleSpinBox_MACH->setSuffix(" km/h");
            DialogSettings::cruiseFormat = 1;
        }
        else if(DialogSettings::cruiseFormat == 1)
        {
            ui->doubleSpinBox_MACH->setValue(ui->doubleSpinBox_MACH->value()/1.852);
            ui->doubleSpinBox_MACH->setSuffix(" kn");
            DialogSettings::cruiseFormat = 2;
        }
        else
        {
            double FL = ui->doubleSpinBoxFL->value();
            if(DialogSettings::showFeet) FL = LMATH::feetToMeter(FL);
            FL = FL/1000.0;
            ui->doubleSpinBox_MACH->setValue(LMATH::IAS_to_MACH(ui->doubleSpinBox_MACH->value()*1.852, FL, ui->doubleSpinBox_ISA->value()));
            ui->doubleSpinBox_MACH->setSuffix(" M");
            DialogSettings::cruiseFormat = 0;
        }
        */
    }
    else if(_label == ui->labelVS)
    {
        if(DialogSettings::VSFormat == 0)
        {
            ui->tableWidget->fplData.vs = LMATH::meterToFeet(ui->doubleSpinBox_VS->value())*60.0;
            ui->doubleSpinBox_VS->setValue(LMATH::meterToFeet(ui->doubleSpinBox_VS->value())*60.0);
            ui->doubleSpinBox_VS->setSuffix(" ft/m");
            DialogSettings::VSFormat = 1;
        }
        else
        {
            ui->tableWidget->fplData.vs = LMATH::feetToMeter(ui->doubleSpinBox_VS->value())/60.0;
            ui->doubleSpinBox_VS->setValue(LMATH::feetToMeter(ui->doubleSpinBox_VS->value())/60.0);
            ui->doubleSpinBox_VS->setSuffix(" m/s");
            DialogSettings::VSFormat = 0;
        }
    }
    else if(_label == ui->labelTWC)
    {
        if(DialogSettings::TWCFormat == 0)
        {
            ui->doubleSpinBox_TWC->setValue(ui->doubleSpinBox_TWC->value()/1.852);
            ui->doubleSpinBox_TWC->setSuffix(" kn");
            DialogSettings::TWCFormat = 1;
        }
        else
        {
            ui->doubleSpinBox_TWC->setValue(ui->doubleSpinBox_TWC->value()*1.852);
            ui->doubleSpinBox_TWC->setSuffix(" km/h");
            DialogSettings::TWCFormat = 0;
        }
    }
    else if(_label == ui->labelFlightLevel)
    {
        if(DialogSettings::showFeet)
        {
            ui->tableWidget->fplData.fl = LMATH::feetToMeter(ui->doubleSpinBoxFL->value());
            ui->doubleSpinBoxFL->setValue(LMATH::feetToMeter(ui->doubleSpinBoxFL->value()));
            ui->doubleSpinBoxFL->setSuffix(" m");
            //ui->tableWidget->fplData.fl = ui->doubleSpinBoxFL->value();

            DialogSettings::showFeet = false;
        }
        else
        {
            ui->tableWidget->fplData.fl = LMATH::meterToFeet(ui->doubleSpinBoxFL->value());
            ui->doubleSpinBoxFL->setValue(LMATH::meterToFeet(ui->doubleSpinBoxFL->value()));
            ui->doubleSpinBoxFL->setSuffix(" ft");
            //ui->tableWidget->fplData.fl = ui->doubleSpinBoxFL->value();
            DialogSettings::showFeet = true;
        }//else

        ui->tableWidget->refreshFlightplan();
    }
    else if(_label == ui->labelTOD)
    {
        if(DialogSettings::showTOD_METRIC)
        {
            DialogSettings::showTOD_METRIC = false;

        }
        else
        {
            DialogSettings::showTOD_METRIC = true;
        }

        ui->tableWidget->refreshFlightplan(); //Uncessary CPU nuclear power wattage consumption, refreshing flightplan only because of change from metric to nautical or vice versa feels overkill. :S
    }

}
