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
    connect(ui->actionExport_X_Plane_FMS, SIGNAL(triggered()), this, SLOT(exportFMS()));
    connect(ui->actionImport_X_Plane_FMS, SIGNAL(triggered()), this, SLOT(importFMS()));
    connect(ui->actionSave_XNVU_flightplan, SIGNAL(triggered()), this, SLOT(saveNVUFlightPlan()));
    connect(ui->actionLoad_XNVU_flightplan, SIGNAL(triggered()), this, SLOT(loadNVUFlightplan()));
    connect(ui->actionX_Plane_folder, SIGNAL(triggered()), this, SLOT(showXPlaneSettings()));
    connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(on_pushButtonPrint_clicked()));
    connect(ui->actionExport_to_PDF, SIGNAL(triggered()), this, SLOT(printOnPDF()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(quit()));
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showFlightplanContextMenu(const QPoint&)));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showSearchListContextMenu(const QPoint&)));
    connect(ui->frameDescription, SIGNAL(clicked(QPoint)), this, SLOT(on_frameDescription_clicked()), Qt::UniqueConnection);
    connect(&clearFlightplan_timer, SIGNAL(timeout()), this, SLOT(clearFlightplanTimeout()));
    connect(ui->lineEdit_DTCourseFrom, SIGNAL(clicked(QLineEditWP*)), this, SLOT(goDirectToFieldClicked(QLineEditWP*)));
    connect(ui->lineEdit_DTCourseTo, SIGNAL(clicked(QLineEditWP*)), this, SLOT(goDirectToFieldClicked(QLineEditWP*)));
    connect(ui->lineEdit_DTTo, SIGNAL(clicked(QLineEditWP*)), this, SLOT(goDirectToFieldClicked(QLineEditWP*)));

    //Connect clickable labels for TOD calculations
    connect(ui->labelCruise, SIGNAL(clicked(QLabelClick*)), this, SLOT(clickedDataLabels(QLabelClick*)));
    connect(ui->labelVS, SIGNAL(clicked(QLabelClick*)), this, SLOT(clickedDataLabels(QLabelClick*)));
    connect(ui->labelTWC, SIGNAL(clicked(QLabelClick*)), this, SLOT(clickedDataLabels(QLabelClick*)));
    connect(ui->labelFlightLevel, SIGNAL(clicked(QLabelClick*)), this, SLOT(clickedDataLabels(QLabelClick*)));

    //Give tablewidget a reference to labelFork and labelTOD and set its size
    ui->tableWidget->setColumnCount(QFlightplanTable::COL::_SIZE);
    ui->tableWidget->qFork = ui->labelFork;
    ui->tableWidget->qTOD = ui->labelTOD;
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
    ui->actionShow_feet->setChecked(DialogSettings::showFeet);
    ui->actionNightmode_print_export->setChecked(DialogSettings::nightMode);



    ui->tableWidget->showFeet = DialogSettings::showFeet;
    if(DialogSettings::showFeet)
    {
        ui->doubleSpinBoxFL->setSuffix(" ft");
    }//if
    else ui->doubleSpinBoxFL->setSuffix(" m");

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


    ui->label_alignATS->setText("ATS align");
    ui->label_alignEarthNav->setText("EarthNav align");
    ui->label_alignWPS->setText("WPS align");
    ui->label_alignFMS->setText("FMS align");

    ui->label_alignATS->setVisible(DialogSettings::distAlignATS);
    ui->label_alignEarthNav->setVisible(DialogSettings::distAlignEarthNav);
    ui->label_alignWPS->setVisible(DialogSettings::distAlignWPS);
    ui->label_alignFMS->setVisible(DialogSettings::distAlignFMS);

    if(DialogSettings::cruiseFormat == 1){ ui->doubleSpinBox_MACH->setSuffix(" km/h"); ui->doubleSpinBox_MACH->setValue(476);}
    else if(DialogSettings::cruiseFormat == 2) {ui->doubleSpinBox_MACH->setSuffix(" kn"); ui->doubleSpinBox_MACH->setValue(257);}
    else {ui->doubleSpinBox_MACH->setSuffix(" M"); ui->doubleSpinBox_MACH->setValue(0.8);}
    if(DialogSettings::VSFormat == 1){ ui->doubleSpinBox_VS->setSuffix(" ft/m"); ui->doubleSpinBox_VS->setValue(1575);}
    else{ui->doubleSpinBox_VS->setSuffix(" m/s"); ui->doubleSpinBox_VS->setValue(8.0);}
    if(DialogSettings::TWCFormat == 1) ui->doubleSpinBox_TWC->setSuffix(" kn");
    else ui->doubleSpinBox_TWC->setSuffix(" km/h");
    if(DialogSettings::showFeet){ui->doubleSpinBoxFL->setSuffix(" ft"); ui->doubleSpinBoxFL->setValue(35000);}
    else{ui->doubleSpinBoxFL->setSuffix(" m"); ui->doubleSpinBoxFL->setValue(10668);}

    //ui->tableWidget->updateShownColumns();

    this->resize(DialogSettings::windowSize);
    this->move(DialogSettings::windowPos);
    ui->tableWidget->horizontalHeader()->restoreState(DialogSettings::tableState);

    if(XFMS_DATA::__ERROR_LOADING.isEmpty()) labelWarning->setText("");
    else labelWarning->setText("WARNING: " + XFMS_DATA::__ERROR_LOADING + " is not loaded!");
}

