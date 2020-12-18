#include <stdio.h>
//#include <sys/time.h>
//#include <sys/ioctl.h>
//#include <sys/socket.h>
//#include <net/if.h>
//#include <linux/sockios.h>
//#include <arpa/inet.h>
#include <QtWidgets>
#include <QTouchEvent>
#include <QList>
#include <QTime>
#include <QMessageBox>
#include<queue>
#include <synchapi.h>
//#include "dbserver.h"
//#include <rkfacial/rkfacial.h>
#include <QTranslator>
#ifdef TWO_PLANE
#include <rkfacial/display.h>
#include <rkfacial/draw_rect.h>
#endif
#include "savethread.h"
#include "desktopview.h"
//#include <rkfacial/sdk_struct.h>
//#include "rkfacial/new_ipcc.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//提供摄像头用
#define CAMERA_WIDTH 1280
#define CAMERA_HEIGHT 720
#define SAVE_FRAMES 30

DesktopView *DesktopView::desktopView = nullptr;
static pthread_mutex_t      g_paintFaceRecgInfo_mutex;
static pthread_mutex_t      g_paintStatusBarInfo_mutex;
static pthread_mutex_t      g_paintStatusBarInfoDraw_mutex;
static CYCLE_QUEUE_T         g_paintFaceRecgInfo_queue;
static CYCLE_QUEUE_T         g_paintStatusBarInfo_queue;

static void queueCycleInit(CYCLE_QUEUE_T *Q, int maxQueueNum)
{
    Q->maxQueueNum = maxQueueNum;
    Q->base = (queue_cycle_elem_data *)malloc(Q->maxQueueNum * sizeof(queue_cycle_elem_data));
    Q->front = Q->rear =0;
}

static void queueCycleDestroy(CYCLE_QUEUE_T *Q)
{
    free(Q->base);
}

static void queueCycleClear(CYCLE_QUEUE_T *Q)
{
    Q->front=Q->rear=0;
}

static int queueCycleEmpty(CYCLE_QUEUE_T Q)
{
    if(Q.front == Q.rear)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int queueCycleLength(CYCLE_QUEUE_T Q)
{
    return (Q.rear + Q.maxQueueNum - Q.front)%Q.maxQueueNum;
}

static void queueCycleGetHead(CYCLE_QUEUE_T Q, queue_cycle_elem_data *e)
{
    if(Q.front != Q.rear)
    {
            memcpy(e, &(Q.base[Q.front]), sizeof(queue_cycle_elem_data));
    }
}

static int queueCycleIn(CYCLE_QUEUE_T *Q, queue_cycle_elem_data e)
{
    if((Q->rear+1)%Q->maxQueueNum == Q->front)
    {
        return 0;
    }
    memcpy(&(Q->base[Q->rear]), &e, sizeof(queue_cycle_elem_data));
    Q->rear = (Q->rear+1)%Q->maxQueueNum;
    return 1;
}

static int queueCycleOut(CYCLE_QUEUE_T *Q, queue_cycle_elem_data *e)
{
    if(Q->front == Q->rear)
    {
        return 0;
    }
    memcpy(e, &(Q->base[Q->front]), sizeof(queue_cycle_elem_data));
    Q->front = (Q->front+1)%Q->maxQueueNum;
    return 1;
}


static void queueCycleTraverse(CYCLE_QUEUE_T Q)
{
    int i;
    printf("Queue: ");
    if(Q.rear < Q.front)
    {
        Q.rear = Q.rear + Q.maxQueueNum;
    }
    for(i = Q.front; i < Q.rear; i++)
    {
        printf("%d\r\n", Q.base[i%Q.maxQueueNum]);
    }
}

#if 0
/* Լɪ������ */
void queueLinkJoseffer(int n)
{
    LINK_QUEUE_T Q;
    int i;
    queueLinkElemType x;
    queueLinkInit(&Q);
    for(i = 1;i <= n; i++)
        queueLinkIn(&Q,i);
    while(!queueLinkEmpty(Q))
    {
        for(i = 1;i <= 3; i++)
        {
            queueLinkOut(&Q,&x);
            if(i != 3)
            {
                queueLinkIn(&Q,x);
            }
            else
            {
                printf("%5d\r\n",x);
            }
        }
    }
}
#endif

#if 0
/* ������ */
int main()
{
    LINK_QUEUE_T Q;
    int i;
    queueLinkElemType x;

    queueLinkInit(&Q);
    for(i = 2;i <= 5;i++)
        queueLinkIn(&Q, i);

    printf("len:%d\r\n", queueLinkLength(Q));
    while(!queueLinkEmpty(Q))
    {
        queueLinkOut(&Q, &x);
        printf("%d\r\n", x);
    }
    //queueLinkTraverse(Q);
    Joseffer(6);

    return 0;
}
#endif


//static int getLocalIp(char *interface, char *ip, int ip_len)
//{
//    int sd;
//    struct sockaddr_in sin;
//    struct ifreq ifr;

//    memset(ip, 0, ip_len);
//    sd = socket(AF_INET, SOCK_DGRAM, 0);
//    if (-1 == sd) {
//        //qDebug("socket error: %s\n", strerror(errno));
//        return -1;
//    }

//    strncpy(ifr.ifr_name, interface, IFNAMSIZ);
//    ifr.ifr_name[IFNAMSIZ - 1] = 0;

//    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0) {
//        //qDebug("ioctl error: %s\n", strerror(errno));
//        close(sd);
//        return -1;
//    }

//    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
//    sprintf(ip, "%s", inet_ntoa(sin.sin_addr));

//    close(sd);
//    return 0;
//}


void DesktopView::initTimer()
{
    timer = new QTimer;
    timer->setSingleShot(false);
    timer->start(1000); //ms
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeOut()));

//    faceTimer = new QTimer;
//    faceTimer->setSingleShot(false);
//    faceTimer->start(1000); //ms
//    connect(faceTimer, SIGNAL(timeout()), this, SLOT(faceTimerTimeOut()));
}

void DesktopView::timerTimeOut()
{
#if 0
    static QTime t_time;
    static int cnt = 1;

    if(cnt) {
        t_time.start();
        cnt--;
    }

    qDebug("%s: %ds", __func__, t_time.elapsed() / 1000);
#endif
//    char ip[MAX_IP_LEN];

//    getLocalIp("eth0", ip, MAX_IP_LEN);
//    if(!strcmp(ip, videoItem->getIp()))
//        return;

//    videoItem->setIp(ip);

     emit updateTime();


//    QDateTime time = QDateTime::currentDateTime();
//    QString timeData = time.toString("hh:mm");
//    timeLabel->setText(timeData);
//    QString date = QString("");
//    if(languageType == 1) {
//        QLocale locale = QLocale::Chinese;
//        date = locale.toString(time,QString("yyyy/MM/dd  dddd"));
//    } else {
//        date = time.toString("yyyy/MM/dd  dddd");
//    }
//    dateLabel->setText(date);

//    timeLabel->update();
//    dateLabel->update();

//    if(percent > 100) {
//       percent = 0;
//    }
//    waterProcess1->setValue(percent);
//    percent++;

}

void DesktopView::faceTimerTimeOut()
{
	updateFace = true;

#ifdef TWO_PLANE
	if(videoItem->isVisible())
		updateUi(0, desktopRect.height()*4/5, desktopRect.width(), desktopRect.height()/5);
#endif
}

bool DesktopView::event(QEvent *event)
{
    switch(event->type()) {
#if 0 //when support mouse, the touch event is also converted to a mouse event
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        {
            QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            if(touchPoints.count() != 1)
                break;

            const QTouchEvent::TouchPoint &touchPoint = touchPoints.first();
            switch (touchPoint.state()) {
                case Qt::TouchPointStationary:
                case Qt::TouchPointReleased:
                    // don't do anything if this touch point hasn't moved or has been released
                    break;
                default:
                {
                    bool flag;
                    QRectF rect = touchPoint.rect();
                    if (rect.isEmpty())
                        break;

                    bool settingFlag = rect.y() > menuWidget->height()
                        && rect.y() < (desktopRect.height()*4/5 - 110);
    #ifdef BUILD_TEST
                    bool testFlag = rect.x() > testWidget->width()
                        || (rect.y() < testWidget->y()
                        || rect.y() > testWidget->y() + testWidget->height());

                    if(testWidget->isVisible())
                        flag = settingFlag && testFlag;
                    else
    #endif
                        flag = settingFlag;

                    if(flag) {
                        if(menuWidget->isVisible())
                            menuWidget->setVisible(false);
                        else
                            menuWidget->setVisible(true);
    #ifdef BUILD_TEST
                        if(testWidget->isVisible())
                            testWidget->setVisible(false);
                        else
                            testWidget->setVisible(true);
#endif
                    }
                    break;
                }
            }
            break;
        }
#endif
        case QEvent::MouseButtonPress:
        {
            if(editWidget->isVisible())
                break;

            if(menuWidget->isVisible())
                menuWidget->setVisible(false);
            else
                menuWidget->setVisible(true);
#ifdef BUILD_TEST
            if(testWidget->isVisible())
                testWidget->setVisible(false);
            else
                testWidget->setVisible(true);

#endif
            break;
        }

        default:
            break;
    }

    return QGraphicsView::event(event);
}

bool DesktopView::eventFilter(QObject *obj, QEvent *e)
{
    if((obj == ipEdit || obj == netmaskEdit || obj == gatewayEdit)
        && e->type() == QEvent::MouseButtonPress){
        if(keyBoard->isHidden())
            keyBoard->showPanel();

        keyBoard->focusLineEdit((QLineEdit*)obj);
    }
    return QGraphicsView::eventFilter(obj,e);
}

static void setButtonFormat(QBoxLayout *layout, QPushButton *btn, QRect rect)
{
    bool two_plane = false;

    #ifdef TWO_PLANE
    two_plane = true;
    #endif

    if(!two_plane) {
        btn->setFixedSize(rect.width()/4, rect.width()/10);
        btn->setStyleSheet("QPushButton{font-size:30px}");
    }
    layout->addWidget(btn);
}

#ifdef BUILD_TEST
void DesktopView::paintTestInfo(struct test_result *test)
{
    desktopView->videoItem->setTesIntfo(test);
    desktopView->collectBtn->setEnabled(true);
    desktopView->realBtn->setEnabled(true);
    desktopView->photoBtn->setEnabled(true);
    desktopView->testing = false;
}

void DesktopView::initTestUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setMargin(0);
    vLayout->setSpacing(0);

    collectBtn = new QPushButton(tr("Start collecting"));
    setButtonFormat(vLayout, collectBtn, desktopRect);
    realBtn = new QPushButton(tr("test the reality"));
    setButtonFormat(vLayout, realBtn, desktopRect);
    photoBtn = new QPushButton(tr("test the image"));
    setButtonFormat(vLayout, photoBtn, desktopRect);

    testWidget = new QWidget();
    testWidget->setLayout(vLayout);
    //testWidget->setObjectName("testWidget");
    //testWidget->setStyleSheet("#testWidget {background-color:rgba(10, 10, 10,100);}");
    testWidget->setWindowOpacity(0.8);
    testWidget->setGeometry(0, desktopRect.height()/3, 0, desktopRect.width()/10*3);

    connect(collectBtn, SIGNAL(clicked()), this, SLOT(saveAllSlots()));
    connect(realBtn, SIGNAL(clicked()), this, SLOT(saveFakeSlots()));
    connect(photoBtn, SIGNAL(clicked()), this, SLOT(saveRealSlots()));

    testing = false;
    register_get_test_callback(paintTestInfo);
}

static void setSaveIrFlag(bool real, bool fake)
{
    save_ir_real(real);
    save_ir_fake(fake);
}

void DesktopView::saveAllSlots()
{
    if(testing)
        return;

    bool equal = collectBtn->text().compare("Start collecting", Qt::CaseSensitive);

    if(!equal) {
        setSaveIrFlag(true, true);
        collectBtn->setText(tr("End collecting"));
        realBtn->setEnabled(false);
        photoBtn->setEnabled(false);
    } else {
        setSaveIrFlag(false, false);
        collectBtn->setText(tr("Start collecting"));
        realBtn->setEnabled(true);
        photoBtn->setEnabled(true);
    }
}

