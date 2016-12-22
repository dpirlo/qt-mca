#ifndef SETPMTS_H
#define SETPMTS_H

#include <QDialog>
#include <iostream>
#include "inc/apMCAE.hpp"

using namespace std;

namespace Ui {
class SetPMTs;
}

class SetPMTs : public QDialog
{
    Q_OBJECT

public:
    explicit SetPMTs(QWidget *parent = 0);
    void ConfigurePMTList(QPushButton * button);
    void SetLabelState(bool state, QPushButton * button);
    void accept();
    void reject();
    ~SetPMTs();

private:
    void SetPMTPushButtonList();
    void ClearPMTBoard();

private slots:
    void on_pushButton_select_all_clicked();
    void on_pushButton_select_none_clicked();
    void on_pushButton_01_clicked();
    void on_pushButton_02_clicked();
    void on_pushButton_03_clicked();
    void on_pushButton_04_clicked();
    void on_pushButton_05_clicked();
    void on_pushButton_06_clicked();
    void on_pushButton_07_clicked();
    void on_pushButton_08_clicked();
    void on_pushButton_09_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_12_clicked();
    void on_pushButton_13_clicked();
    void on_pushButton_14_clicked();
    void on_pushButton_15_clicked();
    void on_pushButton_16_clicked();
    void on_pushButton_17_clicked();
    void on_pushButton_18_clicked();
    void on_pushButton_19_clicked();
    void on_pushButton_20_clicked();
    void on_pushButton_21_clicked();
    void on_pushButton_22_clicked();
    void on_pushButton_23_clicked();
    void on_pushButton_24_clicked();
    void on_pushButton_25_clicked();
    void on_pushButton_26_clicked();
    void on_pushButton_27_clicked();
    void on_pushButton_28_clicked();
    void on_pushButton_29_clicked();
    void on_pushButton_30_clicked();
    void on_pushButton_31_clicked();
    void on_pushButton_32_clicked();
    void on_pushButton_33_clicked();
    void on_pushButton_34_clicked();
    void on_pushButton_35_clicked();
    void on_pushButton_36_clicked();
    void on_pushButton_37_clicked();
    void on_pushButton_38_clicked();
    void on_pushButton_39_clicked();
    void on_pushButton_40_clicked();
    void on_pushButton_41_clicked();
    void on_pushButton_42_clicked();
    void on_pushButton_43_clicked();
    void on_pushButton_44_clicked();
    void on_pushButton_45_clicked();
    void on_pushButton_46_clicked();
    void on_pushButton_47_clicked();
    void on_pushButton_48_clicked();

private:
    Ui::SetPMTs *ui;
    QList<QString> pmt_selected_list;
    QList<QPushButton*> pmt_button_table;

public:
    QList<QString> GetPMTSelectedList() const { return pmt_selected_list; }

};

#endif // SETPMTS_H
