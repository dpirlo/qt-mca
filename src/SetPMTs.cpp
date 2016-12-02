#include "inc/SetPMTs.h"
#include "ui_SetPMTs.h"

SetPMTs::SetPMTs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetPMTs)
{
    ui->setupUi(this);
    SetPMTPushButtonList();
}

SetPMTs::~SetPMTs()
{
    delete ui;
}

void SetPMTs::SetPMTPushButtonList()
{
    pmt_button_table.push_back(ui->pushButton_01);
    pmt_button_table.push_back(ui->pushButton_02);
    pmt_button_table.push_back(ui->pushButton_03);
    pmt_button_table.push_back(ui->pushButton_04);
    pmt_button_table.push_back(ui->pushButton_05);
    pmt_button_table.push_back(ui->pushButton_06);
    pmt_button_table.push_back(ui->pushButton_07);
    pmt_button_table.push_back(ui->pushButton_08);
    pmt_button_table.push_back(ui->pushButton_09);
    pmt_button_table.push_back(ui->pushButton_10);
    pmt_button_table.push_back(ui->pushButton_11);
    pmt_button_table.push_back(ui->pushButton_12);
    pmt_button_table.push_back(ui->pushButton_13);
    pmt_button_table.push_back(ui->pushButton_14);
    pmt_button_table.push_back(ui->pushButton_15);
    pmt_button_table.push_back(ui->pushButton_16);
    pmt_button_table.push_back(ui->pushButton_17);
    pmt_button_table.push_back(ui->pushButton_18);
    pmt_button_table.push_back(ui->pushButton_19);
    pmt_button_table.push_back(ui->pushButton_20);
    pmt_button_table.push_back(ui->pushButton_21);
    pmt_button_table.push_back(ui->pushButton_22);
    pmt_button_table.push_back(ui->pushButton_23);
    pmt_button_table.push_back(ui->pushButton_24);
    pmt_button_table.push_back(ui->pushButton_25);
    pmt_button_table.push_back(ui->pushButton_26);
    pmt_button_table.push_back(ui->pushButton_27);
    pmt_button_table.push_back(ui->pushButton_28);
    pmt_button_table.push_back(ui->pushButton_29);
    pmt_button_table.push_back(ui->pushButton_30);
    pmt_button_table.push_back(ui->pushButton_31);
    pmt_button_table.push_back(ui->pushButton_32);
    pmt_button_table.push_back(ui->pushButton_33);
    pmt_button_table.push_back(ui->pushButton_34);
    pmt_button_table.push_back(ui->pushButton_35);
    pmt_button_table.push_back(ui->pushButton_36);
    pmt_button_table.push_back(ui->pushButton_37);
    pmt_button_table.push_back(ui->pushButton_38);
    pmt_button_table.push_back(ui->pushButton_39);
    pmt_button_table.push_back(ui->pushButton_40);
    pmt_button_table.push_back(ui->pushButton_41);
    pmt_button_table.push_back(ui->pushButton_42);
    pmt_button_table.push_back(ui->pushButton_43);
    pmt_button_table.push_back(ui->pushButton_44);
    pmt_button_table.push_back(ui->pushButton_45);
    pmt_button_table.push_back(ui->pushButton_46);
    pmt_button_table.push_back(ui->pushButton_47);
    pmt_button_table.push_back(ui->pushButton_48);
}

void SetPMTs::ConfigurePMTList(QPushButton * button)
{
    QString pmt_value=button->text();
    if(pmt_selected_list.contains(pmt_value))
    {
        int index=pmt_selected_list.indexOf(pmt_value);
        pmt_selected_list.removeAt(index);
        SetLabelState(false, button);
    }
    else
    {
        pmt_selected_list.append(pmt_value);
        SetLabelState(true, button);
    }
    qSort(pmt_selected_list);
}


void SetPMTs::SetLabelState(bool state, QPushButton * button)
{
    QString color;

    if (state)
    {
        color="background-color: green";
    }
    else
    {
        color="";
    }
    button->setStyleSheet(color);
    button->update();
}


void SetPMTs::accept()
{
   QDialog::accept();
}

void SetPMTs::reject()
{
   QDialog::reject();
}


void SetPMTs::on_pushButton_select_all_clicked()
{
    ClearPMTBoard();
    for(int pmt = 0; pmt < PMTs; pmt++)
    {
        ConfigurePMTList(pmt_button_table[pmt]);
    }
}

void SetPMTs::on_pushButton_select_none_clicked()
{
   ClearPMTBoard();
}

void SetPMTs::ClearPMTBoard()
{
    for(int pmt = 0; pmt < PMTs; pmt++)
    {
        pmt_button_table[pmt]->setStyleSheet("");
        pmt_button_table[pmt]->update();
    }
    //qDeleteAll(pmt_selected_list.begin(), pmt_selected_list.end());
}

void SetPMTs::on_pushButton_01_clicked()
{
    ConfigurePMTList(ui->pushButton_01);
}

void SetPMTs::on_pushButton_02_clicked()
{
    ConfigurePMTList(ui->pushButton_02);
}

void SetPMTs::on_pushButton_03_clicked()
{
    ConfigurePMTList(ui->pushButton_03);
}

void SetPMTs::on_pushButton_04_clicked()
{
    ConfigurePMTList(ui->pushButton_04);
}