void DesktopView::saveRealSlots()
{
    setSaveIrFlag(true, false);
    rockface_start_test();

    testing = true;
    collectBtn->setEnabled(false);
    realBtn->setEnabled(false);
    photoBtn->setEnabled(false);
}

void DesktopView::saveFakeSlots()
{
    setSaveIrFlag(false, true);
    rockface_start_test();

    testing = true;
    collectBtn->setEnabled(false);
    realBtn->setEnabled(false);
    photoBtn->setEnabled(false);
}
#endif

void DesktopView::updateUi(int x, int y, int w, int h)
{
#if 0
    //printf fps
    static QTime paint_time;
    static int paint_frames = 0;
    if (!paint_frames)
        paint_time.start();

    if(paint_time.elapsed()/1000 >= 10) {
        paint_frames = 0;
        paint_time.restart();
    }

    if(!(++paint_frames % 50))
        printf("+++++ %s FPS: %2.2f (%u frames in %ds)\n",
                __func__, paint_frames * 1000.0 / paint_time.elapsed(),
                paint_frames, paint_time.elapsed() / 1000);
#endif

    emit itemDirty(x, y, w, h);
}

void DesktopView::updateStatus(QString path, QString text,int mode)
{
    qDebug()<< "path:"<< path <<"text:"<< text;
    pthread_mutex_lock(&g_paintStatusBarInfoDraw_mutex);
    QPixmap picture;
    if(!picture.load(path))
    {
        qDebug("==================================return==========\n");
        return;
    }else{

        qDebug("%s %d==================================start\n", __FILE__, __LINE__);
        desktopView->iconLabel->clear();
        desktopView->ipTextLabel->clear();
        desktopView->iconLabel->setPixmap(picture);
        qDebug("%s %d==================================start\n", __FILE__, __LINE__);
        desktopView->ipTextLabel->setText(text);
        desktopView->ipTextLabel->show();
        desktopView->iconLabel->show();
        desktopView->netLinkStatus = mode;
        qDebug("%s %d==================================start\n", __FILE__, __LINE__);
        desktopView->updateUi(0,0,800,50);
        pthread_mutex_unlock(&g_paintStatusBarInfoDraw_mutex);
        qDebug("%s %d==================================end\n", __FILE__, __LINE__);

    }

}



//void DesktopView::cameraSwitch()
//{
//    qDebug()<<"cameraType:"<<cameraType;
//    if(cameraType == ISP) {
//    //      videoItem->setBoxRect(0, 0, -1, -1);
//        switchBtn->setText(tr("IR"));
//        cameraType = CIF;
//#ifdef TWO_PLANE
//        display_switch(DISPLAY_VIDEO_IR);
//#endif
//    } else {
//        switchBtn->setText(tr("RGB"));
//        cameraType = ISP;
//#ifdef TWO_PLANE
//        display_switch(DISPLAY_VIDEO_RGB);
//#endif

//    }
//}

//void DesktopView::registerSlots()
//{
//    rkfacial_register();
//}

//void DesktopView::deleteSlots()
//{
//    rkfacial_delete();
//    videoItem->clear();
//}

//void DesktopView::saveSlots()
//{
//    saving = true;
//    saveBtn->setEnabled(false);
//    switchBtn->setEnabled(false);
//}

//void DesktopView::updateScene(int x, int y, int w, int h)
//{
//    scene()->update(x, y, w, h);
//    update(x, y, w, h);
//}

//void DesktopView::setSlots()
//{
//    if(videoItem->isVisible()) {
//        videoItem->setVisible(false);
//        setWidget->setVisible(false);
//        menuWidget->setVisible(false);
//        editWidget->setVisible(true);

//#ifdef BUILD_TEST
//        if(testWidget->isVisible())
//            testWidget->setVisible(false);
//#endif
//    }
//}

//void DesktopView::closeSlots()
//{
//    if(editWidget->isVisible()) {
//        videoItem->setVisible(true);
//        setWidget->setVisible(true);
//        editWidget->setVisible(false);
//        keyBoard->hidePanel();
//    }
//}

static bool checkAddress(QString address, bool isNetmask)
{
    int p0, p1, p2, p3;
    int maxP0;

    QList<QString> list = address.split('.');

    if(list.count() != 4)
        return true;

    p0 = list[0].toInt();
    p1 = list[1].toInt();
    p2 = list[2].toInt();
    p3 = list[3].toInt();

    if(isNetmask)
        maxP0 = 255;
    else
        maxP0 = 233;

    if(p0 < 0 || p0 > maxP0
        || p1 < 0 || p1 > 255
        || p2 < 0 || p2 > 255
        || p3 < 0 || p3 > 255)
        return true;

    return false;
}

//void DesktopView::editSetSlots()
//{
//    bool check;

//    if(checkAddress(ipEdit->text(), false)) {
//        QMessageBox::warning(NULL, "warning", "Invalid IP");
//        return;
//    }

//    if(checkAddress(netmaskEdit->text(), true)) {
//        QMessageBox::warning(NULL, "warning", "Invalid Netmask");
//        return;
//    }

//    if(checkAddress(gatewayEdit->text(), false)) {
//        QMessageBox::warning(NULL, "warning", "Invalid Gateway");
//        return;
//    }

//    dbserver_network_ipv4_set("eth0", "manual", qPrintable(ipEdit->text()),
//        qPrintable(netmaskEdit->text()), qPrintable(gatewayEdit->text()));
//}

void DesktopView::updateTimeScene() {
        QDateTime time = QDateTime::currentDateTime();
        QString timeData = time.toString("hh:mm:ss");
        timeLabel->setText(timeData);
        QString date = QString("");
        QString dateTimeType = QString("yyyy/MM/dd  dddd");
        QLocale locale = QLocale::English;
        if(timeType == 1) {
            dateTimeType = QString("dd/MM/yyyy  dddd");
        } else if (timeType == 2) {
            dateTimeType = QString("MM/dd/yyyy  dddd");
        }
        if(languageType == 0) {
            locale = QLocale::Chinese;
        } else {
            locale = QLocale::English;
        }
        date = locale.toString(time,dateTimeType);
        dateLabel->setText(date);
        timeLabel->update();
        dateLabel->update();
}

//void DesktopView::clearFaceRecgInfoScene() {
//    faceRecgInfoItem->bShowInfo = false;
//    faceRecgInfoItem->bDrawThermal = 0;
//    faceRecgInfoItem->iMeasuringResult = -1;
//    desktopView->verificatStatus = -1;
//    desktopView->facePosition = -1;
//  //    thermal->tipText = QString("");
//  //    thermal->nameText = QString("");
//  //    thermal->cardText = QString("");
//    faceShadeWidget->setVisible(false);
//  //    idCardWidget->setVisible(false);
//    faceRecgInfoItem->idCardFlags = -1;
//    faceRecgInfoItem->pictureFlags = -1;
//  //    thermal->compareResult = -1;
//    faceRecgInfoItem->showIdCardResult = -1;
//    faceRecgInfoItem->showTemperature = false;
//    pictureLable->hide();
//    faceRecgInfoItem->bShowInfo = false;

//    greenLabel->hide();
//    redLabel->hide();
//    yellowLabel->hide();
//    faceLable->hide();
//    label->show();

//    desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());

//}

void DesktopView::paintFaceRecgInfoScene() {
//    qDebug("%s %d==================================start\n", __FILE__, __LINE__);
//    qDebug("Stranger      = %d\n",  desktopView->faceRecgInfo->FaceID);          // 1: 陌生人(黑名单)          其它:认证人员(白名单)
//    qDebug("NumberMatch      = %d\n",  desktopView->faceRecgInfo->NumberMatch);          // 1:请刷卡 2.匹配的卡号 3.未知的卡号
//    qDebug("Username      = %s\n",  desktopView->faceRecgInfo->Username);          // 名字
//    qDebug("MeasuringResult = %d\n",  desktopView->faceRecgInfo->MeasuringResult);      // 0(温度正常),     1(温度偏低),     2(温度异常)
//    qDebug("MaskStatus    = %d\n",  desktopView->faceRecgInfo->MaskStatus);        // 1: 佩戴口罩        其它: 未戴口罩
//    qDebug("Thermodynamic    = %d\n",  desktopView->faceRecgInfo->Thermodynamic);        // 1: 有热力图数据      其它: 无数据
//    qDebug("TextType      = %d\n",  desktopView->faceRecgInfo->TextType);          // (QT界面端给出支持的语言提示序号)
//    qDebug("Temperature   = %.2f\n",desktopView->faceRecgInfo->Temperature);        //温度
//    qDebug("PicPath   = %s\n", desktopView->faceRecgInfo->PicPath);        //
//    qDebug("VerificatMode      = %d\n",  desktopView->faceRecgInfo->VerificatMode);
//    qDebug("IdentityMatch      = %d\n",  desktopView->faceRecgInfo->IdentityMatch);

//    qDebug("%s %d==================================end\n", __FILE__, __LINE__);
#if 0
    QString tipText = QString("");
    thermal->nameText = QString("");

    if(faceRecgInfo->VerificatMode == 5 || faceRecgInfo->VerificatMode == 6){
        if(faceRecgInfo->IdentityMatch == 3) {
                  pictureLable->setPixmap(QPixmap(QString(faceRecgInfo->PicPath)));
                  pictureLable->show();
                  return;
          }
    }

    //温度提示语
    if(faceRecgInfo->MeasuringResult == 0) {
        tipText = tr("Passing");
    } else if(faceRecgInfo->MeasuringResult == 1) {
        tipText = tr("Low body temperature No Passing");
    } else if(faceRecgInfo->MeasuringResult == 2) {
        tipText = tr("Hight body temperature No Passing");
    }

    if(faceRecgInfo->FaceID == -1) {
        QString abbyText = tr("Abby");
        thermal->tipText = abbyText + QString(" ") + tipText;
    } else {
        thermal->tipText = QString::fromUtf8(faceRecgInfo->Username) + QString(" ") + tipText;
    }

//    thermal->iMeasuringResult = faceRecgInfo->MeasuringResult;
//    thermal->bShowInfo = true;
//        //图片
//    if(thermal->snapshotThread->setName(faceRecgInfo->PicPath)) thermal->snapshotThread->start();

   //测温模式
   if(faceRecgInfo->VerificatMode == 0 || faceRecgInfo->VerificatMode == 1 ||faceRecgInfo->VerificatMode == 3 || faceRecgInfo->VerificatMode == 5) {
       thermal->showTemperature = false;
       thermal->bDrawThermal = 0;
       thermal->paint_thermal_image(faceRecgInfo->TempArray,0);
   } else {
       thermal->showTemperature = true;
       thermal->bDrawThermal = faceRecgInfo->Thermodynamic;
       if(faceRecgInfo->Thermodynamic == 1){
           thermal->paint_thermal_image(faceRecgInfo->TempArray,faceRecgInfo->Thermodynamic);
       }
   }
   //温度信息
   thermal->tempuratureText = QString::number(faceRecgInfo->Temperature, 'f', 2) + QString("℃");
   if(faceRecgInfo->Temperature <= 0) {
       thermal->showTemperature = false;
   }

    //刷卡模式
    thermal->cardText = QString("");
    if(faceRecgInfo->VerificatMode == 1 || faceRecgInfo->VerificatMode == 3 || faceRecgInfo->VerificatMode == 4) {
       if(faceRecgInfo->NumberMatch == 1) {
           thermal->cardText = QString("请刷卡");
       } else if(faceRecgInfo->NumberMatch == 2) {
           thermal->cardText = QString("匹配的卡号");
       }  else if(faceRecgInfo->NumberMatch == 3) {
           thermal->cardText = QString("未知的卡号");
       }
    }
#endif
#if 0
    //身份证模式
    if(faceRecgInfo->VerificatMode == 5 || faceRecgInfo->VerificatMode == 6){
       thermal->compareResult = -1;
       thermal->showIdCardResult = 1;
       thermal->tipText = QString("请刷二代身份证卡");
//       if(faceRecgInfo->IdentityMatch == 1) {
//           faceShadeWidget->setStyleSheet("QLabel{border-image:url(./images/temperature_box.png)}");
//           faceShadeWidget->setVisible(true);
//       }
       if(faceRecgInfo->IdentityMatch == 2) {
           faceShadeWidget->setVisible(false);
           idCardWidget->setVisible(true);
           thermal->idCardFlags = 1;
       } else if(faceRecgInfo->IdentityMatch == 3) {
          pictureLable->setPixmap(QPixmap(QString(faceRecgInfo->PicPath)));
          pictureLable->show();
          return;
       } else if(faceRecgInfo->IdentityMatch == 4) {
           thermal->bShowInfo = true;
           thermal->pictureFlags = 1;
           if(thermal->snapshotThread->setName(faceRecgInfo->PicPath)) thermal->snapshotThread->start();
       } else if(faceRecgInfo->IdentityMatch == 5) {
           thermal->compareResult = 0;
       } else if(faceRecgInfo->IdentityMatch == 6) {
           thermal->compareResult = 1;
       }
       label->show();
    } else {
//        faceShadeWidget->setStyleSheet("QLabel{border-image:url(./images/face_shade.png)}");;
//        idCardWidget->setVisible(false);
//        thermal->idCardFlags = 0;
//        thermal->bShowInfo = true;
//        thermal->showIdCardResult = 0;
//         //图片
//        if(thermal->snapshotThread->setName(faceRecgInfo->PicPath)) thermal->snapshotThread->start();

        faceLable->setPixmap(QPixmap(QString(faceRecgInfo->PicPath)));
        faceLable->show();
        label->hide();
    }
#endif
//    desktopView->updateUi(800, 600, desktopView->desktopRect.width(), desktopView->desktopRect.height());

}

