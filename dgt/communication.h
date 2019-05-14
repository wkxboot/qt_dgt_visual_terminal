#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "qserialport.h"
#include "qobject.h"
#include "crc16.h"
#include "qserialportinfo.h"
#include "qthread.h"
#include "qdialog.h"
#include "qmutex.h"
#include "request.h"
#include "crc16.h"
#include "qqueue.h"


class communication : public QObject
{
    Q_OBJECT

public:

    QSerialPort *m_serial;
    enum {
        SERIAL_OPEN,
        SERIAL_CLOSE,
        SERIAL_SUCCESS,
        SERIAL_FAIL,

        QUERY_WEIGHT,
        REMOVE_TARE_WEIGHT,
        CALIBRATION_WEIGHT_ZERO,
        CALIBRATION_WEIGHT_FULL,

        QUEUE_TIMEOUT = 5,
        CONTINUE_TIMEOUT = 10,
        RSP_TIMEOUT = 50
    };

    communication(QObject *parent = nullptr);
    ~communication();


    QByteArray wait_rsp(int timeout);
    void handle_rsp(QByteArray rsp, int addr,int type);
    void handle_query_weight(int);
    void handle_remove_tare_weight(int);
    void handle_calibration_weight(int,int);

public slots:
    void handle_scale(int,int,int);
    void handle_open_serial(QString,int,int,int);

    void handle_request_queue();

signals:
    void rsp_open_serial(int,int);
    void rsp_result(int,int,int);


private:
    bool m_open;
    crc16 *m_crc;

    QTimer *m_queue_timer;
    QQueue<request> *m_queue;
    int m_addr;

};
#endif // COMMUNICATION_H