void SetPMTs::on_pushButton_05_clicked()
{
    ConfigurePMTList(ui->pushButton_05);
}

void SetPMTs::on_pushButton_06_clicked()
{
    ConfigurePMTList(ui->pushButton_06);
}

void SetPMTs::on_pushButton_07_clicked()
{
    ConfigurePMTList(ui->pushButton_07);
}

void SetPMTs::on_pushButton_08_clicked()
{
    ConfigurePMTList(ui->pushButton_08);
}

void SetPMTs::on_pushButton_09_clicked()
{
    ConfigurePMTList(ui->pushButton_09);
}

void SetPMTs::on_pushButton_10_clicked()
{
    ConfigurePMTList(ui->pushButton_10);
}

void SetPMTs::on_pushButton_11_clicked()
{
    ConfigurePMTList(ui->pushButton_11);
}

void SetPMTs::on_pushButton_12_clicked()
{
    ConfigurePMTList(ui->pushButton_12);
}

void SetPMTs::on_pushButton_13_clicked()
{
    ConfigurePMTList(ui->pushButton_13);
}

void SetPMTs::on_pushButton_14_clicked()
{
    ConfigurePMTList(ui->pushButton_14);
}

void SetPMTs::on_pushButton_15_clicked()
{
    ConfigurePMTList(ui->pushButton_15);
}

void SetPMTs::on_pushButton_16_clicked()
{
    ConfigurePMTList(ui->pushButton_16);
}

void SetPMTs::on_pushButton_17_clicked()
{
    ConfigurePMTList(ui->pushButton_17);
}

void SetPMTs::on_pushButton_18_clicked()
{
    ConfigurePMTList(ui->pushButton_18);
}

void SetPMTs::on_pushButton_19_clicked()
{
    ConfigurePMTList(ui->pushButton_19);
}

void SetPMTs::on_pushButton_20_clicked()
{
    ConfigurePMTList(ui->pushButton_20);
}

void SetPMTs::on_pushButton_21_clicked()
{
    ConfigurePMTList(ui->pushButton_21);
}

void SetPMTs::on_pushButton_22_clicked()
{
    ConfigurePMTList(ui->pushButton_22);
}

void SetPMTs::on_pushButton_23_clicked()
{
    ConfigurePMTList(ui->pushButton_23);
}

void SetPMTs::on_pushButton_24_clicked()
{
    ConfigurePMTList(ui->pushButton_24);
}

void SetPMTs::on_pushButton_25_clicked()
{
    ConfigurePMTList(ui->pushButton_25);
}

void SetPMTs::on_pushButton_26_clicked()
{
    ConfigurePMTList(ui->pushButton_26);
}

void SetPMTs::on_pushButton_27_clicked()
{
    ConfigurePMTList(ui->pushButton_27);
}

void SetPMTs::on_pushButton_28_clicked()
{
    ConfigurePMTList(ui->pushButton_28);
}

void SetPMTs::on_pushButton_29_clicked()
{
    ConfigurePMTList(ui->pushButton_29);
}

void SetPMTs::on_pushButton_30_clicked()
{
    ConfigurePMTList(ui->pushButton_30);
}

void SetPMTs::on_pushButton_31_clicked()
{
    ConfigurePMTList(ui->pushButton_31);
}

void SetPMTs::on_pushButton_32_clicked()
{
    ConfigurePMTList(ui->pushButton_32);
}

void SetPMTs::on_pushButton_33_clicked()
{
    ConfigurePMTList(ui->pushButton_33);
}

void SetPMTs::on_pushButton_34_clicked()
{
    ConfigurePMTList(ui->pushButton_34);
}

void SetPMTs::on_pushButton_35_clicked()
{
    ConfigurePMTList(ui->pushButton_35);
}

void SetPMTs::on_pushButton_36_clicked()
{
    ConfigurePMTList(ui->pushButton_36);
}

void SetPMTs::on_pushButton_37_clicked()
{
    ConfigurePMTList(ui->pushButton_37);
}

void SetPMTs::on_pushButton_38_clicked()
{
    ConfigurePMTList(ui->pushButton_38);
}

void SetPMTs::on_pushButton_39_clicked()
{
    ConfigurePMTList(ui->pushButton_39);
}

void SetPMTs::on_pushButton_40_clicked()
{
    ConfigurePMTList(ui->pushButton_40);
}

void SetPMTs::on_pushButton_41_clicked()
{
    ConfigurePMTList(ui->pushButton_41);
}

void SetPMTs::on_pushButton_42_clicked()
{
    ConfigurePMTList(ui->pushButton_42);
}

void SetPMTs::on_pushButton_43_clicked()
{
    ConfigurePMTList(ui->pushButton_43);
}

void SetPMTs::on_pushButton_44_clicked()
{
    ConfigurePMTList(ui->pushButton_44);
}

void SetPMTs::on_pushButton_45_clicked()
{
    ConfigurePMTList(ui->pushButton_45);
}

void SetPMTs::on_pushButton_46_clicked()
{
    ConfigurePMTList(ui->pushButton_46);
}

void SetPMTs::on_pushButton_47_clicked()
{
    ConfigurePMTList(ui->pushButton_47);
}

void SetPMTs::on_pushButton_48_clicked()
{
    ConfigurePMTList(ui->pushButton_48);
}


