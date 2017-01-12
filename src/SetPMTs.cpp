#include "inc/SetPMTs.h"
#include "ui_SetPMTs.h"

/**
 * @brief SetPMTs::SetPMTs
 *
 * Constructor de la clase.
 * @param parent
 */
SetPMTs::SetPMTs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetPMTs)
{
    ui->setupUi(this);
    SetPMTPushButtonList();
}

/**
 * @brief SetPMTs::~SetPMTs
 *
 * Destructor de la clase.
 */
SetPMTs::~SetPMTs()
{
    delete ui;
}

/**
 * @brief SetPMTs::SetPMTPushButtonList
 *
 * Método privado que llena una lista con los QPushButton's correspondientes a los fotomultiplicadores (48 PMTs).
 */
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

/**
 * @brief SetPMTs::ConfigurePMTList
 *
 * Método público que configura el estado actual de la lista de fotomultiplicadores seleccionados en función del botón presionado.
 * Además cambia también el estado del botón en cuestión.
 * @param button
 * @see pmt_selected_list
 */
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

/**
 * @brief SetPMTs::SetLabelState
 *
 * Método privado que configura el estado del botón presionado, si _state_ es *true*, el botón se pone en color verde, caso contrario toma el color por defecto.
 * @param state
 * @param button
 */
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


/**
 * @brief SetPMTs::accept
 * @overload
 */
void SetPMTs::accept()
{
   QDialog::accept();
}

/**
 * @brief SetPMTs::reject
 * @overload
 */
void SetPMTs::reject()
{
   QDialog::reject();
}

/**
 * @brief SetPMTs::ClearPMTBoard
 *
 * Método privado que elimina la selección actual de PMTs.
 */
void SetPMTs::ClearPMTBoard()
{
    for(int pmt = 0; pmt < PMTs; pmt++)
    {
        pmt_button_table[pmt]->setStyleSheet("");
        pmt_button_table[pmt]->update();
    }

    pmt_selected_list.clear();
}
/**
 * @brief SetPMTs::on_pushButton_select_all_clicked
 */
void SetPMTs::on_pushButton_select_all_clicked()
{
    ClearPMTBoard();
    for(int pmt = 0; pmt < PMTs; pmt++)
    {
        ConfigurePMTList(pmt_button_table[pmt]);
    }
}
/**
 * @brief SetPMTs::on_pushButton_select_none_clicked
 */
void SetPMTs::on_pushButton_select_none_clicked()
{
   ClearPMTBoard();
}
/**
 * @brief SetPMTs::on_pushButton_01_clicked
 */
void SetPMTs::on_pushButton_01_clicked()
{
    ConfigurePMTList(ui->pushButton_01);
}

/**
 * @brief SetPMTs::on_pushButton_02_clicked
 */
void SetPMTs::on_pushButton_02_clicked()
{
    ConfigurePMTList(ui->pushButton_02);
}
/**
 * @brief SetPMTs::on_pushButton_03_clicked
 */
void SetPMTs::on_pushButton_03_clicked()
{
    ConfigurePMTList(ui->pushButton_03);
}
/**
 * @brief SetPMTs::on_pushButton_04_clicked
 */
void SetPMTs::on_pushButton_04_clicked()
{
    ConfigurePMTList(ui->pushButton_04);
}
/**
 * @brief SetPMTs::on_pushButton_05_clicked
 */
void SetPMTs::on_pushButton_05_clicked()
{
    ConfigurePMTList(ui->pushButton_05);
}
/**
 * @brief SetPMTs::on_pushButton_06_clicked
 */
void SetPMTs::on_pushButton_06_clicked()
{
    ConfigurePMTList(ui->pushButton_06);
}
/**
 * @brief SetPMTs::on_pushButton_07_clicked
 */
void SetPMTs::on_pushButton_07_clicked()
{
    ConfigurePMTList(ui->pushButton_07);
}
/**
 * @brief SetPMTs::on_pushButton_08_clicked
 */
