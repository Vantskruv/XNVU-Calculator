#ifndef QLABELCLICK_H
#define QLABELCLICK_H

#include <QLabel>

namespace Ui {
class QLabelClick;
}

class QLabelClick : public QLabel
{
    Q_OBJECT

    public:
        explicit QLabelClick(QWidget *parent = 0);
        explicit QLabelClick(const QString text = "", QWidget *parent = 0);

    protected:
        void mousePressEvent(QMouseEvent* event);


    signals:
        void clicked(QLabelClick* _label);

    public slots:
};

#endif // QLABELCLICK_H
