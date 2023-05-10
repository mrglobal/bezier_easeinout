#ifndef FRAME_H
#define FRAME_H

#include <QObject>
#include <QWidget>
#include <QImage>

#define NUMBER_FRAMES 142
#define INTER_FRAME_INTERVAL_MSECS 35

class Frame : public QWidget
{
    Q_OBJECT

public:
    explicit Frame(QWidget *parent = nullptr);
    int index;
    int src_index;
    int delta;
    bool overwritten;
    QString filename;
    QImage *image;

signals:

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // FRAME_H
