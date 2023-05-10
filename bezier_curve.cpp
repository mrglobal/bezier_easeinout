#include <QPainter>
#include <QPolygonF>
#include <QPointF>
#include <QDebug>
#include <QtMath>
#include "bezier_curve.h"
#include "mainwindow.h"

/*
 * Bezier_Curve is a window which the Bezier Curve will be drawn
 */
Bezier_Curve::Bezier_Curve(QWidget *parent)
    : QWidget{parent}
{
    /*
     * Setup Bezier points as a straight line at some angle. The Begin Angle (this->begin_angle) is formed by a linear line which
     * ascending from left to right. Subsequently, when the line is curved by Bezier function (eg Ease-In), each point along the frame list
     * on that curve is calculated relative to to tghe begin angle
     */
    QPoint p0= QPoint(37,110);
    QPoint c1= QPoint(37,110);
    QPoint c2= QPoint(1884,37);
    QPoint p1= QPoint(1884,37);

    //Setup the QPainterPath as a linear line
    this->bezier_path.clear();
    this->bezier_path.moveTo(p0);
    this->bezier_path.cubicTo(c1,c2, p1);

    //Setup the BezierCurve Window size with some padding
    int width = p1.x() - p0.x() + 100;
    int height = p0.y() - p1.y() + 100;
    this->setFixedSize(width, height);

    /*
     * Calculate the Begin Angle (this->begin_angle) of the linear line
     */
    QList<Frame *>empty_list;
    this->calculate_bezier_degrees(true, empty_list);
}

//Draw the Bezier Curve
void Bezier_Curve::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.drawPath(bezier_path);
}

void Bezier_Curve::deploy_bezier_curve(QList<Frame *>frame_new_list)
{
    //Ease-In Bezier Curve
    QPoint p0= QPoint(37,110);
    QPoint c1= QPoint(127,20);
    QPoint c2= QPoint(1884,37);
    QPoint p1= QPoint(1884,37);

    /*
     * S-shaped Ease-In Ease-Out Bezier Curve
     */
    /*
    p0= QPoint(37,110);
    c1= QPoint(117,4);
    c2= QPoint(1775,162);
    p1= QPoint(1884,37);
    */

    //Setup the Bezier Curve
    this->bezier_path.clear();
    this->bezier_path.moveTo(p0);
    this->bezier_path.cubicTo(c1,c2, p1);

    //Ensure Bezier Curve Window draws the latest Bezier Curve
    this->repaint();

    /*
     * Calculate the dy/dx in degrees of each Frame instance's tangent vs the next Frame's tangent
     * The output is degrees_skip_index_list.
     *   contents of each item in the list:
     *      "accelerate to frame N" (+N)
     *      "slow down N frames" (-N)
     *      "maintain sequence" (0)
     */
    this->calculate_bezier_degrees(false, frame_new_list);

    //Shape Animation according to this->degrees_skip_index_list
    reinterpolate_frames(frame_new_list);

}

void debug_frames(QList<Frame *>frame_list, int dst_index)
{
    Frame *frame;
    QString temp_str;

    qDebug() << " ";
    qDebug() << "***************Debug Frame dst_index=" << dst_index;

    for (int i=0; i< dst_index/2; i++) {
        frame = frame_list.at(i);
        temp_str.append(QString::number(frame->src_index));
        temp_str.append( "/");
        temp_str.append(QString::number(frame->index));
        temp_str.append( "/");
        temp_str.append(QString::number(frame->delta));
        if (frame->overwritten)
           temp_str.append("* #");
        else
           temp_str.append(" #");
    }
    qDebug() << temp_str;

    temp_str.clear();
    for (int i=dst_index/2; i<= dst_index; i++) {
        frame = frame_list.at(i);
        temp_str.append(QString::number(frame->src_index));
        temp_str.append( "/");
        temp_str.append(QString::number(frame->index));
        temp_str.append( "/");
        temp_str.append(QString::number(frame->delta));
        if (frame->overwritten)
           temp_str.append("* #");
        else
           temp_str.append(" #");
    }
    qDebug() << temp_str;

}

