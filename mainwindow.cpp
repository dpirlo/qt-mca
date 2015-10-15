#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include <QtSerialPort>


QSerialPort serial;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectPlottables  );
    ui->customPlot->xAxis->setRange(-8, 8);
    ui->customPlot->yAxis->setRange(-5, 5);
    ui->customPlot->axisRect()->setupFullAxesBox();

    ui->customPlot->plotLayout()->insertRow(0);
    ui->customPlot->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->customPlot, "Multi MCA"));
    ui->customPlot->xAxis->setLabel("Energia");
    ui->customPlot->yAxis->setLabel("Cuentas");

    ui->customPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->customPlot->legend->setFont(legendFont);
    ui->customPlot->legend->setSelectedFont(legendFont);
    ui->customPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

}




MainWindow::~MainWindow()
{
    delete ui;
    serial.close();         //libero el puerto cuando cierro el programa
}

void MainWindow::recibirdatosSerie()
{
     QByteArray data = serial.readAll();
     ui->textEdit->append(QString::fromUtf8(data));
}

void MainWindow::on_pushButton_clicked()
{

    QMessageBox *mbox = new QMessageBox(this);
    if(serial.isOpen())
    {
        serial.close();
        ui->pushButton->setText("Conectar");
        disconnect(&serial,SIGNAL(readyRead()));
    }
    else
    {
        serial.setPortName("/dev/ttyUSB0");
        if (!serial.open(QIODevice::ReadWrite)){
            mbox->setText("No se pudo abrir el puerto");
            mbox->exec();
        }
        else
        {
            serial.setParity(QSerialPort::NoParity);
            serial.setDataBits(QSerialPort::Data8);
            serial.setBaudRate(921600,QSerialPort::AllDirections);
            serial.setFlowControl(QSerialPort::NoFlowControl);
            serial.flush();
            connect(&serial,SIGNAL(readyRead()),this,SLOT(recibirdatosSerie()));
        }
        ui->pushButton->setText("Desconectar");
    }

    delete mbox;
}

void MainWindow::on_pushButton_2_clicked()
{
    if(serial.isOpen())
    {
        QString msg = "Hola!!!";
        serial.write(msg.toUtf8());
        connect(&serial,SIGNAL(readyRead()),this,SLOT(Graficaalgo()));
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->textEdit->clear();
}


void MainWindow::Graficaalgo(){

    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0; // x goes from 0 to 2
      y[i] = x[i]*x[i];  // let's plot a quadratic function
    }
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(x, y);

    ui->customPlot->xAxis->setRange(0, 2);
    ui->customPlot->yAxis->setRange(0, 2);

}
