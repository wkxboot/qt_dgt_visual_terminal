#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "qserialport.h"
#include "qobject.h"
#include "crc16.h"
#include "qserialportinfo.h"
#include "qthread.h"
#include "qdialog.h"
#include "qmutex.h"
#include "crc16.h"


class communication : public QObject
{
    Q_OBJECT

public:

    communication(QObject *parent = nullptr);
    ~communication();
    QSerialPort *serial;
public slots:
    void on_data_ready();
    void timer_timeout(void);
    void on_serial_req(QString port,int bandrates);
    void on_scale_req(uint8_t addr,uint8_t opt,int param);
signals:
    void rsp_scale_req_result(uint8_t code,int16_t rc,QString err);
    void rsp_serial_open_req_result(int rc);
    void rsp_serial_close_req_result(int rc);


private:

    QString port_name;
    bool opened;
    uint8_t scale_addr;
    uint8_t opt_code;
    QDialog *set_addr_dialog;
    crc16 *crc;
    QTimer *rsp_timer;

};
#endif // COMMUNICATION_H
