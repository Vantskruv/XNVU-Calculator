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

#define XNVU_VERSION    "XNVU version 0.25"
#define XPLANE_DIR  "/media/sda3/Documents/Utveckling/cc/XNVU_calc/"

//XFMS_DATA xdata;
NVU nvu;
int dat;
QLabel* labelWarning;

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


    //Setup columns and horizontal header
    ui->tableWidget->setColumnCount(9);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "N" << "Identifier" << "Type" << "Altitude" << "Latitude" << "Longitude" << "S" << "Spas"<< "RSBN and/or VORDME");
    ui->tableWidget->setColumnWidth(0, 50);
    ui->tableWidget->setColumnWidth(1, 100);
    ui->tableWidget->setColumnWidth(2, 100);
    ui->tableWidget->setColumnWidth(3, 100);
    ui->tableWidget->setColumnWidth(4, 150);
    ui->tableWidget->setColumnWidth(5, 150);
    ui->tableWidget->setColumnWidth(6, 75);
    ui->tableWidget->setColumnWidth(7, 75);
    ui->tableWidget->setColumnWidth(8, 360);
    ui->tableWidget->horizontalHeader()->show();
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(20);


    //Set color for status bar
    QPalette pal = ui->statusBar->palette();
    pal.setColor(ui->statusBar->foregroundRole(), QColor(255, 255, 0));
    ui->statusBar->setPalette(pal);

    //Set lineEdit to always show as capitalized (not this does not mean that the font IS capitalized)
    QFont font = ui->lineEdit->font();
    font.setCapitalization(QFont::AllUppercase);
    ui->lineEdit->setFont(font);
    ui->lineEditRoute->setFont(font);

    //Setup warning label
    labelWarning = new QLabel("");
    labelWarning->setStyleSheet("background-color: rgb(0, 30, 0);"
                                "color: rgb(255, 255, 0);");

    //Load settings and data
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    dat = yymmdd_to_julian_days(now->tm_year, now->tm_mon+1, now->tm_mday);
    DialogSettings::loadSettings();
    this->resize(QSize(DialogSettings::windowWidth, DialogSettings::windowHeight));

    labelWarning = new QLabel("");
    labelWarning->setStyleSheet("background-color: rgb(0, 30, 0);"
                                "color: rgb(255, 255, 0);");
    ui->statusBar->addPermanentWidget(labelWarning);

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
}

MainWindow::~MainWindow()
{
    DialogSettings::windowWidth = size().width();
    DialogSettings::windowHeight = size().height();

    DialogSettings::saveSettings();
    XFMS_DATA::saveXNVUData();
    nvu.clear();
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

    int row = ui->tableWidget->rowAt(pos.y());
    QTableWidgetItemData* itemD = (QTableWidgetItemData*) ui->tableWidget->item(row, 0);
    //QComboBox* qCombo = (QComboBox*) ui->tableWidget->item(row, 5);

    QMenu fpMenu;
    QAction* selectedItem;
    QString styleSheet = "background-color: rgb(0, 30, 0);"
                         "color: rgb(107, 239, 0);"
                         "selected-background-color: rgb(107, 239, 0);"
                         "selected-color: rgb(0, 0, 0);";
    fpMenu.setStyleSheet(styleSheet);

    if(itemD)
    {
        fpMenu.addAction("Move up");
        fpMenu.addAction("Move down");
        fpMenu.addAction("Edit waypoint...");
        fpMenu.addAction("Set correction beacon...");
        fpMenu.addSeparator();
        fpMenu.addAction("Delete");
    }//if
    else
    {
        //Clicked outside
        fpMenu.addAction("Create custom waypoint...");
        selectedItem = fpMenu.exec(globalPos);
        if(selectedItem)
        {
            DialogWaypointEdit dEdit(NULL, true);
            int dr = dEdit.exec();
            if(dr == QDialog::Rejected) return;
            if(dr==2)
            {
                XFMS_DATA::addXNVUWaypoint(dEdit.nvupoint);
                qDebug() << "Added";
            }
            else if(dr==3)
            {
                XFMS_DATA::addXNVUWaypointTempory(dEdit.nvupoint);
            }
            dEdit.nvupoint->rsbn = NULL;
            insertWaypoint(dEdit.nvupoint, ui->tableWidget->rowCount(), 0);
        }

        return;
    }

    selectedItem = fpMenu.exec(globalPos);
    if (selectedItem)
    {
        NVUPOINT* wp = itemD->nvupoint;
        if(selectedItem == fpMenu.actions()[0])
        {
            if(row>0)
            {
                deleteWaypoint(row);
                insertWaypoint(wp, row-1, -1);
                ui->tableWidget->selectRow(row - 1);
            }
        }
        else if(selectedItem == fpMenu.actions()[1])
        {
            if(row<(ui->tableWidget->rowCount()-1))
            {
                deleteWaypoint(row);
                insertWaypoint(wp, row, 1);
                ui->tableWidget->selectRow(row + 1);
            }
        }
        else if(selectedItem == fpMenu.actions()[2])
        {
            DialogWaypointEdit dEdit(wp, true);
            int dr = dEdit.exec();
            if(dr == QDialog::Rejected) return;

            if(dr==2) //Create new
            {
                XFMS_DATA::addXNVUWaypoint(dEdit.nvupoint);
            }
            else if(dr==3)
            {
                XFMS_DATA::addXNVUWaypointTempory(dEdit.nvupoint);
            }
            dEdit.nvupoint->rsbn = NULL;
            insertWaypoint(dEdit.nvupoint, row, 0);
        }
        else if(selectedItem == fpMenu.actions()[3])
        {
            DialogRSBN dRSBN(wp);
            int dr = dRSBN.exec();
            if(dr == QDialog::Rejected) return;

            wp->rsbn = dRSBN.rsbn;
            insertWaypoint(wp, row, 0);
        }
        else if(selectedItem == fpMenu.actions()[5])
        {
            deleteWaypoint(row);

        }
    }
}