void DesktopView::paintStatusBarInfoScene() {
#if 0
    localIconLabel->hide();
    ipTextLabel->hide();
    wifIconLabel->hide();
    operatorTextLabel->hide();
    singnalLabel->hide();
    iconLabel->hide();
    ipTextLabel->setText(QString(tr("")));
   if(statusBarInfo->NetLinkStat == 1)
   {
       localIconLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_local.png)}"));
       ipTextLabel->setText(QString(statusBarInfo->IPAddr));
       ipTextLabel->show();
       localIconLabel->show();
   } else if (statusBarInfo->NetLinkStat == 2) {
       ipTextLabel->setText(QString(tr("IP conflict")));
       localIconLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_local_conflict.png)}"));
       ipTextLabel->show();
       localIconLabel->show();
   } else if (statusBarInfo->NetLinkStat == 3) {
       localIconLabel->setStyleSheet("QLabel{border-image:url(./images/icon_local_off.png)}");
       ipTextLabel->setText(QString(tr("Not connected to the network")));
       ipTextLabel->show();
       localIconLabel->show();
   } else if (statusBarInfo->NetLinkStat == 4) {
      if(statusBarInfo->WIFISignal == 0){
          wifIconLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("mages/icon_wifi_0.png)}"));
      } else if(statusBarInfo->WIFISignal == 1) {
          wifIconLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_wifi_1.png)}"));
      } else if (statusBarInfo->WIFISignal == 2) {
          wifIconLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_wifi_2.png)}"));
      } else if (statusBarInfo->WIFISignal == 3) {
          wifIconLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_wifi_3.png)}"));
      }
      wifIconLabel->show();
   } else if (statusBarInfo->NetLinkStat == 5) {
       if(statusBarInfo->CellularOperator == 1) {
          operatorTextLabel->setText(QString(tr("China_Unicom 4G")));
       } else if (statusBarInfo->CellularOperator == 2) {
          operatorTextLabel->setText(QString(tr("China_Mobile 4G")));
       } else if (statusBarInfo->CellularOperator == 3) {
          operatorTextLabel->setText(QString(tr("China_Telecom 4G")));
       } else if (statusBarInfo->CellularOperator == 4) {
          operatorTextLabel->setText(QString(tr("Other 4G")));
       }
       operatorTextLabel->adjustSize();
       if(statusBarInfo->CellularSignal == 0) {
          singnalLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_signal_0.png)}"));
       } else if(statusBarInfo->CellularSignal == 1) {
          singnalLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_signal_1.png)}"));
       } else if (statusBarInfo->CellularSignal == 2) {
          singnalLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_signal_2.png)}"));
       } else if (statusBarInfo->CellularSignal == 3) {
          singnalLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_signal_3.png)}"));
       }  else if (statusBarInfo->CellularSignal == 4) {
          singnalLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_signal_4.png)}"));
       }
       operatorTextLabel->show();
       singnalLabel->show();
   } else if (statusBarInfo->NetLinkStat == 6) {
       ipTextLabel->setText(QString(tr("No SIM card")));
       iconLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path + QString("images/icon_signal_no.png)}"));
       ipTextLabel->show();
       iconLabel->show();
   }

   if(statusBarInfo->BTEnableStat == 0)
   {
       blueToothTextLabel->hide();
       blueToothIconLabel->hide();
   } else if(statusBarInfo->BTEnableStat == 1){
       blueToothTextLabel->setText(QString(statusBarInfo->BTMatchDev));
       blueToothTextLabel->setAlignment(Qt::AlignRight);
       blueToothIconLabel->show();
   }

#endif
}


void DesktopView::initUi()
{
    QString Path = QString("D:\\");
    //底图
    faceShadeWidget = new QLabel(this) ;
    bgTopWidget  = new QLabel(this);
      bgbottomWidget  = new QLabel(this);
   // bgbottomWidget = new QWidget;
//    idCardWidget = new QWidget;

//    faceShadeWidget->setStyleSheet("QLabel{border-image:url(./images/face_shade.png)}");
    faceShadeWidget->setPixmap(QPixmap(Path + QString("images/face_shade.png")));
    faceShadeWidget->setGeometry( 110, 370, 580, 580);

//    faceShadeWidget->setPixmap(QPixmap(QString("./images/face_shade1.png")));
//    faceShadeWidget->setGeometry( 50, 336, 696, 488);

//    faceShadeWidget->setPixmap(QPixmap(QString("./images/face_shade2.png")));
//    faceShadeWidget->setGeometry( 110, 369, 580, 487);

    faceShadeWidget->hide();


    bgTopWidget->setPixmap(Path + QString("images/bg-top.png"));
    bgTopWidget->setGeometry( 0,  0, 800, 40);

    bgbottomWidget->setStyleSheet(QString("QWidget{border-image:url(") + Path + QString("images/bg_bottom.png)}"));
    bgbottomWidget->setGeometry( 5, 700, 780, 243);

//    idCardWidget->setStyleSheet("QWidget{border-image:url(./images/contrast_box.png)}");
//    idCardWidget->setGeometry( 27, 386, 746, 444);

    //时间轴
    timeLabel = new QLabel(this);
    dateLabel = new QLabel(this);
    timeLabel->setStyleSheet("QLabel{width: 131px;height: 62px; font-size: 52px; font-family: Helvetica Light, Helvetica;font-weight: 300; color: #FFFFFF; line-height: 62px}");
    timeLabel->setGeometry( 200, 760, 250, 62);
    dateLabel->setStyleSheet("QLabel{width: 266px; height: 45px; font-size: 32px;font-family: Helvetica Light, Helvetica; font-weight: 300;color: #FFFFFF; line-height: 38px}");
    dateLabel->setGeometry( 200, 830, 500, 45);
    QDateTime time = QDateTime::currentDateTime();
    QString timeData = time.toString("hh:mm:ss");
    timeLabel->setText(timeData);
    QString date = QString("");
    if(languageType == 0) {
        QLocale locale = QLocale::Chinese;
        date = locale.toString(time,QString("yyyy/MM/dd  dddd"));
    } else {
        date = time.toString("yyyy/MM/dd  dddd");
    }
    dateLabel->setText(date);

    //状态栏
    localIconLabel = new QLabel(this);
    iconLabel = new QLabel(this);
    blueToothIconLabel = new QLabel(this);
    blueToothTextLabel = new QLabel(this);

    ipTextLabel = new QLabel(this);
    operatorTextLabel = new QLabel(this);

    iconLabel->setGeometry(20, 5, 30, 30);
    blueToothIconLabel->setGeometry(750, 5, 31, 31);

    blueToothTextLabel->setStyleSheet("QLabel{font: 18px;font-family: Helvetica;color:rgb(255, 255, 255)}");
    blueToothTextLabel->setGeometry(440, 9, 300, 31);

    ipTextLabel->setStyleSheet("QLabel{font: 18px;font-family: Helvetica;color:rgb(255, 255, 255)}");
    ipTextLabel->setGeometry(60, 9, 368, 25);

    faceLable = new QLabel(this);
    faceLable->setGeometry(40, 1082, 136, 136);
    faceLable->setScaledContents(true);
    faceLable->hide();
    //人员信息更新
    updateLabel = new QLabel(this);
    tipLabel = new QLabel(this);
    greenLabel = new QLabel(this);
    redLabel = new QLabel(this);
    yellowLabel = new QLabel(this);
    colorLabel = new QLabel(this);

    updateLabel->setGeometry(141,580,518,92);
    updateLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path +  QString("/images/update.png);font-size: 20px;font-family: SourceHanSansCN-Normal, SourceHanSansCN;font-weight: 400;color: #FFFFFF}"));
    updateLabel->setAlignment(Qt::AlignCenter);
    updateLabel->setWordWrap(true);
    updateLabel->hide();

    tipLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path +  QString("/images/bg_green.png);font-size: 44px; font-family: Helvetica; color: #FFFFFF; line-height: 60px}"));
    tipLabel->setGeometry(0, 854, 800, 150);
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setWordWrap(true);
    tipLabel->hide();

    greenLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path +  QString("images/bg_green.png);font-size: 44px; font-family: Helvetica; color: #FFFFFF; line-height: 60px}"));
    greenLabel->setGeometry(0, 854, 800, 150);
    greenLabel->setAlignment(Qt::AlignCenter);
    greenLabel->setWordWrap(true);
    greenLabel->hide();

    redLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path +  QString("images/bg_red.png);font-size: 44px; font-family: Helvetica; color: #FFFFFF; line-height: 60px}"));
    redLabel->setGeometry(0, 854, 800, 150);
    redLabel->setAlignment(Qt::AlignCenter);
    redLabel->setWordWrap(true);
    redLabel->hide();

    yellowLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path +  QString("images/bg_yellow.png);font-size: 44px; font-family: Helvetica; color: #FFFFFF; line-height: 60px;background-color: transparent;}"));
    yellowLabel->setGeometry(0, 854, 800, 150);
    yellowLabel->setAlignment(Qt::AlignCenter);
    yellowLabel->setWordWrap(true);
    yellowLabel->hide();


    colorLabel->setGeometry(0, 854, 800, 150);
    colorLabel->hide();

    pictureLable = new QLabel(this);
    pictureLable->setGeometry(501, 477, 232, 285);
    pictureLable->setScaledContents(true);
    pictureLable->hide();

    //升级
    waterProcess1 = new WaterProcess(this);
    waterProcess1->hide();
    updatefailure = new QLabel(this);
    //gif图
    label = new QLabel(this);
    movie = new QMovie(Path +QString("images/face.gif"));
    label->setGeometry(30, 750, 136, 136);
    label->setMovie(movie); // 1. 设置要显示的 GIF 动画图片
    label->show();
    movie->start(); // 2. 启动动画

    movie->setSpeed(25);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(0);

    registerBtn = new QPushButton(tr("Register"));
//    setButtonFormat(hLayout, registerBtn, desktopRect);
    switchBtn = new QPushButton(tr("RGB"));
    setButtonFormat(hLayout, switchBtn, desktopRect);
#ifdef ONE_PLANE
    saveBtn = new QPushButton(tr("Capture"));
    setButtonFormat(hLayout, saveBtn, desktopRect);
#endif
    deleteBtn = new QPushButton(tr("Delete"));
//    setButtonFormat(hLayout, deleteBtn, desktopRect);

    menuWidget = new QWidget();
    menuWidget->setLayout(hLayout);
    //menuWidget->setObjectName("menuWidget");
    //menuWidget->setStyleSheet("#menuWidget {background-color:rgba(10, 10, 10, 100);}");
    menuWidget->setWindowOpacity(0.8);
    menuWidget->setGeometry(0, 0, desktopRect.width(), desktopRect.width()/10);

    int btnSize = desktopRect.width()/15;
    QIcon setIcon;
    setIcon.addFile(":/images/icon_set.png");
    setBtn.setIcon(setIcon);
    setBtn.setIconSize(QSize(btnSize, btnSize));

    QHBoxLayout *setLayout = new QHBoxLayout;
    setLayout->setMargin(0);
    setLayout->setSpacing(0);
    setLayout->addWidget(&setBtn);
    setWidget = new QWidget;
    setWidget->setLayout(setLayout);
    setWidget->setGeometry(desktopRect.width() - btnSize * 2,
        desktopRect.height()*4/5 - btnSize * 2, btnSize, btnSize);

    initEditUi();
}

