// UTF-8 with BOM

// Avoid gibberish when use MSVC
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMenu>
#include <QPainter>
#include <QWidget>
#include <unordered_map>

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#ifndef IMAGEMARQUEES_H
#define IMAGEMARQUEES_H

// Common function
QPoint getCursorPosInImage(const QRect& inputImgRect, const QRect& imageLeftTopPos, const QPoint& cursorPos, double (*procFunc)(double));

class ImageMarquees : public QWidget {
    Q_OBJECT
public:
    ImageMarquees(QWidget* parent = nullptr, int marqueesEdgeWidth = 10);
    ~ImageMarquees();
    void setImage(QImage* inputImg, QImage* paintImg, QRect* paintImageRect);
    void setMarqueesEdgeWidth(int width);

protected:
signals:
    void sendExitSignal();

public slots:

private slots:
    void recvParentWidgetSizeChangeSignal();
    void exit();
    void reset();
    void cropPaintImage();
    void cropOriginalImage();

private:
    enum CROPRECT { CR_NULL = -1, CR_CENTER, CR_TOPLEFT, CR_TOPRIGHT, CR_BOTTOMRIGHT, CR_BOTTOMLEFT, CR_ENTIRETY, CR_TOP, CR_RIGHT, CR_BOTTOM, CR_LEFT };
    const static std::unordered_map<CROPRECT, Qt::CursorShape> CURSORCRPS;
    static const CROPRECT CROPRECTGRP[3][3];
    static const char* ERR_MSG_NULL_IMAGE;
    static const char* ERR_MSG_INVALID_FILE_PATH;

    // Widget中选中的范围
    std::unordered_map<CROPRECT, QRect*> cropRectCorners;
    std::unordered_map<CROPRECT, QRect> cropRects;

    int cropRectPoints[2][4];
    QRect prevCropRect;
    // Image中选中的范围
    QRect cropRectInImage;
    QRect* paintImageRect = nullptr;
    QPoint mouseLeftClickedPos = QPoint(0, 0);
    int mouseStatus;
    QImage* inputImg = nullptr;
    QImage* paintImg = nullptr;
    bool isLoadImage = false;
    bool isCropRectStable = false;
    bool isCropRectExisted = false;
    CROPRECT cursorPosInCropRect = CR_NULL;
    int marqueesEdgeWidth = 5;

    QMenu* mMenu = nullptr;
    QAction* mActionReset = nullptr;
    QAction* mActionSavePaintImage = nullptr;
    QAction* mActionSaveOriginalImage = nullptr;
    QAction* mActionExit = nullptr;

    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);
    void wheelEvent(QWheelEvent* event);
    bool eventFilter(QObject* watched, QEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void saveImage(const QImage* img, const QRect& rect);
    QRect getCropRectInImage(const QRect& paintImageRect, const QRect& rect);
    void calcMarqueesEdgeRect();
    CROPRECT getSubRectInCropRect(QPoint cursorPos);
    void cropRectChangeEvent(int SR_LOCATION, const QPoint& cursorPos);
    bool keyEscapePressEvent();
    void showErrorMsgBox(const char* errMsg);
};

#endif

class ImageWidget : public QWidget {
    Q_OBJECT
public:
    // 图像限定模式
    enum RestrictMode { RM_INNER, RM_OUTTER };
    explicit ImageWidget(QWidget* parent = nullptr);
    ~ImageWidget();
    // 对外统一呈现setImage()接口
    bool setImage(const QImage& img, bool isDeepCopy = false);
    bool setImage(const QString& filePath);
    bool setImage(const std::string& filePath);

    void setEnableOnlyShowImage(bool flag = false);

    // 发送点击位置坐标信号默认关闭，使用前需要开启
    ImageWidget* setEnableSendLeftClickedPosInWidget(bool flag = false);
    ImageWidget* setEnableSendLeftClickedPosInImage(bool flag = false);
    QPoint getDrawImageTopLeftPos() const;

