#include "inc/SetPreferences.h"
#include "ui_SetPreferences.h"

/**
 * @brief SetPreferences::SetPreferences
 *
 * Constructor de la clase.
 * @param parent
 */
SetPreferences::SetPreferences(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SetPreferences)
{
  ui->setupUi(this);
}
/**
 * @brief SetPreferences::accept
 * @overload
 */
void SetPreferences::accept()
{
  debconsole = ui->checkBox_Debug->isChecked();
  QDialog::accept();
}
/**
 * @brief SetPreferences::reject
 * @overload
 */
void SetPreferences::reject()
{
  ui->checkBox_Debug->setChecked(debconsole);
  QDialog::reject();
}
/**
 * @brief SetPreferences::exec
 * @overload
 */
int SetPreferences::exec()
{
  ui->lineEdit_config_file->setText(readPreferencesFile(conf_set_file));
  ui->lineEdit_config_calib->setText(readPreferencesFile(calib_set_file));
  QDialog::exec();
}

/**
 * @brief SetPreferences::~SetPreferences
 *
 * Destructor de la clase.
 */
SetPreferences::~SetPreferences()
{
  delete ui;
}

/**
 * @brief SetPreferences::openConfigurationFile
 * @return
 */
QString SetPreferences::openConfigurationFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de configuración"),
                                                    QDir::homePath(),
                                                    tr("Configuración (*.ini);;Texto (*.txt);;Todos (*.*)"));
    return filename;
}

QString SetPreferences::readPreferencesFile(QString conf_set_file)
{
    QString pref_path = QDir::homePath() + "/" + preferencesdir + "/" + conf_set_file;
    QFile prefconfigfile(pref_path);
    if (!prefconfigfile.open(QIODevice::ReadOnly))
      {
        if(debconsole) cout << "No se puede abrir el archivo de preferencias. Por favor vuelva a configurar la ruta desde este menú. Error: " << prefconfigfile.errorString().toStdString() << endl;
        QMessageBox::critical(this,tr("Atención"),tr("No se encuentra el archivo de preferencias. Por favor configure la ruta desde este menú."));
      }
    QString filename = prefconfigfile.readLine();
    return filename;
}

/**
 * @brief SetPreferences::on_pushButton_open_config_file_clicked
 */
void SetPreferences::on_pushButton_open_config_file_clicked()
{
    initfile = openConfigurationFile();
    ui->lineEdit_config_file->setText(initfile);
}
/**
 * @brief SetPreferences::on_pushButton_open_config_calib_clicked
 */
void SetPreferences::on_pushButton_open_config_calib_clicked()
{
    root_calib_path = openConfigurationFile();
    ui->lineEdit_config_calib->setText(root_calib_path);

}