void DesktopView::initEditUi()
{
    QIcon setIcon;
    setIcon.addFile(":/images/icon_close.png");
    int btnSize = desktopRect.width()/15;
    closeBtn.setIcon(setIcon);
    closeBtn.setIconSize(QSize(btnSize, btnSize));
    closeBtn.setFixedSize(btnSize, btnSize);

    int editSize = desktopRect.width()/10;
    ipEdit = new QLineEdit();
    ipEdit->setFixedHeight(editSize);
    ipEdit->setPlaceholderText(tr("Please enter IP"));
    ipEdit->installEventFilter(this);

    netmaskEdit = new QLineEdit();
    netmaskEdit->setFixedHeight(editSize);
    netmaskEdit->setPlaceholderText(tr("Please enter Netmask"));
    netmaskEdit->installEventFilter(this);

    gatewayEdit = new QLineEdit();
    gatewayEdit->setFixedHeight(editSize);
    gatewayEdit->setPlaceholderText(tr("Please enter Gateway"));
    gatewayEdit->installEventFilter(this);

    int editWidgetWidth = (desktopRect.width() - desktopRect.width()/7);
    editSetBtn.setText(tr("Setting"));
    editSetBtn.setFixedSize(editWidgetWidth/3, editSize);

    QVBoxLayout *editLayout = new QVBoxLayout;
    editLayout->setContentsMargins(20, 0, 20, 0);
    editLayout->addWidget(&closeBtn, 0, Qt::AlignRight);
    editLayout->addWidget(ipEdit);
    editLayout->addWidget(netmaskEdit);
    editLayout->addWidget(gatewayEdit);
    editLayout->addWidget(&editSetBtn, 0, Qt::AlignHCenter);

    editWidget = new QWidget;
    editWidget->setLayout(editLayout);
    editWidget->setGeometry(desktopRect.width()/14, desktopRect.height()/6, editWidgetWidth, desktopRect.height()/3);
}

void DesktopView::iniSignalSlots()
{
    connect(switchBtn, SIGNAL(clicked()), this, SLOT(cameraSwitch()));
    connect(registerBtn, SIGNAL(clicked()), this, SLOT(registerSlots()));
    connect(deleteBtn, SIGNAL(clicked()), this, SLOT(deleteSlots()));
    #ifdef ONE_PLANE
    connect(saveBtn, SIGNAL(clicked()), this, SLOT(saveSlots()));
    #endif
    connect(this, SIGNAL(itemDirty(int, int, int, int)), this, SLOT(updateScene(int, int, int, int)));
    connect(&setBtn, SIGNAL(clicked()), this, SLOT(setSlots()));
    connect(&closeBtn, SIGNAL(clicked()), this, SLOT(closeSlots()));
    connect(&editSetBtn, SIGNAL(clicked()), this, SLOT(editSetSlots()));

    connect(this, SIGNAL(updateTime()), this, SLOT(updateTimeScene()));
//    connect(this, SIGNAL(clearFaceRecgInfoSignals()), this, SLOT(clearFaceRecgInfoScene()));
    connect(this, SIGNAL(updateFaceRecgInfoSignals()), this, SLOT(paintFaceRecgInfoScene()));
    connect(this, SIGNAL(updateStatusBarInfoSignals()), this, SLOT(paintStatusBarInfoScene()));
    connect(this, SIGNAL(updateTranslatorSignals()), this, SLOT(updateTranslatorScene()));
    connect(this, SIGNAL(updateStatusSignals()), this, SLOT(updateStatusScene()));
}

//static bool coordIsVaild(int left, int top, int right, int bottom)
//{
//    if(left < 0 || top < 0 || right < 0 || bottom < 0) {
//        qDebug("%s: invalid rect(%d, %d, %d, %d)", __func__, left, top, right, bottom);
//        return false;
//    }

//    if(left > right || top > bottom) {
//        qDebug("%s: invalid rect(%d, %d, %d, %d)", __func__, left, top, right, bottom);
//        return false;
//    }

//    return true;
//}

//void DesktopView::paintBox(int left, int top, int right, int bottom)
//{
//    bool ret;
//    int mod = 0;
//    static int refreshCount = 0;

//    if(desktopView->cameraType == CIF) {
//#ifdef TWO_PLANE
// //      display_paint_box(0, 0, 0, 0);
//#endif
//        return;
//    }

//    if(desktopView->refreshFrame) {
//        mod = refreshCount % desktopView->refreshFrame;

//        refreshCount++;
//        if(refreshCount == 65535)
//            refreshCount = 0;

//        if(mod)
//            return;
//    }

//    if(!coordIsVaild(left, top, right, bottom))
//        return;

//    if(!left && !top && !right && !bottom) {
//        ret = desktopView->videoItem->setBoxRect(0, 0, -1, -1);
//        goto update_paint;
//    }

//    ret = desktopView->videoItem->setBoxRect(left, top, right, bottom);

//update_paint:
//#ifdef TWO_PLANE
//    if(ret) {
//        display_paint_box(left, top, right, bottom);
//        switch(desktopView->videoItem->facial.state) {
//            case USER_STATE_REAL_UNREGISTERED:
//                display_set_color(set_yuv_color(COLOR_B));
//                break;
//            case USER_STATE_REAL_REGISTERED_WHITE:
//                display_set_color(set_yuv_color(COLOR_G));
//                break;
//            case USER_STATE_REAL_REGISTERED_BLACK:
//                display_set_color(set_yuv_color(COLOR_BK));
//                break;
//            default:
//                display_set_color(set_yuv_color(COLOR_R));
//                break;
//        }
//    }
//#endif
//    return;
//}

//void DesktopView::paintInfo(struct user_info *info, bool real)
//{
//    if(desktopView->cameraType == CIF)
//        return;

//    desktopView->videoItem->setUserInfo(info, real);
//    desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
//}

//void DesktopView::paintFace(void *ptr, int fmt, int width, int height, int x, int y, int w, int h)
//{
//    if(desktopView->updateFace) {
//        desktopView->videoItem->setFaceInfo(ptr, fmt, width, height, x, y, w, h);
//        desktopView->updateFace = false;
//        desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
//    }
//}

//void DesktopView::configRegion(int x, int y, int w, int h)
//{
//    desktopView->videoItem->setRegion(x, y, w, h);
//    desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
//}

//void DesktopView::saveFile(uchar *buf, int len, uchar *flag)
//{
//    if(!saving)
//        return;

//    if(saveFrames) {
//        SaveThread *thread = new SaveThread(buf, len, flag, saveFrames);
//        thread->start();
//        saveFrames--;
//    } else {
//        saveFrames = SAVE_FRAMES;
//        saving = false;
//        saveBtn->setEnabled(true);
//        switchBtn->setEnabled(true);
//    }
//}

//void DesktopView::displayRgb(void *src_ptr, int src_fd, int src_fmt, int src_w, int src_h, int rotation)
//{
//    if(desktopView->cameraType != ISP)
//        return;

//    if (src_fmt != RK_FORMAT_YCbCr_420_SP) {
//        qDebug("%s: src_fmt = %d", __func__, src_fmt);
//        return;
//    }

//    desktopView->saveFile((uchar *)src_ptr, src_w * src_h * 3 / 2, "rgb");

//    //qDebug("%s, tid(%lu)\n", __func__, pthread_self());
//    desktopView->videoItem->render((uchar *)src_ptr, src_fmt, rotation,
//                        src_w, src_h);
//    desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
//}

//void DesktopView::displayIr(void *src_ptr, int src_fd, int src_fmt, int src_w, int src_h, int rotation)
//{
//    if(desktopView->cameraType != CIF)
//        return;

//    if (src_fmt != RK_FORMAT_YCbCr_420_SP) {
//        qDebug("%s: src_fmt = %d", __func__, src_fmt);
//        return;
//    }

//    desktopView->saveFile((uchar *)src_ptr, src_w * src_h * 3 / 2, "ir");

//    desktopView->videoItem->render((uchar *)src_ptr, src_fmt, rotation,
//                        src_w, src_h);
//    desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
//}



//static int DesktopView::initRkfacial(int faceCnt)
//{
//#ifdef TWO_PLANE
//    set_rgb_param(CAMERA_WIDTH, CAMERA_HEIGHT, NULL, true);
//    set_ir_param(CAMERA_WIDTH, CAMERA_HEIGHT, NULL);
//    set_rgb_rotation(90);
//    //    set_ir_rotation(90);
//    display_switch(DISPLAY_VIDEO_RGB);
//    if (display_init(desktopRect.width(), desktopRect.height())) {
//        qDebug("%s: display_init failed", __func__);
//        return -1;
//    }
//#else
//    set_rgb_param(CAMERA_WIDTH, CAMERA_HEIGHT, displayRgb, true);
//    set_ir_param(CAMERA_WIDTH, CAMERA_HEIGHT, displayIr);
//#endif

//    set_face_param(CAMERA_WIDTH, CAMERA_HEIGHT, faceCnt);

////    register_rkfacial_paint_box(paintBox);
////    register_rkfacial_paint_info(paintInfo);
////    register_rkfacial_paint_face(paintFace);
////    register_get_face_config_region(configRegion);

//    //升级
//    upgrade_schedule_info(upgradeScheduleInfo);
//    upgrade_begin_info(upgradeBeginInfo);
//    upgrade_fail_info(upgradeFailInfo);


//    time_format_info(timeFormatInfo);
//    system_language_info(systemLanguageInfo);
//    dev_display_info(devDisplayInfo);

//    register_facerecg_info(paintFaceRecgInfo);
//    register_statusbar_info(paintStatusBarInfo);
//    register_facerecg_info_clear(paintFaceRecgInfoClear);

//    if(rkfacial_init() < 0) {
//        qDebug("%s: rkfacial_init failed", __func__);
//        return -1;
//    }
//    return 0;
//}

//void DesktopView::deinitRkfacial()
//{
//    rkfacial_exit();

//#ifdef TWO_PLANE
//    display_exit();
//#endif
//}

void DesktopView::paintFaceRecgInfo(void *arg)
{
//        desktopView->faceRecgInfo = (LS_faceRecg_t*)arg;
//        if(desktopView->verificatStatus != QT_MAIN_MASK_STATUS)
//        {
//            qDebug("%s %d==================================start\n", __FILE__, __LINE__);
//            printf("Stranger=%d\n",             desktopView->faceRecgInfo->FaceID);                   // 1: 陌生人(黑名单)          其它:认证人员(白名单)
//            printf("NumberMatch=%d\n",          desktopView->faceRecgInfo->NumberMatch);              // 1:请刷卡 2.匹配的卡号 3.未知的卡号
//            printf("Username=%s\n",             desktopView->faceRecgInfo->Username);                 // 名字
//            printf("MeasuringResult=%d\n",      desktopView->faceRecgInfo->MeasuringResult);          // 0(温度正常),     1(温度偏低),     2(温度异常)
//            printf("MaskStatus=%d\n",           desktopView->faceRecgInfo->MaskStatus);               // 1: 佩戴口罩        其它: 未戴口罩
//            printf("Thermodynamic=%d\n",        desktopView->faceRecgInfo->Thermodynamic);            // 1: 有热力图数据      其它: 无数据
//            printf("TextType=%d\n",             desktopView->faceRecgInfo->TextType);                 // (QT界面端给出支持的语言提示序号)
//            printf("Temperature=%.2f\n",        desktopView->faceRecgInfo->Temperature);              //温度
//            qDebug("PicPath= %s\n",             desktopView->faceRecgInfo->PicPath);
//            printf("TipBoxType=%d" ,            desktopView->faceRecgInfo->TipBoxType);
//            printf("PicType=%d" ,               desktopView->faceRecgInfo->PicType);
//            printf("VerificatMode=%d\n",        desktopView->faceRecgInfo->VerificatMode);
//            printf("TempPosition=%d\n",         desktopView->faceRecgInfo->TempPosition);
//            printf("PicPosition=%d\n",          desktopView->faceRecgInfo->PicPosition);
//            qDebug("%s %d==================================end\n", __FILE__, __LINE__);
//        }

//    queue_cycle_elem_data msg = {0};
//    memcpy(msg.data, (LS_faceRecg_t*)arg, sizeof(LS_faceRecg_t));
//    pthread_mutex_lock(&g_paintFaceRecgInfo_mutex);
//    queueCycleIn(&g_paintFaceRecgInfo_queue, msg);
//    pthread_mutex_unlock(&g_paintFaceRecgInfo_mutex);
}