void MainWindow::showXPlaneSettings()
{
    DialogSettings dSettings;
    if(QDialog::Rejected == dSettings.exec()) return;

    DialogSettings::saveSettings();

    XFMS_DATA::saveXNVUData();

    while(ui->tableWidget->rowCount()>0) ui->tableWidget->removeRow(0);
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
    QString fileName = QFileDialog::getOpenFileName(this, "Import FMS flightplan", XPLANE_DIR, "FMS Files (*.fms)");
    if(fileName.isEmpty()) return;

    on_pushButton_ClearFlightplan_clicked();
    XFMS_DATA::removeFMS();
    XFMS_DATA::removeXNVUFlightplan();

    XFMS_DATA::loadFMS(fileName);

    for(int i=0; i<XFMS_DATA::lFMS.size(); i++)
    {
        insertWaypoint(XFMS_DATA::lFMS[i], ui->tableWidget->rowCount(), 1);
    }
}

void MainWindow::exportFMS()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export FMS flightplan", XPLANE_DIR, "FMS Files (*.fms)");
    if(fileName.isEmpty()) return;

    std::vector<NVUPOINT*> lFMS;

    QTableWidgetItemData* iD;
    for(int i=0; i<ui->tableWidget->rowCount(); i++)
    {
        iD = (QTableWidgetItemData*) ui->tableWidget->item(i, 0);
        lFMS.push_back(iD->nvupoint);
    }

    XFMS_DATA::saveFMS(fileName, lFMS);
}

void MainWindow::loadNVUFlightplan()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load XNVU flightplan", XPLANE_DIR, "XWP Files (*.xwp)");
    if(fileName.isEmpty()) return;

    on_pushButton_ClearFlightplan_clicked();
    XFMS_DATA::removeFMS();
    XFMS_DATA::removeXNVUFlightplan();

    XFMS_DATA::loadXNVUFlightplan(fileName);

    for(int i=0; i<XFMS_DATA::lXNVUFlightplan.size(); i++)
    {
        insertWaypoint(XFMS_DATA::lXNVUFlightplan[i], ui->tableWidget->rowCount(), 0);
    }
}

void MainWindow::saveNVUFlightPlan()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save XNVU flightplan", XPLANE_DIR, "XWP Files (*.xwp)");
    if(fileName.isEmpty()) return;

    std::vector<NVUPOINT*> lXWP;

    for(int i=0; i<ui->tableWidget->rowCount(); i++)
    {
        QTableWidgetItemData* itemD = (QTableWidgetItemData*) ui->tableWidget->item(i, 0);
        lXWP.push_back(itemD->nvupoint);
    }

    XFMS_DATA::saveXNVUFlightplan(fileName, lXWP);
}

