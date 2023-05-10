#ifndef BEZIER_CURVE_H
#define BEZIER_CURVE_H

#include <QObject>
#include <QWidget>
#include "frame.h"

class Bezier_Curve : public QWidget
{
    Q_OBJECT
public:
    explicit Bezier_Curve(QWidget *parent = nullptr);
    QPainterPath bezier_path;
    qreal begin_angle;
    QList<qreal>degrees_list;
    QList<float>skip_extend_index_list;

    void deploy_bezier_curve(QList<Frame *>frame_new_list);
    void calculate_bezier_degrees(bool initialize, QList<Frame *>frame_list);
    void reinterpolate_frames(QList<Frame *>frame_new_list);
    void copy_src_to_dst_frame(int src_index, int dst_index, int delta, QList<Frame *>frame_list);
    void extend_src_delta_times(int src_index, int delta, QList<Frame *>frame_list);

signals:

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // BEZIER_CURVE_H
