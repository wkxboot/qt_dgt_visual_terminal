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

    /*添加端口*/
    QStringList m_serialPortName;

    m_serialPortName  = comm->get_serial_port_name_list();
    QString name;
    foreach(name,m_serialPortName) {
        ui->port_list->addItem(name);
    }

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
    ui->parity_list->addItem("偶校验");
    ui->parity_list->setCurrentIndex(0);

    QRegExp regExp("[0-9]{0,3}");
    ui->addr_input->setValidator(new QRegExpValidator(regExp, this));

    ui->addr_input->setText(QString::number(1));

    ui->weight_display->display("----------");

    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<int16_t>("int16_t");

    QObject::connect(this,SIGNAL(ui_request(int,int)),comm,SLOT(handle_ui_request(int,int)));
    QObject::connect(comm,SIGNAL(rsp_ui_request_result(int,int,int)),this,SLOT(handle_ui_request_result(int,int,int)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handle_ui_request_result(int rc,int id,int value)
{
    switch (id) {
    case communication::MSG_ID_NET_WEIGHT:
        if (rc == 0 && value != 0x7FFF) {
            ui->weight_display->display(QString::number((int16_t)value));
         } else {
            ui->weight_display->display("err");
         }
        break;
    case communication::MSG_ID_REMOVE_TARE:
        if (rc == 0) {
            QMessageBox::information(this,"成功","去皮成功",QMessageBox::Ok);
        } else {
            QMessageBox::information(this,"失败","去皮失败",QMessageBox::Ok);
        }
        break;
    case communication::MSG_ID_CALIBRATE_ZERO:
        if (rc == 0) {
            QMessageBox::information(this,"成功","0点校准成功",QMessageBox::Ok);
        } else {
            QMessageBox::information(this,"失败","0点校准失败",QMessageBox::Ok);
        }
        break;
    case communication::MSG_ID_CALIBRATE_FULL:
        if (rc == 0) {
             QMessageBox::information(this,"成功","增益校准成功",QMessageBox::Ok);
        } else {
             QMessageBox::information(this,"失败","增益校准失败",QMessageBox::Ok);
        }
        break;
    case communication::MSG_ID_SENSOR_ID:
        if (rc == 0) {
            ui->sensor_id_display->setText(QString::number(value,16));
        } else {
            ui->sensor_id_display->setText("错误");
        }
        break;
    case communication::MSG_ID_FW_VERSION:
        if (rc == 0) {
            ui->fw_version_display->setText(QString::number(value >> 8) + "." + QString::number(value & 0xFF));
        } else {
            ui->fw_version_display->setText("错误");
        }
        break;
    case communication::MSG_ID_SET_ADDR:
        if (rc == 0) {
             QMessageBox::information(this,"成功","设置地址成功",QMessageBox::Ok);
        } else {
             QMessageBox::information(this,"失败","设置地址失败",QMessageBox::Ok);
        }
        break;
    default:
        qDebug()<<"err in msg id.\r\n";
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

int MainWindow::get_addr_setting(void)
{
    int addr;

    if (ui->addr_input_setting->text().isEmpty()){
       QMessageBox::information(this,"错误","变送器地址为空",QMessageBox::Ok);
       return -1;
    }

    addr = ui->addr_input_setting->text().toInt();

    return addr;
}

void MainWindow::on_calibration_zero_button_clicked()
{
    if (!comm->is_serial_open()) {
        QMessageBox::information(this,"错误","串口没有打开",QMessageBox::Ok);
        return;
    }
    emit ui_request(communication::MSG_ID_CALIBRATE_ZERO,0);
}

void MainWindow::on_calibration_1000_button_clicked()
{
    if (!comm->is_serial_open()) {
        QMessageBox::information(this,"错误","串口没有打开",QMessageBox::Ok);
        return;
    }
    emit ui_request(communication::MSG_ID_CALIBRATE_FULL,1000);
}

void MainWindow::on_calibration_2000_button_clicked()
{
    if (!comm->is_serial_open()) {
        QMessageBox::information(this,"错误","串口没有打开",QMessageBox::Ok);
        return;
    }
    emit ui_request(communication::MSG_ID_CALIBRATE_FULL,2000);
}

void MainWindow::on_calibration_5000_button_clicked()
{
    if (!comm->is_serial_open()) {
        QMessageBox::information(this,"错误","串口没有打开",QMessageBox::Ok);
        return;
    }
    emit ui_request(communication::MSG_ID_CALIBRATE_FULL,5000);
}

void MainWindow::on_remove_tare_button_clicked()
{
    if (!comm->is_serial_open()) {
        QMessageBox::information(this,"错误","串口没有打开",QMessageBox::Ok);
        return;
    }
    emit ui_request(communication::MSG_ID_REMOVE_TARE,0);
}


void MainWindow::on_set_addr_button_clicked()
{
    int addr_setting;

    if (!comm->is_serial_open()) {
        QMessageBox::information(this,"错误","串口没有打开",QMessageBox::Ok);
        return;
    }

    addr_setting = get_addr_setting();
    if (addr_setting >= 0) {
        emit ui_request(communication::MSG_ID_SET_ADDR,addr_setting);
    }
}

void MainWindow::on_open_clicked()
{
    int addr;

    addr = get_addr();
    if (addr >= 0) {
       if (comm->is_serial_open()) {
           comm->close_serial(ui->port_list->currentText());
           ui->port_list->setEnabled(true);
           ui->bandrate_list->setEnabled(true);
           ui->databits_list->setEnabled(true);
           ui->parity_list->setEnabled(true);
           ui->addr_input->setEnabled(true);

           ui->open->setText("打开");
           qDebug("关闭串口成功.\r\n");
           ui->weight_display->display("----------");
           ui->sensor_id_display->setText("未知");
           ui->fw_version_display->setText("未知");
       } else {
           if (comm->open_serial(ui->port_list->currentText(),ui->bandrate_list->currentText().toInt(),ui->databits_list->currentText().toInt(),ui->parity_list->currentText().toInt()) == 0) {
               ui->port_list->setEnabled(false);
               ui->bandrate_list->setEnabled(false);
               ui->databits_list->setEnabled(false);
               ui->parity_list->setEnabled(false);
               ui->addr_input->setEnabled(false);

               ui->open->setText("关闭");
               comm->set_comm_addr(addr);

               qDebug("打开串口成功.\r\n");
           } else {
               QMessageBox::information(this,"错误",ui->port_list->currentText() + "串口打开失败",QMessageBox::Ok);
           }
       }
    }

}

