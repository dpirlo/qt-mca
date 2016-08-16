#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    TimeOut(1000),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->maximumSize());
    arpet=shared_ptr<MCAE>(new MCAE(TimeOut));
    ui->comboBox_port->addItems(availablePortsName());
    manageHeadCheckBox("config",false);
    manageHeadCheckBox("graph",false);
}

MainWindow::~MainWindow()
{
    delete ui;   
}

void MainWindow::checkCombosStatus()
{
     QObject::connect(ui->comboBox_head_mode_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(setHeadModeGraph(int)));
     QObject::connect(ui->comboBox_head_mode_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(setHeadModeConfig(int)));
     QObject::connect(ui->comboBox_adquire_mode ,SIGNAL(currentIndexChanged (int)),this,SLOT(setAdquireMode(int)));
}

/* Pestaña: "Configuración" */

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
        ui->pushButton_conectar->setText("Conectar");
        arpet->portDisconnect();
    }
    else
    {
        ui->pushButton_conectar->setText("Desconectar");
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
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("config").toStdString() + arpet->getFunCHead());

    /** TODO:
     * Implementar una función que realice el seteo de los HV a los PMTs en función del cabezal seleccionado
     * Ver de obtener todos los path, de los seis cabezales, solo de un .ini
     */
    cout<<arpet->getHeader_MCAE()<<endl;
}

void MainWindow::on_pushButton_hv_set_clicked()
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("config").toStdString() + arpet->getFunCHV());

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

    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("config").toStdString() + arpet->getFunCHV());
    arpet->setTrama_MCAE(arpet->getHeader_MCAE()+arpet->getHV_ON()+arpet->getEnd_HV());
    cout<<arpet->getTrama_MCAE()<<endl;
}

void MainWindow::on_pushButton_hv_off_clicked()
{
    /** TODO:
     * Ver el tamaño de la trama
     */

    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("config").toStdString() + arpet->getFunCHV());
    arpet->setTrama_MCAE(arpet->getHeader_MCAE()+arpet->getHV_OFF()+arpet->getEnd_HV());
    cout<<arpet->getTrama_MCAE()<<endl;
}

void MainWindow::on_pushButton_hv_estado_clicked()
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("config").toStdString()+arpet->getFunCHV());

    /** TODO:
     * Obtener estado de la fuente HV y mostrarlo en el label
     */
    cout<<arpet->getHeader_MCAE()<<endl;
}

void MainWindow::setHeadModeConfig(int index)
{
    setHeadMode(index,"config");
}


/* Pestaña: "Gráficos" */

void MainWindow::on_pushButton_adquirir_clicked()
{
    int bytes_int=CHANNELS*6+16;
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead("graph").toStdString() + arpet->getFunCSP3());
    string pmt=ui->textEdit_pmt->toPlainText().toStdString();    
    arpet->setMCAEStream(pmt,bytes_int,arpet->getData_MCA());
    SendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
    string msg=ReadString();
    QString q_msg=QString::fromStdString(msg);
    string msg_data=ReadBufferString(bytes_int);
    arpet->getMCASplitData(msg_data,CHANNELS);

    int frame=arpet->getFrameMCA();
    long time_mca=arpet->getTimeMCA();
    cout<<"Frame: "<< frame <<endl;
    cout<<"Adquisition time: "<< time_mca <<endl;
    cout<<"Temperature: "<<arpet->getTempMCA()<<endl;

    // generate some data:
    QVector<double> x=arpet->getChannels();
    QVector<double> y=arpet->getHitsMCA();
    double c_max = *max_element(y.begin(),y.end());
    // create graph and assign data to it:
    ui->specHead->addGraph();
    ui->specHead->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->specHead->xAxis2->setVisible(true);
    ui->specHead->xAxis2->setTickLabels(false);
    ui->specHead->yAxis2->setVisible(true);
    ui->specHead->yAxis2->setTickLabels(false);
    ui->specHead->xAxis->setLabel("Canales");
    ui->specHead->yAxis->setLabel("Hits");
    // set axes ranges, so we see all data:
    ui->specHead->xAxis->setRange(0, 1024);
    ui->specHead->yAxis->setRange(0, c_max+1000);
    ui->specHead->replot();

    ui->label_21->setText(q_msg);

    cout<<arpet->getTrama_MCAE()<<endl;
}

