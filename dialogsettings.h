#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>

#define XNVU_SETTINGS_FILE "xnvu.dat"

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QWidget *parent = 0);
    ~DialogSettings();

private slots:
    void on_toolButton_6_clicked();
    void on_toolButton_4_clicked();
    void on_lineEditXPlaneDirectory_textChanged(const QString &arg1);
    void on_buttonBox_accepted();
    void on_checkBoxManual_clicked(bool checked);

    void on_toolButton_5_clicked();

private:
    Ui::DialogSettings *ui;

public:
    static void loadSettings();
    static void saveSettings();

    static bool XP11;
    static bool XP11_includeCustomAirports;
    static QString xDir;
    static QString defaultLoadSaveDir;
    static QString customAirportsDir; //TODO: Currently a dummy, either implement it or define it.
    static QString fileAirports;
    static QString fileNavaids;
    static QString fileWaypoints;
    static QString fileAirways;
    static QString fileNavdata;
    static bool manualSetDir;
    static QString fileRSBN;
    static bool correctionVORDME;
    static int beaconDistance;
    static QSize windowSize;
    static QPoint windowPos;
    static QByteArray tableState;
    static bool distAlignXWP;
    static bool distAlignFMS;
    static double distAlignMargin;
    static bool nightMode;              //Only in printed flightplan
    static bool showFeet;               //Only in UI
    //static bool showFeet_PRINT;         //Only in printed flightplan
    static int cruiseFormat;            //Only in UI
    static int VSFormat;                //Only in UI
    static int TWCFormat;               //Only in UI
    static bool customDateIsTrue;
    static QString customDate;
    static bool showTOD_METRIC;         //Only in UI
    //static bool showTOD_METRIC_PRINT;   //Only in printed flightplan
};

#endif // DIALOGSETTINGS_H
