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
    static QString fileNavdata;
    static QString fileRSBN;
    static bool correctionVORDME;
    static int beaconDistance;
    static int windowWidth;
    static int windowHeight;

    static QString _xDir;
    static QString _fileAirports;
    static QString _fileNavaids;
    static QString _fileWaypoints;
    static QString _fileNavdata;
    static QString _fileRSBN;
    static bool _correctionVORDME;
    static int _beaconDistance;
    static int _windowWidth;
    static int _windowHeight;
};

#endif // DIALOGSETTINGS_H
