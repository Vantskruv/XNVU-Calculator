#ifndef QRSBNTABLE_H
#define QRSBNTABLE_H

#include <QObject>
#include <QTableWidget>
#include <NVU.h>

class QRSBNTable : public QTableWidget
{
    private:
        std::vector< std::pair<NVUPOINT*, double> > lRSBN;  //A list of RSBNS with their distance to waypoint.

public:
        enum COL{S, ZM, SM, ID, NAME, _SIZE};
        const QStringList COL_STR = (QStringList() << "S" << "Zm" << "Sm" << "ID" << "Name");
        void refresh(NVUPOINT* _nvupoint, NVUPOINT* _nvupoint2, int _dist, bool _showVORDME, NVUPOINT* rsbn = NULL);
        void clearRSBNTable();
        NVUPOINT* getRSBN(int row, bool lastOutOfIndex = false);

        QRSBNTable(QWidget*w);
};

#endif // QRSBNTABLE_H
