#include "startitem.h"
#include <QPalette>
#include <QDateTime>
#include <QPainter>
#include <QPaintEngine>
#include <QDebug>
// 开启此宏统计绘制热成像图耗时
//#define PERFORMANCE

static float min_temp = 20.0;
static float max_temp = 40.0;
static float temp_range = 0;

#define CELL_WIDTH     5
#define CELL_HEIGHT    5

static int image_start_x = 600;
static int image_start_y = 1070;

static int temp_array_width = 0; // 温度阵列宽度，即每行多少个点，例如32
static int temp_array_height = 0; // 温度阵列高度，即每列多少个点，例如24

#if 1

// 从低到高
const static uint8_t palette[][3] = {
    {0, 0, 0},
    {0, 0, 10},
    {0, 0, 20},
    {0, 0, 30},
    {0, 0, 37},
    {0, 0, 44},
    {0, 0, 44},
    {0, 0, 52},
    {0, 0, 52},
    {0, 0, 58},
    {0, 0, 62},
    {0, 0, 66},
    {0, 0, 70},
    {0, 0, 74},
    {0, 0, 79},
    {0, 0, 82},
    {1, 0, 86},
    {1, 0, 86},
    {2, 0, 89},
    {2, 0, 92},
    {3, 0, 94},
    {4, 0, 98},
    {4, 0, 98},
    {5, 0, 101},
    {6, 0, 104},
    {6, 0, 104},
    {8, 0, 107},
    {9, 0, 111},
    {9, 0, 111},
    {11, 0, 115},
    {12, 0, 117},
    {12, 0, 117},
    {12, 0, 117},
    {15, 0, 119},
    {15, 0, 119},
    {18, 0, 122},
    {18, 0, 122},
    {22, 0, 124},
    {22, 0, 124},
    {25, 0, 126},
    {27, 0, 129},
    {27, 0, 129},
    {31, 0, 132},
    {31, 0, 132},
    {35, 0, 134},
    {35, 0, 134},
    {38, 0, 135},
    {41, 0, 137},
    {41, 0, 137},
    {44, 0, 138},
    {47, 0, 140},
    {47, 0, 140},
    {50, 0, 141},
    {53, 0, 142},
    {53, 0, 142},
    {56, 0, 143},
    {57, 0, 144},
    {59, 0, 146},
    {59, 0, 146},
    {62, 0, 147},
    {62, 0, 147},
    {65, 0, 149},
    {65, 0, 149},
    {68, 0, 149},
    {70, 0, 150},
    {70, 0, 150},
    {73, 0, 150},
    {74, 0, 150},
    {77, 0, 151},
    {77, 0, 151},
    {80, 0, 151},
    {80, 0, 151},
    {83, 0, 152},
    {83, 0, 152},
    {87, 0, 153},
    {87, 0, 153},
    {91, 0, 153},
    {91, 0, 153},
    {94, 0, 154},
    {94, 0, 154},
    {97, 0, 155},
    {99, 0, 155},
    {99, 0, 155},
    {103, 0, 155},
    {103, 0, 155},
    {106, 0, 155},
    {108, 0, 156},
    {108, 0, 156},
    {111, 0, 156},
    {111, 0, 156},
    {114, 0, 157},
    {114, 0, 157},
    {118, 0, 157},
    {118, 0, 157},
    {121, 0, 157},
    {121, 0, 157},
    {125, 0, 157},
    {125, 0, 157},
    {128, 0, 157},
    {128, 0, 157},
    {132, 0, 157},
    {132, 0, 157},
    {134, 0, 157},
    {135, 0, 157},
    {138, 0, 157},
    {138, 0, 157},
    {138, 0, 157},
    {142, 0, 157},
    {142, 0, 157},
    {145, 0, 156},
    {147, 0, 156},
    {150, 0, 156},
    {150, 0, 156},
    {152, 0, 155},
    {153, 0, 155},
    {156, 0, 155},
    {156, 0, 155},
    {156, 0, 155},
    {160, 0, 155},
    {160, 0, 155},
    {163, 0, 155},
    {163, 0, 155},
    {164, 0, 155},
    {167, 0, 154},
    {167, 0, 154},
    {167, 0, 154},
    {170, 0, 153},
    {170, 0, 153},
    {170, 0, 153},
    {174, 0, 153},
    {174, 0, 153},
    {176, 1, 152},
    {176, 1, 152},
    {176, 1, 152},
    {178, 1, 151},
    {178, 1, 151},
    {179, 1, 150},
    {180, 2, 150},
    {182, 2, 149},
    {182, 2, 149},
    {184, 3, 149},
    {184, 3, 149},
    {186, 4, 149},
    {186, 4, 149},
    {186, 4, 149},
    {187, 5, 147},
    {187, 5, 147},
    {189, 5, 147},
    {191, 6, 146},
    {191, 6, 146},
    {191, 6, 146},
    {193, 7, 145},
    {193, 7, 145},
    {193, 7, 145},
    {193, 7, 145},
    {194, 10, 143},
    {195, 10, 142},
    {195, 10, 142},
    {197, 12, 141},
    {197, 12, 141},
    {199, 14, 138},
    {199, 14, 138},
    {199, 14, 138},
    {201, 16, 136},
    {201, 16, 136},
    {203, 19, 133},
    {203, 19, 133},
    {203, 19, 133},
    {203, 19, 133},
    {204, 21, 130},
    {206, 22, 129},
    {206, 22, 129},
    {207, 24, 125},
    {207, 24, 125},
    {207, 25, 123},
    {209, 26, 120},
    {209, 26, 120},
    {210, 28, 117},
    {210, 28, 117},
    {210, 28, 117},
    {211, 30, 114},
    {211, 32, 113},
    {212, 33, 110},
    {212, 33, 110},
    {213, 35, 107},
    {213, 36, 105},
    {215, 37, 102},
    {215, 37, 102},
    {216, 39, 99},
    {216, 39, 99},
    {217, 42, 96},
    {218, 43, 93},
    {218, 43, 93},
    {219, 46, 88},
    {219, 46, 88},
    {220, 47, 84},
    {221, 48, 81},
    {221, 49, 78},
    {222, 50, 72},
    {222, 50, 72},
    {223, 52, 66},
    {223, 52, 66},
    {223, 54, 61},
    {224, 55, 56},
    {224, 55, 56},
    {224, 57, 51},
    {225, 58, 48},
    {226, 59, 43},
    {226, 59, 43},
    {227, 61, 36},
    {227, 61, 36},
    {228, 63, 32},
    {228, 65, 28},
    {228, 65, 28},
    {229, 67, 27},
    {229, 68, 24},
    {229, 68, 24},
    {230, 70, 22},
    {231, 71, 20},
    {231, 71, 20},
    {232, 73, 18},
    {232, 73, 18},
    {233, 76, 14},
    {233, 76, 14},
    {233, 76, 14},
    {233, 76, 14},
    {234, 78, 12},
    {234, 78, 12},
    {235, 81, 10},
    {235, 81, 10},
    {235, 81, 10},
    {236, 83, 9},
    {236, 83, 9},
    {236, 87, 8},
    {236, 87, 8},
    {236, 87, 8},
    {237, 89, 7},
    {237, 90, 6},
    {237, 90, 6},
    {238, 92, 5},
    {238, 92, 5},
    {238, 92, 5},
    {239, 95, 4},
    {239, 95, 4},
    {239, 95, 4},
    {239, 97, 4},
    {239, 97, 4},
    {240, 99, 3},
    {240, 99, 3},
    {240, 101, 3},
    {241, 102, 3},
    {241, 102, 3},
    {241, 102, 3},
    {241, 104, 2},
    {241, 104, 2},
    {241, 107, 2},
    {241, 107, 2},
    {241, 107, 2},
    {242, 108, 1},
    {242, 108, 1},
    {242, 110, 1},
    {243, 112, 1},
    {243, 112, 1},
    {243, 112, 1},
    {244, 114, 0},
    {244, 114, 0},
    {244, 116, 0},
    {244, 116, 0},
    {244, 118, 0},
    {244, 118, 0},
    {244, 120, 0},
    {245, 123, 0},
    {245, 123, 0},
    {245, 123, 0},
    {245, 126, 0},
    {245, 126, 0},
    {246, 129, 0},
    {246, 129, 0},
    {246, 130, 0},
    {247, 132, 0},
    {247, 132, 0},
    {247, 132, 0},
    {247, 134, 0},
    {248, 136, 0},
    {248, 136, 0},
    {248, 136, 0},
    {248, 136, 0},
    {248, 139, 0},
    {248, 139, 0},
    {248, 139, 0},
    {249, 141, 0},
    {249, 141, 0},
    {249, 141, 0},
    {249, 144, 0},
    {249, 144, 0},
    {249, 144, 0},
    {249, 146, 0},
    {249, 147, 0},
    {250, 149, 0},
    {250, 149, 0},
    {250, 149, 0},
    {251, 153, 0},
    {251, 153, 0},
    {251, 154, 0},
    {252, 157, 0},
    {252, 157, 0},
    {252, 160, 0},
    {252, 160, 0},
    {252, 161, 0},
    {253, 163, 0},
    {253, 163, 0},
    {253, 164, 0},
    {253, 167, 0},
    {253, 167, 0},
    {253, 168, 0},
    {253, 171, 0},
    {253, 171, 0},
    {253, 171, 0},
    {253, 173, 0},
    {253, 174, 0},
    {254, 176, 0},
    {254, 176, 0},
    {254, 176, 0},
    {254, 178, 0},
    {254, 178, 0},
    {254, 180, 0},
    {254, 181, 0},
    {254, 182, 0},
    {254, 185, 0},
    {254, 185, 0},
    {254, 185, 0},
    {254, 185, 0},
    {254, 187, 0},
    {254, 187, 0},
    {254, 189, 0},
    {254, 190, 0},
    {254, 193, 0},
    {254, 193, 0},
    {254, 193, 0},
    {254, 195, 0},
    {254, 196, 0},
    {254, 196, 0},
    {254, 198, 0},
    {254, 199, 0},
    {254, 200, 0},
    {254, 201, 1},
    {254, 202, 1},
    {254, 202, 1},
    {254, 203, 1},
    {254, 205, 2},
    {254, 205, 2},
    {254, 207, 3},
    {254, 207, 3},
    {254, 207, 3},
    {254, 209, 5},
    {254, 209, 5},
    {254, 211, 8},
    {254, 212, 9},
    {254, 214, 10},
    {254, 214, 10},
    {254, 216, 11},
    {254, 216, 11},
    {255, 218, 13},
    {255, 218, 13},
    {255, 218, 13},
    {255, 220, 17},
    {255, 220, 17},
    {255, 221, 21},
    {255, 221, 21},
    {255, 222, 26},
    {255, 222, 26},
    {255, 223, 30},
    {255, 224, 32},
    {255, 226, 35},
    {255, 226, 35},
    {255, 227, 39},
    {255, 227, 39},
    {255, 228, 44},
    {255, 228, 44},
    {255, 230, 51},
    {255, 230, 51},
    {255, 231, 58},
    {255, 231, 58},
    {255, 232, 63},
    {255, 234, 68},
    {255, 234, 68},
    {255, 235, 73},
    {255, 235, 77},
    {255, 236, 80},
    {255, 238, 85},
    {255, 238, 85},
    {255, 238, 91},
    {255, 238, 95},
    {255, 239, 99},
    {255, 240, 104},
    {255, 240, 104},
    {255, 240, 110},
    {255, 241, 114},
    {255, 241, 119},
    {255, 241, 123},
    {255, 242, 128},
    {255, 242, 133},
    {255, 242, 138},
    {255, 243, 142},
    {255, 244, 146},
    {255, 244, 150},
    {255, 244, 154},
    {255, 245, 158},
    {255, 245, 162},
    {255, 245, 166},
    {255, 246, 170},
    {255, 246, 175},
    {255, 247, 181},
    {255, 247, 181},
    {255, 248, 186},
    {255, 248, 189},
    {255, 248, 195},
    {255, 248, 195},
    {255, 249, 201},
    {255, 249, 201},
    {255, 249, 205},
    {255, 250, 211},
    {255, 250, 211},
    {255, 252, 218},
    {255, 252, 218},
    {255, 252, 223},
    {255, 253, 228},
    {255, 253, 228},
    {255, 253, 232},
    {255, 254, 235},
    {255, 254, 238},
    {255, 254, 241},
    {255, 255, 245},
    {255, 255, 245},
    {255, 255, 255},
};

