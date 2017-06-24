#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QPrinter>
#include <QtPrintSupport>
#include <NVU.h>
#include "XFMS_DATA.h"
#include "dialogsettings.h"
#include "qlineeditwp.h"
#include <qlabelclick.h>

#define XNVU_VERSION    "XNVU version 0.371"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    //void calculateTOD();
    void fplDataChangeCheck();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void finished();

private slots:
    void quit();
    void showFlightplanContextMenu(const QPoint& pos);
    void showSearchListContextMenu(const QPoint& pos);
    void showXPlaneSettings();
    void importFMS();
    void exportFMS();
    void importFMS_KLN90B();
    void exportFMS_KLN90B();
    void printOnPDF();
    void loadNVUFlightplan();
    void saveNVUFlightPlan();
    void tableGoUp();
    void tableGoDown();
    void goDirectToFieldClicked(QLineEditWP *wp);
    void clickedDataLabels(QLabelClick *_label);

    void deleteCurrentWaypoint();
    void setWaypointDescription(NVUPOINT* wp);

    void printPreview(QPrinter*);
    void drawNVUHeader(QPainter& painter, NVUPOINT *dep, NVUPOINT *arr, double fork, int& y);
    void painterDrawSummary(QPainter& painter, std::vector<NVUPOINT*>& lWP, int y);
    void painterDrawNVUPoint(QPainter& painter, NVUPOINT*wp, int wpNumber, bool isArr, int &y);
    void clearFlightplanTimeout();


    void on_lineEdit_textChanged(const QString &arg1);
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButtonPrint_clicked();

    void on_pushButtonInsertBefore_clicked();

    void on_pushButtonReplace_clicked();

    void on_pushButtonInsertAfter_clicked();

    void on_tableWidget_clicked(const QModelIndex &index);

    void on_listWidget_currentRowChanged(int currentRow);

    void on_tableWidget_itemSelectionChanged();

    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_pushButton_ClearFlightplan_clicked();

    void on_pushButtonRouteInsertAfter_clicked();

    void on_pushButtonRouteInsertBefore_clicked();

    void on_pushButtonRouteReplace_clicked();

    void on_actionOptions_triggered();

    void on_frameDescription_clicked();

    //void on_actionXNVU_library_triggered();

    //void on_pushButton_showAIRAC_Airports_clicked();

    //void on_pushButton_showAIRAC_Navaids_clicked();

    //void on_pushButton_showAIRAC_Fixes_clicked();

    void on_pushButton_showAIRAC_Airways_clicked();

    //void on_pushButton_showEarthNav_clicked();

    void on_pushButton_showRSBN_clicked();

    void on_pushButton_showXNVU_clicked();

    void on_pushButton_showNDB_clicked();

    void on_pushButton_showVOR_clicked();

    void on_pushButton_showVORDME_clicked();

    void on_pushButton_showDME_clicked();

    void on_pushButton_showFix_clicked();

    void on_pushButton_showAirports_clicked();

    void on_actionShow_feet_triggered();

    void on_pushButtonDeleteWaypoint_clicked();

    void on_actionColumns_2_triggered();

    void on_actionXNVU_library_triggered();

    void on_dateEdit_userDateChanged(const QDate &date);

    void on_listWidget_clicked(const QModelIndex &index);

    void on_pushButtonDTInsert_clicked();

    void on_pushButtonSetDate_clicked();
    void on_doubleSpinBox_MACH_valueChanged(double arg1);
    void on_doubleSpinBox_VS_valueChanged(double arg1);
    void on_doubleSpinBox_TWC_valueChanged(double arg1);
    void on_doubleSpinBoxFL_valueChanged(double arg1);
    void on_doubleSpinBox_ISA_valueChanged(double arg1);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