void MainWindow::deleteWaypoint(int row)
{
    if(row<0 || row>=ui->tableWidget->rowCount()) return;

    QTableWidgetItemData* pItem;
    QTableWidgetItemData* nItem;


    if(row  == (ui->tableWidget->rowCount() - 1)) //We change the distance of previous point, as that point will be the last one.
    {
        if(row>0) ui->tableWidget->item(row-1, 6)->setText("0.0");
    }
    else if(row>0)                                //We set distance between previous and next waypoint, at the previous point
    {
        pItem = (QTableWidgetItemData*) ui->tableWidget->item(row-1, 0);
        nItem = (QTableWidgetItemData*) ui->tableWidget->item(row+1, 0);
        double d = LMATH::calc_distance(pItem->nvupoint->latlon, nItem->nvupoint->latlon);
        ui->tableWidget->item(row - 1, 6)->setText(QString::number(d, 'f', 1));
    }

    ui->tableWidget->removeRow(row);

    updateDistanceAndN();
}

void MainWindow::deleteCurrentWaypoint()
{
    deleteWaypoint(ui->tableWidget->currentRow());
    //ui->tableWidget->deleteWaypoint(ui->tableWidget->currentRow());
}

void MainWindow::updateDistanceAndN()
{
    QTableWidgetItemData* iN;
    QTableWidgetItemData* iC;
    double currentDistance = 0.0;
    double legDistance;

    if(ui->tableWidget->rowCount()>0)
    {
        ui->tableWidget->item(0, 7)->setText("0.0");
        ui->tableWidget->item(ui->tableWidget->rowCount()-1, 7)->setText(QString::number(currentDistance, 'f', 1));
        ui->tableWidget->item(ui->tableWidget->rowCount()-1, 0)->setText(QString::number(ui->tableWidget->rowCount()));

        iN = (QTableWidgetItemData*) ui->tableWidget->item(0, 0);
        iC = (QTableWidgetItemData*) ui->tableWidget->item(ui->tableWidget->rowCount()-1, 0);
        double f = LMATH::calc_fork(iN->nvupoint->latlon.x, iN->nvupoint->latlon.y, iN->nvupoint->alt, iC->nvupoint->latlon.x, iC->nvupoint->latlon.y, iC->nvupoint->alt, dat);
        ui->labelFork->setText("Fork   " + QString::number(f, 'f', 1));
    }
    else ui->labelFork->setText("Fork   0.0");

    for(int i=0; i<ui->tableWidget->rowCount() - 1; i++)
    {
        iC = (QTableWidgetItemData*) ui->tableWidget->item(i, 0);
        iN = (QTableWidgetItemData*) ui->tableWidget->item(i+1, 0);

        legDistance = LMATH::calc_distance(iN->nvupoint->latlon, iC->nvupoint->latlon);
        ui->tableWidget->item(i, 6)->setText(QString::number(legDistance, 'f', 1));
        ui->tableWidget->item(i, 7)->setText(QString::number(currentDistance, 'f', 1));
        ui->tableWidget->item(i, 0)->setText(QString::number(i+1));
        currentDistance+=legDistance;
    }

    if(ui->tableWidget->rowCount()>0) ui->tableWidget->item(ui->tableWidget->rowCount()-1, 7)->setText(QString::number(currentDistance, 'f', 1));
}

void MainWindow::tableGoUp()
{
    int c = ui->tableWidget->currentRow();
    c--;
    if(c<0) return;

    ui->tableWidget->selectRow(c);
/*
    QTableWidgetItemData* itemD = (QTableWidgetItemData*) ui->tableWidget->item(c, 0);
    ui->lineEdit->setText("");
    ui->lineEdit->setText(itemD->nvupoint->name);
    setWaypointDescription(itemD->nvupoint);
    */
}

