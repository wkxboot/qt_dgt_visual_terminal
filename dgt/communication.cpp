#include "mainwindow.h"
#include "communication.h"
#include "qmessagebox.h"
#include "crc16.h"
#include "qtimer.h"
#include "qdebug.h"
#include "qqueue.h"
#include "request.h"
#include "QTime"

communication::communication(QObject *parent):m_serial(new QSerialPort(this)),m_crc(new crc16)
{
    m_open = false;
    m_queue = new QQueue<request>();

    QObject::connect(m_serial,SIGNAL(readyRead()),this,SLOT(on_data_ready()));
}

communication::~communication()
{

}

void communication::handle_open_serial(QString port_name,int baud_rates,int data_bits,int stop_bits,int parity)
{
    bool success;

    if (m_open) {
       m_serial->close();
       qDebug() << QString(port_name + "关闭成功.");
       emit rsp_open_serial(SERIAL_SUCCESS,SERIAL_CLOSE);
       m_open = false;
   } else {
       m_serial->setPortName(port_name);
       m_serial->setBaudRate(baud_rates);
       if (data_bits == 8) {
          m_serial->setDataBits(QSerialPort::Data8);
       } else {
          m_serial->setDataBits(QSerialPort::Data7);
       }

       if (parity == 0){
           m_serial->setParity(QSerialPort::NoParity);
       } else if (parity == 1){
           m_serial->setParity(QSerialPort::OddParity);
       } else  {
           m_serial->setParity(QSerialPort::EvenParity);
       }

       success = m_serial->open(QSerialPort::ReadWrite);
       if (success) {
           qDebug() << QString(port_name + "打开成功.");
           m_open = true;
           m_serial->flush();

           emit rsp_open_serial(SERIAL_SUCCESS,SERIAL_OPEN);/*发送成功信号*/
       } else {
           qDebug() << QString(port_name + "打开失败.");
           emit rsp_open_serial(SERIAL_FAIL,SERIAL_OPEN);/*发送失败信号*/
       }
    }
}


void communication::handle_query_weight(int addr)
{
    request req;
    uint16_t crc16;

    req.m_addr = addr;
    req.m_type = QUERY_WEIGHT;
    req.m_timeout = RSP_TIMEOUT;

    req.m_buffer[0] = 0x4d;
    req.m_buffer[1] = 0x4c;
    req.m_buffer[2] = 0x02;
    req.m_buffer[3] = addr;
    req.m_buffer[4] = QUERY_WEIGHT;

    m_crc->calculate((uint8_t *)req.m_buffer.data(),req.m_buffer.size());
    req.m_buffer[5] = crc16 & 0xFF
    req.m_buffer[6] = crc16 >> 8;;

    m_queue->append(req);
}

void communication::handle_remove_tare_weight(int addr)
{
    request req;
    uint16_t crc16;

    req.m_addr = addr;
    req.m_type = REMOVE_TARE_WEIGHT;
    req.m_timeout = RSP_TIMEOUT;

    req.m_buffer[0] = 0x4d;
    req.m_buffer[1] = 0x4c;
    req.m_buffer[2] = 0x02;
    req.m_buffer[3] = addr;
    req.m_buffer[4] = REMOVE_TARE_WEIGHT;

    m_crc->calculate((uint8_t *)req.m_buffer.data(),req.m_buffer.size());
    req.m_buffer[5] = crc16 & 0xFF
    req.m_buffer[6] = crc16 >> 8;;

    m_queue->append(req);

}

void communication::handle_calibration_weight(int addr,int weight)
{
    request req;
    uint16_t crc16;

    req.m_addr = addr;
    if (weight == 0) {
        req.m_type = CALIBRATION_WEIGHT_ZERO;
    } else {
       req.m_type = CALIBRATION_WEIGHT_FULL;
    }
    req.m_timeout = RSP_TIMEOUT;

    req.m_buffer[0] = 0x4d;
    req.m_buffer[1] = 0x4c;
    req.m_buffer[2] = 0x04;
    req.m_buffer[3] = addr;
    req.m_buffer[4] = req.m_type;

    req.m_buffer[5] = weight & 0xFF;
    req.m_buffer[6] = weight >> 8;

    m_crc->calculate((uint8_t *)req.m_buffer.data(),req.m_buffer.size());
    req.m_buffer[7] = crc16 & 0xFF;
    req.m_buffer[8] = crc16 >> 8;;

    m_queue->append(req);
}

