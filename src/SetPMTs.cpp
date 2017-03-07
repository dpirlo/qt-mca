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
 *
 * @see pmt_selected_list
 */
void SetPMTs::ConfigurePMTList()
{
  pmt_selected_list.clear();
  for(int pmt = 0; pmt < PMTs; pmt++)
    {
      if(pmt_button_table[pmt]->isChecked())
        {
          QString pmt_value=pmt_button_table[pmt]->text();
          pmt_selected_list.append(pmt_value);
        }
    }
}
/**
 * @brief SetPMTs::accept
 * @overload
 */
void SetPMTs::accept()
{   
  ConfigurePMTList();

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
 *
 */
void SetPMTs::ClearPMTBoard()
{
  for(int pmt = 0; pmt < PMTs; pmt++)
    {
      pmt_button_table[pmt]->setChecked(false);
    }
  pmt_selected_list.clear();
}
/**
 * @brief SetPMTs::SetAllPMTBoard
 *
 * Método privado que selecciona la totalidad de PMTs.
 *
 */
void SetPMTs::SetAllPMTBoard()
{
  for(int pmt = 0; pmt < PMTs; pmt++)
    {
      pmt_button_table[pmt]->setChecked(true);
    }
}
/**
 * @brief SetPMTs::on_pushButton_select_all_clicked
 */
void SetPMTs::on_pushButton_select_all_clicked()
{
  SetAllPMTBoard();
}
/**
 * @brief SetPMTs::on_pushButton_select_none_clicked
 */
void SetPMTs::on_pushButton_select_none_clicked()
{
  ClearPMTBoard();
}