/*
 * The gist is to capture the delta between tangent at each point on the original linear line vs the tangent at the next point
 * Essentially the dy/dx of each frame instance's tangent to next higher frame.
 * The dy/dx is then converted to number of frames to jump forward (if value is +), extend current frame's content to next (-), or continue
 * normal frame increment (if value is 0). The conversion is (dy/dx)/begin_angle
 * begin_angle is the tangent's angle when the bezier curve is a linear straight line.
 *
 * The beginning instance's (index=0) dy/dx is obtained by (tangent angle - begin_angle). The number of frames is obtained by (tangnet angle - begin_angle)/begin_angle.
 * The other subsequent number of frames is computed by (next frame tangent angle - previous frame tangent angle)/begin_angle
 */
void Bezier_Curve::calculate_bezier_degrees(bool initialize, QList<Frame *>frame_list)
{
    this->degrees_list.clear();
    this->skip_extend_index_list.clear();

    /*
     * Setup degree_list which is tangent at each point from 1% to 100% along QPainterPath
     */
    QPainterPath *painter_path;
    painter_path = &this->bezier_path;

    for (qreal percent=0.0; percent <= 1.0; percent=percent+0.01){
        //Get the slope (tangent) at specific percentage
        qreal slope_temp = painter_path->slopeAtPercent(percent);

        int res = std::fpclassify(slope_temp);
        switch (res){
                case FP_INFINITE:
                case FP_NAN:
                    slope_temp = 0.0;
                break;
            default:
                   break;
        }

        //From slope, get the angle (in radians) and convert to degrees
        qreal angle = qAtan(slope_temp);
        qreal degree = angle * 180/3.142;
        qDebug() << "degree=" << degree << " radian=" << angle << " Slope=" << slope_temp << " percent=" << percent;

        //Store in degrees list for further processing
        this->degrees_list.append(degree);

        /*
         * If Initalize, store the degree to begin_angle. Note all points along the path is the same degree
         * since the line between the 2 end points is straight, linear line  when setup initially.
         */
        if (initialize)
            this->begin_angle = abs(degree);

    }

    /*
     * Make all degrees positive.
     * The Degrees are mainly negative because the coordinates system is based on (0,0) on the top Left.
     * We will simplify it by making all degrees positive based on the reference axis for the Animation Graph based on
     * (0,0) on bottom left. This makes the slope positive by such reference. Note that we reserve negative value to a different meaning
     */
    QList<qreal>temp_list;
    for (int i=0; i < this->degrees_list.length(); i++){
        if (this->degrees_list.at(i) > 0){
            qDebug() << "Unexpected positive";
            temp_list.append(this->begin_angle);
        } else
            temp_list.append(abs(this->degrees_list.at(i)));
    }
    this->degrees_list = temp_list;

    //Dont need to go further if initialize
    if (initialize)
        return;

    /*
     * Go through each instance of Frame and calculate the number of Frame instances it should
     * "accelerate" (+) or "slow down" (-) or maintain sequence (0)
     */
    int prev_adjusted_index_topath = 0;
    for (int i=0; i < frame_list.length(); i++){
        /*
         * All lists so far are based on 0% - 100% along the QPainterPath between the two end points.
         * This is not the same scale as the number of Frame instances.
         * We will thus build the skip_delta_list based on the latter scale.
         */
        float scale_ratio = (float) 100/this->degrees_list.length();
        float index_temp = i * scale_ratio;
        int adjusted_index_topath = round(index_temp);

        //Make sure adjusted_index_topath does not exceed this->degrees_list
        if (adjusted_index_topath > (this->degrees_list.length()-1))
            adjusted_index_topath = this->degrees_list.length()-1;

        float delta;
        if (i== 0)
            delta = (this->degrees_list.at(adjusted_index_topath) - this->begin_angle)/this->begin_angle;
        else
            delta = (this->degrees_list.at(adjusted_index_topath) - this->degrees_list.at(prev_adjusted_index_topath))/this->begin_angle;

        int skip_index = round(delta);
        this->skip_extend_index_list.append(skip_index);
        prev_adjusted_index_topath = adjusted_index_topath;

    }
    qDebug() << "Degrees List=" << this->degrees_list;
    qDebug() << "Degrees Skip Index=" << this->skip_extend_index_list;

}

