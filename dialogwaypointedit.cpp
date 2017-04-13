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

    if(_nvuPoint) nvupoint = *_nvuPoint;

    QFont font = ui->lineEdit_Identifier->font();
    font.setCapitalization(QFont::AllUppercase);
    ui->lineEdit_Identifier->setFont(font);
    ui->lineEdit_Name->setFont(font);
    ui->lineEdit_Country->setFont(font);

    ui->doubleSpinBox_Freq->setDisabled(true);
    ui->spinBox_Range->setDisabled(true);
    ui->doubleSpinBox_AngleDev->setDisabled(true);
    ui->spinBox_TransAlt->setDisabled(true);
    ui->spinBox_TransLevel->setDisabled(true);
    ui->spinBox_LongestRwy->setDisabled(true);
    if(isSave) ui->pushButton_SaveCurrent->setDisabled(false);
    else ui->pushButton_SaveCurrent->setDisabled(true);


    std::vector<QString> lTypes = WAYPOINT::getTypeStrList();
    for(int i=0; i<lTypes.size(); i++) ui->comboBox_Type->addItem(lTypes[i]);

    ui->lineEdit_Identifier->setText(nvupoint.name);
    ui->lineEdit_Name->setText(nvupoint.name2);
    ui->lineEdit_Country->setText(nvupoint.country);

    ui->doubleSpinBox_Lat->setValue(nvupoint.latlon.x);
    ui->doubleSpinBox_Lon->setValue(nvupoint.latlon.y);
    ui->spinBox_Elev->setValue(nvupoint.elev);
    ui->spinBox_Alt->setValue(nvupoint.alt);

    //TODO: Bad coding, very bad coding. Need to reprogram.
    ui->comboBox_Type->setCurrentIndex(nvupoint.type-1);
    if(nvupoint.type == WAYPOINT::TYPE_AIRPORT ||
       nvupoint.type == WAYPOINT::TYPE_NDB ||
       nvupoint.type == WAYPOINT::TYPE_VORDME ||
       nvupoint.type == WAYPOINT::TYPE_VOR ||
       nvupoint.type == WAYPOINT::TYPE_ILS ||
       nvupoint.type == WAYPOINT::TYPE_DME ||
       nvupoint.type == WAYPOINT::TYPE_RSBN ||
       nvupoint.type == WAYPOINT::TYPE_FIX ||
       nvupoint.type == WAYPOINT::TYPE_LATLON ||
       nvupoint.type == WAYPOINT::TYPE_AIRWAY ||
       nvupoint.type == WAYPOINT::TYPE_TACAN ||
       nvupoint.type == WAYPOINT::TYPE_VORTAC) ui->comboBox_Type->setCurrentIndex(nvupoint.type-1);
    else ui->comboBox_Type->setCurrentIndex(WAYPOINT::TYPE_FIX-1);

    ui->doubleSpinBox_Freq->setValue(nvupoint.freq);
    ui->spinBox_Range->setValue(nvupoint.range);
    ui->doubleSpinBox_AngleDev->setValue(nvupoint.ADEV);
    ui->spinBox_TransAlt->setValue(nvupoint.trans_alt);
    ui->spinBox_TransLevel->setValue(nvupoint.trans_level);
    ui->spinBox_LongestRwy->setValue(nvupoint.longest_runway);
}

DialogWaypointEdit::~DialogWaypointEdit()
{
    delete ui;
}

void DialogWaypointEdit::on_comboBox_Type_currentIndexChanged(int index)
{
    index++;
    if(index == WAYPOINT::TYPE_DME ||
       index == WAYPOINT::TYPE_NDB ||
       index == WAYPOINT::TYPE_VOR ||
       index == WAYPOINT::TYPE_VORDME ||
       index == WAYPOINT::TYPE_ILS ||
       index == WAYPOINT::TYPE_RSBN ||
       index == WAYPOINT::TYPE_VORTAC ||
       index == WAYPOINT::TYPE_TACAN)
    {
        ui->spinBox_TransAlt->setDisabled(true);
        ui->spinBox_TransLevel->setDisabled(true);
        ui->spinBox_LongestRwy->setDisabled(true);

        ui->doubleSpinBox_Freq->setDisabled(false);
        //ui->spinBox_Range->setDisabled(false);
        //ui->doubleSpinBox_AngleDev->setDisabled(false);
    }
    else if(index == WAYPOINT::TYPE_AIRPORT)
    {
        ui->doubleSpinBox_Freq->setDisabled(true);
        //ui->spinBox_Range->setDisabled(true);
        //ui->doubleSpinBox_AngleDev->setDisabled(true);

        ui->spinBox_TransAlt->setDisabled(false);
        ui->spinBox_TransLevel->setDisabled(false);
        ui->spinBox_LongestRwy->setDisabled(false);
   }
   else
   {
        ui->doubleSpinBox_Freq->setDisabled(true);
        ui->spinBox_TransAlt->setDisabled(true);
        ui->spinBox_TransLevel->setDisabled(true);
        ui->spinBox_LongestRwy->setDisabled(true);
    }
}


