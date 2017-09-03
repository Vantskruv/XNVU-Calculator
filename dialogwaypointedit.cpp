#include "dialogwaypointedit.h"
#include "ui_dialogwaypointedit.h"
#include "coremag.h"

//constexpr int DialogWaypointEdit::CANCEL = 0;
//constexpr int DialogWaypointEdit::SAVE_CURRENT = 1;
//constexpr int DialogWaypointEdit::CREATE_NEW = 2;
//constexpr int DialogWaypointEdit::CREATE_TEMP = 3;

DialogWaypointEdit::DialogWaypointEdit(const NVUPOINT* _nvuPoint, bool isSave, QWidget *parent) :   QDialog(parent),
    ui(new Ui::DialogWaypointEdit)
{
    ui->setupUi(this);

    newPoint = new NVUPOINT();
    if(_nvuPoint)
    {
        newPoint->latlon = _nvuPoint->latlon;
        newPoint->type = _nvuPoint->type;
        newPoint->name = _nvuPoint->name;
        newPoint->name2 = _nvuPoint->name2;
        newPoint->range = _nvuPoint->range;
        newPoint->freq = _nvuPoint->freq;
        newPoint->alt = _nvuPoint->alt;
        newPoint->elev = _nvuPoint->elev;
        newPoint->rsbn = _nvuPoint->rsbn;
        newPoint->country = _nvuPoint->country;
        newPoint->MD = _nvuPoint->MD;
        newPoint->ADEV = _nvuPoint->ADEV;
    }//if



    QFont font = ui->lineEdit_Identifier->font();
    font.setCapitalization(QFont::AllUppercase);
    ui->lineEdit_Identifier->setFont(font);
    ui->lineEdit_Name->setFont(font);
    ui->lineEdit_Country->setFont(font);

    ui->doubleSpinBox_Freq->setDisabled(true);
    ui->spinBox_Range->setDisabled(true);
    ui->doubleSpinBox_AngleDev->setDisabled(true);

    if(isSave) ui->pushButton_SaveCurrent->setDisabled(false);
    else ui->pushButton_SaveCurrent->setDisabled(true);


    ui->comboBox_Type->addItem(WAYPOINT::getTypeStr(NULL, WAYPOINT::TYPE_LATLON), QVariant(WAYPOINT::TYPE_LATLON));
    ui->comboBox_Type->addItem(WAYPOINT::getTypeStr(NULL, WAYPOINT::TYPE_FIX), QVariant(WAYPOINT::TYPE_FIX));
    ui->comboBox_Type->addItem(WAYPOINT::getTypeStr(NULL, WAYPOINT::TYPE_NDB), QVariant(WAYPOINT::TYPE_NDB));
    ui->comboBox_Type->addItem(WAYPOINT::getTypeStr(NULL, WAYPOINT::TYPE_VOR), QVariant(WAYPOINT::TYPE_VOR));
    ui->comboBox_Type->addItem(WAYPOINT::getTypeStr(NULL, WAYPOINT::TYPE_DME), QVariant(WAYPOINT::TYPE_DME));
    ui->comboBox_Type->addItem(WAYPOINT::getTypeStr(NULL, WAYPOINT::TYPE_VORDME), QVariant(WAYPOINT::TYPE_VORDME));
    ui->comboBox_Type->addItem(WAYPOINT::getTypeStr(NULL, WAYPOINT::TYPE_ILS), QVariant(WAYPOINT::TYPE_ILS));
    ui->comboBox_Type->addItem(WAYPOINT::getTypeStr(NULL, WAYPOINT::TYPE_TACAN), QVariant(WAYPOINT::TYPE_TACAN));
    ui->comboBox_Type->addItem(WAYPOINT::getTypeStr(NULL, WAYPOINT::TYPE_VORTAC), QVariant(WAYPOINT::TYPE_VORTAC));


    ui->lineEdit_Identifier->setText(newPoint->name);
    ui->lineEdit_Name->setText(newPoint->name2);
    ui->lineEdit_Country->setText(newPoint->country);

    ui->doubleSpinBox_Lat->setValue(newPoint->latlon.x);
    ui->doubleSpinBox_Lon->setValue(newPoint->latlon.y);
    ui->spinBox_Elev->setValue(newPoint->elev);
    ui->spinBox_Alt->setValue(newPoint->alt);

    bool isFound = false;
    for(unsigned int i=0; i<ui->comboBox_Type->count(); i++)
    {
        QVariant variant = ui->comboBox_Type->itemData(i);
        if(variant.toInt() == newPoint->type)
        {
            isFound = true;
            ui->comboBox_Type->setCurrentIndex(i);
        }
    }

    if(!isFound)
    {
        ui->comboBox_Type->setCurrentIndex(0);
    }

    ui->doubleSpinBox_Freq->setValue(newPoint->freq);
    ui->spinBox_Range->setValue(newPoint->range);
    ui->doubleSpinBox_AngleDev->setValue(newPoint->ADEV);
}

DialogWaypointEdit::~DialogWaypointEdit()
{
    delete ui;
}

void DialogWaypointEdit::on_comboBox_Type_currentIndexChanged(int index)
{
}


void DialogWaypointEdit::on_pushButton_Cancel_clicked()
{
    delete newPoint;
    newPoint = NULL;
    done(CANCEL);
}

void DialogWaypointEdit::on_pushButton_SaveCurrent_clicked()
{
    newPoint->name = ui->lineEdit_Identifier->text().toUpper();
    newPoint->name2 = ui->lineEdit_Name->text().toUpper();
    newPoint->country = ui->lineEdit_Country->text().toUpper();

    newPoint->latlon.x = ui->doubleSpinBox_Lat->value();
    newPoint->latlon.y = ui->doubleSpinBox_Lon->value();
    newPoint->elev = ui->spinBox_Elev->value();
    newPoint->alt = ui->spinBox_Alt->value();

    newPoint->freq = ui->doubleSpinBox_Freq->value();
    newPoint->range = ui->spinBox_Range->value();
    newPoint->ADEV = ui->doubleSpinBox_AngleDev->value();

    newPoint->type = ui->comboBox_Type->currentData().toInt();

    done(SAVE);
}

void DialogWaypointEdit::on_pushButton_CreateNew_clicked()
{
    newPoint->name = ui->lineEdit_Identifier->text().toUpper();
    newPoint->name2 = ui->lineEdit_Name->text().toUpper();
    newPoint->country = ui->lineEdit_Country->text().toUpper();

    newPoint->latlon.x = ui->doubleSpinBox_Lat->value();
    newPoint->latlon.y = ui->doubleSpinBox_Lon->value();
    newPoint->elev = ui->spinBox_Elev->value();
    newPoint->alt = ui->spinBox_Alt->value();

    newPoint->freq = ui->doubleSpinBox_Freq->value();
    newPoint->range = ui->spinBox_Range->value();
    newPoint->ADEV = ui->doubleSpinBox_AngleDev->value();

    newPoint->type = ui->comboBox_Type->currentData().toInt();

    newPoint->wpOrigin = WAYPOINT::ORIGIN_XNVU;
    done(ADD_XNVU);
}
