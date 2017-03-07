/**
 * @class SetPreferences
 *
 * @brief Clase de preferencias
 *
 * Esta clase provee métodos y propiedades para el manejo
 * del QDialog que administra las preferencias de la aplicación qt-mca.
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
#ifndef SETPREFERENCES_H
#define SETPREFERENCES_H

#include <QDialog>
#include <iostream>

using namespace std;

namespace Ui {
  class SetPreferences;
}

class SetPreferences : public QDialog
{
  Q_OBJECT

public:
  explicit SetPreferences(QWidget *parent = 0);
  void accept();
  void reject();
  ~SetPreferences();

private:    
  Ui::SetPreferences *ui;
  /**
     * @brief debconsole
     *
     * Variable _booleana_ que indica si la aplicación tiene activado el modo _debug_.
     */
  bool debconsole;

public:
  /**
     * @brief GetDegugConsoleValue
     *
     * Obtiene el estado de la variable _booleana_ debconsole.
     * @return debconsole
     * @see debconsole
     */
  bool GetDegugConsoleValue() const { return debconsole; }

};

#endif // SETPREFERENCES_H
