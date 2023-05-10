#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QSlider>
#include <QRect>
#include "frame.h"

/*
 * The Test Program is a framework to apply a Ease-In and/or Ease Out Bezier Curve to a series of animated sequence of
 * .png files such that the new animated sequence is viewable, This gives the user experience that the animation speeds up or slows down
 * as per the Bezier Curve shape dictates.
 *
 * There are the major User Interface components.
 *   Bezier Curve Window
 *      A separate window from MainWindow which displays the Bezier Curve. Initially the curve is just a straight
 *      linear line. When "Deploy Bezier Curve" button (see below) is clicked, the selected bezier curve is applied and
 *      the modifications are to be made to frame_new_list per the bezier curve shape.
 *
 *  MainWindow View
 *    This MainWindow window serves to display the original animation (Left side of MainWindow) and the modified, new animation
 *    (right side of MainWindow)
 *
 *  MainWindow Slider
 *    Allow user to move from frame to frame along a fixed timeline (0 to NUMBER_FRAMES-1)
 *
 *  Play Button
 *    Click to play animation (both left and right animation windows)
 *
 *  Pause Button
 *    Click to Pause animation (both left and right animation windows)
 *
 *  Deploy Bezier Curve
 *    When clicked, the selected bezier curve is applied and the modifications are
 *    to be made to frame_new_list per the bezier curve shape.
 *
 * Major internal code organization:
 *
 *    Bezier_Curve Class
 *      Corresponds to a separate window from MainWindow which displays the selected bezier curve. Initially the curve is just a
 *      straight linear line. When "Deploy Bezier Curve" button is clicked, the selected bezier curve is applied and the modifications are to
 *      be made to frame_new_list per the bezier curve shape.
 *      Note the Ease-In bezier curve or Ease-In/Ease-out bezier curve are "selectable" by compiling
 *      in the selection. See Bezier_Curve's deploy_bezier_curve
 *
 *    NUMBER_FRAMES - no of frames to be read in from known directory.
 *                  know directory contains the .png files arrange in sequence
 *                  from 0.png, 1.png ..... <NUMBER_FRAMES-1>.png
 *                  The series of .png files is an Animated sequence of a clock with minute and seconds hand. This animated sequence is
 *                  specifically selected to better visualize how realistic the Ease-in/Ease-Out function is to the human eye
 *
 *    Frames Class
 *      Frame Object where each instance represents a Frame at the index along a fixed timeline
 *      (0 to NUMBER_FRAMES-1)
 *
 *    frames_list
 *      List of Frames instances for original animation
 *
 *    frames_new_list
 *      List of Frames instances for a modified animation per bezier curve shape
 *      This list starts off being identical to frames_list at the outset
 *
 *    timer
 *      - timer which fires to advance the frames in the MainWIndow - it drives the animation
 *      INTER_FRAME_INTERVAL_MSECS specifies the time interval in msecs between each Frame animation
 *
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Setup Slider
    ui->horizontalSlider->setRange(0, NUMBER_FRAMES-1);
    ui->horizontalSlider->setTickPosition(QSlider::TicksAbove);

    active_left_frame = nullptr;
    active_right_frame = nullptr;

    //Setup Timer to play Frames
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_fired()));

    //Read in Frames
    read_in_frames();

    /*
     * Create a Bezier Curve Window and setup the Bezier Curve (eg Ease-In) and draw
     * the bezier Curve in Bezier Curve Window
     */
    setup_bezier_curve();

    //Jigger the slider so that the first frame is displayed in Bezier Curve Window
    ui->horizontalSlider->setValue(NUMBER_FRAMES-1);
    ui->horizontalSlider->setValue(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//Setup the Bezier Curve as a separate window to display the bezier curve in
void MainWindow::setup_bezier_curve()
{
    bezier_curve = new Bezier_Curve();
    bezier_curve->show();
    bezier_curve->repaint();
}

/*
 * Read into 2 frame lists - frame_list and frame_new_list from known directory.
 * Arrange the Frames positions from these 2 lists on left and right side of MainWindow.
 * frame_list - contains the original frames read from the known directory. Don't modify this
 * frame_new_list - starts off with identical as frame_lsit (sans filename) and is to be modified
 * per the Bezier Curve shape
 */
void MainWindow::read_in_frames()
{
    QString directory = "C:/Users/Sean/VideoAd/interpolate_data/src/";
    QPoint left_pos, right_pos;
    QPoint center = this->rect().center();

    /*
     * Create Frames in frame_list from known directory containing filenames in format "1.png", "2.png", ..."<NUMBER_FRAMEs-1>.png"
     * Note that the NUMBER_FRAMES is the presribed number of frames and last filename is <NUMBER_FRAMEs-1>.png
     */
    Frame *frame;
    for (int i=0; i< NUMBER_FRAMES; i++){
        frame = new Frame(this);
        frame->image = new QImage();
        frame->index = i;
        frame->src_index = i;
        frame_list.append(frame);
        QString file_str = directory + "3_" + QString::number(i) + "#.png";
        QFile filename(file_str);
        if (filename.exists()){
            frame->filename = file_str;
            frame->image->load(file_str);
            frame->setFixedSize(frame->image->width(), frame->image->height());
        }
   }

   //Create Frames in frame_new_list. The index, image and size are identical to frames_list.
   for (int i=0; i< NUMBER_FRAMES; i++){
        frame = new Frame(this);
        frame->image = new QImage();
        frame->index = i;
        frame->src_index = i;
        frame->image->load(frame_list.at(i)->filename);
        frame->setFixedSize(frame->image->width(), frame->image->height());
        frame_new_list.append(frame);
   }

   /*
    * Setup left Frame and right Frame position in MainWindow display
    */
   left_pos.setX(center.x() - frame->rect().width() - 10);
   left_pos.setY(center.y() - frame->rect().height()/2);
   right_pos.setX(center.x() + 10);
   right_pos.setY(left_pos.y());

   //Update frames positions in the 2 lists accordingly
   for (int i=0; i< NUMBER_FRAMES; i++){
        frame_list.at(i)->move(left_pos);
        frame_new_list.at(i)->move(right_pos);
   }
}

/*
 * Any user initiated change to slider position will call this.
 */
void MainWindow::on_horizontalSlider_valueChanged(int value)
{

    //Hide the current active window. Update the active frame of left window.
    if (active_left_frame)
        active_left_frame->hide();
    active_left_frame = this->frame_list.at(value);
    active_left_frame->show();
    active_left_frame->repaint();

    //Hide the current active window. Update the active frame of right window.
    if (active_right_frame)
        active_right_frame->hide();
    active_right_frame = this->frame_new_list.at(value);
    active_right_frame->show();
    active_right_frame->repaint();

    /*
     * If new index of slider is at end of Slider range (end of frames list and
     * frames_new_list, stop the timer
     */
    if (value == NUMBER_FRAMES-1)
        timer->stop();
}

//Play the Left and Right Frames Lists
void MainWindow::on_pushButton_2_pressed()
{
    timer->stop();
    timer->start(INTER_FRAME_INTERVAL_MSECS);
}

//Stop Play on Left and Right Frames Lists
void MainWindow::on_pushButton_3_pressed()
{
    timer->stop();
}

/*
 * When Timer fires, call here. Advance to next frame in frames_list
 */
void MainWindow::timer_fired()
{
    int current_index = ui->horizontalSlider->value();
    if (current_index < NUMBER_FRAMES)
        ui->horizontalSlider->setValue(current_index+1);
}

//Deploy the Bezier Curve and alter the new Frames List(frames_new_list) accordingly
void MainWindow::on_pushButton_clicked()
{
    this->bezier_curve->deploy_bezier_curve(this->frame_new_list);
}

