#include "protocol.h"

protocol::protocol()
{

}

protocol::~protocol()
{

}

int protocol::construct(int msg_id,int addr,int code,int value_cnt,int *value,int timeout)
{
    QByteArray request_buffer;
    uint16_t crc16;

    int index = 0;
    m_msg_id = msg_id;
    m_addr = addr;
    m_code = code;
    m_timeout = timeout;

    request_buffer[index ++] = 0x4d;
    request_buffer[index ++] = 0x4c;
    request_buffer[index ++] = value_cnt + 2;
    request_buffer[index ++] = addr;
    request_buffer[index ++] = code;

    for (uint8_t i = 0; i < value_cnt;i ++) {
        request_buffer[index ++] = (uint8_t)(*value ++);
    }
    crc16 = m_crc.calculate((uint8_t *)request_buffer.data(),index);

    request_buffer[index ++] = crc16 & 0xFF;
    request_buffer[index ++] = crc16 >> 8;

    m_request_buffer = request_buffer;
    return 0;
}


int protocol::parse(QByteArray response)
{
    int rc;
    int addr,code,status,size,value_size;
    uint16_t crc_cal,crc_recv;

    size = response.size();

    for (int i = 0; i < size; i++) {
         qDebug("%x",(uint8_t)response[i]);
    }

    if (size < PROTOCOL_MIN_SIZE || size > PROTOCOL_MAX_SIZE) {
        qDebug() << QString("回应字节数量错误");
        return -10;
    }
    if (response.at(0) != 0x4d || response.at(1) != 0x4c) {
        qDebug() << QString("回应head错误");
        return -11;
    }

    value_size = response.at(2);

    if (value_size != size - 2 - 2 - 1) {
        qDebug() << QString("回应数据域长度值错误") << value_size;
        return -12;
    }

    crc_cal = m_crc.calculate((uint8_t *)response.data(),size - 2);
    crc_recv = (uint16_t)response.at(size - 1) << 8  | (uint8_t) response.at(size - 2);
    if (crc_cal != crc_recv) {
        qDebug() << QString("回应crc错误") << "cal:" << crc_cal <<"recv:" << crc_recv;
        return -13;
    }
    addr = (uint8_t)response.at(3);
    code = response.at(4);
    if (addr != m_addr ) {
        qDebug() << QString("回应地址错误");
        return -14;
    }
    if (code != m_code) {
        qDebug() << QString("回应code错误");
        return -15;
    }
    switch (code) {
    case PROTOCOL_CODE_NET_WEIGHT:
        if (value_size != 4) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        m_parse_value = (uint8_t)response[6] * 256 + (uint8_t)response[5];
        rc = 0;
        break;
    case PROTOCOL_CODE_REMOVE_TARE:
        if (value_size != 3) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        if (response.at(5) == 0) {
            rc = 0;
        } else {
            rc = -1;
        }
        break;
    case PROTOCOL_CODE_CALIBRATE_ZERO:
        if (value_size != 3) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        if (response.at(5) == 0) {
            rc = 0;
        } else {
            rc = -1;
        }
        break;
    case PROTOCOL_CODE_CALIBRATE_FULL:
        if (value_size != 3) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        if (response.at(5) == 0) {
            rc = 0;
        } else {
            rc = -1;
        }
        break;
    case PROTOCOL_CODE_SENSOR_ID:
        if (value_size != 3) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        m_parse_value = (uint8_t)response.at(5);
        rc = 0;
        break;
    case PROTOCOL_CODE_FW_VERSION:
        if (value_size != 4) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        m_parse_value =(int)response.at(5) + (int)response.at(6) * 256;
        rc = 0;
        break;
    case PROTOCOL_CODE_SET_ADDR:
        if (value_size != 3) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        if (response.at(5) == 0) {
            rc = 0;
        } else {
            rc = -1;
        }
        break;
    }
    qDebug()<<"rc = " << rc;
    return  rc;
}


int protocol::get_msg_id()
{
   return m_msg_id;
}

int protocol::get_parse_value()
{
   return m_parse_value;
}


int protocol::get_timeout()
{
    return m_timeout;
}
















