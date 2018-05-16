#include "validate_cal.hpp"
#include "ui_validate_cal.h"

Validate_Cal::Validate_Cal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Validate_Cal)
{
    ui->setupUi(this);


    FWHM = new float[2]();

    Coef_energia = new QVector<double>[2]();
    Espectro = new QVector<double>[2]();
    Espectro_bins = new QVector<double>[2]();
    Coef_pos_X = new QVector<double>[2]();
    Coef_pos_Y = new QVector<double>[2]();

    nombre_log_file = new QString[2]();




    param_cab[0][0]=0;
    param_cab[0][1]=128;
    param_cab[0][2]=0;
    param_cab[0][3]=rand()%5+1; //LineStyle
    param_cab[0][4]=rand()%14+1;//ScatterShape
    param_cab[0][5]=rand()/(double)RAND_MAX*2+1;//setWidthF

    param_cab[1][0]=255;
    param_cab[1][1]=0;
    param_cab[1][2]=0;
    param_cab[1][3]=rand()%5+1; //LineStyle
    param_cab[1][4]=rand()%14+1;//ScatterShape
    param_cab[1][5]=rand()/(double)RAND_MAX*2+1;//setWidthF


}

Validate_Cal::~Validate_Cal()
{
    delete ui;
}

void Validate_Cal::on_buttonBox_accepted()
{
    proceso = new QProcess();

    this->connect(proceso, SIGNAL(readyReadStandardError()), this, SLOT(updateError()));
    this->connect(proceso, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readyRead()));

    //Armo un zip de toda las cosas dentro de la carpeta
    QString programa = "/usr/bin/zip";
    QStringList listasparametros;
    listasparametros.append("-j");
    listasparametros.append(files_old+nombre_log_file[1]);
    listasparametros.append(files_old+"Almohadon_Cabezal_"+QString::number(cab_test)+".pgm");
    listasparametros.append(files_old+"Almohadon_Cabezal_"+QString::number(cab_test)+".ppm");
    listasparametros.append(files_old+"Coef_Energia_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old+"Cx_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old+"Cy_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old+"Log_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old+"Tiempos_Cabezal_"+QString::number(cab_test)+".txt");
    // listasparametros.append("*"); // El asterisco no funciona......
    proceso->start(programa,listasparametros);
    proceso->waitForFinished();
    int error_flag = proceso->exitCode();

    if (error_flag != 0)
    {
        cout<<"ABORTANDO - No se pudo crear el archivo .zip"<<endl;
        QMessageBox::critical(this,tr("ABORTANDO"),tr((string("No se pudo crear el archivo .zip")).c_str()));
        this->~Validate_Cal();
        return;
    }


    // Copio al servidor
    programa = "/bin/cp";
    listasparametros.clear();
    listasparametros.append(files_old+nombre_log_file[1]);
    listasparametros.append(files_back+"cabezal_"+QString::number(cab_test)+"/Constantes/BACK/"+nombre_log_file[1]);
    proceso->start(programa,listasparametros);
    proceso->waitForFinished();

    error_flag = proceso->exitCode();

    if (error_flag != 0)
    {
        cout<<"ABORTANDO - No se pudo copiar backup al server."<<endl;
        QMessageBox::critical(this,tr("ABORTANDO"),tr((string("No se pudo copiar backup al server.")).c_str()));
        this->~Validate_Cal();
        return;
    }

    // Borro todo el contenido de la carpeta
    programa = "/bin/rm";
    listasparametros.clear();
    // listasparametros.append("*"); // El asterisco no funciona......
    listasparametros.append(files_old+nombre_log_file[1]);
    listasparametros.append(files_old+"Almohadon_Cabezal_"+QString::number(cab_test)+".pgm");
    listasparametros.append(files_old+"Almohadon_Cabezal_"+QString::number(cab_test)+".ppm");
    listasparametros.append(files_old+"Coef_Energia_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old+"Cx_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old+"Cy_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old+"Log_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old+"Tiempos_Cabezal_"+QString::number(cab_test)+".txt");
    proceso->start(programa,listasparametros);
    proceso->waitForFinished();

    error_flag = proceso->exitCode();

    if (error_flag != 0)
    {
        cout<<"ABORTANDO - No se pudo eliminar copia local."<<endl;
        QMessageBox::critical(this,tr("ABORTANDO"),tr((string("No se pudo eliminar copia local.")).c_str()));
        this->~Validate_Cal();
        return;
    }


    // Copio los nuevos archivos

    programa = "/bin/cp";
    listasparametros.clear();
    // listasparametros.append("*"); // El asterisco no funciona......
    listasparametros.append(files_new+"Almohadon_Cabezal_"+QString::number(cab_test)+".pgm");
    listasparametros.append(files_old);
    proceso->start(programa,listasparametros);
    proceso->waitForFinished();
    error_flag = proceso->exitCode();
    if (error_flag != 0)
    {
        cout<<"ABORTANDO - No se pudo copiar el archivo "<<listasparametros[0].toStdString()<<" al directorio local."<<endl;
        QMessageBox::critical(this,tr("ABORTANDO"),tr((string("No se pudo copiar el archivo "+listasparametros[0].toStdString()+" al directorio local.")).c_str()));
        this->~Validate_Cal();
        return;
    }

    listasparametros.clear();
    // listasparametros.append("*"); // El asterisco no funciona......
    listasparametros.append(files_new+"Almohadon_Cabezal_"+QString::number(cab_test)+".ppm");
    listasparametros.append(files_old);
    proceso->start(programa,listasparametros);
    proceso->waitForFinished();
    error_flag = proceso->exitCode();
    if (error_flag != 0)
    {
        cout<<"ABORTANDO - No se pudo copiar el archivo "<<listasparametros[0].toStdString()<<" al directorio local."<<endl;
        QMessageBox::critical(this,tr("ABORTANDO"),tr((string("No se pudo copiar el archivo "+listasparametros[0].toStdString()+" al directorio local.")).c_str()));
        this->~Validate_Cal();
        return;
    }

    listasparametros.clear();
    // listasparametros.append("*"); // El asterisco no funciona......
    listasparametros.append(files_new+"Coef_Energia_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old);
    proceso->start(programa,listasparametros);
    proceso->waitForFinished();
    error_flag = proceso->exitCode();
    if (error_flag != 0)
    {
        cout<<"ABORTANDO - No se pudo copiar el archivo "<<listasparametros[0].toStdString()<<" al directorio local."<<endl;
        QMessageBox::critical(this,tr("ABORTANDO"),tr((string("No se pudo copiar el archivo "+listasparametros[0].toStdString()+" al directorio local.")).c_str()));
        this->~Validate_Cal();
        return;
    }

    listasparametros.clear();
    // listasparametros.append("*"); // El asterisco no funciona......
    listasparametros.append(files_new+"Cx_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old);
    proceso->start(programa,listasparametros);
    proceso->waitForFinished();
    error_flag = proceso->exitCode();
    if (error_flag != 0)
    {
        cout<<"ABORTANDO - No se pudo copiar el archivo "<<listasparametros[0].toStdString()<<" al directorio local."<<endl;
        QMessageBox::critical(this,tr("ABORTANDO"),tr((string("No se pudo copiar el archivo "+listasparametros[0].toStdString()+" al directorio local.")).c_str()));
        this->~Validate_Cal();
        return;
    }


    listasparametros.clear();
    // listasparametros.append("*"); // El asterisco no funciona......
    listasparametros.append(files_new+"Cy_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old);
    proceso->start(programa,listasparametros);
    proceso->waitForFinished();
    error_flag = proceso->exitCode();
    if (error_flag != 0)
    {
        cout<<"ABORTANDO - No se pudo copiar el archivo "<<listasparametros[0].toStdString()<<" al directorio local."<<endl;
        QMessageBox::critical(this,tr("ABORTANDO"),tr((string("No se pudo copiar el archivo "+listasparametros[0].toStdString()+" al directorio local.")).c_str()));
        this->~Validate_Cal();
        return;
    }

    listasparametros.clear();
    // listasparametros.append("*"); // El asterisco no funciona......
    listasparametros.append(files_new+"Log_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old);
    proceso->start(programa,listasparametros);
    proceso->waitForFinished();
    error_flag = proceso->exitCode();
    if (error_flag != 0)
    {
        cout<<"ABORTANDO - No se pudo copiar el archivo "<<listasparametros[0].toStdString()<<" al directorio local."<<endl;
        QMessageBox::critical(this,tr("ABORTANDO"),tr((string("No se pudo copiar el archivo "+listasparametros[0].toStdString()+" al directorio local.")).c_str()));
        this->~Validate_Cal();
        return;
    }

    listasparametros.clear();
    // listasparametros.append("*"); // El asterisco no funciona......
    listasparametros.append(files_new+"Tiempos_Cabezal_"+QString::number(cab_test)+".txt");
    listasparametros.append(files_old);
    proceso->start(programa,listasparametros);
    proceso->waitForFinished();
    error_flag = proceso->exitCode();
    if (error_flag != 0)
    {
        cout<<"ABORTANDO - No se pudo copiar el archivo "<<listasparametros[0].toStdString()<<" al directorio local."<<endl;
        QMessageBox::critical(this,tr("ABORTANDO"),tr((string("No se pudo copiar el archivo "+listasparametros[0].toStdString()+" al directorio local.")).c_str()));
        this->~Validate_Cal();
        return;
    }


    // Escribo en el log
    QFile file(files_back+"Log_Global_Calibraciones.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream stream(&file);
    stream << nombre_log_file[1] << " ; Cabezal ; " << QString::number(cab_test) << endl;
    file.close();

    // Finalmente destruyo la ventana.
    this->~Validate_Cal();
}

void Validate_Cal::on_buttonBox_rejected()
{
    // No hago nada, solo destruyo la ventana.
    this->~Validate_Cal();
}


void Validate_Cal::load_data(int checked_Cab, QString Path_Calib_Actual, QString Path_Calib_Base, QString path_files_back)
{
    cab_test = checked_Cab;
    files_old = Path_Calib_Base+"cabezal_"+QString::number(cab_test)+"/Constantes/";
    files_new = Path_Calib_Actual;
    files_back = path_files_back;


    QString Log_file_new = files_new+"Log_Cabezal_"+QString::number(cab_test)+".txt";
    QString Log_file_old = files_old+"Log_Cabezal_"+QString::number(cab_test)+".txt";

    nombre_almohadon_viejo = files_old+"Almohadon_Cabezal_"+QString::number(cab_test)+".ppm";
    nombre_almohadon_nuevo = files_new+"Almohadon_Cabezal_"+QString::number(cab_test)+".ppm";

    // Parseo el anterior
    parse_log_file(Log_file_old, 0);
    // Parseo el nuevo
    parse_log_file(Log_file_new, 1);

    // Ploteo el viejo
    plot_data(0);
    // Ploteo el nuevo
    plot_data(1);


    return;
}


void Validate_Cal::plot_data(int mark_idx)
{
    QString graph_legend;

    if (mark_idx == 0) graph_legend = "Anterior -- FWHM = "+QString::number(FWHM[mark_idx]);
    else  graph_legend = "Actual -- FWHM = "+QString::number(FWHM[mark_idx]);

    // Vector de indice de 48 PMTs
    QVector<double> vector_aux;
    vector_aux.clear();
    for(int k=0; k<48; k++) { vector_aux.append( k ); }

    // Seteo Color
    QPen graphPen;
    graphPen.setColor(QColor(param_cab[mark_idx][0], param_cab[mark_idx][1], param_cab[mark_idx][2]));
    graphPen.setWidthF(param_cab[mark_idx][5]);


    // Ploteo Espectro
    ui->Espectro->addGraph();
    ui->Espectro->graph()->setName(graph_legend);
    ui->Espectro->graph()->setData(Espectro_bins[mark_idx],Espectro[mark_idx]);
    ui->Espectro->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(param_cab[mark_idx][4])));
    ui->Espectro->graph()->setPen(graphPen);
    ui->Espectro->legend->setVisible(true);
    ui->Espectro->legend->setWrap(4);
    ui->Espectro->legend->setRowSpacing(1);
    ui->Espectro->legend->setColumnSpacing(2);
    ui->Espectro->rescaleAxes();
    ui->Espectro->yAxis->setVisible(false);
    ui->Espectro->replot();

    // Ploteo Coeficientes de energía
    ui->Energia->addGraph();
    ui->Energia->graph()->setData(vector_aux,Coef_energia[mark_idx]);
    ui->Energia->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(param_cab[mark_idx][4])));
    //ui->Energia->graph()->setLineStyle(QCPGraph::lsNone);
    ui->Energia->graph()->setPen(graphPen);
    ui->Energia->rescaleAxes();
    ui->Energia->replot();

    // Ploteo Coeficientes X e Y
    ui->C_x->addGraph();
    ui->C_x->graph()->setData(vector_aux,Coef_pos_X[mark_idx]);
    ui->C_x->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(param_cab[mark_idx][4])));
    //ui->C_x->graph()->setLineStyle(QCPGraph::lsNone);
    ui->C_x->graph()->setPen(graphPen);
    ui->C_x->rescaleAxes();
    ui->C_x->replot();

    ui->C_y->addGraph();
    ui->C_y->graph()->setData(vector_aux,Coef_pos_Y[mark_idx]);
    ui->C_y->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(param_cab[mark_idx][4])));
    //ui->C_y->graph()->setLineStyle(QCPGraph::lsNone);
    ui->C_y->graph()->setPen(graphPen);
    ui->C_y->rescaleAxes();
    ui->C_y->replot();


    // Plot de almohadon
    if (mark_idx == 0)
    {
        QPixmap file(nombre_almohadon_viejo);

        int w = ui->Almohadon_Viejo->width();
        int h = ui->Almohadon_Viejo->height();

        ui->Almohadon_Viejo->setPixmap(file.scaled(w,h));
        //ui->Almohadon_Viejo->setPixmap(file.scaled(w,h,Qt::KeepAspectRatio));
    }
    else
    {
        QPixmap file(nombre_almohadon_nuevo);

        int w = ui->Almohadon_Nuevo->width();
        int h = ui->Almohadon_Nuevo->height();

        ui->Almohadon_Nuevo->setPixmap(file.scaled(w,h));
    }







    return;
}


