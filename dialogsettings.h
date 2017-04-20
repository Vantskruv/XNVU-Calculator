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

private:
    Ui::DialogSettings *ui;

public:
    static void loadSettings();
    static void saveSettings();
    //static bool isChanged();


    static bool XP11;
    static bool XP11_includeCustomAirports;
    static QString xDir;
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
    //static int windowWidth;
    //static int windowHeight;
    static bool distAlignWPS;
    static bool distAlignFMS;
    static bool distAlignATS;
    static bool distAlignEarthNav;
    static double distAlignMargin;
    static bool nightMode;
    static bool showFeet;
    static int cruiseFormat;
    static int VSFormat;
    static int TWCFormat;
    static bool customDateIsTrue;
    static QString customDate;
/*
    static bool showN;
    static bool showID;
    static bool showType;
    static bool showAlt;
    static bool showLat;
    static bool showLon;
    static bool showS;
    static bool showSpas;
    static bool showSrem;
    static bool showMD;
    static bool showOZMPUv;
    static bool showOZMPUp;
    static bool showPv;
    static bool showPp;
    static bool showMPU;
    static bool showIPU;
    static bool showRSBN;
    static bool showSm;
    static bool showZm;
    static bool showMapAngle;
    static bool showAtarg;
    static bool showDtarg;
*/
    /*
    static int showN_size;
    static int showID_size;
    static int showType_size;
    static int showAlt_size;
    static int showLat_size;
    static int showLon_size;
    static int showS_size;
    static int showSpas_size;
    static int showSrem_size;
    static int showMD_size;
    static int showOZMPUv_size;
    static int showOZMPUp_size;
    static int showPv_size;
    static int showPp_size;
    static int showMPU_size;
    static int showIPU_size;
    static int showRSBN_size;
    static int showSm_size;
    static int showZm_size;
    static int showMapAngle_size;
    static int showAtarg_size;
    static int showDtarg_size;

    static int showN_pos;
    static int showID_pos;
    static int showType_pos;
    static int showAlt_pos;
    static int showLat_pos;
    static int showLon_pos;
    static int showS_pos;
    static int showSpas_pos;
    static int showSrem_pos;
    static int showMD_pos;
    static int showOZMPUv_pos;
    static int showOZMPUp_pos;
    static int showPv_pos;
    static int showPp_pos;
    static int showMPU_pos;
    static int showIPU_pos;
    static int showRSBN_pos;
    static int showSm_pos;
    static int showZm_pos;
    static int showMapAngle_pos;
    static int showAtarg_pos;
    static int showDtarg_pos;
*/

/*
    static QString _xDir;
    static QString _fileAirports;
    static QString _fileNavaids;
    static QString _fileWaypoints;
    static QString _fileAirways;
    static QString _fileNavdata;
    static bool _manualSetDir;
    static QString _fileRSBN;
    static bool _correctionVORDME;
    static int _beaconDistance;
    static int _windowWidth;
    static int _windowHeight;
    static bool _distAlignWPS;
    static bool _distAlignFMS;
    static bool _distAlignATS;
    static bool _distAlignEarthNav;
    static double _distAlignMargin;
    static bool _showFeet;

    static bool _showN;
    static bool _showID;
    static bool _showType;
    static bool _showAlt;
    static bool _showLat;
    static bool _showLon;
    static bool _showS;
    static bool _showSpas;
    static bool _showSrem;
    static bool _showMD;
    static bool _showOZMPUv;
    static bool _showOZMPUp;
    static bool _showPv;
    static bool _showPp;
    static bool _showMPU;
    static bool _showIPU;
    static bool _showRSBN;
    static bool _showSm;
    static bool _showZm;
    static bool _showMapAngle;
    static bool _showAtarg;
    static bool _showDtarg;
    */

};

#endif // DIALOGSETTINGS_H
