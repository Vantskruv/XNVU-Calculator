#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QtPrintSupport/QtPrintSupport>
#include <NVU.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void quit();
    void showFlightplanContextMenu(const QPoint& pos);
    void showXPlaneSettings();
    void importFMS();
    void exportFMS();
    void loadNVUFlightplan();
    void saveNVUFlightPlan();
    void tableGoUp();
    void tableGoDown();
    void updateDistanceAndN();
    void insertRoute(std::vector<NVUPOINT*> route, int row, int offset);
    void insertWaypoint(NVUPOINT* wp, int row, int offset);
    void insertTableWidgetWaypoint(NVUPOINT* waypoint, int row);
    //void replaceTableWidgetWaypoint(NVUPOINT* nvupoint, int row);
    void deleteWaypoint(int row);
    void deleteCurrentWaypoint();
    void setWaypointDescription(NVUPOINT* wp);

    void printPreview(QPrinter*);
    void drawNVUHeader(QPainter& painter, int& y);
    void painterDrawNVUPoint(QPainter& painter, NVUPOINT*wp, int wpNumber, bool isArr, int &y);



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

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
