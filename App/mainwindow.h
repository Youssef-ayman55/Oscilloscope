#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include <QSerialPort>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QCustomPlot * display;
    QVector<double> x, y;
    QVector<QString> data;
    QSerialPort * port;
    int SPS; // samples per scan
    int samples;
    int delay;
    int TPS; // time per scan in milliseconds
    double V_ref; // Max Voltage supported by the board
    int resolution; // Analog pin resolution of the board
    bool AC_coupling; // True if AC Coupling is on
    void request_data();
    QVector<QString> port_list;
    QString serial_buffer;
    bool request_complete;
    QTextStream* stream;
    double V_max, V_pp, freq; // Output parameters
    QButtonGroup coupling; // button group for coupling options
    bool paused;

private slots:
    void on_open_port_btn_clicked();
    void update_available_ports();
    void read_serial();
    void refresh();
    void on_AC_rbutton_toggled(bool checked);

    void on_DC_rbutton_toggled(bool checked);

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_lineEdit_2_textEdited(const QString &arg1);

    void on_horizontalSlider_valueChanged(int value);

    void on_lineEdit_textEdited(const QString &arg1);

    void on_pushButton_clicked();

    void on_verticalSlider_valueChanged(int value);

    void on_resolution_slot_textEdited(const QString &arg1);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
