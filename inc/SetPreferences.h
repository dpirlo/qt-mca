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
#include <QFileDialog>
#include "QMessageBox"
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
  virtual int exec();
  ~SetPreferences();

private:
  QString openConfigurationFile();
  QString readPreferencesFile(QString conf_set_file);

private:    
  Ui::SetPreferences *ui;
  /**
     * @brief debconsole
     *
     * Variable _booleana_ que indica si la aplicación tiene activado el modo _debug_.
     */
  bool debconsole;
  QString initfile, root_calib_path;
  QString preferencesdir, conf_set_file, calib_set_file;

public:
  /**
     * @brief GetDegugConsoleValue
     *
     * Obtiene el estado de la variable _booleana_ debconsole.
     * @return debconsole
     * @see debconsole
     */
  bool getDegugConsoleValue() const { return debconsole; }
  QString getInitFileConfigPath() const { return initfile; }
  QString getCalibDirectoryPath() const { return root_calib_path; }
  void setPreferencesDir(QString pref_path) { preferencesdir=pref_path; }
  void setCalibSetFile(QString pref_path) { calib_set_file=pref_path; }
  void setConfSetFile(QString pref_path) { conf_set_file=pref_path; }


private slots:
  void on_pushButton_open_config_file_clicked();
  void on_pushButton_open_config_calib_clicked();
};

#endif // SETPREFERENCES_H
