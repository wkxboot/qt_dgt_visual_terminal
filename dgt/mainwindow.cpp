#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qobject.h"
#include "qmessagebox.h"
#include "communication.h"
#include "qtimer.h"
#include "qdebug.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),comm(new communication(0))
{
    ui->setupUi(this);
    ui->port_list->addItem("COM1");
    ui->port_list->addItem("COM2");
    ui->port_list->addItem("COM3");
    ui->port_list->addItem("COM4");
    ui->port_list->addItem("COM5");
    ui->port_list->addItem("COM6");
    ui->port_list->addItem("COM7");
    ui->port_list->addItem("COM8");
    ui->port_list->addItem("COM9");
    ui->port_list->addItem("COM10");
    ui->port_list->setCurrentIndex(0);

    ui->bandrate_list->addItem("115200");
    ui->bandrate_list->addItem("57600");
    ui->bandrate_list->addItem("9600");
    ui->bandrate_list->setCurrentIndex(0);

    ui->databits_list->addItem("8");
    ui->databits_list->addItem("7");
    ui->bandrate_list->setCurrentIndex(0);

    ui->parity_list->addItem("无校验");
    ui->parity_list->addItem("奇校验");
    ui->bandrate_list->addItem("偶校验");
    ui->bandrate_list->setCurrentIndex(0);

    QRegExp regExp("[1-9]{0,3}");
    ui->addr_input->setValidator(new QRegExpValidator(regExp, this));

    ui->addr_input->setText(QString::number(1));

    ui->weight_display->display("----------");

    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<int16_t>("int16_t");

    QObject::connect(this,SIGNAL(req_serial(QString,int,int,int)),comm,SLOT(handle_open_serial(QString,int,int,int)));

    QObject::connect(this,SIGNAL(req_scale(int,int,int)),comm,SLOT(handle_scale(int,int,int)));


    QObject::connect(comm,SIGNAL(rsp_open_serial(int,int)),this,SLOT(handle_open_serial(int,int)));
    QObject::connect(comm,SIGNAL(rsp_result(int,int,int)),this,SLOT(handle_scale_result(int,int,int)));

    QThread *thread = new QThread(0);
    comm->moveToThread(thread);
    comm->m_serial->moveToThread(thread);
    thread->start();


}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handle_open_serial(int rc,int type)
{
    if (type == communication::SERIAL_OPEN) {
        if (rc == communication::SERIAL_SUCCESS) {

            ui->port_list->setEnabled(false);
            ui->bandrate_list->setEnabled(false);
            ui->databits_list->setEnabled(false);
            ui->parity_list->setEnabled(false);
            ui->addr_input->setEnabled(false);

            ui->open->setText("关闭");
            qDebug("打开串口成功.\r\n");

            query_weight();
        } else {
            QMessageBox::information(this,"错误",ui->port_list->currentText() + "串口打开失败",QMessageBox::Ok);
        }

    } else {
        if (rc == communication::SERIAL_SUCCESS) {

            ui->port_list->setEnabled(true);
            ui->bandrate_list->setEnabled(true);
            ui->databits_list->setEnabled(true);
            ui->parity_list->setEnabled(true);
            ui->addr_input->setEnabled(true);

            ui->open->setText("打开");
            qDebug("关闭串口成功.\r\n");
            ui->weight_display->display("----------");
        } else {
            QMessageBox::information(this,"错误",ui->port_list->currentText() + "串口关闭失败",QMessageBox::Ok);
        }
    }
}

void MainWindow::handle_scale_result(int rc,int type,int value)
{
    switch (type) {
    case communication::QUERY_WEIGHT:
        if (rc == communication::SERIAL_SUCCESS && value != 0x7FFF) {
            ui->weight_display->display(QString::number(value));
         } else {
            ui->weight_display->display("err");
         }

        /*重新轮询净重*/
        emit req_scale(get_addr(),communication::QUERY_WEIGHT,0);
        break;
    case communication::REMOVE_TARE_WEIGHT:
        if (rc == 0) {
            QMessageBox::information(this,"成功","去皮成功",QMessageBox::Ok);
        } else {
            QMessageBox::information(this,"失败","去皮失败",QMessageBox::Ok);
        }
        break;
    case communication::CALIBRATION_WEIGHT_ZERO:
        if (rc == 0) {
            QMessageBox::information(this,"成功","0点校准成功",QMessageBox::Ok);
        } else {
            QMessageBox::information(this,"失败","0点校准失败",QMessageBox::Ok);
        }
        break;
    case communication::CALIBRATION_WEIGHT_FULL:
        if (rc == 0) {
             QMessageBox::information(this,"成功","增益校准成功",QMessageBox::Ok);
        } else {
             QMessageBox::information(this,"失败","增益校准失败",QMessageBox::Ok);
        }
        break;
    default:
        qDebug()<<"err in handle scale rsp.\r\n";

    }
}



int MainWindow::get_addr(void)
{
    uint8_t addr;

    if (ui->addr_input->text().isEmpty()){
       QMessageBox::information(this,"错误","变送器地址为空",QMessageBox::Ok);
       return -1;
    }

    addr = (uint8_t)ui->addr_input->text().toInt();

    return addr;
}

void MainWindow::query_weight()
{
    int addr;

    addr = get_addr();

    if (addr > 0) {
        emit req_scale(addr,communication::QUERY_WEIGHT,0);
    }
}

void MainWindow::on_calibration_zero_button_clicked()
{
    int addr;
    addr = get_addr();

    if (addr > 0) {
        emit req_scale(addr,communication::CALIBRATION_WEIGHT_ZERO,0);
    }
}

void MainWindow::on_calibration_1000_button_clicked()
{
    int addr;

    addr = get_addr();

    if (addr > 0) {
        emit req_scale(addr,communication::CALIBRATION_WEIGHT_FULL,1000);
    }
}

void MainWindow::on_calibration_2000_button_clicked()
{
    int addr;

    addr = get_addr();

    if (addr > 0) {
        emit req_scale(addr,communication::CALIBRATION_WEIGHT_FULL,2000);
    }
}

void MainWindow::on_calibration_5000_button_clicked()
{
    int addr;

    addr = get_addr();

    if (addr > 0) {
        emit req_scale(addr,communication::CALIBRATION_WEIGHT_FULL,5000);
    }
}

void MainWindow::on_remove_tare_button_clicked()
{
    int addr;

    addr = get_addr();

    if (addr > 0) {
        emit req_scale(addr,communication::REMOVE_TARE_WEIGHT,0);
    }
}

void MainWindow::on_open_clicked()
{
    int addr;

    addr = get_addr();
    if (addr > 0) {
        emit req_serial(ui->port_list->currentText(),ui->bandrate_list->currentText().toInt(),ui->databits_list->currentText().toInt(),ui->parity_list->currentIndex());
    }
}