MainWindow::~MainWindow()
{
    DialogSettings::windowSize = size();
    DialogSettings::windowPos = pos();
    DialogSettings::tableState = ui->tableWidget->horizontalHeader()->saveState();
    DialogSettings::showFeet = ui->actionShow_feet->isChecked();
    DialogSettings::nightMode = ui->actionNightmode_print_export->isChecked();
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
        fpMenu.addAction("Set direct to");
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
                ui->lineEdit_DTTo->setWaypoint(wp);
            }
            else if(selectedItem == fpMenu.actions()[3])
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
            else if(selectedItem == fpMenu.actions()[4])
            {
                DialogRSBN dRSBN(wp);
                int dr = dRSBN.exec();
                if(dr == QDialog::Rejected) return;

                wp->rsbn = dRSBN.rsbn;
                ui->tableWidget->refreshFlightplan();
            }
            else if(selectedItem == fpMenu.actions()[6])
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

void MainWindow::showSearchListContextMenu(const QPoint& pos) // this is a slot
{
    // for most widgets
    QPoint globalPos = ui->listWidget->mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);



    QMenu fpMenu;
    QAction* selectedItem;
    QString styleSheet = "background-color: rgb(0, 30, 0);"
                         "color: rgb(107, 239, 0);"
                         "selected-background-color: rgb(107, 239, 0);"
                         "selected-color: rgb(0, 0, 0);";
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
    if(itemData->nvupoint->type == WAYPOINT::TYPE_AIRWAY)
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
    if(ui->actionNightmode_print_export->isChecked()) painter.fillRect(prect, Qt::black);
    prect.setY(1000);
    prect.setX(1000);
    painter.setViewport(prect);


