#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <iostream>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize Parameters
    SPS = 500;
    TPS = 10;
    AC_coupling = true;
    V_ref = 3.3;
    resolution = 1024;
    request_complete = 1;
    paused = false;

    // coupling button group initialization
    coupling.addButton(ui->AC_rbutton);
    coupling.addButton(ui->DC_rbutton);

    // Initialize Display Settings
    this->setFixedSize(1024, 768);
    ui->open_port_btn->setText("Open");
    display = ui->plot;
    display->addGraph();
    display->graph(0)->setScatterStyle(QCPScatterStyle::ssDot);
    display->graph(0)->setLineStyle(QCPGraph::lsLine);
    display->xAxis->setLabel("Time (ms)");
    display->yAxis->setLabel("Voltage (V)");
    display->xAxis->setRange(0, 10);
    display->yAxis->setRange(-4, 4);
    display->xAxis->setLabelColor(Qt::white);
    display->xAxis->setTickLabelColor(Qt::white);
    display->yAxis->setLabelColor(Qt::white);
    display->yAxis->setTickLabelColor(Qt::white);
    display->setBackground(Qt::black);
    QPen pen;
    pen.setWidth(1);
    pen.setColor(QColor(247, 254, 32));
    display->graph(0)->setPen(pen);


    port = new QSerialPort;
    QTimer* timer;
    QTimer* timer2;
    timer = new QTimer;
    timer2 = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(update_available_ports()));
    connect(timer2, SIGNAL(timeout()), this, SLOT(refresh()));
    timer2->start(100);
    timer->start(500);
    ui->available_ports->addItem("Choose Port");
}

void MainWindow::update_available_ports()
{
    // get a new list of available ports
    QVector<QString> new_list;
    Q_FOREACH(QSerialPortInfo available_port, QSerialPortInfo::availablePorts()) {
        new_list.push_back(available_port.portName());
    }

    // check if port_list has not changed
    if(port_list == new_list) return;

    // update the list while keeping the current choice if it still exists
    QString current = ui->available_ports->currentText();
    ui->available_ports->clear();
    if(new_list.contains(current))
        ui->available_ports->addItem(current);
    ui->available_ports->addItem("Choose Port");
    Q_FOREACH(QString available_port, new_list) {
        if(available_port != current)
            ui->available_ports->addItem(available_port);
    }
    port_list = new_list;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_open_port_btn_clicked()
{
    if(!port->isOpen())
    {
        QString port_name = ui->available_ports->currentText();
        if(port_name == "Choose Port")
            return;
        port->setPortName(port_name);
        port->setBaudRate(2000000);
        port->setParity(QSerialPort::NoParity);
        port->setDataBits(QSerialPort::Data8);
        port->setStopBits(QSerialPort::OneStop);
        port->open(QIODeviceBase::ReadWrite);
        connect(port, SIGNAL(readyRead()), this, SLOT(read_serial()));
        ui->open_port_btn->setText("Close");
        serial_buffer = "";
        request_complete = 1;
        samples = SPS;
    }
    else{
        port->close();
        ui->open_port_btn->setText("Open");
    }

}

void MainWindow::read_serial()
{
    while(port->canReadLine()){
        serial_buffer += (QString::fromUtf8(port->readLine()));
        //if(data.back() == "END\n"){
        //    end = true;
        //}
    }
}

void MainWindow::request_data()
{
    if(paused)
        return;
    if(!stream)
        delete stream;
    stream = new QTextStream(&serial_buffer);
    serial_buffer.clear();
    data.clear();
    if(SPS / double(TPS) >= 10){
        samples = TPS * 10;
        delay = 0;
    }
    else{
        samples = SPS;
        delay = (1000 * (TPS - 0.1 * samples)) / samples;
    }
    port->write(("S" + QString::number(samples) + "\n").toStdString().c_str());
    data.reserve(samples);
    port->write(("D" + QString::number(delay) + "\n").toStdString().c_str());
}


void MainWindow::refresh(){
    int idx = serial_buffer.indexOf('\n');
    while(idx != -1 && data.size() < samples){
        data.push_back(stream->readLine());
        idx = serial_buffer.indexOf('\n', idx + 1);
    }

    if(!request_complete && data.size() == samples){
        y.clear(); x.clear();
        y.reserve(SPS); x.reserve(SPS);

        V_max = 0;
        double V_min = 1024;
        for(int i = 0; i < samples; i++){
            y.push_back(data[i].toDouble());
            if(y.back() > 1024){
                std::cout<<y.back() << std::endl;
                y.pop_back();
                continue;
            }
            V_max = std::max(V_max, y.back());
            V_min = std::min(V_min, y.back());
            x.push_back(i * (TPS / double(samples - 1)));
        }
        V_pp = V_max - V_min;
        for(int i = 0; i < y.size(); i++){
            if(AC_coupling)
                y[i] = (y[i] - (V_max - V_pp /2.0)) * V_ref / resolution;
            else
                y[i] *= V_ref / resolution;
        }
        V_pp *= V_ref / resolution;

        display->graph(0)->data()->clear();
        display->graph(0)->setData(x, y);
        display->replot();
        request_complete = 1;
    }
    if(port->isOpen() && request_complete){
        request_data();
        request_complete = 0;
    }

}



void MainWindow::on_AC_rbutton_toggled(bool checked)
{
    AC_coupling = checked;
}


void MainWindow::on_DC_rbutton_toggled(bool checked)
{
    AC_coupling = !checked;
}


void MainWindow::on_lineEdit_2_textChanged(const QString &arg1)
{
}


void MainWindow::on_lineEdit_2_textEdited(const QString &arg1)
{
    if(arg1 == ""){
        if(port->isOpen())
            ui->open_port_btn->click();
        return;
    }
    bool valid;
    double value = arg1.toDouble(&valid);
    if(valid){
        if(value <= 0 || value > 1000){
            ui->lineEdit_2->setText(QString::number(V_ref));
            return;
        }
        V_ref = value;
        display->yAxis->setRange(-ceil(value), ceil(value));
    }
    else
        ui->lineEdit_2->setText(QString::number(V_ref));
}


void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    TPS = 10 + (value - 1) * (190.0 / 99.0);
    display->xAxis->setRange(0, TPS);
    if(!port->isOpen())
        display->replot();
}


void MainWindow::on_lineEdit_textEdited(const QString &arg1)
{
    if(arg1 == ""){
        if(port->isOpen())
            ui->open_port_btn->click();
        return;
    }
    bool valid;
    int value = arg1.toInt(&valid);
    if(valid){
        if(value <= 0){
            ui->lineEdit->setText(QString::number(SPS));
            return;
        }
        SPS = value;
    }
    else
        ui->lineEdit->setText(QString::number(SPS));
}


void MainWindow::on_pushButton_clicked()
{
    if(!paused)
        ui->pushButton->setText("Unpause");
    else
        ui->pushButton->setText("Pause");
    paused = !paused;
}


void MainWindow::on_verticalSlider_valueChanged(int value)
{
    int amp = ceil(3.3 * value / 100.0);
    display->yAxis->setRange(-amp, amp);
}


void MainWindow::on_resolution_slot_textEdited(const QString &arg1)
{
    if(arg1 == ""){
        if(port->isOpen())
            ui->open_port_btn->click();
        return;
    }
    bool valid;
    int value = arg1.toInt(&valid);
    if(valid){
        if(value <= 0){
            ui->resolution_slot->setText(QString::number(resolution));
            return;
        }
        resolution = value;
    }
    else
        ui->resolution_slot->setText(QString::number(resolution));
}

