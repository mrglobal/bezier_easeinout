#include <QImage>
#include <QPainter>
#include "frame.h"

Frame::Frame(QWidget *parent)
    : QWidget{parent}
{
    hide();
    index = 0;
    src_index = 0;
    delta = 0;
    overwritten = false;
}

//Display the Frame image
void Frame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.drawRect(this->rect());
    painter.drawImage(0, 0, *this->image);
}