//#ifdef _WIN32
//    painter.scale(0.5, 0.5);
//#endif

    int y = 0;
    drawNVUHeader(painter, lWPs[0], lWPs[lWPs.size() - 1], fork, y);
    y+=30;
    int j = 0;
    for(int i=0; i<lWPs.size(); i++, j++)
    {
        if(j==19)
        {
            printer->newPage();
            painter.setViewport(defvp);
            prect = printer->pageRect();
            if(ui->actionNightmode_print_export->isChecked()) painter.fillRect(prect, Qt::black);
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

void MainWindow::setWaypointDescription(NVUPOINT* wp)
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
            wp->type == WAYPOINT::TYPE_VORDME ||
            wp->type == WAYPOINT::TYPE_ILS ||
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
           wp->type == WAYPOINT::TYPE_VORDME ||
           wp->type == WAYPOINT::TYPE_ILS ||
           wp->type == WAYPOINT::TYPE_TACAN ||
           wp->type == WAYPOINT::TYPE_VORTAC
           ) qstr = qstr + "        Elev: " + (ui->actionShow_feet->isChecked() ? QString::number(wp->elev, 'f', 0) + " ft": QString::number(LMATH::feetToMeter(wp->elev), 'f', 0) + " m");

        ui->labelWPMagVar->setText(qstr);
    }

    if(wp->wpOrigin == WAYPOINT::ORIGIN_X11_CUSTOM_AIRPORTS)
    {
        ui->labelWPNote->setText("Source: Custom airports");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_X11_DEFAULT_AIRPORTS)
    {
        ui->labelWPNote->setText("Source: Default airports");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_X11_GATEWAY_AIRPORTS)
    {
        ui->labelWPNote->setText("Source: Gateway airports");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_AIRAC_AIRPORTS)
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
    QColor black(0, 0, 0);
    QColor gray(207, 207, 207);

    if(ui->actionNightmode_print_export->isChecked())
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
    painter.drawText(6000*xscale, y, "TOD, km.");
    font.setBold(true);
    painter.setFont(font);

    QString str = ui->labelTOD->text();;
    str.remove(0, 4);
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

    if(ui->actionNightmode_print_export->isChecked())
    {
        white = QColor(0, 0, 0);
        black = QColor(0, 154, 0);
        red = QColor(242, 30, 30, 255);
        blue = QColor(0, 242, 242, 255);
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
            wp->type == WAYPOINT::TYPE_VORDME ||
            wp->type == WAYPOINT::TYPE_ILS ||
            wp->type == WAYPOINT::TYPE_TACAN ||
            wp->type == WAYPOINT::TYPE_VORTAC)
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
    ui->tableWidget->showFeet = ui->actionShow_feet->isChecked();
    if(ui->actionShow_feet->isChecked())
    {
        ui->doubleSpinBoxFL->setValue(LMATH::meterToFeet(ui->doubleSpinBoxFL->value()));
        ui->doubleSpinBoxFL->setSuffix(" ft");
        ui->tableWidget->fplData.fl = ui->doubleSpinBoxFL->value();
    }
    else
    {
        ui->doubleSpinBoxFL->setValue(LMATH::feetToMeter(ui->doubleSpinBoxFL->value()));
        ui->doubleSpinBoxFL->setSuffix(" m");
        ui->tableWidget->fplData.fl = ui->doubleSpinBoxFL->value();
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
    n->name = a->name + "_DTO";
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
    else; //Set color to default

    if(ui->doubleSpinBox_VS->value() != ui->tableWidget->fplData.vs)
    {
        changed = true;
        //TODO: Set color to yellow
    }
    else; //Set color to default

    if(ui->doubleSpinBox_TWC->value() != ui->tableWidget->fplData.twc)
    {
        changed = true;
        //TODO: Set color to yellow
    }
    else; //Set color to default
    if(ui->doubleSpinBoxFL->value() != ui->tableWidget->fplData.fl)
    {
        changed = true;
        //TODO: Set color to yellow
    }
    else; //Set color to default
    if(ui->doubleSpinBox_ISA->value() != ui->tableWidget->fplData.isa)
    {
        changed = true;
        //TODO: Set color to yellow
    }
    else; //Set color to default
    if(ui->dateEdit->date().toJulianDay() != dat)
    {
        changed = true;
        //TODO: Set color to yellow
    }
    else; //Set color to default


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

    if(ui->tableWidget->fplData.fl!=ui->doubleSpinBoxFL->value())
    {
        std::vector<NVUPOINT*> lP = ui->tableWidget->getWaypoints();
        for(int i=0; i<lP.size(); i++)
        {
            NVUPOINT* p = lP[i];
            if(i==0 || i == (lP.size()-1)) p->alt = p->elev;
            else p->alt = (ui->actionShow_feet->isChecked() ? ui->doubleSpinBoxFL->value() : LMATH::meterToFeet(ui->doubleSpinBoxFL->value()));
        }

        ui->tableWidget->fplData.fl = ui->doubleSpinBoxFL->value();

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
    }
    else if(_label == ui->labelVS)
    {
        if(DialogSettings::VSFormat == 0)
        {
            ui->doubleSpinBox_VS->setValue(LMATH::meterToFeet(ui->doubleSpinBox_VS->value())*60.0);
            ui->doubleSpinBox_VS->setSuffix(" ft/m");
            DialogSettings::VSFormat = 1;
        }
        else
        {
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
        DialogSettings::showFeet = !ui->actionShow_feet->isChecked();
        ui->actionShow_feet->setChecked(DialogSettings::showFeet);
        ui->tableWidget->showFeet = ui->actionShow_feet->isChecked();
        if(ui->actionShow_feet->isChecked())
        {
            ui->doubleSpinBoxFL->setValue(LMATH::meterToFeet(ui->doubleSpinBoxFL->value()));
            ui->doubleSpinBoxFL->setSuffix(" ft");
            ui->tableWidget->fplData.fl = ui->doubleSpinBoxFL->value();
        }
        else
        {
            ui->doubleSpinBoxFL->setValue(LMATH::feetToMeter(ui->doubleSpinBoxFL->value()));
            ui->doubleSpinBoxFL->setSuffix(" m");
            ui->tableWidget->fplData.fl = ui->doubleSpinBoxFL->value();
        }//else

        ui->tableWidget->refreshFlightplan();
    }

}
