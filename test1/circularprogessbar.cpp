#include "circularprogessbar.h"
#include <QDebug>

CircularProgessBar::CircularProgessBar(QWidget *parent) : QWidget(parent)
{
    this->currentProgressValue = 0;
    this->isShowDashBaseBrush = false;
    this->isShowGradient = false;
    //初始化使用默认的属性参数
    setCircleRatio();
    setCircleBrush();
    setCirclePen();
    setArcPenBrush();
    setTextProperty();
    this->resize(300,300);
}

void CircularProgessBar::setCircleRatio(const qreal &arcRatio,
                                        const qreal &outerCircleRatio, const qreal &innerCircleRatio)
{
    this->arcRatio = arcRatio;
    this->outerCircleRatio = outerCircleRatio;
    this->innerCircleRatio = innerCircleRatio;
}

void CircularProgessBar::setCirclePen(const QPen &outerCirclePen,
                                      const QPen &innerCirclePen)
{
    this->outerCirclePen = outerCirclePen;
    this->innerCirclePen = innerCirclePen;
}

void CircularProgessBar::setCircleBrush(const QBrush &outerCircleBrush,
                                        const QBrush &innerCircleBrush)
{
    this->outerCircleBrush = outerCircleBrush;
    this->innerCircleBrush = innerCircleBrush;
}

void CircularProgessBar::setArcPenBrush(const QBrush &arcPenBrush)
{

    this->arcPen.setCapStyle(Qt::FlatCap);
    this->arcPen.setBrush(arcPenBrush);
}

void CircularProgessBar::setArcPenDashPattern(const QVector<qreal> &dashs,const bool &isShowDashBaseBrush, const QBrush &dashBaseBrush)
{
    arcPen.setDashPattern(dashs);
    this->isShowDashBaseBrush = isShowDashBaseBrush;
    this->dashBaseBrush = dashBaseBrush;
}


void CircularProgessBar::setArcPenGradient(const QConicalGradient &conicalGrad, const bool &isShowGradient)
{
    this->conicalGrad = conicalGrad;
    this->isShowGradient = isShowGradient;
}

void CircularProgessBar::setTextProperty(const QColor &color, const QFont &font)
{
    this->textColor = color;
    this->textFont = font;
}

void CircularProgessBar::setText(const QString &text)
{
    this->text = text;
    update();
}

void CircularProgessBar::setProgressValue(const qreal &progressValue)
{
    this->currentProgressValue = progressValue;
    update();
}

void CircularProgessBar::paintEvent(QPaintEvent *event)
{
    //qDebug()<<"CircularProgessBar:paintEvent()............................";
    QWidget::paintEvent(event);
    int width = this->width();
    int height = this->height();
    //确定部件的中心(圆心)
    circleCenter.setX(width/2.0);
    circleCenter.setY(height/2.0);
    //确定可正常显示圆的最大半径
    int maxDiameter = width > height ? height : width;
    maxRadius = maxDiameter*0.5;
    //定义painter,设置属性
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);//反锯齿

    //绘制圆形进度条
    drawCircleProgressBar(painter);

}

void CircularProgessBar::drawCircleProgressBar(QPainter &painter)
{

    qreal outerCircleRadius = maxRadius*outerCircleRatio;
    qreal innerCircleRadius = maxRadius*innerCircleRatio;
    qreal annulusWidth = outerCircleRadius - innerCircleRadius;//圆环宽度
    qreal arcPenWidth = annulusWidth*arcRatio;//圆弧(画笔)宽度
    qreal arcRadius = annulusWidth/2.0+innerCircleRadius;//圆弧所在圆半径

    QRectF arcRectF(circleCenter.x()-arcRadius,circleCenter.y()-arcRadius,arcRadius*2,arcRadius*2);

    painter.setPen(outerCirclePen);
    painter.setBrush(outerCircleBrush);
    painter.drawEllipse(circleCenter,outerCircleRadius,outerCircleRadius);
    painter.setPen(innerCirclePen);
    painter.setBrush(innerCircleBrush);
    painter.drawEllipse(circleCenter,innerCircleRadius,innerCircleRadius);
    arcPen.setWidthF(arcPenWidth);//自适应画笔宽度
    if(isShowDashBaseBrush)//使用虚线底色绘制一遍进度条背景
    {
        QBrush penBrush = arcPen.brush();//先记录一下当前弧形画笔的画刷
        arcPen.setBrush(dashBaseBrush);
        painter.setPen(arcPen);
        painter.drawArc(arcRectF,90*16,-360*16);
        arcPen.setBrush(penBrush);//还原画刷
    }
    if(isShowGradient)
    {
        conicalGrad.setCenter(circleCenter);//设置锥形渐变的圆心
        arcPen.setBrush(QBrush(conicalGrad));
    }
    painter.setPen(arcPen);
    painter.drawArc(arcRectF,90*16,-(int)(currentProgressValue*57.6));
    painter.setPen(QPen(textColor));
    painter.setFont(textFont);
    painter.drawText(arcRectF,Qt::AlignCenter,text);
}
