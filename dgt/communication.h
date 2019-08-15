#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "qobject.h"
#include "qthread.h"
#include "qdialog.h"
#include "qmutex.h"
#include "protocol.h"
#include "qqueue.h"
#include "qserialport.h"
#include "qserialportinfo.h"

class communication : public QObject
{
    Q_OBJECT

public:

    enum {
        MSG_ID_NOTIFY_ADDR = 0x11,
        MSG_ID_NET_WEIGHT = 0x12,
        MSG_ID_REMOVE_TARE = 0x13,
        MSG_ID_CALIBRATE_ZERO = 0x14,
        MSG_ID_CALIBRATE_FULL = 0x15,
        MSG_ID_SENSOR_ID = 0x16,
        MSG_ID_FW_VERSION = 0x17,
        MSG_ID_SET_ADDR = 0x18,
    };

    enum {
        LOOP_QUEUE_TIMEOUT = 5,
        CONTINUE_TIMEOUT = 5
    };

    enum {
        QUERY_WEIGHT_TIMEOUT = 50,
        REMOVE_TARE_TIMEOUT = 500,
        CALIBRATION_TIMEOUT = 500,
        QUERY_SENSOR_ID_TIMEOUT = 50,
        QUERY_FW_VERSION_TIMEOUT = 50,
        SET_ADDR_TIMEOUT = 500
       };

    communication(QObject *parent = nullptr);
    ~communication();

    QStringList get_serial_port_name_list();
    int serial_send_request(QByteArray request);
    QByteArray serial_wait_rsp(int timeout);

    int open_serial(QString,int,int,int);
    int close_serial(QString);
    int is_serial_open();
    int set_comm_addr(int addr);

public slots:
    void handle_ui_request(int,int);
    void loop_request_queue();

signals:
    void rsp_ui_request_result(int,int,int);


private:
    void query_weight();
    void remove_tare_weight();
    void calibration_zero(int);
    void calibration_full(int);
    void query_sensor_id();
    void query_fw_version();
    void set_addr(int);
    void insert_period_query_req();

    QTimer *m_queue_timer;
    QQueue<protocol> *m_queue;

    QSerialPort *m_serial;
    int m_addr;
};
#endif // COMMUNICATION_H
