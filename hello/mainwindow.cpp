#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qmessagebox.h"
#include "qobject.h"
#include "communication.h"
#include "qtimer.h"

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
    comm->serial->moveToThread(thread);
    thread->start();

    loop_weight_timer = new QTimer(this);
    loop_weight_timer->setInterval(1100);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loop_weight_timeout()
{

}

void MainWindow::rsp_serial_open_result(int rc)
{
    if (rc == 0) {
        ui->port_list->setEnabled(false);
        ui->bandrate_list->setEnabled(false);
        ui->open->setText("关闭");
        qDebug("打开串口成功.\r\n");
    } else {
        QMessageBox::information(this,"错误",ui->port_list->currentText() + "端口无效",QMessageBox::Ok);
    }
}

void MainWindow::rsp_serial_close_result(int rc)
{
    if (rc == 0) {
    ui->port_list->setEnabled(true);
    ui->bandrate_list->setEnabled(true);
    ui->open->setText("打开");
    qDebug("关闭串口成功.");
    } else {
        QMessageBox::information(this,"错误",ui->port_list->currentText() + "关闭串口失败.\r\n",QMessageBox::Ok);
    }
}


void MainWindow::rsp_scale_result(uint8_t opt,int16_t result,QString err)
{
    switch (opt) {
    case MainWindow::OPT_CALIBRATION_ZERO:
        ui->calibration_zero_button->setEnabled(true);
         if (result == 0) {
             QMessageBox::information(this,"成功","0点校准成功",QMessageBox::Ok);
         } else {
             QMessageBox::information(this,"失败","0点校准失败",QMessageBox::Ok);
        }
        break;
    case MainWindow::OPT_CALIBRATION_FULL:
        //ui->calibration_1000_button->setEnabled(true);
         if (result == 0) {
             QMessageBox::information(this,"成功","增益校准成功",QMessageBox::Ok);
         } else {
             QMessageBox::information(this,"失败","增益校准失败",QMessageBox::Ok);
        }
        break;
    case MainWindow::OPT_QUERY_NET_WEIGHT:
         if (result < 0) {
             if (loop_weight) {
              ui->weight_display->display("Err");
             } else {
              ui->weight_display->display("------");
             }
         } else {
             ui->weight_display->display(err.toInt());
             QThread::msleep(20);
             emit req_scale((uint8_t)get_addr(),MainWindow::OPT_QUERY_NET_WEIGHT,0);
         }
        /*重新轮询*/
         emit
        break;
    case MainWindow::OPT_REMOVE_TARE:
        if (result == 0) {
            QMessageBox::information(this,"成功","去皮成功",QMessageBox::Ok);
        } else {
            QMessageBox::information(this,"失败","去皮失败",QMessageBox::Ok);
       }
        break;

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