void Bezier_Curve::extend_src_delta_times(int dst_index, int delta, QList<Frame *>frame_list)
{
    Frame *prv_frame, *dst_frame, *src_frame;
    if (dst_index > 0){
        prv_frame = frame_list.at(dst_index-1);
        if (prv_frame && prv_frame->overwritten){
           if ((prv_frame->src_index+1) < frame_list.length())
               copy_src_to_dst_frame(prv_frame->src_index+1, dst_index, delta, frame_list);
           else
               copy_src_to_dst_frame(prv_frame->src_index, dst_index, delta, frame_list);

           //debug_frames(frame_list, dst_index);
        }
        dst_index++;

        if ((abs(delta)-1) >=1) {
           prv_frame = frame_list.at(dst_index-1);
           for (int i=0; i < (abs(delta)-1); i++){
               copy_src_to_dst_frame(prv_frame->src_index+1, dst_index, delta, frame_list);
               //debug_frames(frame_list, dst_index);
               dst_index++;
           }
        }
    }

}

void Bezier_Curve::copy_src_to_dst_frame(int src_index, int dst_index, int delta, QList<Frame *>frame_list)
{
    Frame *src_frame, *dst_frame;
    src_frame = frame_list.at(src_index);
    dst_frame = frame_list.at(dst_index);

    *dst_frame->image = src_frame->image->copy();
    dst_frame->src_index = src_index;
    dst_frame->overwritten = true;
    dst_frame->delta = delta;
}

/*
 * Reinterpolate the Frames in frame_list to follow the bezier curve shape
 */
void Bezier_Curve::reinterpolate_frames(QList<Frame *>frame_list)
{
    int delta;
    int dst_index = 0;
    int src_index = 0;
    Frame *prv_frame;

    /*
     * skip_extend_index_list contains the delta by which the frames
     * are to be jumped forward to, extended or just to maintain sequence.
     * E.g.
     * Degrees Skip Index= (19, -4, -3, -2, -2, -1, -1, 0....
     *
     * +N  : Jump to frame N
     *      e.g. +19 : Jump to Frame 19
     * -N  : Delay forward sequence by extending same frame contents N times
     *      e.g. -4 extend same frame 4 times
     *  0  : Maintain sequence
     *
     */
    qDebug() << this->skip_extend_index_list;

    for (int i=0; i < this->skip_extend_index_list.length(); i++){

        if (dst_index >= frame_list.length())
            return;

        delta = this->skip_extend_index_list.at(i);

        if (delta > 0){

            if (dst_index >0){
                prv_frame = frame_list.at(dst_index-1);
                if (prv_frame->overwritten){
                    extend_src_delta_times(dst_index, 1, frame_list);
                    debug_frames(frame_list, dst_index);
                    prv_frame = frame_list.at(dst_index);
                    src_index = prv_frame->src_index + delta;
                    dst_index++;
                }
            } else
                src_index = delta + dst_index;

            copy_src_to_dst_frame(src_index, dst_index, delta, frame_list);
            debug_frames(frame_list, dst_index);
            dst_index++;
        } else if (delta < 0){
            extend_src_delta_times(dst_index, delta, frame_list);
            dst_index = dst_index + abs(delta);
            debug_frames(frame_list, dst_index-1);
        } else {
            extend_src_delta_times(dst_index, 1, frame_list);
            debug_frames(frame_list, dst_index);
            dst_index++;
        }
    }
}