void MainWindow::tableGoDown()
{
    int c = ui->tableWidget->currentRow();
    c++;
    if(c>=ui->tableWidget->rowCount()) return;

    ui->tableWidget->selectRow(c);

/*
    QTableWidgetItemData* itemD = (QTableWidgetItemData*) ui->tableWidget->item(c, 0);
    ui->lineEdit->setText("");
    ui->lineEdit->setText(itemD->nvupoint->name);
    setWaypointDescription(itemD->nvupoint);

    */
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
    QString sSearch = ui->lineEdit->text();
    ui->listWidget->search(sSearch, true);

    /*
    sSearch = sSearch.toUpper();
    std::vector<NVUPOINT*> lWP = XFMS_DATA::search(sSearch);

    ui->listWidget->clear();
    if(ui->lineEdit->text().length()<1) return;
    for(int i=0; i<lWP.size(); i++)
    {
        QListWidgetItemData *newItem = new QListWidgetItemData;
        QString qstr = lWP[i]->name;
        if(!lWP[i]->name2.isEmpty()) qstr = qstr + " - " + lWP[i]->name2;
        if(!lWP[i]->country.isEmpty()) qstr = qstr + " [" + lWP[i]->country + "]";
        newItem->setText(qstr);
        newItem->nvupoint = lWP[i];
        ui->listWidget->addItem(newItem);
    }
    */
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *_item)
{
    QListWidgetItemData* itemData = (QListWidgetItemData*) _item;
    //ui->tableWidget->insertWaypoint(itemData->nvupoint, ui->tableWidget->rowCount());
    insertWaypoint(itemData->nvupoint, ui->tableWidget->rowCount(), 0);
    ui->tableWidget->selectRow(ui->tableWidget->rowCount()-1);
}

void MainWindow::refreshRow(int row, NVUPOINT* waypoint)
{
    if(row>=ui->tableWidget->rowCount()) return;


    QString qstr;
    QTableWidgetItemData* itemD;

    itemD = (QTableWidgetItemData*) ui->tableWidget->item(row, 0);
    itemD->setText(QString::number(row));
    if(waypoint==NULL) waypoint = itemD->nvupoint;
    itemD->nvupoint = (NVUPOINT*) waypoint;
    ui->tableWidget->item(row, 1)->setText(waypoint->name);

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
    ui->tableWidget->item(row, 2)->setText(qstr);

    ui->tableWidget->item(row, 3)->setText(QString::number(waypoint->alt));//LMATH::feetToMeter(waypoint->alt)));

    double l1, l2;
    l1 = fabs(modf(waypoint->latlon.x, &l2)*60.0);
    int i2 = (int) fabs(l2);
    ui->tableWidget->item(row, 4)->setText((waypoint->latlon.x < 0 ? "S" : "N") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));

    l1 = fabs(modf(waypoint->latlon.y, &l2)*60.0);
    i2 = (int) fabs(l2);
    itemD->setText((waypoint->latlon.y < 0 ? "W" : "E") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));
    ui->tableWidget->item(row, 5)->setText((waypoint->latlon.y < 0 ? "W" : "E") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));

    ui->tableWidget->item(row, 6)->setText("");
    ui->tableWidget->item(row, 7)->setText("");

    if(waypoint->rsbn)
    {
        double d = LMATH::calc_distance(waypoint->latlon, waypoint->rsbn->latlon);
        qstr =        waypoint->rsbn->name +
                      (waypoint->rsbn->country.isEmpty() ? "" : + " (" + waypoint->rsbn->country + ")") +
                      (waypoint->rsbn->name2.isEmpty() ? "" : "  " + waypoint->rsbn->name2) +
                      " (" + QString::number(d, 'f', 0) + " KM)";
    }
    else qstr = "NO CORRECTION";
    ui->tableWidget->item(row, 8)->setText(qstr);
    //updateDistanceAndN();
}

void MainWindow::insertTableWidgetWaypoint(NVUPOINT* waypoint, int row)
{
    ui->tableWidget->insertRow(row);
    for(int i=0; i<9; i++)
    {
        ui->tableWidget->setItem(row, i, new QTableWidgetItemData);
    }

    refreshRow(row, waypoint);
    updateDistanceAndN();
}

