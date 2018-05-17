#ifndef VALIDATE_CAL_HPP
#define VALIDATE_CAL_HPP

#include <QDialog>


// Gracias Ari por usar boost...
#include <boost/asio/serial_port.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include "inc/apAutoCalib.hpp"

namespace Ui {
class Validate_Cal;
}

class Validate_Cal : public QDialog
{
    Q_OBJECT

public:

    Validate_Cal(QWidget *parent = 0);
    ~Validate_Cal();

    void load_data(int checked_Cab, QString Path_Calib_Actual, QString Path_Calib_Base, QString path_files_back, bool include_HV=false, bool Delete_Output=false);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_readyRead();
    void updateError();

private:
    Ui::Validate_Cal *ui;

    QString nombre_almohadon_viejo, nombre_almohadon_nuevo, files_old, files_new, files_back, path_to_rsa_key, IP_server;
    QString* nombre_log_file;

    float* FWHM;

    bool Send_HV;
    bool Delete_Salidas;


    QVector<double> *Coef_energia , *Espectro, *Espectro_bins , *Coef_pos_X , *Coef_pos_Y , *Almohadon;

    void parse_log_file(QString File_in, int mark_idx);
    void plot_data(int mark_idx);
    int param_cab[2][6];
    int cab_test;

    QProcess *proceso;


};

#endif // VALIDATE_CAL_HPP