#if 0
void DesktopView::paintFaceRecgInfo_Thread(void *arg)
{
    int ret;
    queue_cycle_elem_data msg = {0};
    LS_faceRecg_t LsFaceRecgInfo;
    QString Text = QString("");
    while(1)
    {
        memset(&msg, 0, sizeof(msg));
        memset(&LsFaceRecgInfo,  0, sizeof(LS_faceRecg_t));

        pthread_mutex_lock(&g_paintFaceRecgInfo_mutex);
        ret = queueCycleOut(&g_paintFaceRecgInfo_queue, &msg);
        pthread_mutex_unlock(&g_paintFaceRecgInfo_mutex);
        if(ret == 0)
        {
            //usleep(100*1000);
            continue;
        }

        if(desktopView->showNetType == 9) desktopView->showNetType = 1;
        LsFaceRecgInfo.VerificatMode = desktopView->showNetType;
        LsFaceRecgInfo.PicType = 3;
        desktopView->showNetType++;

        memcpy(&LsFaceRecgInfo, msg.data, sizeof(LS_faceRecg_t));
        if(desktopView->verificatStatus != QT_MAIN_MASK_STATUS)
        {
            qDebug("%s %d==================================start\n", __FILE__, __LINE__);
            printf("Stranger=%d\n",             LsFaceRecgInfo.FaceID);                   // 1: 陌生人(黑名单)          其它:认证人员(白名单)
            printf("NumberMatch=%d\n",          LsFaceRecgInfo.NumberMatch);              // 1:请刷卡 2.匹配的卡号 3.未知的卡号
            printf("Username=%s\n",             LsFaceRecgInfo.Username);                 // 名字
            printf("MeasuringResult=%d\n",      LsFaceRecgInfo.MeasuringResult);          // 0(温度正常),     1(温度偏低),     2(温度异常)
            printf("MaskStatus=%d\n",           LsFaceRecgInfo.MaskStatus);               // 1: 佩戴口罩        其它: 未戴口罩
            printf("Thermodynamic=%d\n",        LsFaceRecgInfo.Thermodynamic);            // 1: 有热力图数据      其它: 无数据
            printf("TextType=%d\n",             LsFaceRecgInfo.TextType);                 // (QT界面端给出支持的语言提示序号)
            printf("Temperature=%.2f\n",        LsFaceRecgInfo.Temperature);              //温度
            qDebug("PicPath= %s\n", LsFaceRecgInfo.PicPath);
            printf("TipBoxType=%d" ,            LsFaceRecgInfo.TipBoxType);
            printf("PicType=%d" ,               LsFaceRecgInfo.PicType);
            printf("VerificatMode=%d\n",        LsFaceRecgInfo.VerificatMode);
            printf("TempPosition=%d\n",         LsFaceRecgInfo.TempPosition);
            printf("PicPosition=%d\n",          LsFaceRecgInfo.PicPosition);
            printf("FacePosition=%d\n",         LsFaceRecgInfo.FacePosition);
            printf("temp_display_type=%d\n",    LsFaceRecgInfo.temp_display_type);
            printf("VerificatResult=%d\n",      LsFaceRecgInfo.VerificatResult);
            qDebug("%s %d==================================end\n", __FILE__, __LINE__);
        }





        switch(LsFaceRecgInfo.VerificatMode)
        {
            case QT_MAIN_MEASURETEMP_BOX:   //画出人脸识别框
            {
                if(desktopView->verificatStatus != -1 )
                    break;
                desktopView->faceShadeWidget->show();
                desktopView->tipBoxType = -1;
                desktopView->verificatStatus = QT_MAIN_MEASURETEMP_BOX;

            }
            case QT_MAIN_DIS_FACERECG_INFO:    //1. 陌生人/认证人员  , 2. 图片路径, 3.人员名字
            {
                if(!strlen(LsFaceRecgInfo.PicPath))
                {
//                    qDebug("============================================error !LsFaceRecgInfo.PicPath not found \n", LsFaceRecgInfo.PicPath);
                    break;
                }
                if(LsFaceRecgInfo.PicType == QT_SUB_IDENTITYCARD_USE)
                {
                    desktopView->pictureLable->clear();
                    desktopView->pictureLable->setPixmap(QPixmap(QString(LsFaceRecgInfo.PicPath)));
                    desktopView->pictureLable->show();
                    desktopView->yellowLabel->setText(QString(" "));
                    desktopView->updateUi(27, 380, 746, 444);
//                    desktopView->updateUi(339, 561, 121, 121);
                }
                else if(LsFaceRecgInfo.PicType == QT_SUB_OTHER_MODE_USE)
                {
                    desktopView->faceRecgInfoItem->bShowInfo = true;
//                    if(desktopView->faceRecgInfoItem->snapshotThread->setName(LsFaceRecgInfo.PicPath))
//                    {
//                        if(!desktopView->faceRecgInfoItem->snapshotThread->isRunning())
//                              desktopView->faceRecgInfoItem->snapshotThread->start();
//                    }
                    if(LsFaceRecgInfo.PicPosition == QT_SUB_OTHER_MODE_USE)
                    {
                        desktopView->faceRecgInfoItem->pictureFlags = QT_SUB_OTHER_MODE_USE;
                        desktopView->updateUi(40, 1082, 136, 136);
                        desktopView->label->hide();
                    }
                    else
                    {
                        desktopView->faceRecgInfoItem->pictureFlags = QT_SUB_IDENTITYCARD_USE;
                        desktopView->faceShadeWidget->hide();
                        desktopView->updateUi(27, 380, 746, 444);
                        desktopView->updateUi(40, 1082, 136, 136);
                    }
                }
                desktopView->verificatStatus = QT_MAIN_DIS_FACERECG_INFO;
                break;
            }
            case QT_MAIN_MASK_STATUS:  //是否戴口罩
            {
                if(LsFaceRecgInfo.FaceID == -1)
                    Text = QString(tr("Stranger"));
                else
                    Text = QString::fromUtf8(LsFaceRecgInfo.Username);
                if(LsFaceRecgInfo.MaskStatus == QT_SUB_NOMASK)
                {
                    desktopView->yellowLabel->hide();
                    desktopView->greenLabel->hide();
                    desktopView->redLabel->setText(Text + QString(" , ") + QString(tr("No Mask")));
                    desktopView->redLabel->show();
                }
                else if(LsFaceRecgInfo.MaskStatus == QT_SUB_HAVEMASK)
                {
                   if(desktopView->verificatStatus == QT_MAIN_VERIFICAT_RESULT || desktopView->verificatStatus == QT_MAIN_SWIPE_IDENTITYCARD_RESULT || desktopView->verificatStatus == QT_MAIN_SWIPE_IDCARD_RESULT)
                        break;
                    desktopView->redLabel->hide();
                }
                desktopView->verificatStatus = QT_MAIN_MASK_STATUS;
                break;
            }
            case QT_MAIN_MEASURETEMP_INFO:    //温度信息, 温度
            {
                if(LsFaceRecgInfo.Temperature > 0)
                    desktopView->faceRecgInfoItem->showTemperature = true;
                else
                    desktopView->faceRecgInfoItem->showTemperature = false;

                desktopView->faceRecgInfoItem->iMeasuringResult = LsFaceRecgInfo.MeasuringResult;
                desktopView->faceRecgInfoItem->idCardFlags = LsFaceRecgInfo.TempPosition;

                if(LsFaceRecgInfo.FaceID == -1)
                    Text = QString(tr("Stranger"));
                else
                    Text = QString::fromUtf8(LsFaceRecgInfo.Username);
                if(LsFaceRecgInfo.MeasuringResult == QT_SUB_TEMP_NORMAL )
                {
                    if(LsFaceRecgInfo.temp_display_type == QT_CENTIGRADE)
                    {
                        desktopView->faceRecgInfoItem->tempuratureText = QString::number(LsFaceRecgInfo.Temperature, 'f', 2) + QString("℃");
                    }
                    else if(LsFaceRecgInfo.temp_display_type == QT_FAHRENHEIT)
                    {
                        desktopView->faceRecgInfoItem->tempuratureText = QString::number(((LsFaceRecgInfo.Temperature*9)/5+32.0), 'f', 2) + QString("℉");
                    }
                    else if(LsFaceRecgInfo.temp_display_type == QT_KELVIN)
                    {
                        desktopView->faceRecgInfoItem->tempuratureText = QString::number((LsFaceRecgInfo.Temperature + 273.15), 'f', 2) + QString("K");
                    }
                    desktopView->redLabel->hide();
                }
                else if(LsFaceRecgInfo.MeasuringResult == QT_SUB_TEMP_LOW)
                {
                    desktopView->yellowLabel->setText(Text + QString(" , ") + QString(tr("low Temperature")));
                    desktopView->faceRecgInfoItem->tempuratureText = QString(tr("Low Temp."));
                    desktopView->yellowLabel->show();
                    desktopView->redLabel->hide();
                    desktopView->greenLabel->hide();
                }
                else if(LsFaceRecgInfo.MeasuringResult == QT_SUB_TEMP_HIGH)
                {
                    if(LsFaceRecgInfo.temp_display_type == QT_CENTIGRADE)
                    {
                        desktopView->faceRecgInfoItem->tempuratureText = QString::number(LsFaceRecgInfo.Temperature, 'f', 2) + QString("℃");
                    }
                    else if(LsFaceRecgInfo.temp_display_type == QT_FAHRENHEIT)
                    {
                        desktopView->faceRecgInfoItem->tempuratureText = QString::number(((LsFaceRecgInfo.Temperature*9)/5+32.0), 'f', 2) + QString("℉");
                    }
                    else if(LsFaceRecgInfo.temp_display_type == QT_KELVIN)
                    {
                        desktopView->faceRecgInfoItem->tempuratureText = QString::number((LsFaceRecgInfo.Temperature + 273.15), 'f', 2) + QString("K");
                    }
                    desktopView->redLabel->setText(Text + QString(" , ") + QString(tr("Abnormal temperature")));
                    desktopView->redLabel->show();
                    desktopView->yellowLabel->hide();
                    desktopView->greenLabel->hide();
                }

                else if(LsFaceRecgInfo.MeasuringResult == QT_SUB_TEMP_SUPER_HIGH)
                {
                    desktopView->redLabel->setText(Text + QString(" , ") + QString(tr("Abnormal temperature")));
                    desktopView->faceRecgInfoItem->tempuratureText = QString(tr("High Temp."));
                    desktopView->redLabel->show();
                    desktopView->yellowLabel->hide();
                    desktopView->greenLabel->hide();
                }
                if(LsFaceRecgInfo.TempPosition == QT_SUB_IDENTITYCARD_USE)
                {
                    desktopView->updateUi(68, 422, 263, 50);
                    desktopView->updateUi(339, 561, 121, 121);
                    desktopView->updateUi(27, 380, 746, 444);
                }
                else
                {
                    desktopView->updateUi(343, 1093, 360, 60);
                }
                desktopView->verificatStatus = QT_MAIN_MEASURETEMP_INFO;
                break;
            }
            case QT_MAIN_THERMODYNAMIC_INFO:    //热力图
            {
                desktopView->faceRecgInfoItem->bDrawThermal = LsFaceRecgInfo.Thermodynamic;
                if(LsFaceRecgInfo.Thermodynamic == 1)
                {
                    desktopView->faceRecgInfoItem->paint_thermal_image(LsFaceRecgInfo.TempArray,LsFaceRecgInfo.Thermodynamic);
                    desktopView->updateUi(600, 1070, 160, 160);
                }
                desktopView->verificatStatus = QT_MAIN_THERMODYNAMIC_INFO;
                break;
            }
            case QT_MAIN_VERIFICAT_RESULT:    //通行结果, 请通行, 禁止通行
            {
                if(LsFaceRecgInfo.FaceID == -1)
                    Text = QString(tr("Stranger"));
                else
                    Text = QString::fromUtf8(LsFaceRecgInfo.Username);

                if(LsFaceRecgInfo.VerificatResult == QT_SUB_VERIFICAT_MATCH)
                {
                    desktopView->greenLabel->setText(Text + QString(" , ") + QString(tr("Please Passing")));
                    desktopView->greenLabel->show();
                    desktopView->redLabel->hide();
                    desktopView->yellowLabel->hide();
                }
                else if(LsFaceRecgInfo.VerificatResult == QT_SUB_VERIFICAT_UNMATCH)
                {

                    desktopView->redLabel->setText(Text + QString(" , ") + QString(tr("No Permission")));
                    desktopView->redLabel->show();
                    desktopView->greenLabel->hide();
                    desktopView->yellowLabel->hide();
                }
                desktopView->verificatStatus = QT_MAIN_VERIFICAT_RESULT;
                break;
            }
            case QT_MAIN_TIP_SWIPE_IDCARD:    //提示请刷IC卡
            {
                if(desktopView->verificatStatus == QT_MAIN_TIP_SWIPE_IDCARD)
                    break;
                if(LsFaceRecgInfo.FaceID == -1)
                {
                     Text = QString("");
                }
                else
                {
                     Text = QString::fromUtf8(LsFaceRecgInfo.Username) + QString("  , ");
                }
                desktopView->greenLabel->hide();
                desktopView->redLabel->hide();
                desktopView->yellowLabel->show();
                desktopView->yellowLabel->setText(Text + QString(tr("Please swipe your IC card")));
                desktopView->verificatStatus = QT_MAIN_TIP_SWIPE_IDCARD;
                break;
            }
            case QT_MAIN_SWIPE_IDCARD_RESULT:    //IC卡比对结果
            {
                continue;
                desktopView->yellowLabel->hide();
                if(LsFaceRecgInfo.NumberMatch == QT_SUB_IDCARD_MATCH)
                {
                    desktopView->redLabel->hide();
                    desktopView->greenLabel->show();
                    desktopView->greenLabel->setText(QString(tr("IC Compare Success")));
                }
                else if(LsFaceRecgInfo.NumberMatch == QT_SUB_IDCARD_UNMATCH)
                {
                    desktopView->greenLabel->hide();
                    desktopView->redLabel->show();
                    desktopView->redLabel->setText(QString(tr("IC Compare Failed")));
                }
                desktopView->verificatStatus = QT_MAIN_SWIPE_IDCARD_RESULT;
                break;
            }
            case QT_MAIN_TIP_SWIPE_IDENTITYCARD:    //提示请刷身份证
            {
               if(LsFaceRecgInfo.TipBoxType == QT_SUB_TIP_BOX)
               {
//                   if(desktopView->tipBoxType == QT_SUB_TIP_BOX)
//                       break;
                   desktopView->greenLabel->hide();
                   desktopView->redLabel->hide();
                   desktopView->yellowLabel->show();
                   desktopView->yellowLabel->setText(QString(tr("Please swipe your ID card")));
               }
               else if (LsFaceRecgInfo.TipBoxType == QT_SUB_COMPARE_BOX)
               {

                    desktopView->faceShadeWidget->hide();
                    desktopView->faceRecgInfoItem->showIdCardResult = 1;
                    desktopView->faceRecgInfoItem->pictureidCard = -1;
                    desktopView->updateUi(27, 380, 746, 444);
               }
                desktopView->verificatStatus = QT_MAIN_TIP_SWIPE_IDENTITYCARD;
                break;
            }
            case QT_MAIN_SWIPE_IDENTITYCARD_RESULT:    //身份证比对通过
            {

                desktopView->yellowLabel->hide();
                if(LsFaceRecgInfo.IdentityMatch == QT_SUB_IDENTITYCARD_MATCH)
                {
                    desktopView->faceRecgInfoItem->pictureidCard = QT_SUB_IDENTITYCARD_MATCH;
                }
                else if(LsFaceRecgInfo.IdentityMatch == QT_SUB_IDENTITYCARD_UNMATCH)
                {
                    desktopView->faceRecgInfoItem->pictureidCard = QT_SUB_IDENTITYCARD_UNMATCH;
                }
                else
                {
                    desktopView->faceRecgInfoItem->pictureidCard = -1;
                }
                 desktopView->updateUi(27, 380, 746, 444);
                desktopView->verificatStatus = QT_MAIN_SWIPE_IDENTITYCARD_RESULT;
                break;
            }

            case QT_MAIN_FACE_POSITION:
            {
                if(desktopView->verificatStatus != QT_MAIN_FACE_POSITION)
                {
                    desktopView->facePosition = -1;
                }

                desktopView->greenLabel->hide();
                desktopView->redLabel->hide();
                if(LsFaceRecgInfo.FacePosition == QT_SUB_FACE_POSE_OK)
                {
                   if(desktopView->facePosition == QT_SUB_FACE_POSE_OK)
                       break;
                   desktopView->yellowLabel->setText(QString(tr("Please hold")));
                   desktopView->facePosition = QT_SUB_FACE_POSE_OK;
                }
                else if(LsFaceRecgInfo.FacePosition == QT_SUB_FACE_MOVE_LEFT)
                {
                    if(desktopView->facePosition == QT_SUB_FACE_MOVE_LEFT)
                        break;
                    desktopView->yellowLabel->setText(QString(tr("Please move left")));
                    desktopView->facePosition = QT_SUB_FACE_MOVE_LEFT;
                }
                else if(LsFaceRecgInfo.FacePosition == QT_SUB_FACE_MOVE_RIGHT)
                {
                    if(desktopView->facePosition == QT_SUB_FACE_MOVE_RIGHT)
                        break;
                    desktopView->yellowLabel->setText(QString(tr("Please move right")));
                    desktopView->facePosition = QT_SUB_FACE_MOVE_RIGHT;
                }
                else if(LsFaceRecgInfo.FacePosition == QT_SUB_FACE_MOVE_DOWN)
                {
                    if(desktopView->facePosition == QT_SUB_FACE_MOVE_DOWN)
                        break;
                    desktopView->yellowLabel->setText(QString(tr("Please move down")));
                    desktopView->facePosition = QT_SUB_FACE_MOVE_DOWN;
                }
                else if(LsFaceRecgInfo.FacePosition == QT_SUB_FACE_MOVE_UP)
                {
                    if(desktopView->facePosition == QT_SUB_FACE_MOVE_UP)
                        break;
                    desktopView->yellowLabel->setText(QString(tr("Please move up")));
                    desktopView->facePosition = QT_SUB_FACE_MOVE_UP;
                }
                else if(LsFaceRecgInfo.FacePosition == QT_SUB_FACE_MOVE_ALONG)
                {
                    if(desktopView->facePosition == QT_SUB_FACE_MOVE_ALONG)
                        break;
                    desktopView->yellowLabel->setText(QString(tr("Please move along")));
                    desktopView->facePosition = QT_SUB_FACE_MOVE_ALONG;
                }
                desktopView->yellowLabel->show();
                break;

            }
            default:
                break;
        }
    }
}
#endif
void DesktopView::paintFaceRecgInfoClear()
{
    desktopView->updateClearFaceRecgInfo();

}

