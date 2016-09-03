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
    static bool isChanged();


    static QString xDir;
    static QString fileAirports;
    static QString fileNavaids;
    static QString fileWaypoints;
    static QString fileAirways;
    static QString fileNavdata;
    static bool manualSetDir;
    static QString fileRSBN;
    static bool correctionVORDME;
    static int beaconDistance;
    static int windowWidth;
    static int windowHeight;
    static bool distAlignWPS;
    static bool distAlignFMS;
    static bool distAlignATS;
    static bool distAlignEarthNav;
    static double distAlignMargin;


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

};

#endif // DIALOGSETTINGS_H