void SetPMTs::on_pushButton_08_clicked()
{
    ConfigurePMTList(ui->pushButton_08);
}
/**
 * @brief SetPMTs::on_pushButton_09_clicked
 */
void SetPMTs::on_pushButton_09_clicked()
{
    ConfigurePMTList(ui->pushButton_09);
}
/**
 * @brief SetPMTs::on_pushButton_10_clicked
 */
void SetPMTs::on_pushButton_10_clicked()
{
    ConfigurePMTList(ui->pushButton_10);
}
/**
 * @brief SetPMTs::on_pushButton_11_clicked
 */
void SetPMTs::on_pushButton_11_clicked()
{
    ConfigurePMTList(ui->pushButton_11);
}
/**
 * @brief SetPMTs::on_pushButton_12_clicked
 */
void SetPMTs::on_pushButton_12_clicked()
{
    ConfigurePMTList(ui->pushButton_12);
}
/**
 * @brief SetPMTs::on_pushButton_13_clicked
 */
void SetPMTs::on_pushButton_13_clicked()
{
    ConfigurePMTList(ui->pushButton_13);
}
/**
 * @brief SetPMTs::on_pushButton_14_clicked
 */
void SetPMTs::on_pushButton_14_clicked()
{
    ConfigurePMTList(ui->pushButton_14);
}
/**
 * @brief SetPMTs::on_pushButton_15_clicked
 */
void SetPMTs::on_pushButton_15_clicked()
{
    ConfigurePMTList(ui->pushButton_15);
}
/**
 * @brief SetPMTs::on_pushButton_16_clicked
 */
void SetPMTs::on_pushButton_16_clicked()
{
    ConfigurePMTList(ui->pushButton_16);
}
/**
 * @brief SetPMTs::on_pushButton_17_clicked
 */
void SetPMTs::on_pushButton_17_clicked()
{
    ConfigurePMTList(ui->pushButton_17);
}
/**
 * @brief SetPMTs::on_pushButton_18_clicked
 */
void SetPMTs::on_pushButton_18_clicked()
{
    ConfigurePMTList(ui->pushButton_18);
}
/**
 * @brief SetPMTs::on_pushButton_19_clicked
 */
void SetPMTs::on_pushButton_19_clicked()
{
    ConfigurePMTList(ui->pushButton_19);
}
/**
 * @brief SetPMTs::on_pushButton_20_clicked
 */
void SetPMTs::on_pushButton_20_clicked()
{
    ConfigurePMTList(ui->pushButton_20);
}
/**
 * @brief SetPMTs::on_pushButton_21_clicked
 */
void SetPMTs::on_pushButton_21_clicked()
{
    ConfigurePMTList(ui->pushButton_21);
}
/**
 * @brief SetPMTs::on_pushButton_22_clicked
 */
void SetPMTs::on_pushButton_22_clicked()
{
    ConfigurePMTList(ui->pushButton_22);
}
/**
 * @brief SetPMTs::on_pushButton_23_clicked
 */
void SetPMTs::on_pushButton_23_clicked()
{
    ConfigurePMTList(ui->pushButton_23);
}
/**
 * @brief SetPMTs::on_pushButton_24_clicked
 */
void SetPMTs::on_pushButton_24_clicked()
{
    ConfigurePMTList(ui->pushButton_24);
}
/**
 * @brief SetPMTs::on_pushButton_25_clicked
 */
void SetPMTs::on_pushButton_25_clicked()
{
    ConfigurePMTList(ui->pushButton_25);
}
/**
 * @brief SetPMTs::on_pushButton_26_clicked
 */
void SetPMTs::on_pushButton_26_clicked()
{
    ConfigurePMTList(ui->pushButton_26);
}
/**
 * @brief SetPMTs::on_pushButton_27_clicked
 */
void SetPMTs::on_pushButton_27_clicked()
{
    ConfigurePMTList(ui->pushButton_27);
}
/**
 * @brief SetPMTs::on_pushButton_28_clicked
 */
