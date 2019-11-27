#ifndef REQUEST_H
#define REQUEST_H

#include "qbytearray.h"
#include "qdebug.h"
#include "crc16.h"

class protocol
{
public:
    enum {
        PROTOCOL_MIN_SIZE = 7,
        PROTOCOL_MAX_SIZE = 9
    };

    enum {
        PROTOCOL_CODE_NET_WEIGHT = 0,
        PROTOCOL_CODE_REMOVE_TARE = 1,
        PROTOCOL_CODE_CALIBRATE_ZERO = 2,
        PROTOCOL_CODE_CALIBRATE_FULL = 3,
        PROTOCOL_CODE_SENSOR_ID = 4,
        PROTOCOL_CODE_FW_VERSION = 5,
        PROTOCOL_CODE_SET_ADDR = 6
    };


    QByteArray m_request_buffer;
    protocol();
    ~protocol();
    int construct(int msg_id,int addr,int code,int value_cnt,int *value,int timeout);
    int parse (QByteArray response);
    int get_parse_value();
    int get_code();
    int get_msg_id();
    int get_timeout();

private:
    /*请求部分*/

    int m_msg_id;/**/
    int m_code;
    int m_addr;
    int m_timeout;
    crc16 m_crc;
    int16_t m_parse_value;

};

#endif // REQUEST_H
