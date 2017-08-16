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
    logfile = ui->checkBox_Log->isChecked();
    stdoutmode = ui->checkBox_StdOut->isChecked();
    QDialog::accept();
}
/**
 * @brief SetPreferences::reject
 * @overload
 */
void SetPreferences::reject()
{
    ui->checkBox_Debug->setChecked(debconsole);
    ui->checkBox_Log->setChecked(logfile);
    ui->checkBox_StdOut->setChecked(stdoutmode);
    QDialog::reject();
}
/**
 * @brief SetPreferences::exec
 * @overload
 */
int SetPreferences::exec()
{
    ui->checkBox_Debug->setChecked(debconsole);
    ui->checkBox_Log->setChecked(logfile);
    ui->checkBox_StdOut->setChecked(stdoutmode);
    ui->lineEdit_config_file->setText(initfile);
    ui->lineEdit_config_calib->setText(root_calib_path);
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
QString SetPreferences::openConfigurationFile(bool dir)
{
    QString filename;

    if(!dir)
    {
        filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de configuración"),
                                                QDir::homePath(),
                                                tr("Configuración (*.ini);;Texto (*.txt);;Todos (*.*)"));
    }
    else
    {
        filename = QFileDialog::getExistingDirectory(this, tr("Seleccionar el directorio de configuración"),
                                                     QDir::homePath());
    }

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
    root_calib_path = openConfigurationFile(true);
    ui->lineEdit_config_calib->setText(root_calib_path);
}
