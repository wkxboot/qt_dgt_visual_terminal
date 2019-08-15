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

public slots:

    void handle_ui_request_result(int,int,int);

    void on_remove_tare_button_clicked();

    void on_calibration_zero_button_clicked();

    void on_calibration_1000_button_clicked();

    void on_calibration_2000_button_clicked();

    void on_calibration_5000_button_clicked();

    void on_open_clicked();

signals:
    void ui_request(int,int);

private slots:
    void on_set_addr_button_clicked();

private:
    int get_addr(void);
    int get_addr_setting(void);

    Ui::MainWindow *ui;
    communication   *comm;
};

#endif // MAINWINDOW_H
