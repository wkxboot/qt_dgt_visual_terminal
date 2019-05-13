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
    void handle_open_serial(int,int);
    void handle_scale_result(int,int,int);

    void on_weight_loop_button_clicked();

    void on_remove_tare_button_clicked();

    void on_calibration_zero_button_clicked();

    void on_calibration_1000_button_clicked();

    void on_calibration_2000_button_clicked();

    void on_calibration_5000_button_clicked();

    void on_open_clicked();

signals:
    void req_scale(uint8_t addr,uint8_t type,int value);
    void req_serial(QString port,int baudrate,int baud_rates,int data_bits,int stop_bits);
private:
    Ui::MainWindow *ui;
    communication   *comm;
    QTimer *m_query_weight_timer;
};

#endif // MAINWINDOW_H