void DesktopView::paintStatusBarInfo(void *arg)
{
//        desktopView->statusBarInfo = (StatusBar_t*)arg;
#if 0
    qDebug("%s %d==================================start\n", __FILE__, __LINE__);
    qDebug("NetLinkStat    =%d\n", desktopView->statusBarInfo->NetLinkStat);      //1(有线网络正常), 2(有线网络IP冲突), 3(有线网络未接入网线), 4(无线网络正常), 5(蜂窝网络正常), 6(未插入SIM卡)
    qDebug("IPAddr        =%s\n", desktopView->statusBarInfo->IPAddr);        //ip地址
    qDebug("CellularOperator  =%d\n", desktopView->statusBarInfo->CellularOperator);    //NetLinkStat=5时 取用此值 (联通)China_Unicom: 1,  (移动)China_Mobile: 2,  (电信)China_Telecom: 3,  (广电)Other: 4
    qDebug("CellularSignal    =%d\n", desktopView->statusBarInfo->CellularSignal);    //NetLinkStat=5时 取用此值 蜂窝信号量
    qDebug("WIFISignal      =%d\n", desktopView->statusBarInfo->WIFISignal);      //NetLinkStat=4时 取用此值 无线信号量
    qDebug("BTEnableStat    =%d\n", desktopView->statusBarInfo->BTEnableStat);      //蓝牙开关状态
    qDebug("BTMatchDev      =%s\n", desktopView->statusBarInfo->BTMatchDev);      //蓝牙已经配对的设备
    qDebug("DevName      =%s\n", desktopView->statusBarInfo->DevName);        //设备名称
    qDebug("DevSN        =%s\n", desktopView->statusBarInfo->DevSN);        //设备序列号
    qDebug("%s %d==================================end\n", __FILE__, __LINE__);
//    desktopView->updateStatusBarInfo();
#endif

//    queue_cycle_elem_data StatusBarInfomsg = {0};
//    memcpy(StatusBarInfomsg.data, (StatusBar_t*)arg, sizeof(StatusBar_t));
//    pthread_mutex_lock(&g_paintStatusBarInfo_mutex);
//    queueCycleIn(&g_paintStatusBarInfo_queue, StatusBarInfomsg);
//    pthread_mutex_unlock(&g_paintStatusBarInfo_mutex);
}