/*
void MainWindow::insertTableWidgetWaypoint(NVUPOINT* waypoint, int row)
{
    ui->tableWidget->insertRow(row);
    QString qstr;
    QFont font;

    QTableWidgetItemData* itemD = new QTableWidgetItemData;
    itemD->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    itemD->setText(QString::number(row));
    itemD->nvupoint = (NVUPOINT*) waypoint;
    ui->tableWidget->setItem(row, 0, itemD);

    itemD = new QTableWidgetItemData;
    itemD->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    font = itemD->font();
    font.setBold(true);
    itemD->setFont(font);
    itemD->setText(waypoint->name);
    ui->tableWidget->setItem(row, 1, itemD);

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
    ui->tableWidget->setItem(row, 2, item);

    item = new QTableWidgetItemData;
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    double l1, l2;
    l1 = fabs(modf(waypoint->latlon.x, &l2)*60.0);
    int i2 = (int) fabs(l2);
    item->setText((waypoint->latlon.x < 0 ? "S" : "N") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));
    ui->tableWidget->setItem(row, 3, item);

    item = new QTableWidgetItemData;
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    l1 = fabs(modf(waypoint->latlon.y, &l2)*60.0);
    i2 = (int) fabs(l2);
    item->setText((waypoint->latlon.y < 0 ? "W" : "E") + QString::number(i2) + "*" + (l1<10 ? "0" : "") + QString::number(l1, 'f', 2));
    ui->tableWidget->setItem(row, 4, item);

    item = new QTableWidgetItemData;
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    item->setText("0.0");
    ui->tableWidget->setItem(row, 5, item);

    item = new QTableWidgetItemData;
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    item->setText("");
    ui->tableWidget->setItem(row, 6, item);

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
    ui->tableWidget->setItem(row, 7, item);

    updateDistanceAndN();
}
*/


void MainWindow::printPreview(QPrinter* printer)
{
    printer->setPageSize(QPrinter::A4);
    printer->setOrientation(QPrinter::Landscape);
    printer->setPageMargins (10,10,10,10,QPrinter::Millimeter);
    printer->setFullPage(false);
    //printer->setOutputFileName("output.pdf");
    //printer->setOutputFormat(QPrinter::PdfFormat); //you can use native format of system usin QPrinter::NativeFormat

    if(nvu.lWPs.size()<1) return;
    QPainter painter(printer); // create a painter which will paint 'on printer'.

#ifdef _WIN32
    painter.scale(0.5, 0.5);
#endif

    int y=0;
    drawNVUHeader(painter, y);
    y+=30;
    for(int i=0; i<nvu.lWPs.size(); i++)
    {
        if(i==19)
        {
            printer->newPage();
            y = 0;
            drawNVUHeader(painter, y);
            y+=30;
        }
        painterDrawNVUPoint(painter, nvu.lWPs[i], i+1, i == (nvu.lWPs.size() - 1), y);

    }

    painter.end();
}

void MainWindow::on_pushButtonInsertBefore_clicked()
{
    if(ui->listWidget->currentRow()<0) return;

    QListWidgetItemData* itemData = (QListWidgetItemData*)ui->listWidget->currentItem();
    if(itemData==NULL) return;

    insertWaypoint(itemData->nvupoint, ui->tableWidget->currentRow(), -1);

    /*
    if(ui->tableWidget->currentRow()<0) return;
    insertTableWidgetWaypoint(itemData->nvupoint, ui->tableWidget->currentRow());
    ui->tableWidget->selectRow(ui->tableWidget->currentRow()-1);
    */
}

void MainWindow::on_pushButtonReplace_clicked()
{
    if(ui->listWidget->currentRow()<0) return;
    QListWidgetItemData* itemData = (QListWidgetItemData*)ui->listWidget->currentItem();
    if(itemData==NULL) return;

    insertWaypoint(itemData->nvupoint, ui->tableWidget->currentRow(), 0);

/*
    if(ui->tableWidget->currentRow()<0) return;
    int row = ui->tableWidget->currentRow();
    replaceTableWidgetWaypoint(itemData->nvupoint, row);
    */
}


void MainWindow::on_pushButtonInsertAfter_clicked()
{

    if(ui->listWidget->currentRow()<0) return;
    QListWidgetItemData* itemData = (QListWidgetItemData*)ui->listWidget->currentItem();
    if(itemData==NULL) return;

    insertWaypoint(itemData->nvupoint, ui->tableWidget->currentRow(), 1);

    /*
    int row;
    if(ui->tableWidget->currentRow()<0) row = ui->tableWidget->rowCount();
    else row = ui->tableWidget->currentRow()+1;
    insertTableWidgetWaypoint(itemData->nvupoint, row);
    ui->tableWidget->selectRow(row);
    */

}

