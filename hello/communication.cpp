#include "mainwindow.h"
#include "communication.h"
#include "qmessagebox.h"
#include "crc16.h"
#include "qtimer.h"
#include "qdebug.h"


communication::communication(QObject *parent):serial(new QSerialPort(this)),crc(new crc16)
{
    opened = false;
    rsp_timer = new QTimer(this);
    QObject::connect(serial,SIGNAL(readyRead()),this,SLOT(on_data_ready()));
    QObject::connect(rsp_timer,SIGNAL(timeout()),this,SLOT(timer_timeout()));
}

communication::~communication()
{

}


void communication::on_data_ready()
{
    QByteArray src_array;
    uint8_t src[20];
    uint8_t src_size;

    int16_t weight;
    uint16_t crc_recv,crc_calculate;

    uint8_t code_recv;
    uint8_t addr_recv;

    qDebug("收到串口数据.");
    rsp_timer->stop();

    src_array = serial->readAll();
    if (src_array.size() < 8 || src_array.size() > 20)
    {
        /*回应长度错误*/
        emit rsp_scale_req_result(opt_code,-1,"长度错误,或者超时") ;
        return;
    }
    src_size = (uint8_t)src_array.size();
    memcpy(src,(uint8_t *)src_array.data(),src_size);


    crc_recv = src[src_size - 2] + src[src_size - 1] * 256;
    crc_calculate = crc->calculate(src,src_size - 2);
    if (crc_recv != crc_calculate) {
        /*校验错误*/
        emit rsp_scale_req_result(opt_code,-2,"校验错误.") ;
        return;
    }

    addr_recv = src[3];
    code_recv = src[4];

    if (scale_addr != addr_recv) {
        /*回应地址错误*/
        emit rsp_scale_req_result(opt_code,-3,"地址错误.") ;
        return;

    }
    if (code_recv != opt_code) {
        /*回应code错误*/
        emit rsp_scale_req_result(opt_code,-4,"code错误.") ;
        return;
    }

    switch(code_recv) {
    /*净重值*/
    case 0:
        weight = src[5] + src[6] * 256;
        emit rsp_scale_req_result(opt_code,0,QString::number(weight)) ;
    break;
    /*去皮*/
    case 1:
        /*成功*/
        if (src[5] == 0) {
            emit rsp_scale_req_result(opt_code,0,"去皮成功.");
        } else {
            emit rsp_scale_req_result(opt_code,1,"去皮失败.");
        }

    break;
    /*0点校准*/
    case 2:
        /*成功*/
        if (src[5] == 0) {
            emit rsp_scale_req_result(opt_code,0,"0点校准成功.");
        } else {
            emit rsp_scale_req_result(opt_code,1,"0点校准失败.");
        }

    break;
    /*增益校准*/
    case 3:
        /*成功*/
        if (src[5] == 0) {
            emit rsp_scale_req_result(opt_code,0,"增益校准成功.");
        } else {
            emit rsp_scale_req_result(opt_code,1,"增益校准失败.");
        }
    break;
    }
    return;
}


void communication::timer_timeout()
{
    qDebug("定时器超时.");
    on_data_ready();
}

void communication::on_serial_req(QString port,int baudrate)
{
    bool open_success;
    if (!opened) {
        serial->setPortName(port);
        serial->setBaudRate(baudrate);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        open_success = serial->open(QSerialPort::ReadWrite);

        if (open_success) {
             emit rsp_serial_open_req_result(0);
            opened = true;
        } else {
            emit rsp_serial_open_req_result(-1);
        }
    } else {
       opened = false;
       serial->flush();
       serial->close();
       emit rsp_serial_close_req_result(0);

    }
}

void communication::on_scale_req(uint8_t addr,uint8_t code,int param)
{
    uint16_t crc_calculate;
    uint8_t *send;
    uint8_t send_size;

    uint8_t net_weight[7] = {0x4d,0x4c,0x02,0x00,0x00,0x00,0x00};
    uint8_t remove_tare[7] = {0x4d,0x4c,0x02,0x00,0x01,0x00,0x00};

    uint8_t calibration_zero[9] = {0x4d,0x4c,0x04,0x00,0x02,0x00,0x00,0x00,0x00};
    uint8_t calibration_full[9] = {0x4d,0x4c,0x04,0x00,0x03,0x00,0x00,0x00,0x00};

    scale_addr = addr;
    opt_code = code;

    switch (code)
    {
    case MainWindow::OPT_QUERY_NET_WEIGHT:
       net_weight[3] = addr;
       send = net_weight;
       send_size = 7;
    break;
    case MainWindow::OPT_REMOVE_TARE:
       remove_tare[3] = addr;
       send = remove_tare;
       send_size = 7;
    break;
    case MainWindow::OPT_CALIBRATION_ZERO:
       calibration_zero[3] = addr;
       send = calibration_zero;
       send_size = 9;
    break;
    case MainWindow::OPT_CALIBRATION_FULL:
       calibration_full[3] = addr;
       calibration_full[5] = (uint16_t)param & 0xFF;
       calibration_full[6] = (uint16_t)param >> 8;
       send = calibration_full;
       send_size = 9;
    break;
    default:
        return;
    }
     crc_calculate = crc->calculate(send,send_size - 2);

     send[send_size - 2] = crc_calculate & 0xFF;
     send[send_size - 1] = crc_calculate >> 8;


    if (opened) {
        serial->write((char *)send,send_size);
        rsp_timer->start(1000);
    } else {
        emit rsp_scale_req_result(code,-100,"写串口失败.");
    }

}
