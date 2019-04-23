#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "communication.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int get_addr(void);
    enum {
        OPT_QUERY_NET_WEIGHT,
        OPT_REMOVE_TARE,
        OPT_CALIBRATION_ZERO,
        OPT_CALIBRATION_FULL
    };
public slots:
    void loop_weight_timeout(void);
    void rsp_scale_result(uint8_t opt,int16_t result,QString err);
    void rsp_serial_open_result(int rc);
    void rsp_serial_close_result(int rc);

    void on_weight_loop_button_clicked();

    void on_remove_tare_button_clicked();

    void on_calibration_zero_button_clicked();

    void on_calibration_1000_button_clicked();

    void on_calibration_2000_button_clicked();

    void on_calibration_5000_button_clicked();

    void on_open_clicked();

signals:
    void req_scale(uint8_t addr,uint8_t code,int param);
    void req_serial(QString port,int baudrate);
private:
    Ui::MainWindow *ui;
    communication *comm;
    QTimer *loop_weight_timer;
    bool loop_weight;
};

#endif // MAINWINDOW_H