void MainWindow::insertWaypoint(NVUPOINT* wp, int row, int offset)
{
    if(wp->type == WAYPOINT::TYPE_AIRWAY) return;
    if(row<0) row = 0;

    if(offset<0)
    {
        insertTableWidgetWaypoint(wp, row);
        ui->tableWidget->selectRow(row);
    }
    else if(offset>0)
    {
        if(ui->tableWidget->currentRow()<0) row = 0;
        else row++;
        if(row>ui->tableWidget->rowCount()) row = ui->tableWidget->rowCount();
        insertTableWidgetWaypoint(wp, row);
        ui->tableWidget->selectRow(row);
    }
    else
    {
        //if(row == ui->tableWidget->rowCount() && row>0) row++;
        ui->tableWidget->removeRow(row);
        insertTableWidgetWaypoint(wp, row);
        ui->tableWidget->selectRow(row);
    }
}

void MainWindow::on_tableWidget_clicked(const QModelIndex &index)
{
    if(ui->tableWidget->currentRow()<0) return;

    QTableWidgetItemData* itemData = (QTableWidgetItemData*) ui->tableWidget->item(ui->tableWidget->currentRow(), 0);
    if(itemData==NULL) return;

    ui->lineEdit->setText("");
    ui->lineEdit->setText(itemData->nvupoint->name);
    setWaypointDescription(itemData->nvupoint);
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
    else ui->labelWPMagVar->setText("Magnetic Declination: " + QString::number(wp->MD, 'f', 1));


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
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_XNVU_TEMP)
    {
        ui->labelWPNote->setText("Source: Custom user tempory waypoint");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_RSBN)
    {
        ui->labelWPNote->setText("Source: rsbn.dat (RSBN library)");
    }
    else if(wp->wpOrigin == WAYPOINT::ORIGIN_XNVU_FLIGHTPLAN)
    {
        ui->labelWPNote->setText("Source: Imported from XNVU flightplan");
    }
}

void MainWindow::drawNVUHeader(QPainter& painter, int& y)
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
    double xscale;
    QString qstr;



    QPen gridPen(Qt::black, 0, Qt::SolidLine);
    painter.setPen(gridPen);
    painter.setBrush(QColor(207, 207, 207));
    QFont font = QFont("FreeSans");
    font.setBold(true);
    font.setPixelSize(fSize);
    QFontMetrics fM(font);
    painter.setFont(font);

    //Draw route name
    QString routeName = nvu.lWPs[0]->name + " - " + nvu.lWPs[nvu.lWPs.size()-1]->name;
    if(nvu.lWPs[0]->name2.length()>0 && nvu.lWPs[nvu.lWPs.size()-1]->name2.length()>0)
    {
        routeName = routeName + "   ( " + nvu.lWPs[0]->name2 + " - " + nvu.lWPs[nvu.lWPs.size()-1]->name2 + " )";
    }
    painter.drawText(x, y, routeName);





    font.setBold(false);
    painter.setFont(font);

    //Draw version
    painter.drawText(11300, y, XNVU_VERSION);



    //Draw distance and fork
    y+=260;
    painter.drawText(10, y, "Distance, km.");
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(2000, y, QString::number(nvu.lWPs[0]->Srem, 'f', 1));
    font.setBold(false);
    painter.setFont(font);
    painter.drawText(4000, y, "Fork, deg.");
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(6000, y, QString::number(nvu.NVU_FORK, 'f', 1));
    y+=40;
    painter.drawRect(2000, y, 1000, 5);
    painter.drawRect(6000, y, 1000, 5);

    //Draw time and date
    painter.drawText(10900, y, QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd    hh:mm:ss") + "   UTC");


    y+=90;

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
    painter.setPen(Qt::black);
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
    double xscale;
    QString qstr;
    QColor red(143, 30, 30);
    QColor blue(6, 6, 131);


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
    font.setBold(true);
    fM = QFontMetrics(font);
    painter.setFont(font);
    painter.drawRect(x, y, rectW*xscale, rectH);
    dx = fM.boundingRect(wp->name).width();
    dx = (rectW*xscale)/2 - dx/2;
    painter.drawText(x + dx, y + fHOffset, wp->name);
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
    painter.setPen(QColor(0, 0, 0));
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
    painter.setPen(Qt::black);
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
    std::vector< std::pair<NVUPOINT*, NVUPOINT*> > lWP;
    for(int i=0; i<ui->tableWidget->rowCount(); i++)
    {
        QTableWidgetItemData* itemD = (QTableWidgetItemData*) ui->tableWidget->item(i, 0);
        lWP.push_back(std::make_pair(itemD->nvupoint, (NVUPOINT*) itemD->nvupoint->rsbn));
    }

    nvu.clear();
    nvu.generate(lWP, dat);
    //if(nvu.lWPs.size()<2) return;


    QPrinter printer(QPrinter::HighResolution); //create your QPrinter (don't need to be high resolution, anyway)
    QPrintPreviewDialog dialog(&printer);
    dialog.setWindowFlags(Qt::Window);
    dialog.setWindowTitle("Print NVU-plan");
    connect(&dialog, SIGNAL(paintRequested(QPrinter *)), SLOT(printPreview(QPrinter *)));
    dialog.exec();
}