void DesktopView::paintStatusBarInfo_Thread(void *arg)
{
    int ret;
    queue_cycle_elem_data StatusBarInfomsg = {0};
    StatusBar_t LsStatusBarInfo;
    QString Text = QString("");
    QString Path = QString("D:\\");

    Sleep(100);
    while(1)
    {
        memset(&StatusBarInfomsg, 0, sizeof(StatusBarInfomsg));
        memset(&LsStatusBarInfo,  0, sizeof(StatusBar_t));

        pthread_mutex_lock(&g_paintStatusBarInfo_mutex);
        ret = queueCycleOut(&g_paintStatusBarInfo_queue, &StatusBarInfomsg);
        pthread_mutex_unlock(&g_paintStatusBarInfo_mutex);
        //        if(ret == 0)
        //        {
        //


        //            continue;
        //        }
        // 避免界面冻结
        QCoreApplication::processEvents();
        memcpy(&LsStatusBarInfo, StatusBarInfomsg.data, sizeof(StatusBar_t));

#if 0
        qDebug("%s %d==================================start\n", __FILE__, __LINE__);
        qDebug("NetLinkStat    =%d\n", LsStatusBarInfo.NetLinkStat);      //1(有线网络正常), 2(有线网络IP冲突), 3(有线网络未接入网线), 4(无线网络正常), 5(蜂窝网络正常), 6(未插入SIM卡)
        qDebug("IPAddr        =%s\n", LsStatusBarInfo.IPAddr);        //ip地址
        qDebug("CellularOperator  =%d\n", LsStatusBarInfo.CellularOperator);    //NetLinkStat=5时 取用此值 (联通)China_Unicom: 1,  (移动)China_Mobile: 2,  (电信)China_Telecom: 3,  (广电)Other: 4
        qDebug("CellularSignal    =%d\n", LsStatusBarInfo.CellularSignal);    //NetLinkStat=5时 取用此值 蜂窝信号量
        qDebug("WIFISignal      =%d\n", LsStatusBarInfo.WIFISignal);      //NetLinkStat=4时 取用此值 无线信号量
        qDebug("BTEnableStat    =%d\n", LsStatusBarInfo.BTEnableStat);      //蓝牙开关状态
        qDebug("BTMatchDev      =%s\n", LsStatusBarInfo.BTMatchDev);      //蓝牙已经配对的设备
        qDebug("DevName      =%s\n", LsStatusBarInfo.DevName);        //设备名称
        qDebug("DevSN        =%s\n", LsStatusBarInfo.DevSN);        //设备序列号
        qDebug("%s %d==================================end\n", __FILE__, __LINE__);
#endif

        if(desktopView->showNetType == 7) desktopView->showNetType = 1;
        LsStatusBarInfo.NetLinkStat = desktopView->showNetType;
        LsStatusBarInfo.WIFISignal = 3;
        LsStatusBarInfo.CellularOperator = 2;
        LsStatusBarInfo.CellularSignal = 3;
        LsStatusBarInfo.BTEnableStat = 1;
        desktopView->showNetType++;
        //usleep(100*1000);

//        if(desktopView->showNetType == 1){

//            switch(desktopView->statusType)
//            {
//                case DEVICE_DISPLAY_NO:
//                {
//                    if(desktopView->typeStatus == DEVICE_DISPLAY_NO)
//                        break;
//                    desktopView->ipTextLabel->clear();
//                    desktopView->typeStatus = DEVICE_DISPLAY_NO;
//                    break;
//                }
//                case DEVICE_DISPLAY_SERIAL_NUM:
//                {
//                    if(desktopView->typeStatus == DEVICE_DISPLAY_SERIAL_NUM)
//                        break;
//                    desktopView->ipTextLabel->setText(QString(LsStatusBarInfo.DevSN));
//                    desktopView->ipTextLabel->show();
//                    desktopView->typeStatus = DEVICE_DISPLAY_SERIAL_NUM;
//                    break;
//                }
//                case DEVICE_DISPLAY_DEV_NAME:
//                {
//                    if(desktopView->typeStatus == DEVICE_DISPLAY_DEV_NAME)
//                        break;
//                    desktopView->ipTextLabel->setText(QString(LsStatusBarInfo.DevName));
//                    desktopView->typeStatus = DEVICE_DISPLAY_DEV_NAME;
//                    break;
//                }
//            }
//             continue;
//        }

        qDebug("NetLinkStat    =%d\n", LsStatusBarInfo.NetLinkStat);

        if(LsStatusBarInfo.NetLinkStat == QT_IpAddress)
        {
            if(desktopView->netLinkStatus == QT_IpAddress && desktopView->iPAddr == QString(LsStatusBarInfo.IPAddr))
                continue;
            desktopView->iPAddr = QString(LsStatusBarInfo.IPAddr);
            desktopView->updateStatus((Path + QString("images/icon_local.png")),QString("LsStatusBarInfo.IPAddr"),QT_IpAddress);

        }
        else if (LsStatusBarInfo.NetLinkStat == QT_IpConflict)
        {
            if(desktopView->netLinkStatus == QT_IpConflict && desktopView->iPAddr == QString(LsStatusBarInfo.IPAddr))
                continue;
            desktopView->iPAddr = QString(LsStatusBarInfo.IPAddr);
            desktopView->updateStatus((Path + QString("images/icon_local_conflict.png")),(QString(LsStatusBarInfo.IPAddr) + QString("  ") + QString(tr("IP conflict"))),QT_IpConflict);
        }
        else if (LsStatusBarInfo.NetLinkStat == QT_NotConnected)
        {
            if(desktopView->netLinkStatus == QT_NotConnected && desktopView->iPAddr == QString(LsStatusBarInfo.IPAddr))
                continue;
            desktopView->iPAddr = QString(LsStatusBarInfo.IPAddr);
            desktopView->updateStatus((Path +QString("images/icon_local_off.png")),(QString(LsStatusBarInfo.IPAddr) + QString("  ") + QString(tr("Not connected to the network"))),QT_NotConnected);
        }



        else  if (LsStatusBarInfo.NetLinkStat == QT_Wifi)
        {
            if(desktopView->netLinkStatus != QT_Wifi)
                desktopView->wifiSignal = -1;

            if(LsStatusBarInfo.WIFISignal == WIFISignal_NO)
            {

                if(desktopView->wifiSignal == WIFISignal_NO && desktopView->iPAddr == QString(LsStatusBarInfo.IPAddr))
                    continue;


                desktopView->iPAddr = QString(LsStatusBarInfo.IPAddr);
                desktopView->updateStatus((Path +QString("images/icon_wifi_no.png")),(QString("LsStatusBarInfo.IPAddr")+ QString("  ") + QString(tr("LsStatusBarInfo.IPAddr")) ),WIFISignal_NO);


            }
            else if(LsStatusBarInfo.WIFISignal == WIFISignal_ONE)
            {

                if(desktopView->wifiSignal == WIFISignal_ONE && desktopView->iPAddr == QString(LsStatusBarInfo.IPAddr))
                    continue;

                desktopView->iPAddr = QString(LsStatusBarInfo.IPAddr);
                desktopView->updateStatus((Path +QString("images/icon_wifi_no.png")),(QString("LsStatusBarInfo.IPAddr") ),WIFISignal_ONE);
            }
            else if (LsStatusBarInfo.WIFISignal == WIFISignal_TWO)
            {

                if(desktopView->wifiSignal == WIFISignal_TWO && desktopView->iPAddr == QString(LsStatusBarInfo.IPAddr))
                    continue;
                desktopView->iPAddr = QString(LsStatusBarInfo.IPAddr);
                desktopView->updateStatus((Path +QString("icon_wifi_2.png")),(QString("LsStatusBarInfo.IPAddr") ),WIFISignal_TWO);

            }
            else if (LsStatusBarInfo.WIFISignal == WIFISignal_THREE)
            {

                if(desktopView->wifiSignal == WIFISignal_THREE && desktopView->iPAddr == QString(LsStatusBarInfo.IPAddr))
                    continue;
                desktopView->iPAddr = QString(LsStatusBarInfo.IPAddr);
                desktopView->updateStatus((Path +QString("images/icon_wifi_3.png")),(QString("LsStatusBarInfo.IPAddr") ),WIFISignal_THREE);
            }
        }


        else if (LsStatusBarInfo.NetLinkStat == QT_Operator)
        {

            if(desktopView->netLinkStatus != QT_Operator)
            {
                desktopView->cellularSignal = -1;
                desktopView->cellularOperator = -1;
            }


            if(LsStatusBarInfo.CellularOperator == QT_China_Unicom)
            {
                if(desktopView->cellularOperator == QT_China_Unicom)
                    continue;
                desktopView->CellularOperator = QString(tr("China_Unicom 4G"));
                //                    desktopView->ipTextLabel->setText(QString(tr("China_Unicom 4G")));
                //                    desktopView->cellularOperator = QT_China_Unicom;
            }



            else if (LsStatusBarInfo.CellularOperator == QT_China_Mobile)
            {
                if(desktopView->cellularOperator == QT_China_Mobile)
                    continue;
                desktopView->CellularOperator = QString(tr("China_Mobile 4G"));
                //                    desktopView->ipTextLabel->setText(QString(tr("China_Mobile 4G")));
                //                    desktopView->cellularOperator = QT_China_Mobile;
            }
            else if (LsStatusBarInfo.CellularOperator == QT_China_Telecom)
            {
                if(desktopView->cellularOperator == QT_China_Telecom)
                    continue;
                desktopView->CellularOperator = QString(tr("China_Telecom 4G"));
                //                    desktopView->ipTextLabel->setText(QString(tr("China_Telecom 4G")));
                //                    desktopView->cellularOperator = QT_China_Telecom;
            }
            else if (LsStatusBarInfo.CellularOperator == Qt_Other)
            {
                if(desktopView->cellularOperator == Qt_Other)
                    continue;
                desktopView->CellularOperator = QString(tr("Other 4G"));
                //                    desktopView->ipTextLabel->setText(QString(tr("Other 4G")));
                //                    desktopView->cellularOperator = Qt_Other;

            }



            if(LsStatusBarInfo.CellularSignal == CellularSignal_NO )
            {

                if(desktopView->cellularSignal == CellularSignal_NO)
                    continue;
                //                 desktopView->iconLabel->setPixmap(Path + QString("images/icon_signal_0.png"));
                //                 desktopView->cellularSignal = CellularSignal_NO;
                desktopView-> CellularSignal= QString(("images/icon_signal_0.png"));
            }

            else if(LsStatusBarInfo.CellularSignal == CellularSignal_ONE)
            {
                if(desktopView->cellularSignal == CellularSignal_ONE)
                    continue;
                //                   desktopView->iconLabel->setPixmap(Path + QString("images/icon_signal_1.png"));
                //                    desktopView->cellularSignal = CellularSignal_ONE;
                desktopView->CellularSignal= QString(("images/icon_signal_1.png"));
            }

            else if (LsStatusBarInfo.CellularSignal == CellularSignal_TWO)
            {
                if(desktopView->cellularSignal == CellularSignal_TWO)
                    continue;
                //  desktopView->iconLabel->setPixmap(Path + QString("images/icon_signal_2.png"));
                // desktopView->cellularSignal = CellularSignal_TWO;
                desktopView->CellularSignal= QString(("images/icon_signal_2.png"));
            }

            else if (LsStatusBarInfo.CellularSignal == CellularSignal_THREE)
            {
                if(desktopView->cellularSignal == CellularSignal_THREE)
                    continue;
                // desktopView->updateStatus((Path +QString("images/icon_signal_3.png")),(QString("China_Telecom 4G")),CellularSignal_THREE);
                // desktopView->iconLabel->setPixmap(Path + QString("images/icon_signal_3.png"));
                //                  desktopView->cellularSignal = CellularSignal_THREE;
                desktopView->CellularSignal= QString(("images/icon_signal_3.png"));


            }

            else if (LsStatusBarInfo.CellularSignal == CellularSignal_FOUR)
            {
                if(desktopView->cellularSignal == CellularSignal_FOUR)
                    continue;
                desktopView->CellularSignal= QString(("images/icon_signal_4.png"));
                //   desktopView->cellularSignal = CellularSignal_FOUR;



            }

            desktopView->updateStatus((Path + desktopView->CellularSignal),desktopView->CellularOperator, QT_Operator);

            //               desktopView->ipTextLabel->show();
            //               desktopView->iconLabel->show();
            //               desktopView->updateUi(0,0,800,50);
            //               desktopView->netLinkStatus == QT_Operator;

        }
        else if (LsStatusBarInfo.NetLinkStat == QT_SIM)
        {
            if(desktopView->netLinkStatus == QT_SIM)
                continue;
            desktopView->updateStatus((Path + QString("images/icon_signal_no.png")),QString(tr("No SIM card")),QT_SIM);
        }

           if(LsStatusBarInfo.BTEnableStat == QT_BlueTooth_OFF)
           {
               if(desktopView->blueToothStatus == QT_BlueTooth_OFF)
                   continue;
               pthread_mutex_lock(&g_paintStatusBarInfoDraw_mutex);
               desktopView->blueToothTextLabel->clear();
               desktopView->blueToothIconLabel->clear();
               pthread_mutex_unlock(&g_paintStatusBarInfoDraw_mutex);
               desktopView->blueToothStatus = QT_BlueTooth_OFF;
           }
           else if(LsStatusBarInfo.BTEnableStat == QT_BlueTooth_ON)
           {
               if(desktopView->blueToothStatus == QT_BlueTooth_ON)
                   continue;
               pthread_mutex_lock(&g_paintStatusBarInfoDraw_mutex);
               desktopView->blueToothTextLabel->setText(QString(LsStatusBarInfo.BTMatchDev));
               desktopView->blueToothTextLabel->setAlignment(Qt::AlignRight);
               desktopView->blueToothIconLabel->setPixmap(Path + QString("images/icon_bluetooth_nor.png"));
               desktopView->blueToothTextLabel->show();
               desktopView->blueToothIconLabel->show();
               pthread_mutex_unlock(&g_paintStatusBarInfoDraw_mutex);
               desktopView->blueToothStatus = QT_BlueTooth_ON;
           }
    }
}