void MainWindow::on_pushButton_decrease_clicked()
{
    QString pmt=ui->textEdit_pmt->toPlainText();
    if (pmt.toInt()>1)
    {
        int pmt_decrease=pmt.toInt()-1;
        ui->textEdit_pmt->setText(QString::number(pmt_decrease));
    }
}

void MainWindow::on_pushButton_increase_clicked()
{
    QString pmt=ui->textEdit_pmt->toPlainText();
    if (pmt.toInt()<48)
    {
        int pmt_increase=pmt.toInt()+1;
        ui->textEdit_pmt->setText(QString::number(pmt_increase));
    }
}

void MainWindow::setHeadModeGraph(int index)
{
    setHeadMode(index,"graph");
}

void MainWindow::setAdquireMode(int index)
{
    switch (index) {
    case MONOMODE:
        ui->pushButton_increase->show();
        ui->textEdit_pmt->show();
        ui->pushButton_decrease->show();
        break;
    case MULTIMODE:
        ui->pushButton_increase->hide();
        ui->textEdit_pmt->hide();
        ui->pushButton_decrease->hide();
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButton_8_clicked()
{

    // generate some data:
    QVector<double> x=arpet->getChannels();
    QVector<double> y=arpet->getHitsMCA();
    double c_max = *max_element(y.begin(),y.end());
    // create graph and assign data to it:
    ui->specHead->addGraph();
    ui->specHead->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->specHead->xAxis2->setVisible(true);
    ui->specHead->xAxis2->setTickLabels(false);
    ui->specHead->yAxis2->setVisible(true);
    ui->specHead->yAxis2->setTickLabels(false);
    ui->specHead->xAxis->setLabel("Canales");
    ui->specHead->yAxis->setLabel("Hits");
    // set axes ranges, so we see all data:
    ui->specHead->xAxis->setRange(0, 1024);
    ui->specHead->yAxis->setRange(0, c_max+1000);
    ui->specHead->replot();



}

void MainWindow::getMCA()
{
    QVector<double> x=arpet->getChannels();
    QVector<double> y=arpet->getHitsMCA();
    double c_max = *max_element(y.begin(),y.end());


    ui->specHead->addGraph();
    ui->specHead->graph(0)->setData(x, y);
    ui->specHead->xAxis2->setVisible(true);
    ui->specHead->xAxis2->setTickLabels(false);
    ui->specHead->yAxis2->setVisible(true);
    ui->specHead->yAxis2->setTickLabels(false);
    ui->specHead->xAxis->setLabel("Canales");
    ui->specHead->yAxis->setLabel("Cuentas");
    ui->specHead->xAxis->setRange(0, CHANNELS);
    ui->specHead->yAxis->setRange(0, c_max);
    ui->specHead->replot();

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

    QString head= getHead("config");
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

QString MainWindow::getHead(string tab)
{
    QString head;
    if (tab=="graph")
    {
        if (ui->comboBox_head_mode_select_graph->currentIndex()==MONOHEAD)
        {
           head=ui->comboBox_head_select_graph->currentText();
        }
    }
    else if (tab=="config")
    {
        if (ui->comboBox_head_mode_select_config->currentIndex()==MONOHEAD)
        {
           head=ui->comboBox_head_select_config->currentText();
        }
    }
    else head="";

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

string MainWindow::ReadBufferString(int buffer_size)
{
    string msg;
    try{
         arpet->portReadBufferString(&msg,buffer_size);
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
        QMessageBox::critical(this,tr("Error"),tr("No se puede acceder al puerto serie. Error: ") + tr(e.what()));
    }

    return bytes_transfered;
}

void MainWindow::manageHeadCheckBox(string tab, bool show)
{
    if (tab=="config")
    {
        if (show)
        {
            ui->checkBox_8->show();
            ui->checkBox_9->show();
            ui->checkBox_10->show();
            ui->checkBox_11->show();
            ui->checkBox_12->show();
            ui->checkBox_13->show();           
        }
        else
        {
            ui->checkBox_8->hide();
            ui->checkBox_9->hide();
            ui->checkBox_10->hide();
            ui->checkBox_11->hide();
            ui->checkBox_12->hide();
            ui->checkBox_13->hide();            
        }
    }
    else if(tab=="graph")
    {
        if (show)
        {
            ui->checkBox_1->show();
            ui->checkBox_2->show();
            ui->checkBox_3->show();
            ui->checkBox_4->show();
            ui->checkBox_5->show();
            ui->checkBox_6->show();
        }
        else
        {
            ui->checkBox_1->hide();
            ui->checkBox_2->hide();
            ui->checkBox_3->hide();
            ui->checkBox_4->hide();
            ui->checkBox_5->hide();
            ui->checkBox_6->hide();
        }

    }
    else return;
}

void MainWindow::manageHeadComboBox(string tab, bool show)
{
    if (tab=="config"){
        if (show) ui->comboBox_head_select_config->show();
        else ui->comboBox_head_select_config->hide();
    }
    else if(tab=="graph")
    {
        if (show) ui->comboBox_head_select_graph->show();
        else ui->comboBox_head_select_graph->hide();
    }
    else return;
}


void MainWindow::setHeadMode(int index, string tab)
{
    switch (index) {
    case MONOHEAD:
        manageHeadComboBox(tab, true);
        manageHeadCheckBox(tab, false);
        break;
    case MULTIHEAD:
        manageHeadComboBox(tab, false);
        manageHeadCheckBox(tab, true);
        break;
    default:
        break;
    }
}

/* TEST BUTTONS. TODO: Delete all of them after debug */

void MainWindow::on_pushButton_clicked()
{   
   QString sended = ui->plainTextEdit->toPlainText();

   size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
   string msg=ReadString();
   QString q_msg=QString::fromStdString(msg);

   ui->label_12->setText(q_msg);

}

void MainWindow::on_pushButton_2_clicked()
{
   arpet->portFlush();
}

void MainWindow::on_pushButton_3_clicked()
{
    QString sended="#C501090009@0064010;";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg=ReadString();
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::on_pushButton_4_clicked()
{
    QString sended="#C502090009@0064010;";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg=ReadString();
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::on_pushButton_5_clicked()
{
    QString sended="#C502071552@196515";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg=ReadString();
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    string msg_data=ReadBufferString(6160);

    QByteArray q_msg_data(msg_data.c_str(), msg_data.length());

    QByteArray y = q_msg_data.left(4);

    cout<<y.toStdString()<<endl;

//    sleep(1);
//    arpet->portFlush();
    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::on_pushButton_9_clicked()
{
    QString sended="#C501071552@01650<";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg=ReadString();
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    string msg_data=ReadBufferString(6160);

    arpet->getMCASplitData(msg_data,CHANNELS);

    int frame=arpet->getFrameMCA();
    long time_mca=arpet->getTimeMCA();
    cout<<"Frame: "<< frame <<endl;
    cout<<"Adquisition time: "<< time_mca <<endl;
    cout<<"Temperature: "<<arpet->getTempMCA()<<endl;
    QVector<double> canales=arpet->getChannels();
    QVector<double> hits=arpet->getHitsMCA();

    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::on_pushButton_6_clicked()
{
    QString sended="#C502071552@02650=";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg=ReadString();
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}

void MainWindow::on_pushButton_7_clicked()
{
    QString sended="#C501071552@01650<";
    size_t bytes=SendString(sended.toStdString(),arpet->getEnd_MCA());
    string msg=ReadString();
    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    ui->label_19->setText(q_bytes);
    ui->label_12->setText(q_msg);
    ui->label_20->setText(sended);
}



void MainWindow::on_pushButton_10_clicked()
{

}


/**********************************************************/