static const int palette_color_count = sizeof(palette) / sizeof(palette[0]);
StartItem::StartItem(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{

//    memset(picturePath, 0, 256);
//    bShowInfo = false;
//    bDrawThermal = 0;
//    showTemperature = false;
//    iMeasuringResult = -1;
//    idCardFlags = -1;
//    pictureFlags = -1;
//    compareResult = -1;
//    showIdCardResult = -1;
//    pictureidCard = -1;
//    tipText = QString("");
//    cardText = QString("");
//    flags = Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap;
//    font.setFamily("Helvetica Light"); // Helvetica-Light 字体
//    temp_array = new float[32*32];
//    //人员信息
//    tempuratureText = QString("");
//    nameText = QString("");
//    picture.load("./images/contrast_box.png");
//    facePicture.load(Path +  QString("images/default_user_face.png"));
//    redQImage.load(Path +  QString("/images/bg_red.png"));
//    greenQImage.load(Path +  QString("images/bg_green.png"));
//    yellowQImage.load(Path +  QString("images/bg_yellow.png"));
//    redTemperatureQImage.load(Path +  QString("images/icon_temperature_red.png"));
//    greenTemperatureQImage.load(Path +  QString("images/icon_temperature_nor.png"));
//    yellowTemperatureQImage.load(Path +  QString("images/icon_temperature_yellow.png"));
//    pictureEmptyQImage.load(Path +  QString("images/photo_empty.png"));
//    pictureWaitingQImage.load(Path +  QString("images/photo_waiting.png"));
//    pictureYellowFlagsQImage.load(Path +  QString("/images/icon_waiting.png"));
//    pictureGreenFlagsQImage.load(Path +  QString("images/icon_right.png"));
//    pictureRedFlagsQImage.load(Path +  QString("images/icon_abnormal.png"));
//    snapshotThread = new SnapshotThread();
}

QRectF StartItem::boundingRect() const
{
    return QRectF(0, 0, 800, 1280);
}

//static int rgaPrepareInfo(uchar *buf, RgaSURF_FORMAT format, QRectF rect,
//                int sw, int sh, rga_info_t *info)
//{
//    memset(info, 0, sizeof(rga_info_t));

//    info->fd = -1;
//    info->virAddr = buf;
//    info->mmuFlag = 1;

//    return rga_set_rect(&info->rect, rect.x(), rect.y(), rect.width(), rect.height(), sw, sh, format);
//}

//static int rgaDrawImage(uchar *src, RgaSURF_FORMAT src_format, QRectF srcRect,
//                int src_sw, int src_sh, uchar *dst, RgaSURF_FORMAT dst_format,
//                QRectF dstRect, int dst_sw, int dst_sh, int rotate, unsigned int blend)
//{
//    static int rgaSupported = 1;
//    static int rgaInited = 0;
//    rga_info_t srcInfo;
//    rga_info_t dstInfo;

//    memset(&srcInfo, 0, sizeof(rga_info_t));
//    memset(&dstInfo, 0, sizeof(rga_info_t));

//    if (!rgaSupported)
//        return -1;

//    if (!rgaInited) {
//        if (c_RkRgaInit() < 0) {
//            rgaSupported = 0;
//            return -1;
//        }

//        rgaInited = 1;
//    }

//    if (rgaPrepareInfo(src, src_format, srcRect, src_sw, src_sh, &srcInfo) < 0)
//        return -1;

//    if (rgaPrepareInfo(dst, dst_format, dstRect, dst_sw, dst_sh, &dstInfo) < 0)
//        return -1;

//    srcInfo.rotation = rotate;
//    if(blend)
//        srcInfo.blend = blend;

//    return c_RkRgaBlit(&srcInfo, &dstInfo, NULL);
//}


void StartItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget)
{
//    if(showIdCardResult == 1)
//     {
//        painter->drawImage(QRect(27, 386, 746, 444), picture);//27, 386, 746, 444
//  //        painter->drawImage(QRect(501, 479, 232, 284), pictureWaitingQImage);
//  //        painter->drawImage(QRect(67, 473, 232, 284), pictureEmptyQImage);
//  //        qDebug("++++++pictureidCard:%d++++++++++",pictureidCard);

//        if(pictureidCard == QT_SUB_IDENTITYCARD_UNMATCH)
//        {
//            painter->drawImage(QRect(339, 561, 121, 121),pictureRedFlagsQImage);
//        }
//        else if(pictureidCard == QT_SUB_IDENTITYCARD_MATCH)
//        {
//            painter->drawImage(QRect(339, 561, 121, 121),  pictureGreenFlagsQImage);
//        }
//        else
//        {
//            painter->drawImage(QRect(339, 561, 121, 121), pictureYellowFlagsQImage);
//        }

//    }

//    if(bShowInfo)
//    {
//          QImage *image = static_cast<QImage *>(painter->paintEngine()->paintDevice());
//          QRectF srcRect(0, 0, snapshotThread->snapshotWidth(), snapshotThread->snapshotHeight());

//          if(pictureFlags == QT_SUB_IDENTITYCARD_USE)
//         {
//              QRectF srcInfo(67, 478, 232, 286);
//              rgaDrawImage((uchar *)snapshotThread->snapshotBuf(), snapshotThread->snapshotFormat(), srcRect,
//                              snapshotThread->snapshotWidth(), snapshotThread->snapshotHeight(),
//                              image->bits(), RK_FORMAT_BGRA_8888, srcInfo,
//                              image->width(), image->height(), 0, 0);
//          }
//          else
//          {
//              QRectF srcInfo1(40, 1082, 136, 136);
//              rgaDrawImage((uchar *)snapshotThread->snapshotBuf(), snapshotThread->snapshotFormat(), srcRect,
//                              snapshotThread->snapshotWidth(), snapshotThread->snapshotHeight(),
//                              image->bits(), RK_FORMAT_BGRA_8888, srcInfo1,
//                              image->width(), image->height(), 0, 0);
//          }
//    }
//# if 0
//    qDebug("+++++++++++++++DrawInfo+++++++++++++++");
//    //温度信息
//    if(idCardFlags == 1) {
//        painter->drawImage(QRect(0, 354, 801, 150), greenQImage);
//        painter->drawImage(QRect(27, 386, 746, 444), pictureContrastQImage);
//        painter->drawImage(QRect(363, 400, 50, 50), greenTemperatureQImage);

//        if(compareResult == 0) {
//            painter->drawImage(QRect(339, 561, 121, 121), pictureGreenFlagsQImage);
//        } else if(compareResult == 1) {
//            painter->drawImage(QRect(339, 561, 121, 121), pictureRedFlagsQImage);
//        } else {
//            painter->drawImage(QRect(339, 561, 121, 121), pictureYellowFlagsQImage);
//        }
//        painter->drawImage(QRect(501, 479, 232, 284), pictureWaitingQImage);
//        painter->drawImage(QRect(67, 473, 232, 284), pictureEmptyQImage);

//    }

//    //      painter->drawImage(QRect(40, 1082, 136, 136), facePicture);
//    //      painter->drawText( 200, 1024, 356, 60,Qt::AlignCenter|Qt::TextWordWrap, nameText);
//    }
//#endif
//#if 0
//    if(pictureidCard) {
//        QImage *image = static_cast<QImage *>(painter->paintEngine()->paintDevice());
//        QRectF srcRect(0, 0, snapshotThreadIdCard->snapshotWidth(), snapshotThreadIdCard->snapshotHeight());
//        QRectF srcInfo(501, 479, 232, 284);
//        rgaDrawImage((uchar *)snapshotThreadIdCard->snapshotBuf(), snapshotThreadIdCard->snapshotFormat(), srcRect,
//                        snapshotThreadIdCard->snapshotWidth(), snapshotThreadIdCard->snapshotHeight(),
//                        image->bits(), RK_FORMAT_BGRA_8888, srcInfo,
//                        image->width(), image->height(), 0, 0);
//        qDebug("__________pictureidCard____________");
//    }
//#endif

//#if 1
//    font.setPixelSize(40);
//    painter->setFont(font);


//    if(iMeasuringResult == QT_SUB_TEMP_NORMAL)
//    {

//       if(showTemperature)
//       {
//           painter->setPen(QPen(QColor(121,202,0)));
//           if(idCardFlags == 0)
//           {
//               painter->drawText(QRect(118, 422, 163, 50),tempuratureText);
//           }
//           else
//           {
//               painter->drawImage(QRect(343, 1098, 50, 50), greenTemperatureQImage);
//               painter->drawText(QRect(393, 1096, 163, 50),Qt::AlignVCenter,tempuratureText);
//           }
//       }
//    }
//    else if (iMeasuringResult == QT_SUB_TEMP_LOW)
//    {
//       if(showTemperature)
//       {
//           painter->setPen(QPen(QColor(255,187,0)));
//           if(idCardFlags == 0)
//           {
//               painter->drawText(QRect(68, 422, 232, 50),Qt::AlignCenter|Qt::TextWordWrap,tempuratureText);
//           }
//           else
//           {
//               painter->drawImage(QRect(343, 1098, 50, 50), yellowTemperatureQImage);
//               painter->drawText(QRect(393, 1093, 263, 60),tempuratureText);
//           }
//       }

//    }
//    else if (iMeasuringResult == QT_SUB_TEMP_HIGH)
//    {
//      if(showTemperature)
//      {
//           painter->setPen(QPen(QColor(255,57,59)));
//          if(idCardFlags == 0)
//          {
//              painter->drawText(QRect(68, 422, 232, 50),Qt::AlignCenter|Qt::TextWordWrap,tempuratureText);
//          }
//          else
//          {
//              painter->drawImage(QRect(343, 1098, 50, 50), redTemperatureQImage);
//              painter->drawText(QRect(393, 1096, 263, 50),Qt::AlignVCenter,tempuratureText);
//          }
//      }
//    }
//    else if (iMeasuringResult == QT_SUB_TEMP_SUPER_HIGH)
//    {
//      if(showTemperature)
//      {
//           painter->setPen(QPen(QColor(255,57,59)));
//          if(idCardFlags == 0)
//          {
//              painter->drawText(QRect(68, 422, 232, 50),Qt::AlignCenter|Qt::TextWordWrap,tempuratureText);
//          }
//          else
//          {
//              painter->drawImage(QRect(343, 1098, 50, 50), redTemperatureQImage);
//              painter->drawText(QRect(393, 1098, 283, 50),tempuratureText);
//          }
//      }
//    }
//    //提示语
//    font.setPixelSize(50);
//    painter->setFont(font);
//    painter->setPen(QPen(QColor(255,255,255)));
//    painter->drawText(131, 873, 573, 120,Qt::AlignCenter|Qt::TextWordWrap,tipText);
//    //  painter->drawText(131, 1173, 573, 120,Qt::AlignCenter|Qt::TextWordWrap,nameText);
//    painter->drawText(131, 1173, 573, 120,Qt::AlignCenter|Qt::TextWordWrap,cardText);
//#endif

//    if(!bDrawThermal) {
//    //      clear_thermal_image(painter);
//      return;
//    }
////       qDebug("+++++++++++++++DrawThermal+++++++++++++++");
//        min_temp = 99.0;
//        max_temp = 0.0;
//        for (int i = 0; i < 32; i++)
//        {
//            for (int j = 0; j < 32; j++)
//            {
//                float temp = temp_array[i * 32 + j];
//                if (temp > max_temp) max_temp = temp;
//                if (temp < min_temp) min_temp = temp;
//            }
//        }
//        temp_range = max_temp - min_temp;

////       qDebug("temp:%2f,max_temp:%2f,min_temp:%2f", temp_range, max_temp, min_temp);
//        unsigned char rgb[3];
//        for (int i = 0; i < 32; i++)
//        {
//            for (int j = 0; j < 32; j++)
//            {
//                float temp = temp_array[i * 32 + j];
//                temp_to_color(temp, rgb);
//                int x = image_start_x + j * CELL_WIDTH;
//                int y = image_start_y + i * CELL_HEIGHT;
//                int w = CELL_WIDTH;
//                int h = CELL_HEIGHT;
//                painter->setBrush(QColor(rgb[0], rgb[1], rgb[2]));
//                painter->setPen(QPen(QColor(rgb[0], rgb[1], rgb[2])));
//                painter->setRenderHint(QPainter:: Antialiasing, true);
//                painter->drawRect(QRect(x, y, w, h));                           //FillBox(hdc, x, y, w, h);

//            }
//        }

}