void DesktopView::upgradeScheduleInfo(void *arg)
{
//    upgrade_schedule_t* upgradeschedule = (upgrade_schedule_t*)arg;
//    if(upgradeschedule->ProgressBar <= 100)
//        desktopView->waterProcess1->setValue(upgradeschedule->ProgressBar);


}
void DesktopView::upgradeBeginInfo(void *arg)
{
//    desktopView->greenLabel->setVisible(false);
//    desktopView->redLabel->setVisible(false);
//    desktopView->yellowLabel->setVisible(false);
//    desktopView->timeLabel->setVisible(false);
//    desktopView->dateLabel->setVisible(false);
//    desktopView->faceRecgInfoItem->setVisible(false);
//    desktopView->movie->stop();
//    desktopView->label->setVisible(false);
//    desktopView->bgbottomWidget->setVisible(false);
//    desktopView->faceShadeWidget->setVisible(false);

//    desktopView->waterProcess1->show();
//    desktopView->waterProcess1->setStyleSheet("QWidget{background: transparent}");
//    desktopView->waterProcess1->setGeometry(280, 308, 246, 246);
//    desktopView->waterProcess1->setBorderWidth(3);
//    desktopView->waterProcess1->setValue(0);
//    desktopView->updateLabel->setText(QString(tr("The upgrade process takes 1-10 minutes, Please do not turn off the power, after completion of the upgrade will automatically restart.")));
//    desktopView->updateLabel->show();
//    desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
}

void DesktopView::upgradeFailInfo(void *arg)
{
//    desktopView->faceShadeWidget->hide();
//    desktopView->bgTopWidget->hide();
//    desktopView->bgbottomWidget->hide();
//    desktopView->timeLabel->hide();
//    desktopView->dateLabel->hide();
//    desktopView->movie->stop();
//    desktopView->label->setVisible(false);
//    desktopView->faceRecgInfoItem->setVisible(false);
//    desktopView->updatefailure->setVisible(true);
//    desktopView->updateFailLabel->show();
//    desktopView->updateFailLabel->setGeometry(285,400,210,210);
//    desktopView->updateFailLabel->setStyleSheet(QString("QLabel{border-image:url(") + Path +  QString("images/upgrade_failure.png)}"));
//    desktopView->updateFailLabel->setStyleSheet(" width: 483px;height: 64px;font-size: 20px;font-family: SourceHanSansCN-Normal, SourceHanSansCN;font-weight: 400;color: #FFFFFF;line-height: 32px;border-image:url(./images/update.png)");
//    desktopView->updateFailLabel->setText(QString(tr("Upgrade Fail!")));
//    desktopView->updateFailLabel->setAlignment(Qt::AlignCenter);
//    desktopView->updateFailLabel->setWordWrap(true);

}

void DesktopView::updateClearFaceRecgInfo()
{
    emit clearFaceRecgInfoSignals();
}
void DesktopView::updateFaceRecgInfo()
{/*
    qDebug("%s %d==================================start\n", __FILE__, __LINE__);
    qDebug("Stranger      = %d\n",   faceRecgInfo->FaceID);          // 1: 陌生人(黑名单)          其它:认证人员(白名单)
    qDebug("NumberMatch      = %d\n",   faceRecgInfo->NumberMatch);          // 1:请刷卡 2.匹配的卡号 3.未知的卡号
    qDebug("Username      = %s\n",   faceRecgInfo->Username);          // 名字
    qDebug("MeasuringResult = %d\n",   faceRecgInfo->MeasuringResult);      // 0(温度正常),     1(温度偏低),     2(温度异常)
    qDebug("MaskStatus    = %d\n",   faceRecgInfo->MaskStatus);        // 1: 佩戴口罩        其它: 未戴口罩
    qDebug("Thermodynamic    = %d\n",   faceRecgInfo->Thermodynamic);        // 1: 有热力图数据      其它: 无数据
    qDebug("TextType      = %d\n",   faceRecgInfo->TextType);          // (QT界面端给出支持的语言提示序号)
    qDebug("Temperature   = %.2f\n", faceRecgInfo->Temperature);        //温度
    qDebug("PicPath   = %s\n", faceRecgInfo->PicPath);        //
    qDebug("VerificatMode      = %d\n",   faceRecgInfo->VerificatMode);
    qDebug("%s %d==================================end\n", __FILE__, __LINE__);*/
    emit updateFaceRecgInfoSignals();

}
void DesktopView::updateStatusBarInfo()
{
    emit updateStatusBarInfoSignals();
}


void DesktopView::  timeFormatInfo(void *arg)
{
    qDebug("timeType      = %d\n", desktopView->timeType);          // 0:年/月/日 1:日/月/年 2:月/日/年
    memcpy(&desktopView->timeType, (int *)arg, sizeof(int));
    emit desktopView->updateTime();

}

void DesktopView::systemLanguageInfo(void *arg)
{
    memcpy(&desktopView->languageType, (int *)arg, sizeof(int));
    emit desktopView->updateTranslatorSignals();
    emit desktopView->updateTime();
}

void DesktopView::initLanguage() {
//    sdk_comm_cfg_t sdkCommCfg;
//    HS_Ipcc_Get_Common_config(&sdkCommCfg);
//    desktopView->languageType = sdkCommCfg.language;
//    desktopView->statusType = sdkCommCfg.video_mode;
//    desktopView->timeType = sdkCommCfg.time_format;

//    emit desktopView->updateTranslatorSignals();
//    emit desktopView->updateTime();
//    emit desktopView->updateStatusSignals();
//    qDebug("+++++++++++++++sdkCommCfg.language:%d+++++++++++++",sdkCommCfg.language);
//    qDebug("+++++++++++++++sdkCommCfg.video_mode:%d+++++++++++++",sdkCommCfg.video_mode);
}



void DesktopView::devDisplayInfo(void *arg)
{
//    DEVICE_DISPLAY_CONFIG *deviceDisplayConfig = (DEVICE_DISPLAY_CONFIG*)arg;
//    qDebug("BTMatchDev      =%d\n", deviceDisplayConfig->DevDisplayType);
//    qDebug("DevName      =%s\n", deviceDisplayConfig->DevName);        //设备名称
//    qDebug("DevSN        =%s\n", deviceDisplayConfig->SerialNum);        //设备序列号
//    qDebug("IPAddr        =%s\n", desktopView->iPAddr);        //IPAddr
//    switch(deviceDisplayConfig->DevDisplayType)
//    {
//        case DEVICE_DISPLAY_NO:
//        {
//            desktopView->showNetType = 1;
//            desktopView->iconLabel->clear();
//            desktopView->ipTextLabel->clear();
//                break;
//        }
//        case DEVICE_DISPLAY_SERIAL_NUM:
//        {
//            desktopView->iconLabel->clear();
//            desktopView->showNetType = 1;
//            desktopView->ipTextLabel->setGeometry(10, 9, 368, 25);
//            desktopView->ipTextLabel->setText(deviceDisplayConfig->SerialNum);
//            desktopView->ipTextLabel->show();
//            break;
//        }
//        case DEVICE_DISPLAY_DEV_NAME:
//        {
//            desktopView->iconLabel->clear();
//            desktopView->showNetType = 1;
//            desktopView->ipTextLabel->setGeometry(10, 9, 368, 25);
//            desktopView->ipTextLabel->setText(deviceDisplayConfig->DevName);
//            desktopView->ipTextLabel->show();
//            break;
//        }
//        case DEVICE_DISPLAY_IP:
//        {
//           desktopView->ipTextLabel->setGeometry(60, 9, 368, 25);
//           desktopView->showNetType = 0;
//           desktopView->iconLabel->setPixmap(Path + QString("images/icon_local.png"));
//           desktopView->ipTextLabel->setText(desktopView->iPAddr);
//           desktopView->iconLabel->show();
//           desktopView->ipTextLabel->show();
//            break;
//        }
//    }
}


void DesktopView::updateTranslatorScene()
{
    QString Path = QString("D:");
    QString qmName;
    QTranslator *translator = new QTranslator(desktopView);
    qDebug("+++++++++++languageType:%d+++++++++ ",desktopView->languageType);
    if(desktopView->languageType == 0){
        qmName = QString("zh_CN.qm");
    } else if (desktopView->languageType == 1){
        qmName = QString("en_US.qm");
    }
    if (translator->load(Path + QString("translation/") + qmName));
    {
        qApp->installTranslator(translator);
    }
    desktopView->updateUi(0, 0, desktopView->desktopRect.width(), desktopView->desktopRect.height());
}

void DesktopView::updateStatusScene()
{
//    QString Text;
//    switch(desktopView->statusType)
//    {
//        case DEVICE_DISPLAY_NO:
//        {
//            desktopView->showNetType = 1;
//            desktopView->ipTextLabel->clear();
//                break;
//        }
//        case DEVICE_DISPLAY_SERIAL_NUM:
//        {
//            desktopView->showNetType = 1;
//            desktopView->ipTextLabel->setGeometry(10, 9, 368, 25);
//            desktopView->ipTextLabel->show();
//            break;
//        }
//        case DEVICE_DISPLAY_DEV_NAME:
//        {
//            desktopView->showNetType = 1;
//            desktopView->ipTextLabel->setGeometry(10, 9, 368, 25);
//            desktopView->ipTextLabel->show();
//            break;
//        }
//        case DEVICE_DISPLAY_IP:
//        {
//           desktopView->showNetType = 0;
//            break;
//        }
//    }
}


DesktopView::DesktopView(int faceCnt, int refresh,int languagetype, QWidget *parent)
    : QGraphicsView(parent)
{
    desktopView = this;
    cameraType = ISP;
    saveFrames = SAVE_FRAMES;
    saving = false;
    updateFace = false;
    refreshFrame = refresh;
    languageType = 1;
    statusType = 0;
    percent = 0;
    timeType = 0;
    verificatStatus = -1;
    showNetType = 0;
    iPAddr = QString("");
    blueToothStatus = QT_BlueTooth_OFF;
    netLinkStatus = QT_BLANK;
    cellularOperator = -1;
    typeStatus = -1;
    //人员信息标志
    facePosition = -1;
    tipBoxType = -1;
#ifdef TWO_PLANE
    this->setStyleSheet("background: transparent");
#endif

    desktopRect = QApplication::desktop()->availableGeometry();
    qDebug("new DesktopView Rect(%d, %d, %d, %d)", desktopRect.x(), desktopRect.y(),
        desktopRect.width(), desktopRect.height());

    resize(800, 1280);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setAttribute(Qt::WA_AcceptTouchEvents, true);
    //qApp->setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);
    //qApp->setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);

    faceRecgInfoItem = new StartItem();

//    videoItem = new VideoItem(desktopRect);
//    videoItem->setZValue(0);

//    keyBoard = QKeyBoard::getInstance();

//    scene->addWidget(bgTopWidget);
//    scene->addItem(videoItem);
//    scene->addWidget(keyBoard);

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    initUi();
    iniSignalSlots();

    scene->addWidget(bgbottomWidget);
//    scene->addWidget(idCardWidget);

    scene->addItem(faceRecgInfoItem);

//    scene->addWidget(menuWidget);
//    scene->addWidget(setWidget);
//    scene->addWidget(editWidget);
    menuWidget->setVisible(false);
//    idCardWidget->setVisible(false);
//    editWidget->setVisible(false);
//    setWidget->setVisible(false);


#ifdef BUILD_TEST
    initTestUi();
    scene->addWidget(testWidget);
    testWidget->setVisible(false);
#endif
    scene->setSceneRect(scene->itemsBoundingRect());
    setScene(scene);

    initTimer();
//    initRkfacial(faceCnt);

    //状态栏信息队列&线程
    queueCycleInit(&g_paintStatusBarInfo_queue,32);
    pthread_t paintStatusBarInfoNtid;
    int StatusBarInfoErr = pthread_create(&paintStatusBarInfoNtid, NULL, paintStatusBarInfo_Thread, NULL);
    if (StatusBarInfoErr != 0)
        printf("can't create thread: %s\n", strerror(StatusBarInfoErr));

// //    人员信息队列&线程
//    queueCycleInit(&g_paintFaceRecgInfo_queue,32);
//    pthread_mutex_init(&g_paintFaceRecgInfo_mutex, NULL);
//    pthread_t paintFaceRecgInfoNtid;
//    int FaceRecgInfoErr = pthread_create(&paintFaceRecgInfoNtid, NULL, paintFaceRecgInfo_Thread, NULL);
//    if (FaceRecgInfoErr != 0)
//        printf("can't create thread: %s\n", strerror(FaceRecgInfoErr));

//    //初始化语言
//    initLanguage();

}

DesktopView::~DesktopView()
{
//    deinitRkfacial();
    timer->stop();
    movie->stop();
//  faceTimer->stop();

//  if(keyBoard)
//      delete keyBoard;
    queueCycleDestroy(&g_paintFaceRecgInfo_queue);
    queueCycleDestroy(&g_paintStatusBarInfo_queue);
}
