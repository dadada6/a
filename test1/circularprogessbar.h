#ifndef CIRCULARPROGESSBAR_H
#define CIRCULARPROGESSBAR_H

#include <QWidget>
#include <QPointF>
#include <QPainter>

class CircularProgessBar : public QWidget
{
    Q_OBJECT
public:
    explicit CircularProgessBar(QWidget *parent = 0);
    //设置圆形(弧形/外圆/内圆/)占比
    void setCircleRatio(const qreal &arcRatio=1.0,
                        const qreal &outerCircleRatio=0.8,
                        const qreal &innerCircleRatio = 0.6);
    //设置圆形(外圆/内圆)画笔
    void setCirclePen(const QPen &outerCirclePen=QPen(Qt::NoPen),
                      const QPen &innerCirclePen=QPen(Qt::NoPen));
    //设置圆形(外圆/内圆)画刷
    void setCircleBrush(const QBrush &outerCircleBrush=QBrush("#808080"),
                        const QBrush &innerCircleBrush=QBrush("#CBF6FE"));
    /*设置弧形画笔*/
    void setArcPenBrush(const QBrush &arcPenBrush=QBrush("#008CFE"));//画刷
    void setArcPenDashPattern(const QVector<qreal> &dashs,const bool &showDashBaseBrush=false,const QBrush &dashBaseBrush=QBrush("#DCDCDC"));//虚线模式
    void setArcPenGradient(const QConicalGradient &conicalGrad,
                           const bool &isShowGradient=true);//渐变色
    //设置文本属性
    void setTextProperty(const QColor &color=QColor("#000000"),
                         const QFont &font=QFont());
    void setText(const QString &text);//设置显示的文本

    void setProgressValue(const qreal &progressValue);//设置进度值

protected:
    virtual void paintEvent(QPaintEvent  *event);

private:
    qreal currentProgressValue;//当前进度值(0-100) 由setProgressValue()设置
    QPointF circleCenter;//部件的中心点(做圆心)
    qreal maxRadius;//最大圆半径

    /*弧形/外圆/内圆占比  由setCircleRatio()设置*/
    qreal arcRatio;
    qreal outerCircleRatio;
    qreal innerCircleRatio;
    /*外圆/内圆画笔   由setCirclePen()设置*/
    QPen outerCirclePen;
    QPen innerCirclePen;
    /*外圆/内圆画刷 由setCircleBrush()设置*/
    QBrush outerCircleBrush;
    QBrush innerCircleBrush;
    /*文本  由setText*()设置*/
    QString text;
    QColor textColor;
    QFont textFont;
    /*弧形画笔的相关参数 由setArcPen*()设置*/
    QPen arcPen;
    bool isShowDashBaseBrush;//是否展示弧形虚线底色
    QBrush dashBaseBrush;//弧形虚线底色(由画刷保存)
    bool isShowGradient;//是否显示弧形渐变色
    QConicalGradient conicalGrad;//锥形渐变

    void drawCircleProgressBar(QPainter &painter);//绘制圆形进度条

signals:

public slots:
};

#endif // CIRCULARPROGESSBAR_H