void DialogWaypointEdit::on_pushButton_Cancel_clicked()
{
    done(CANCEL);
}

void DialogWaypointEdit::on_pushButton_SaveCurrent_clicked()
{
    nvupoint.name = ui->lineEdit_Identifier->text().toUpper();
    nvupoint.name2 = ui->lineEdit_Name->text().toUpper();
    nvupoint.country = ui->lineEdit_Country->text().toUpper();

    nvupoint.latlon.x = ui->doubleSpinBox_Lat->value();
    nvupoint.latlon.y = ui->doubleSpinBox_Lon->value();
    nvupoint.elev = ui->spinBox_Elev->value();
    nvupoint.alt = ui->spinBox_Alt->value();

    nvupoint.type = ui->comboBox_Type->currentIndex() + 1;
    nvupoint.freq = ui->doubleSpinBox_Freq->value();
    nvupoint.range = ui->spinBox_Range->value();
    nvupoint.ADEV = ui->doubleSpinBox_AngleDev->value();

    nvupoint.trans_alt = ui->spinBox_TransAlt->value();
    nvupoint.trans_level = ui->spinBox_TransLevel->value();
    nvupoint.longest_runway = ui->spinBox_LongestRwy->value();

    done(SAVE);
}

void DialogWaypointEdit::on_pushButton_CreateNew_clicked()
{
    nvupoint.name = ui->lineEdit_Identifier->text().toUpper();
    nvupoint.name2 = ui->lineEdit_Name->text().toUpper();
    nvupoint.country = ui->lineEdit_Country->text().toUpper();

    nvupoint.latlon.x = ui->doubleSpinBox_Lat->value();
    nvupoint.latlon.y = ui->doubleSpinBox_Lon->value();
    nvupoint.elev = ui->spinBox_Elev->value();
    nvupoint.alt = ui->spinBox_Alt->value();

    nvupoint.type = ui->comboBox_Type->currentIndex() + 1;
    nvupoint.freq = ui->doubleSpinBox_Freq->value();
    nvupoint.range = ui->spinBox_Range->value();
    nvupoint.ADEV = ui->doubleSpinBox_AngleDev->value();

    nvupoint.trans_alt = ui->spinBox_TransAlt->value();
    nvupoint.trans_level = ui->spinBox_TransLevel->value();
    nvupoint.longest_runway = ui->spinBox_LongestRwy->value();
    nvupoint.wpOrigin = WAYPOINT::ORIGIN_XNVU;
    done(ADD_XNVU);
}

/*
void DialogWaypointEdit::on_pushButton_CreateTemp_clicked()
{
    NVUPOINT* new_nvupoint = new NVUPOINT();

    new_nvupoint.name = ui->lineEdit_Identifier->text().toUpper();
    new_nvupoint.name2 = ui->lineEdit_Name->text().toUpper();
    new_nvupoint.country = ui->lineEdit_Country->text().toUpper();

    new_nvupoint.latlon.x = ui->doubleSpinBox_Lat->value();
    new_nvupoint.latlon.y = ui->doubleSpinBox_Lon->value();
    new_nvupoint.elev = ui->spinBox_Elev->value();

    new_nvupoint.type = ui->comboBox_Type->currentIndex() + 1;
    new_nvupoint.freq = ui->doubleSpinBox_Freq->value();
    new_nvupoint.range = ui->spinBox_Range->value();
    new_nvupoint.ADEV = ui->doubleSpinBox_AngleDev->value();

    new_nvupoint.trans_alt = ui->spinBox_TransAlt->value();
    new_nvupoint.trans_level = ui->spinBox_TransLevel->value();
    new_nvupoint.longest_runway = ui->spinBox_LongestRwy->value();
    new_nvupoint.wpOrigin = WAYPOINT::ORIGIN_XNVU_TEMP;

    nvupoint = new_nvupoint;

    done(CREATE_TEMP);
}
*/
