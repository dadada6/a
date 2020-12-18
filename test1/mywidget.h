#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QLabel>
#include <QStyleOption>
#include <QPainter>
#include <QStyle>
#include <QMouseEvent>
#include <QPainter>
#include <QPointF>
#include <QPen>
#include <QList>
#include <QTimer>
//暂时只加双击，后续有需求就再拓展
class MyWidget : public QLabel {
    Q_OBJECT
   public:
    MyWidget(QWidget *parent = 0);
    ~MyWidget();
   QMovie *movie;
   protected:
    virtual void paintEvent(QPaintEvent *e);  //需重载paintEvent 否则样式无效

};

#endif