void MainWindow::on_tableWidget_itemSelectionChanged()
{
    int c = ui->tableWidget->currentRow();
    if(c<0 || c>=ui->tableWidget->rowCount()) return;

    QTableWidgetItemData* itemD = (QTableWidgetItemData*) ui->tableWidget->item(c, 0);
    //ui->lineEdit->setText("");
    //ui->lineEdit->setText(itemD->nvupoint->name);
    //setWaypointDescription(itemD->nvupoint);
}

void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QTableWidgetItemData* itemD = (QTableWidgetItemData*) ui->tableWidget->item(row, 0);
    DialogWaypointEdit dEdit(itemD->nvupoint, true);
    int dr = dEdit.exec();
    if(dr == QDialog::Rejected) return;

    if(dr==2) //Create new
    {
        XFMS_DATA::addXNVUWaypoint(dEdit.nvupoint);
    }
    else if(dr==3)
    {
        XFMS_DATA::addXNVUWaypointTempory(dEdit.nvupoint);
    }
    dEdit.nvupoint->rsbn = NULL;
    insertWaypoint(dEdit.nvupoint, row, 0);
}

void MainWindow::on_pushButton_ClearFlightplan_clicked()
{
    clearFlightplan();
}

void MainWindow::clearFlightplan()
{
    while(ui->tableWidget->rowCount()>0) ui->tableWidget->removeRow(0);
    XFMS_DATA::removeFMS();
    XFMS_DATA::removeXNVUFlightplan();
}

void MainWindow::refreshFlightplan()
{
    int i=0;
    for(int i=0; i<ui->tableWidget->rowCount(); i++)
    {
        refreshRow(i);
    }

    updateDistanceAndN();
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

    insertRoute(route, ui->tableWidget->currentRow(), 1);
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

    insertRoute(route, ui->tableWidget->currentRow(), -1);
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

    insertRoute(route, ui->tableWidget->currentRow(), 0);
}

void MainWindow::insertRoute(std::vector<NVUPOINT*> route, int row, int offset)
{
    row = row -  (offset < 0 ? 1 : 0);
    for(int i=0; i<route.size(); i++)
    {
        if(i == 0 && offset==0) insertWaypoint(route[i], row, 0);
        else insertWaypoint(route[i], row, 1);
        row = ui->tableWidget->currentRow();
    }
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
            //Copy flightplan WPS points and set them as tempory
            for(int i=0; i<ui->tableWidget->rowCount(); i++)
            {
                QTableWidgetItemData* cI = (QTableWidgetItemData*) ui->tableWidget->item(i, 0);
                NVUPOINT* cP = cI->nvupoint;
                for(int k=0; k<dEdit.lRemove.size(); k++)
                {
                    if(cP == dEdit.lRemove[k])
                    {
                        NVUPOINT* nP = new NVUPOINT(*cP);
                        nP->wpOrigin = WAYPOINT::ORIGIN_XNVU_TEMP;
                        cI->nvupoint = nP;
                        break;
                    }
                }
            }

            XFMS_DATA::removeWPSPoints(dEdit.lRemove);
        }

        refreshFlightplan();

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
    ui->tableWidget->horizontalHeader()->hideSection(4);
    for(int i=0; i<ui->tableWidget->rowCount(); i++)
    {
        QTableWidgetItemData* iD = (QTableWidgetItemData* ) ui->tableWidget->item(i, 0);
        NVUPOINT* p = iD->nvupoint;
        if(i==0 || i == (ui->tableWidget->rowCount()-1)) p->alt = p->elev;
        else p->alt = ui->spinBoxFL->value();

    }

    refreshFlightplan();
}
