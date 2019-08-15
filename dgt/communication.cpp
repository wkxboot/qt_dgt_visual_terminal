#include "mainwindow.h"
#include "communication.h"
#include "qmessagebox.h"
#include "crc16.h"
#include "qtimer.h"
#include "qdebug.h"
#include "qqueue.h"
#include "protocol.h"
#include "QTime"



communication::communication(QObject *parent):m_serial(new QSerialPort(this))
{
    m_queue = new QQueue<protocol>();

    m_queue_timer = new QTimer(this);
    m_queue_timer->setInterval(LOOP_QUEUE_TIMEOUT);

    QObject::connect(m_queue_timer,SIGNAL(timeout()),this,SLOT(loop_request_queue()));
/*
    QThread *thread = new QThread(this);
    moveToThread(thread);
    m_serial->moveToThread(thread);
    thread->start();
*/
}


communication::~communication()
{

}

QStringList communication::get_serial_port_name_list()
{
    QStringList m_serialPortName;

    QList<QSerialPortInfo>  serial_info;
    QSerialPortInfo info;

    serial_info = QSerialPortInfo::availablePorts();
    m_serialPortName.clear();

     foreach(info,serial_info) {
         m_serialPortName.append(info.portName());
     }
    return m_serialPortName;
}


int communication::open_serial(QString port_name,int baud_rates,int data_bits,int parity)
{
    bool success;

    if (!m_serial->isOpen()) {
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
            m_serial->flush();
            m_queue_timer->start();
        } else {
            qDebug() << QString(port_name + "打开失败.");
            return -1;
        }
   }

   return 0;
}

int communication::close_serial(QString port_name)
{
    if (m_serial->isOpen()) {
        m_serial->close();
        qDebug() << QString(port_name + "关闭成功.");
        m_queue_timer->stop();
    }
    return 0;
}

int communication::is_serial_open()
{
    return m_serial->isOpen() ? 1 : 0;
}

int communication::set_comm_addr(int addr)
{
    m_addr = addr;
    return 0;
}

void communication::query_weight()
{
    protocol request;

    request.construct(MSG_ID_NET_WEIGHT,m_addr,protocol::PROTOCOL_CODE_NET_WEIGHT,0,0,QUERY_WEIGHT_TIMEOUT);

    m_queue->append(request);
    qDebug() << "insert query weight request";
}

void communication::remove_tare_weight()
{
    protocol request;

    request.construct(MSG_ID_REMOVE_TARE,m_addr,protocol::PROTOCOL_CODE_REMOVE_TARE,0,0,REMOVE_TARE_TIMEOUT);

    m_queue->append(request);
    qDebug() << "insert remove tare weight request";
}

void communication::calibration_zero(int weight)
{
    protocol request;
    int value[2];
    value[0] = weight & 0xFF;
    value[1] = weight >> 8;

    request.construct(MSG_ID_CALIBRATE_ZERO,m_addr,protocol::PROTOCOL_CODE_CALIBRATE_ZERO,2,value,CALIBRATION_TIMEOUT);

    m_queue->append(request);
    qDebug() << "insert cal zero request";
}

void communication::calibration_full(int weight)
{
    protocol request;
    int value[2];
    value[0] = weight & 0xFF;
    value[1] = weight >> 8;

    request.construct(MSG_ID_CALIBRATE_FULL,m_addr,protocol::PROTOCOL_CODE_CALIBRATE_FULL,2,value,CALIBRATION_TIMEOUT);

    m_queue->append(request);
    qDebug() << "insert cal full request";
}

void communication::query_sensor_id()
{
    protocol request;

    request.construct(MSG_ID_SENSOR_ID,m_addr,protocol::PROTOCOL_CODE_SENSOR_ID,0,0,QUERY_SENSOR_ID_TIMEOUT);

    m_queue->append(request);
    qDebug() << "insert sensor id request";
}

void communication::query_fw_version()
{
    protocol request;

    request.construct(MSG_ID_FW_VERSION,m_addr,protocol::PROTOCOL_CODE_FW_VERSION,0,0,QUERY_FW_VERSION_TIMEOUT);

    m_queue->append(request);
    qDebug() << "insert fw version request";
}

void communication::set_addr(int addr)
{
    protocol request;

    request.construct(MSG_ID_SET_ADDR,m_addr,protocol::PROTOCOL_CODE_SET_ADDR,1,&addr,SET_ADDR_TIMEOUT);
    m_queue->append(request);

    qDebug() << "insert set addr request";
}


/*插入周期轮询请求*/
void communication::insert_period_query_req()
{
    static int duty_multiple = 0;
    query_weight();

    /*主要轮询重量 兼顾其他*/
    duty_multiple++;
    if (duty_multiple >= 4) {
        query_sensor_id();
        query_fw_version();
        duty_multiple  = 0;
    }

}


void communication::handle_ui_request(int id,int value)
{

    switch (id)
    {
    case MSG_ID_NOTIFY_ADDR:
        m_addr = value;
        break;
    case MSG_ID_REMOVE_TARE:
        remove_tare_weight();
        break;
    case MSG_ID_CALIBRATE_ZERO:
        calibration_zero(value);
        break;
    case MSG_ID_CALIBRATE_FULL:
        calibration_full(value);
        break;
    case MSG_ID_SET_ADDR:
        set_addr(value);
        break;
    default:
        break;
    }
}


/*处理请求队列*/
void communication::loop_request_queue()
{
    int rc;
    protocol request;
    QByteArray rsp;
    QByteArray temp;
    QTime time;
    int timeout;

    m_queue_timer->stop();

    if (m_queue->isEmpty()) {
        insert_period_query_req();
    }

    request = m_queue->dequeue();
    qDebug("process a req...cur queue size:%d.",m_queue->size());

    m_serial->write(request.m_request_buffer);

    time.start();
    qDebug("wait rsp...");
    timeout = request.get_timeout();
    /*循环读取直到超时*/
    while ( m_serial->waitForReadyRead(timeout)) {
        timeout = CONTINUE_TIMEOUT;
        temp = m_serial->readAll();
        rsp.append(temp);
    }

    qDebug("rsp complete.total time:%d",time.elapsed());
    rc = request.parse(rsp);
    rsp_ui_request_result(rc,request.get_msg_id(),request.get_parse_value());

    m_queue_timer->start();

}
