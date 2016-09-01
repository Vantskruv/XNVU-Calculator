#ifndef XNVUSETTINGS_H
#define XNVUSETTINGS_H

#include <QString>

#define XNVU_SETTINGS_FILE "xnvu.dat"

class XNVUSettings
{
    private:

    public:
        QString fileAirports;
        QString fileNavaids;
        QString fileWaypoints;
        QString fileRSBN;
        QString fileNavdata;

        bool loadSettings();
        XNVUSettings();
};

#endif // XNVUSETTINGS_H
