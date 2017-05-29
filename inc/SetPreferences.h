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
  QString openConfigurationFile(bool dir=false);
  QString readPreferencesFile(QString conf_set_file);

private:    
  Ui::SetPreferences *ui;
  /**
     * @brief debconsole
     *
     * Variable _booleana_ que indica si la aplicación tiene activado el modo _debug_.
     */
  bool debconsole;
  /**
   * @brief logfile
   *
   * Variable _booleana_ que indica si la aplicación tiene activado el modo _log_.
   */
  bool logfile;
  /**
   * @brief stdoutmode
   *
   * Variable _booleana_ que indica si la aplicación tiene activado el modo _stdout_.
   * Este modo permite escribir en un archivo la salida por consola de debug.
   *
   */
  bool stdoutmode;
  QString initfile, root_calib_path;
  QString preferencesdir, preferencesfile;

public:
  /**
     * @brief getDegugConsoleValue
     *
     * Obtiene el estado de la variable _booleana_ debconsole.
     * @return debconsole
     * @see debconsole
     */
  bool getDegugConsoleValue() const { return debconsole; }
  /**
   * @brief getLogFileValue
   *
   * Obtiene el estado de la variable _booleana_ logfile.
   * @return
   * @see logfile
   */
  bool getLogFileValue() const { return logfile; }
  /**
   * @brief getDebugStdOutValue
   *
   * Obtiene el estado de la variable _booleana_ stdoutmode.
   * @return stdoutmode
   */
  bool getDebugStdOutValue() const {return stdoutmode; }
  /**
   * @brief setDegugConsoleValue
   *
   * Configura el estado de la variable _booleana_ debconsole.
   * @param debug
   */
  void setDegugConsoleValue(bool debug) { debconsole = debug; }
  /**
   * @brief setLogFileValue
   *
   * Configura el estado de la variable _booleana_ logfile.
   * @param log
   */  
  void setLogFileValue(bool log) { logfile = log; }
  /**
   * @brief setDebugStdOutValue
   *
   * Configura el estado de la variable _booleana_ stdoutmode.
   * @param stdout
   */
  void setDebugStdOutValue(bool stdout) { stdoutmode = stdout; }
  /**
   * @brief getInitFileConfigPath
   * @return La ruta del archivo de configuración de cabezales
   */
  QString getInitFileConfigPath() const { return initfile; }
  /**
   * @brief getCalibDirectoryPath
   * @return La ruta del directorio de calibración
   */
  QString getCalibDirectoryPath() const { return root_calib_path; }  
  /**
   * @brief setPreferencesDir
   * @param pref_path
   */
  void setPreferencesDir(QString pref_path) { preferencesdir = pref_path;}
  /**
   * @brief getPreferencesDir
   * @return preferencesdir
   */
  QString getPreferencesDir() const { return preferencesdir;}
  /**
   * @brief setPreferencesFile
   * @param pref_path
   */
  void setPreferencesFile(QString pref_path) { preferencesfile = pref_path;}
  /**
   * @brief setCalibDir
   * @param dir_path
   */
  void setCalibDir(QString dir_path){root_calib_path = dir_path;}
  /**
   * @brief setConfFile
   * @param file_path
   */
  void setConfFile(QString file_path){initfile = file_path;}

private slots:
  void on_pushButton_open_config_file_clicked();
  void on_pushButton_open_config_calib_clicked();  
};

#endif // SETPREFERENCES_H