/*处理请求队列*/
void communication::handle_request_queue()
{
    request req;
    QByteArray rsp;

    if (!m_open) {
        qWarning("串口是关闭的，无法发送数据.");
    }

    /*没有请求就直接返回*/
    if (m_queue->isEmpty()) {
        return;

    }

    m_queue_timer->stop();
    req = m_queue->dequeue();
    qDebug("process a req...");


    m_serial->write(req.m_buffer,req.m_buffer.size());

    rsp = wait_rsp(req.m_timeout);
    if (rsp.size() > 0) {
        handle_rsp(rsp,req.m_addr,req.m_type);
    }

     m_queue_timer->start();

}

/*等待回应*/
QByteArray communication::wait_rsp(int timeout)
{
    QByteArray rsp;
    QByteArray temp;
    QTime time;
    time.start();

    qDebug("wait rsp...");

    /*循环读取直到超时*/
    while ( m_serial->waitForReadyRead(timeout)) {

        timeout = CONTINUE_TIMEOUT;
        temp = m_serial->readAll();
        rsp.append(temp);
    }

    qDebug("rsp complete.total time:%d",time.elapsed());

}


void communication::handle_rsp(QByteArray rsp, int addr, int type)
{
    uint16_t crc_recv,crc_calculate;
    int size;
    uint8_t type_recv;
    uint8_t addr_recv;
    int weight;

    qDebug("处理回应...");
    size = rsp.size();

    if (size > 20) {
        /*回应长度错误*/
        qDebug("type:%d回应长度：%d错误.",type,size);
        emit rsp_result(-1,type,size);
        return;
    } 
    crc_recv = rsp[size - 2] + rsp[size - 1] * 256;
    crc_calculate = m_crc->calculate((uint8_t *)rsp.data(),size - 2);

    if (crc_recv != crc_calculate) {
        /*校验错误*/
        qDebug("type:%d校验错误.",type);
        emit rsp_result(-1,type,crc_recv) ;
        return;
    }

    addr_recv = rsp[3];
    type_recv = rsp[4];

    if (addr != addr_recv) {
        /*回应地址错误*/
        qDebug("type:%d回应地址：%d错误.",type,addr_recv);
        emit rsp_result(-1,type,addr_recv);
        return;
    }

    if (type_recv != type) {
        /*回应类型错误*/
        qDebug("type:%d回应类型：%d错误.",type,type_recv);
        emit rsp_result(-1,type,type_recv);
        return;
    }

    switch(type) {
    /*净重值*/
    case QUERY_WEIGHT:
        weight = rsp[5] + rsp[6] * 256;
        /*回应重量*/
        qDebug("重量值：%d",weight);
        emit rsp_result(0,type,weight);
        
    break;
    /*去皮*/
    case REMOVE_TARE_WEIGHT:
        /*成功*/
        if (rsp.at(5) == 0) {
            emit rsp_result(0,type,0);
        } else {
            emit rsp_result(-1,type,0);
        }

    break;
    /*0点校准*/
    case CALIBRATION_WEIGHT_ZERO:
        /*成功*/
        if (rsp.at(5) == 0) {
            emit rsp_result(0,type,0);
        } else {
            emit rsp_result(-1,type,0);
        }

    break;
    /*增益校准*/
    case CALIBRATION_WEIGHT_FULL:
        /*成功*/
        if (rsp.at(5) == 0) {
            emit rsp_result(0,type,0);
        } else {
            emit rsp_result(-1,type,0);
        }

    break;

    default:
        break;
    }

}