void SetPMTs::on_pushButton_28_clicked()
{
    ConfigurePMTList(ui->pushButton_28);
}
/**
 * @brief SetPMTs::on_pushButton_29_clicked
 */
void SetPMTs::on_pushButton_29_clicked()
{
    ConfigurePMTList(ui->pushButton_29);
}
/**
 * @brief SetPMTs::on_pushButton_30_clicked
 */
void SetPMTs::on_pushButton_30_clicked()
{
    ConfigurePMTList(ui->pushButton_30);
}
/**
 * @brief SetPMTs::on_pushButton_31_clicked
 */
void SetPMTs::on_pushButton_31_clicked()
{
    ConfigurePMTList(ui->pushButton_31);
}
/**
 * @brief SetPMTs::on_pushButton_32_clicked
 */
void SetPMTs::on_pushButton_32_clicked()
{
    ConfigurePMTList(ui->pushButton_32);
}
/**
 * @brief SetPMTs::on_pushButton_33_clicked
 */
void SetPMTs::on_pushButton_33_clicked()
{
    ConfigurePMTList(ui->pushButton_33);
}
/**
 * @brief SetPMTs::on_pushButton_34_clicked
 */
void SetPMTs::on_pushButton_34_clicked()
{
    ConfigurePMTList(ui->pushButton_34);
}
/**
 * @brief SetPMTs::on_pushButton_35_clicked
 */
void SetPMTs::on_pushButton_35_clicked()
{
    ConfigurePMTList(ui->pushButton_35);
}
/**
 * @brief SetPMTs::on_pushButton_36_clicked
 */
void SetPMTs::on_pushButton_36_clicked()
{
    ConfigurePMTList(ui->pushButton_36);
}
/**
 * @brief SetPMTs::on_pushButton_37_clicked
 */
void SetPMTs::on_pushButton_37_clicked()
{
    ConfigurePMTList(ui->pushButton_37);
}
/**
 * @brief SetPMTs::on_pushButton_38_clicked
 */
void SetPMTs::on_pushButton_38_clicked()
{
    ConfigurePMTList(ui->pushButton_38);
}
/**
 * @brief SetPMTs::on_pushButton_39_clicked
 */
void SetPMTs::on_pushButton_39_clicked()
{
    ConfigurePMTList(ui->pushButton_39);
}
/**
 * @brief SetPMTs::on_pushButton_40_clicked
 */
void SetPMTs::on_pushButton_40_clicked()
{
    ConfigurePMTList(ui->pushButton_40);
}
/**
 * @brief SetPMTs::on_pushButton_41_clicked
 */
void SetPMTs::on_pushButton_41_clicked()
{
    ConfigurePMTList(ui->pushButton_41);
}
/**
 * @brief SetPMTs::on_pushButton_42_clicked
 */
void SetPMTs::on_pushButton_42_clicked()
{
    ConfigurePMTList(ui->pushButton_42);
}
/**
 * @brief SetPMTs::on_pushButton_43_clicked
 */
void SetPMTs::on_pushButton_43_clicked()
{
    ConfigurePMTList(ui->pushButton_43);
}
/**
 * @brief SetPMTs::on_pushButton_44_clicked
 */
void SetPMTs::on_pushButton_44_clicked()
{
    ConfigurePMTList(ui->pushButton_44);
}
/**
 * @brief SetPMTs::on_pushButton_45_clicked
 */
void SetPMTs::on_pushButton_45_clicked()
{
    ConfigurePMTList(ui->pushButton_45);
}
/**
 * @brief SetPMTs::on_pushButton_46_clicked
 */
void SetPMTs::on_pushButton_46_clicked()
{
    ConfigurePMTList(ui->pushButton_46);
}
/**
 * @brief SetPMTs::on_pushButton_47_clicked
 */
void SetPMTs::on_pushButton_47_clicked()
{
    ConfigurePMTList(ui->pushButton_47);
}
/**
 * @brief SetPMTs::on_pushButton_48_clicked
 */
void SetPMTs::on_pushButton_48_clicked()
{
    ConfigurePMTList(ui->pushButton_48);
}


