#ifndef STARTITEM_H
#define STARTITEM_H

#include <QGraphicsObject>
#include <QPainter>
#include <QWidget>
#include <QMovie>
#include <QLabel>
#include <QVBoxLayout>
#include <QImage>
//#include <rga/rga.h>
//#include <rga/RgaApi.h>
//#include <rkfacial/rkfacial.h>
#include "snapshotthread.h"
#include <QMutex>

typedef enum
{
  QT_SUB_TEMP_NORMAL,            //温度正常
  QT_SUB_TEMP_HIGH,            //温度高
  QT_SUB_TEMP_LOW,            //温度低
  QT_SUB_TEMP_SUPER_HIGH,          //温度超高
}TEMP_STATUS;

typedef enum
{
  QT_SUB_IDENTITYCARD_MATCH,        //身份证匹配
  QT_SUB_IDENTITYCARD_UNMATCH,      //身份证不匹配
}IDENTITYCARD_RESULT;

typedef enum
{
  QT_SUB_IDENTITYCARD_USE,        //身份证模式使用
  QT_SUB_OTHER_MODE_USE,          //其它  模式使用
}PIC_TEMP_DIS_POSITION;

#if 1
class StartItem : public QGraphicsObject
{
public:
    StartItem(QGraphicsItem *parent = 0);
    virtual ~StartItem();
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
//    void temp_to_color(float temp, uint8_t rgb[3]);
//    void init_thermal_imaging(int start_x, int start_y);
//    void paint_thermal_image(const float* ctemp_array,int bthermal);
//    void clear_thermal_image(QPainter *painter);
//private:
//    QPixmap pix;
//    QMovie* move;
//    float *temp_array;
//public:
//    int bDrawThermal;
//    int flags;
//    QFont font;
//    bool bShowInfo;
//    QImage picture;
//    QImage facePicture;

//    QImage redQImage;
//    QImage greenQImage;
//    QImage yellowQImage;

//    QImage redTemperatureQImage;
//    QImage greenTemperatureQImage;
//    QImage yellowTemperatureQImage;

//    QImage pictureEmptyQImage;
//    QImage pictureWaitingQImage;
//    QImage pictureYellowFlagsQImage;
//    QImage pictureGreenFlagsQImage;
//    QImage pictureRedFlagsQImage;

//    QString tempuratureText;
//    QString nameText;
//    QString tipText;
//    QString cardText;
//    int iMeasuringResult;
//    bool showTemperature;
//    char picturePath[256];
//    int idCardFlags;
//    int pictureFlags;
//    int pictureidCard;
//    int compareResult;
//    int showIdCardResult;
//    QString lowText;
//    QString highText;
//    SnapshotThread *snapshotThread;
//private slots:
//    void slot_movieFinish();
};

#endif
#endif // STARTITEM_H
