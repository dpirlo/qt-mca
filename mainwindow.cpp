#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    TimeOut(1000),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    arpet=shared_ptr<MCAE>(new MCAE(TimeOut));
    ui->comboBox_port->addItems(availablePortsName());
}

MainWindow::~MainWindow()
{
    delete ui;   
}

void MainWindow::on_pushButton_triple_ventana_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_triple_ventana->setText(fileName);
}

void MainWindow::on_pushButton_hv_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_hv->setText(fileName);
}

void MainWindow::on_pushButton_energia_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_energia->setText(fileName);
}


void MainWindow::on_pushButton_posicion_X_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_posicion_X->setText(fileName);
}

void MainWindow::on_pushButton_posicion_Y_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_posicion_Y->setText(fileName);
}

void MainWindow::on_pushButton_tiempos_cabezal_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_tiempos_cabezal->setText(fileName);
}

void MainWindow::on_pushButton_salir_clicked()
{
    arpet->portDisconnect();
    QApplication::quit();
}

void MainWindow::on_pushButton_salir_graficos_clicked()
{
    arpet->portDisconnect();
    QApplication::quit();
}

void MainWindow::on_pushButton_obtener_rutas_clicked()
{
    string initfile=".//config_cabs.ini";
    QString filename=QString::fromStdString(initfile);
    parseConfigurationFile(filename);

    ui->textBrowser_triple_ventana->setText(coefest);
    ui->textBrowser_hv->setText(hvtable);
    ui->textBrowser_energia->setText(coefenerg);
    ui->textBrowser_posicion_X->setText(coefx);
    ui->textBrowser_posicion_Y->setText(coefy);
    ui->textBrowser_tiempos_cabezal->setText(coefT);

    ui->plainTextEdit_alta->document()->setPlainText(QString::number(AT));
    ui->plainTextEdit_limiteinferior->document()->setPlainText(QString::number(LowLimit));
}

int MainWindow::on_pushButton_conectar_clicked()
{

    if(arpet->isPortOpen())
    {
        ui->pushButton_conectar->setText("Connect");
        arpet->portDisconnect();
    }
    else
    {
        ui->pushButton_conectar->setText("Disconnect");
        try{
            QString port_name=ui->comboBox_port->currentText();
            arpet->portConnect(port_name.toStdString().c_str());
            QMessageBox::information(this,tr("Información"),tr("Conectado al puerto: ") + port_name);
        }
        catch(boost::system::system_error e)
            {
            QMessageBox::critical(this,tr("Error"),tr("No se puede acceder al puerto serie. Error: ")+tr(e.what()));;
            return MCAE::FAILED;
        }
    }

    return MCAE::OK;
}

void MainWindow::on_pushButton_configurar_clicked()
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead().toStdString() + arpet->getFunCHead());

    /** TODO:
     * Implementar una función que realice el seteo de los HV a los PMTs en función del cabezal seleccionado
     * Ver de obtener todos los path, de los seis cabezales, solo de un .ini
     */
    cout<<arpet->getHeader_MCAE()<<endl;
}

void MainWindow::on_pushButton_hv_set_clicked()
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead().toStdString() + arpet->getFunCHV());

    /** TODO:
     * Seteo de HV a la tensión indicada.
     * El seteo se debe realizar de manera escalonada
     */
    cout<<arpet->getHeader_MCAE()<<endl;

}

void MainWindow::on_pushButton_hv_on_clicked()
{
    /** TODO:
     * Ver el tamaño de la trama
     */

    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead().toStdString() + arpet->getFunCHV());
    arpet->setTrama_MCAE(arpet->getHeader_MCAE()+arpet->getHV_ON()+arpet->getEnd_HV());
    cout<<arpet->getTrama_MCAE()<<endl;
}

void MainWindow::on_pushButton_hv_off_clicked()
{
    /** TODO:
     * Ver el tamaño de la trama
     */

    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead().toStdString() + arpet->getFunCHV());
    arpet->setTrama_MCAE(arpet->getHeader_MCAE()+arpet->getHV_OFF()+arpet->getEnd_HV());
    cout<<arpet->getTrama_MCAE()<<endl;
}

void MainWindow::on_pushButton_hv_estado_clicked()
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead().toStdString()+arpet->getFunCHV());

    /** TODO:
     * Obtener estado de la fuente HV y mostrarlo en el label
     */
    cout<<arpet->getHeader_MCAE()<<endl;
}

