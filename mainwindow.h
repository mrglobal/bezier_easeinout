#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "frame.h"
#include "bezier_curve.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Bezier_Curve *bezier_curve;
    QTimer *timer;
    Frame *active_left_frame;
    Frame *active_right_frame;
    QList<Frame *>frame_list;
    QList<Frame *>frame_new_list;

    void setup_bezier_curve();
    void read_in_frames();

public slots:
    void timer_fired();

private slots:
    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_2_pressed();

    void on_pushButton_3_pressed();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
