/**
 * @class SetPMTs
 *
 * @brief Clase de selección de PMTs
 *
 * Esta clase provee métodos y propiedades para el manejo
 * del QDialog que realiza la selección de los
 * fotomultiplicadores. A partir de esta selección se adquiere
 * los valores de MCA.
 *
 * @note Clase heredada de QDialog
 *
 * @author Ariel Hernández
 *
 * @version $Version
 *
 * Contacto: ahernandez@cae.cnea.gov.ar
 *           ariel.h.estevenz@ieee.org
 *
 */
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
  void ConfigurePMTList();
  void accept();
  void reject();
  ~SetPMTs();

private:
  void SetPMTPushButtonList();
  void ClearPMTBoard();
  void SetAllPMTBoard();

private slots:
  void on_pushButton_select_all_clicked();
  void on_pushButton_select_none_clicked();

private:
  Ui::SetPMTs *ui;
  /**
     * @brief pmt_selected_list
     *
     * Lista de QString que contiene los fotomultiplicadores (PMT) seleccionados.
     *
     */
  QList<QString> pmt_selected_list;
  QList<QPushButton*> pmt_button_table;

public:
  /**
     * @brief GetPMTSelectedList
     *
     * Método público que obtiene la lista seleccionada de fotomultiplicadores.
     *
     * @return pmt_selected_list
     * @see pmt_selected_list
     */
  QList<QString> GetPMTSelectedList() const { return pmt_selected_list; }

};

#endif // SETPMTS_H
