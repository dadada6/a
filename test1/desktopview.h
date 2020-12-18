#ifndef DESKTOPVIEW_H
#define DESKTOPVIEW_H

#include <QGraphicsView>
#include <QGroupBox>
#include <QPushButton>
#include <QTouchEvent>
#include <QLineEdit>
#include <QProgressBar>
//#include <rkfacial/rkfacial.h>

#include "videoitem.h"
#include "qtkeyboard.h"
#include "startitem.h"
#include "waterprocess.h"
#include "circularprogessbar.h"

typedef enum {
	ISP,
	CIF
} CAMERA_TYPE;

#if 1
// 温度阵列最大测温点数量，即宽 x 高
#define MAX_TEMP_ARRAY_WIDTH          32
#define MAX_TEMP_ARRAY_HEIGHT         32
#define MAX_TEMP_ARRAY_DATA_SIZE      (MAX_TEMP_ARRAY_WIDTH * MAX_TEMP_ARRAY_HEIGHT)


typedef struct
{
    int FaceID;										// -1: 陌生人(黑名单)					其它/大于或等于0:认证人员(白名单)
    int NumberMatch;								// 卡号识别	1:匹配的卡号 2:未知的卡号
    int IdentityMatch;								// 身份证识别	:比对通过 2:比对不通过
    int FaceIndex;
    int FacePosition;								//人脸位置

    unsigned char VerificatMode;					//核验方式
    unsigned char VerificatResult;					//核验结果
    unsigned char Username[32];						// 名字
    unsigned char MeasuringResult;					// 0:温度正常,  					1: 温度偏低 	  		  2:温度异常
    unsigned char TempPosition;						//1:用于身份证温信息展示
    unsigned char MaskStatus;						// 0/其它: 	未戴口罩				1: 佩戴口罩
    unsigned char Thermodynamic;					// 0/其它: 	无数据				 	1: 有热力图数据
    unsigned char TextType;							// (QT界面端给出支持的语言提示序号)
    unsigned char PicPosition;						//用于身份证  图片展示
    unsigned char PicType;							//抓拍jpg/身份证bmp
    unsigned char TipBoxType;						//用于身份证提示框
    unsigned char PicPath[256];						//抓拍图片/人脸库图片 路径

    float Temperature;								//温度
    float TempArray[MAX_TEMP_ARRAY_DATA_SIZE];		//Thermodynamic=1 时  热力图有数据

    unsigned char temp_display_type;
    unsigned char Res[8];							//保留
}LS_faceRecg_t;



typedef struct
{
    unsigned char NetLinkStat;						//1(有线网络正常), 2(有线网络IP冲突), 3(有线网络未接入网线), 4(无线网络正常), 5(蜂窝网络正常), 6(未插入SIM卡)
    unsigned char IPAddr[32];						//ip地址
    unsigned char CellularOperator;					//NetLinkStat=5时 取用此值 (联通)China_Unicom: 1,  (移动)China_Mobile: 2,  (电信)China_Telecom: 3,  (广电)Other: 4
    unsigned char CellularSignal;					//NetLinkStat=5时 取用此值 蜂窝信号量 CellularSignal   =0, 1, 2, 3, 4
    unsigned char WIFISignal;						//NetLinkStat=4时 取用此值 无线信号量 WIFISignal=0, 1, 2, 3
    unsigned char BTEnableStat;						//蓝牙开关状态
    unsigned char BTMatchDev[128];					//蓝牙已经配对的设备
        unsigned char DevName[128];						//设备名称
    unsigned char DevSN[32];						//设备序列号
    unsigned char Res[8];							//保留

}StatusBar_t;


#endif


typedef enum
{
  QT_SUB_IDCARD_MATCH,          //卡号匹配
  QT_SUB_IDCARD_UNMATCH,          //卡号不匹配
}IDCARD_RESULT;

typedef enum
{
  QT_SUB_VERIFICAT_MATCH,          //允许通行
  QT_SUB_VERIFICAT_UNMATCH,        //禁止通行
}VERIFICAT_RESULT;

typedef enum
{
  QT_SUB_NOMASK,              //未戴口罩
  QT_SUB_HAVEMASK,            //佩戴口罩
}MASK_STATUS;


typedef enum
{
  QT_SUB_TIP_BOX,              //提示请刷身份证
  QT_SUB_COMPARE_BOX,            //显示人证比对框
}TIP_BOX_TYPE;


typedef enum
{
    QT_SUB_FACE_POSE_OK,              //Please hold
    QT_SUB_FACE_MOVE_LEFT,            //Please move left
    QT_SUB_FACE_MOVE_RIGHT,           //Please move right
    QT_SUB_FACE_MOVE_DOWN,            //Please move down
    QT_SUB_FACE_MOVE_UP,              //Please move up
    QT_SUB_FACE_MOVE_ALONG,           //Please move along
}FACE_POSE_STATUS;