void MainWindow::on_pushButton_adquirir_clicked()
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead().toStdString()+arpet->getFunCSP3());

    /** TODO:
     * Adquirir los datos MCA y parsear la trama.
     */
    cout<<arpet->getHeader_MCAE()<<endl;
}


/* Métodos generales del entorno gráfico */


/**
 * @brief MainWindow::parseConfigurationFile
 * @param filename
 * @return
 */
int MainWindow::parseConfigurationFile(QString filename)
{
    QFile configfile(filename);
    if (!configfile.open(QIODevice::ReadOnly)) {
        filename=openConfigurationFile();
        configfile.setFileName(filename);
        if (!configfile.open(QIODevice::ReadOnly)){
            qDebug() << "No se puede abrir el archivo de configuración. Error: " << configfile.errorString();
            QMessageBox::critical(this,tr("Atención"),tr("No se puede abrir el archivo de configuración."));
            return MCAE::FILE_NOT_FOUND;
        }
    }

    QSettings settings(filename, QSettings::IniFormat);

    /* Parameters */
    AT = settings.value("SetUp/AT", "US").toInt();
    LowLimit = settings.value("SetUp/LowLimit", "US").toInt();

    /* Paths to the configuration files */

    QString head= getHead();
    QString root=settings.value("Paths/root", "US").toString();

    coefT = root+settings.value("Cabezal"+head+"/coefT", "US").toString();
    coefenerg = root+settings.value("Cabezal"+head+"/coefenerg", "US").toString();
    hvtable = root+settings.value("Cabezal"+head+"/hvtable", "US").toString();
    coefx = root+settings.value("Cabezal"+head+"/coefx", "US").toString();
    coefy = root+settings.value("Cabezal"+head+"/coefy", "US").toString();
    coefest = root+settings.value("Cabezal"+head+"/coefest", "US").toString();
    coefT = root+settings.value("Cabezal"+head+"/coefT", "US").toString();

    configfile.close();

    return MCAE::OK;
}

/**
 * @brief MainWindow::openConfigurationFile
 * @return
 */
QString MainWindow::openConfigurationFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de configuración"),
                                                    QDir::homePath(),
                                                    tr("Texto (*.ini)"));
    return filename;
}

/**
 * @brief MainWindow::SetLabelState
 * @param state
 * @param label
 */
void MainWindow::setLabelState(bool state, QLabel *label)
{
    QPalette palette;

    if (!state)
    {
        palette.setColor(QPalette::Background,Qt::green);
        label->setPalette(palette);
    }
    else
    {
        palette.setColor(QPalette::Background,Qt::red);
        label->setPalette(palette);
    }
    return;
}

/**
 * @brief MainWindow::availablePortsName
 * @return
 */
QStringList MainWindow::availablePortsName()
{

    QStringList portsName;

    QDir dir("/dev/");
    QStringList filters;

    filters << "ttyUSB*";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::System);
    QFileInfoList list = dir.entryInfoList();

    for (int i=0; i< list.size(); i++)
    {
        portsName.append(list.at(i).canonicalFilePath ());
    }

    return portsName;
}

QString MainWindow::getHead()
{
    QString head=ui->comboBox_select_cabezal->currentText();
    return head;
}

string MainWindow::ReadString()
{
    string msg;
    try{
         arpet->portReadString(&msg,'\r');
    }
    catch( Exceptions & ex ){
         QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
    }
    return msg;
}

size_t MainWindow::SendString(string msg, string end)
{
    size_t bytes_transfered = 0;

    try{
        string sended=msg + end;
        bytes_transfered = arpet->portWrite(&sended);
    }
    catch(boost::system::system_error e){
        QMessageBox::critical(this,tr("Error"),tr("No se puede acceder al puerto serie. Error: ") + tr(e.what()));;
    }

    return bytes_transfered;
}


/* TOMARLOS COMO EJEMPLO PARA ENVIO Y RECEPCIÓN DEL SERIE */

void MainWindow::on_pushButton_clicked()
{   
   QString sended = ui->plainTextEdit->toPlainText();

   SendString(sended.toStdString(),arpet->getEnd_MCA());
   string msg=ReadString();
   QString q_msg=QString::fromStdString(msg);

   ui->label_12->setText(q_msg);

}

/**********************************************************/
