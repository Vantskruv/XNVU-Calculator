#include "dialogwaypointedit.h"
#include "ui_dialogwaypointedit.h"

DialogWaypointEdit::DialogWaypointEdit(NVUPOINT* _nvuPoint, QWidget *parent) :   QDialog(parent),
    ui(new Ui::DialogWaypointEdit)
{
    ui->setupUi(this);

    nvupoint = _nvuPoint;
    if(!nvupoint)
    {
        done(QDialog::Rejected);
        return;
    }

    ui->doubleSpinBox_Freq->setDisabled(true);
    ui->spinBox_Range->setDisabled(true);
    ui->doubleSpinBox_AngleDev->setDisabled(true);
    ui->spinBox_TransAlt->setDisabled(true);
    ui->spinBox_TransLevel->setDisabled(true);
    ui->spinBox_LongestRwy->setDisabled(true);
    if(nvupoint->wpOrigin == WAYPOINT::ORIGIN_XNVU) ui->pushButton_SaveCurrent->setDisabled(false);
    else ui->pushButton_SaveCurrent->setDisabled(true);


    std::vector<QString> lTypes;
    std::vector<QString> _lTypes = WAYPOINT::getTypeStrList();
    lTypes.push_back("");
    lTypes.insert(lTypes.end(), _lTypes.begin(), _lTypes.end());
    for(int i=0; i<lTypes.size(); i++) ui->comboBox_Type->addItem(lTypes[i]);

    ui->lineEdit_Identifier->setText(nvupoint->name);
    ui->lineEdit_Name->setText(nvupoint->name2);
    ui->lineEdit_Country->setText(nvupoint->country);

    ui->doubleSpinBox_Lat->setValue(nvupoint->latlon.x);
    ui->doubleSpinBox_Lon->setValue(nvupoint->latlon.y);
    ui->spinBox_Elev->setValue(nvupoint->elev);

    ui->comboBox_Type->setCurrentIndex(nvupoint->type);
    ui->doubleSpinBox_Freq->setValue(nvupoint->freq);
    ui->spinBox_Range->setValue(nvupoint->range);
    ui->doubleSpinBox_AngleDev->setValue(nvupoint->ADEV);
    ui->spinBox_TransAlt->setValue(nvupoint->trans_alt);
    ui->spinBox_TransLevel->setValue(nvupoint->trans_level);
    ui->spinBox_LongestRwy->setValue(nvupoint->longest_runway);
}

DialogWaypointEdit::~DialogWaypointEdit()
{
    delete ui;
}

void DialogWaypointEdit::on_comboBox_Type_currentIndexChanged(int index)
{
    if(index == WAYPOINT::TYPE_DME ||
       index == WAYPOINT::TYPE_NDB ||
       index == WAYPOINT::TYPE_VOR ||
       index == WAYPOINT::TYPE_VORDME ||
       index == WAYPOINT::TYPE_RSBN)
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
}


void DialogWaypointEdit::on_pushButton_Cancel_clicked()
{
    this->done(QDialog::Rejected);
}

void DialogWaypointEdit::on_pushButton_SaveCurrent_clicked()
{
    nvupoint->name = ui->lineEdit_Identifier->text();
    nvupoint->name2 = ui->lineEdit_Name->text();
    nvupoint->country = ui->lineEdit_Country->text();

    nvupoint->latlon.x = ui->doubleSpinBox_Lat->value();
    nvupoint->latlon.y = ui->doubleSpinBox_Lon->value();
    nvupoint->elev = ui->spinBox_Elev->value();

    nvupoint->type = ui->comboBox_Type->currentIndex();
    nvupoint->freq = ui->doubleSpinBox_Freq->value();
    //nvupoint->range = ui->spinBox_Range->value();
    //nvupoint->ADEV = ui->doubleSpinBox_AngleDev->value();

    nvupoint->trans_alt = ui->spinBox_TransAlt->value();
    nvupoint->trans_level = ui->spinBox_TransLevel->value();
    nvupoint->longest_runway = ui->spinBox_LongestRwy->value();

    done(1);
}

void DialogWaypointEdit::on_pushButton_CreateNew_clicked()
{
    NVUPOINT* new_nvupoint = new NVUPOINT();

    new_nvupoint->name = ui->lineEdit_Identifier->text();
    new_nvupoint->name2 = ui->lineEdit_Name->text();
    new_nvupoint->country = ui->lineEdit_Country->text();

    new_nvupoint->latlon.x = ui->doubleSpinBox_Lat->value();
    new_nvupoint->latlon.y = ui->doubleSpinBox_Lon->value();
    new_nvupoint->elev = ui->spinBox_Elev->value();

    new_nvupoint->type = ui->comboBox_Type->currentIndex();
    new_nvupoint->freq = ui->doubleSpinBox_Freq->value();
    new_nvupoint->range = ui->spinBox_Range->value();
    new_nvupoint->ADEV = ui->doubleSpinBox_AngleDev->value();

    new_nvupoint->trans_alt = ui->spinBox_TransAlt->value();
    new_nvupoint->trans_level = ui->spinBox_TransLevel->value();
    new_nvupoint->longest_runway = ui->spinBox_LongestRwy->value();
    new_nvupoint->wpOrigin = WAYPOINT::ORIGIN_XNVU;

    nvupoint = new_nvupoint;

    done(2);
}