StartItem::~StartItem() {
//    delete []temp_array;

}


//void StartItem::init_thermal_imaging(int start_x, int start_y) {
//    image_start_x = start_x;
//    image_start_y = start_y;
//}

//void StartItem::temp_to_color(float temp, uint8_t rgb[3]) {
//    int color_index = (temp - min_temp) * palette_color_count / temp_range;
//    if (color_index < 0)
//    {
//        color_index = 0;
//    }
//    else if (color_index >= palette_color_count)
//    {
//        color_index = palette_color_count - 1;
//    }
//    memcpy(rgb, palette[color_index], sizeof(float) * 3);
//    //fprintf(stderr, "temp: %.2f, color: (%d, %d, %d)\n",
//    //	   temp, rgb[0], rgb[1], rgb[2]);
//}

#ifdef PERFORMANCE
static uint64_t get_time_us() {
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0) {
        return (uint64_t)tp.tv_sec * 1000 * 1000 + tp.tv_nsec / 1000;
    }
    return 0;
}
#endif

//void StartItem::paint_thermal_image(const float* ctemp_array,int bthermal)
//{
//     bDrawThermal = bthermal;
//     if(bDrawThermal == 1)
//     {
//       memcpy(temp_array,ctemp_array,1024*sizeof(float));
//     }
//}

//void StartItem::clear_thermal_image(QPainter *painter)
//{
//    painter->setBrush(QColor(0, 0, 0));                                     //SetBrushColor(hdc, RGB2Pixel(hdc, rgb[0], rgb[1], rgb[2]));
//    painter->drawRect(QRect(image_start_x,image_start_y, CELL_WIDTH * temp_array_width,CELL_HEIGHT * temp_array_height));                           //FillBox(hdc, x, y, w, h);

//}

#endif