void Validate_Cal::parse_log_file(QString File_in, int mark_idx)
{

    QFile file_open(File_in);

    if (!file_open.open(QFile::ReadOnly | QFile::Text))
        return;

    QTextStream in_log(&file_open);

    QStringList field = in_log.readAll().split("\n");


    QStringList Marcas;
    Marcas << " %Final: ";
    Marcas << "       Ce_final_"+QString::number(cab_test)+" = " ;
    Marcas << "           Espect_Final_vec_"+QString::number(cab_test)+" =" ;
    Marcas << "           Espect_Final_bins_"+QString::number(cab_test)+" =" ;
    Marcas << "       Cx_"+QString::number(cab_test)+" = ";
    Marcas << "       Cy_"+QString::number(cab_test)+" = ";
    Marcas << "% Log Archive: ";

    for (int i=0 ; i < field.size() ; i++)
    {
        QString line = field[i];

        for (int j=0 ; j < Marcas.size() ; j++)
        {
            QString Marca = Marcas[j];

            if (line.contains(Marca) > 0)
            {

                // Si es el primero, saco el valor del FWHM
                if (j == 0) FWHM[mark_idx] = QString::fromStdString(line.toStdString().substr(Marca.size(),5)).toFloat();
                else if(j < 6)
                {
                    QString linea_vector = QString::fromStdString(line.toStdString().substr(Marca.size()+1,line.size()-Marca.size()-3));

                    QRegExp rx("(\\ , |\\ ; )"); //RegEx for ' ' or ',' or '.' or ':' or '\t'

                    QStringList number_list = linea_vector.split(rx);

                    QVector<double> vector_aux;
                    vector_aux.clear();

                    int dimension = number_list.count();
                    for(int k=0; k<dimension; k++) { vector_aux.append( number_list.value(k).toDouble() ); }


                    switch (j) {
                    case 1:
                        Coef_energia[mark_idx] = vector_aux;
                        break;
                    case 2:
                        Espectro[mark_idx] = vector_aux;
                        break;
                    case 3:
                        Espectro_bins[mark_idx] = vector_aux;
                        break;
                    case 4:
                        Coef_pos_X[mark_idx] = vector_aux;
                        break;
                    case 5:
                        Coef_pos_Y[mark_idx] = vector_aux;
                        break;
                    default:
                        break;
                    }

                }
                else
                {
                    nombre_log_file[mark_idx] = QString::fromStdString(line.toStdString().substr(Marca.size(),19))+".zip";
                }

            }
        }


    }

    return;
}




void Validate_Cal::on_readyRead()
{

    QByteArray data = proceso->readAllStandardOutput();
    cout<<QString(data).toStdString()<<endl;

}

void Validate_Cal::updateError()
{

     QByteArray data = proceso->readAllStandardError();
     cout<<QString(data).toStdString()<<endl;
}