typedef enum
{
    WIFISignal_NO,         //无信号
    WIFISignal_ONE,        //wifi信号强度1
    WIFISignal_TWO,        //wifi信号强度2
    WIFISignal_THREE,      //wifi信号强度3
}WIFISignal_STATUS;

typedef enum
{
    CellularSignal_NO,         //无信号
    CellularSignal_ONE,        //蜂窝信号强度1
    CellularSignal_TWO,        //蜂窝信号强度2
    CellularSignal_THREE,      //蜂窝信号强度3
    CellularSignal_FOUR,       //蜂窝信号强度4
}CellularSignal_STATUS;

typedef enum
{
    QT_MAIN_MEASURETEMP_BOX = 0x1,          //1.测温框
    QT_MAIN_DIS_FACERECG_INFO,              //2.人脸图片
    QT_MAIN_MASK_STATUS,                    //3.口罩状态
    QT_MAIN_MEASURETEMP_INFO,               //4.温度
    QT_MAIN_THERMODYNAMIC_INFO,             //5.热力图
    QT_MAIN_VERIFICAT_RESULT,               //6.通行结果
    QT_MAIN_TIP_SWIPE_IDCARD,               //7.刷IC卡提示
    QT_MAIN_SWIPE_IDCARD_RESULT,            //8.刷IC卡结果
    QT_MAIN_TIP_SWIPE_IDENTITYCARD,         //9.刷身份证    提示
    QT_MAIN_SWIPE_IDENTITYCARD_RESULT,      //10.人证结果
    QT_MAIN_FACE_POSITION,                  //11.人脸位置
}QT_MAIN_MESSAGE;

typedef enum
{
    QT_BlueTooth_OFF,
    QT_BlueTooth_ON,
}QT_BlueTooth_STATUS;

typedef enum
{
    QT_BLANK,
    QT_IpAddress = 0x1,
    QT_IpConflict,
    QT_NotConnected,
    QT_Wifi,
    QT_Operator,
    QT_SIM,
}QT_NetLink_STATUS;

typedef enum
{
    QT_China_Unicom = 0x1,
    QT_China_Mobile,
    QT_China_Telecom,
    Qt_Other,
}QT_CellularOperator;

typedef enum
{
    QT_CENTIGRADE = 0x0,
    QT_FAHRENHEIT,
    QT_KELVIN,
}QT_TemperatureType;

//已经在rkfacial/sdk_struct.h头文件中定义
//typedef enum {
//    DEVICE_DISPLAY_NO = 0x0,        //不显示
//    DEVICE_DISPLAY_SERIAL_NUM,      //显示序列号
//    DEVICE_DISPLAY_DEV_NAME,        //显示设备名字
//    DEVICE_DISPLAY_IP,              //显示IP地址
//    DEVICE_DISPLAY_END,
//}DEVICE_DISPLAY_TYPE;

typedef struct queue_cycle_elem_data
{
    char data[5120];
}queue_cycle_elem_data;

typedef struct
{
    queue_cycle_elem_data  *base;
    int front;
    int rear;
    int maxQueueNum;
}CYCLE_QUEUE_T;

class DesktopView : public QGraphicsView
{
    Q_OBJECT

public:
    static DesktopView *desktopView;