    // ROI相关
    void addROI(const QRect& rect, int label);
    void removeROI(int lable);
    void removeAllROI();

signals:
    void sendParentWidgetSizeChangedSignal();
    void sendLeftClickedPosInWidgetSignal(int x, int y);
    void sendLeftClickedPosInImageSignal(int x, int y);

public slots:
    void clear();
    ImageWidget* setEnableDrag(bool flag = true);
    ImageWidget* setEnableZoom(bool flag = true);
    ImageWidget* setEnableAutoFit(bool flag = true);
    ImageWidget* setMaxZoomScale(double scale);
    ImageWidget* setMinZoomScale(double scale);
    ImageWidget* setMaxZoomedImageSize(int width, int height);
    ImageWidget* setMinZoomedImageSize(int width, int height);
    ImageWidget* setPaintAreaOffset(int offset);
    ImageWidget* setPaintImageRestrictMode(RestrictMode rm);

private slots:
    void resetImageWidget();
    void save();
    void enterCropImageMode();
    void exitCropImageMode();
    // R1
    void updateImageWidget();

protected:
private:
    // 默认背景颜色
    static const QColor BACKGROUD_COLOR;
    // 静态空图像 用于释放内存
    static const QImage NULL_QIMAGE;
    // 静态变量 图像尺寸相关
    static const QPoint NULL_POINT;
    static const QSize NULL_SIZE;
    static const QRect NULL_RECT;
    // 放大倍率
    double MAX_ZOOM_SCALE = 20.0;
    double MIN_ZOOM_SCALE = 0.04;
    // 图像最大尺寸
    QSize MAX_ZOOMED_IMG_SIZE = QSize(100000, 10000);
    // 图像最小尺寸
    QSize MIN_ZOOMED_IMG_SIZE = QSize(10, 10);
    // Paint区域偏移量
    int PAINT_AREA_OFFEST = 0;
    // 原始图像
    QImage inputImg;
    // Paint图像
    QImage paintImg;
    QSize lastPaintImgSize = NULL_SIZE;

    // ImageWidge Paint区域
    QRect imageWidgetPaintRect;
    // 缩放后图像Paint区域
    QRect paintImageRect;

    double zoomScale = 1.0;

    QPoint mouseLeftKeyPressDownPos = NULL_POINT;
    QPoint paintImageLastTopLeft = NULL_POINT;

    // ROI区域相关
    // todo: 使用其他数据结构，减少遍历map，新增设置ROI区域颜色
    std::unordered_map<int, QRect> roi;

    // status flags
    bool isCropImageMode = false;
    bool isImagePosChanged = false;
    bool isImageDragging = false;
    bool isZoomedParametersChanged = false;

    bool enableOnlyShowImage = false;
    bool enableDragImage = true;
    bool enableZoomImage = true;
    bool enableAutoFitWidget = true;
    bool enableLoadImageWithDefaultConfig = false;
    bool enableSendLeftClickedPosInWidget = false;
    bool enableSendLeftClickedPosInImage = false;
    RestrictMode restrictMode = RM_INNER;

    int mouseStatus = Qt::NoButton;
    // 1
    QMenu* mMenu = nullptr;
    QAction* mActionResetParameters = nullptr;
    QAction* mActionSave = nullptr;
    QAction* mActionCrop = nullptr;
    // 2
    QMenu* mMenuAdditionalFunction = nullptr;
    QAction* mActionEnableDrag = nullptr;
    QAction* mActionEnableZoom = nullptr;
    QAction* mActionImageAutoFitWidget = nullptr;

    void updateZoomedImage();
    void imageZoomOut();
    void imageZoomIn();
    void initializeContextmenu();
    void sendLeftClickedSignals(QMouseEvent* e);
    void setDefaultParameters();
    QPoint getImageTopLeftPosWhenShowInCenter(const QImage& img, const QWidget* iw);
    // R1
    void initShowImage();
    bool loadImageFromPath(const QString& filePath);
    void setImageAttributeWithAutoFitFlag(bool enableAutoFit);
    void fixPaintImageTopLeft();
    void fixPaintImageTopLeftInOutterMode(const QRect& imageWidgetPaintRect, QRect& paintImageRect);
    void fixPaintImageTopLefInInnerMode(const QRect& imageWidgetPaintRect, QRect& paintImageRect);

    void wheelEvent(QWheelEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);
    void contextMenuEvent(QContextMenuEvent* e);
    void resizeEvent(QResizeEvent* e);
};

#endif // IMAGEWIDGET_H
