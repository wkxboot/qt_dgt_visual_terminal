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

    ui->addr_input->setText("1");

    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<int16_t>("int16_t");

    QObject::connect(this,SIGNAL(req_serial(QString,int)),comm,SLOT(on_serial_req(QString,int)));
    QObject::connect(comm,SIGNAL(rsp_serial_open_req_result(int)),this,SLOT(rsp_serial_open_result(int)));
    QObject::connect(comm,SIGNAL(rsp_serial_close_req_result(int)),this,SLOT(rsp_serial_close_result(int)));

    QObject::connect(this,SIGNAL(req_scale(uint8_t,uint8_t,int)),comm,SLOT(on_scale_req(uint8_t,uint8_t,int)));
    QObject::connect(comm,SIGNAL(rsp_scale_req_result(uint8_t,int16_t,QString)),this,SLOT(rsp_scale_result(uint8_t,int16_t,QString)));

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
            ui->open->setText("关闭");
            qDebug("打开串口成功.\r\n");
        } else {
            QMessageBox::information(this,"错误",ui->port_list->currentText() + "串口打开失败",QMessageBox::Ok);
        }

    } else {
        if (rc == communication::SERIAL_SUCCESS) {

            ui->port_list->setEnabled(true);
            ui->bandrate_list->setEnabled(true);
            ui->open->setText("打开");
            qDebug("关闭串口成功.\r\n");
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
        emit req_scale((uint8_t)get_addr(),communication::QUERY_WEIGHT,0);
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

    if (addr == 0) {
        QMessageBox::information(this,"错误","变送器地址错误",QMessageBox::Ok);
        return -1;
    }
    return addr;
}

void MainWindow::on_weight_loop_button_clicked()
{
    int rc;
    rc = get_addr();
    if (rc > 0) {
        emit req_scale((uint8_t)rc,MainWindow::OPT_QUERY_NET_WEIGHT,0);
    if (loop_weight == false) {
        loop_weight = true;
        ui->addr_input->setEnabled(false);
        ui->weight_loop_button->setText("关闭轮询净重");
        ui->remove_tare_button->setEnabled(false);
        ui->calibration_zero_button->setEnabled(false);
        ui->calibration_1000_button->setEnabled(false);
        ui->calibration_2000_button->setEnabled(false);
        ui->calibration_5000_button->setEnabled(false);

    } else {
        loop_weight = false;

        ui->weight_loop_button->setText("轮询净重");
        ui->addr_input->setEnabled(true);
        ui->remove_tare_button->setEnabled(true);
        ui->calibration_zero_button->setEnabled(true);
        ui->calibration_1000_button->setEnabled(true);
        ui->calibration_2000_button->setEnabled(true);
        ui->calibration_5000_button->setEnabled(true);
    }
    }
}

void MainWindow::on_calibration_zero_button_clicked()
{
    int rc;
    rc = get_addr();
    if (rc > 0) {
        emit req_scale((uint8_t)rc,MainWindow::OPT_CALIBRATION_ZERO,0);
    }
}

void MainWindow::on_calibration_1000_button_clicked()
{
    int rc;
    rc = get_addr();
    if (rc > 0) {
        emit req_scale((uint8_t)rc,MainWindow::OPT_CALIBRATION_FULL,1000);
    }
}

void MainWindow::on_calibration_2000_button_clicked()
{
    int rc;
    rc = get_addr();
    if (rc > 0) {
        emit req_scale((uint8_t)rc,MainWindow::OPT_CALIBRATION_FULL,2000);
    }
}

void MainWindow::on_calibration_5000_button_clicked()
{
    int rc;
    rc = get_addr();
    if (rc > 0) {
        emit req_scale((uint8_t)rc,MainWindow::OPT_CALIBRATION_FULL,5000);
    }
}

void MainWindow::on_remove_tare_button_clicked()
{
    int rc;
    rc = get_addr();
    if (rc > 0) {
        emit req_scale((uint8_t)rc,MainWindow::OPT_REMOVE_TARE,0);
    }
}

void MainWindow::on_open_clicked()
{
    emit req_serial(ui->port_list->currentText(),ui->bandrate_list->currentText().toInt());
}