    explicit DesktopView(int faceCnt, int refresh,int languagetype, QWidget *parent = 0);
    virtual ~DesktopView();

protected:
    bool event(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

public:
    QString CellularOperator;
    QString CellularSignal;
    QLabel *menuWidget;
    QLabel *setWidget;
    QLabel *editWidget;

    QPushButton *switchBtn;
    QPushButton *registerBtn;
    QPushButton *deleteBtn;
    QPushButton *saveBtn;
    QPushButton setBtn;
    QPushButton closeBtn;
    QPushButton editSetBtn;

    QLineEdit *ipEdit;
    QLineEdit *netmaskEdit;
    QLineEdit *gatewayEdit;

    int refreshFrame;
    int saveFrames;
    bool saving;
    bool updateFace;

    QRect desktopRect;
//    VideoItem *videoItem;
    StartItem *faceRecgInfoItem;
    CAMERA_TYPE cameraType;
    QTimer *timer;
    QTimer *faceTimer;

    QKeyBoard *keyBoard;

    //初始化
    QLabel *faceShadeWidget;
    QLabel *bgTopWidget;
     QLabel *bgbottomWidget;
    //QWidget *bgbottomWidget;
    QWidget *idCardWidget;

    //顶部显示信息和图片
    QLabel *localIconLabel;
    QLabel *wifIconLabel;
    QLabel *singnalLabel;
    QLabel *iconLabel;
    QLabel *blueToothIconLabel;
    QLabel *blueToothTextLabel;
    QLabel *ipTextLabel;
    QLabel *operatorTextLabel;
    //中部显示信息和图片

    QLabel *updateLabel;
    QLabel *tipLabel;
    QLabel *colorLabel;

    QLabel *greenLabel;
    QLabel *yellowLabel;
    QLabel *redLabel;

    //底部显示信息和图片
    QLabel *temperatureTextLabel;
    QLabel *temperatureLabel;



    //时间
    QLabel *timeLabel;
    QLabel *dateLabel;
    QLabel *label;
    QLabel *faceLable;
    QMovie *movie;
    QLabel *pictureLable;
    WaterProcess *waterProcess1;
    QProgressBar *m_pConnectProBar;
    QLabel* updateFailLabel;
    QLabel* updatefailure;
    int languageType;
    int statusType;
    int percent;
    int timeType;
//    LS_faceRecg_t* faceRecgInfo;
//    StatusBar_t* statusBarInfo;
    int showNetType;
    //模式状态
    int verificatStatus;
    QString iPAddr;

    //状态栏标志
    int typeStatus;
    int blueToothStatus;
    int netLinkStatus;
    int wifiSignal;
    int cellularSignal;
    int cellularOperator;
    //人员信息标志
    int facePosition;
    int tipBoxType;
//    StatusBar_t* statusBarInfo;
//    CircularProgessBar *circularProgressBar;
//    CircularProgessBar *circularProgressBar2;
//    CircularProgessBar *circularProgressBar3;
//    CircularProgessBar *circularProgressBar4;
//    CircularProgessBar *circularProgressBar5;
//    CircularProgessBar *circularProgressBar6;
#ifdef BUILD_TEST
    QWidget *testWidget;
    QPushButton *collectBtn;
    QPushButton *realBtn;
    QPushButton *photoBtn;

    bool testing;
    void initTestUi();
    static void paintTestInfo(struct test_result *test);
#endif
//    static void paintFaceRecgInfo_Thread(void *arg);
    static void paintStatusBarInfo_Thread(void *arg);

    void initUi();
    void initEditUi();
    void initTimer();
    void iniSignalSlots();

//    int initRkfacial(int faceCnt);
    void deinitRkfacial();

    void saveFile(uchar *buf, int len, uchar *flag);
    void updateUi(int x, int y, int w, int h);
    void updateStatus(QString path, QString text,int mode);


    void updateClearFaceRecgInfo();
    void updateFaceRecgInfo();
    void updateStatusBarInfo();
    void initLanguage();

    static void paintBox(int left, int top, int right, int bottom);
    static void paintInfo(struct user_info *info, bool real);
    static void paintFace(void *ptr, int fmt, int width, int height, int x, int y, int w, int h);
    static void configRegion(int x, int y, int w, int h);

    static void displayRgb(void *src_ptr, int src_fd, int src_fmt, int src_w, int src_h, int rotation);
    static void displayIr(void *src_ptr, int src_fd, int src_fmt, int src_w, int src_h, int rotation);

    static void paintFaceRecgInfo(void *arg);
    static void paintFaceRecgInfoClear();
    static void paintStatusBarInfo(void *arg);

    static void upgradeScheduleInfo(void *arg);
    static void upgradeBeginInfo(void *arg);
    static void upgradeFailInfo(void *arg);

    static void timeFormatInfo(void *arg);
    static void systemLanguageInfo(void *arg);

    static void devDisplayInfo(void *arg);

signals:
    void itemDirty(int x, int y, int w, int h);

    void updateTime();
    void updateStatusSignals();
    void clearFaceRecgInfoSignals();
    void updateFaceRecgInfoSignals();
    void updateStatusBarInfoSignals();
    void updateTranslatorSignals();
private slots:

    void timerTimeOut();
    void faceTimerTimeOut();

//    void cameraSwitch();
//    void registerSlots();
//    void deleteSlots();
//    void saveSlots();
//    void updateScene(int x, int y, int w, int h);
//    void setSlots();
//    void closeSlots();
//    void editSetSlots();

    //刷新时间
    void updateTimeScene();
//    void clearFaceRecgInfoScene();
    void paintFaceRecgInfoScene();
    void paintStatusBarInfoScene();
    void updateTranslatorScene();
    void updateStatusScene();
#ifdef BUILD_TEST
    void saveAllSlots();
    void saveRealSlots();
    void saveFakeSlots();
#endif
};

#endif // DESKTOPVIEW_H
