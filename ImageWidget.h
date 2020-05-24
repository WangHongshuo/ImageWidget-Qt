// UTF-8 with BOM

// Avoid gibberish when use MSVC
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMenu>
#include <QPainter>
#include <QWidget>

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#ifndef IMAGEMARQUEES_H
#define IMAGEMARQUEES_H

// Common function

class ImageMarquees : public QWidget {
    Q_OBJECT
public:
    ImageMarquees(QWidget* parent = nullptr);
    ~ImageMarquees();

    void setImage(QImage* inputImg, QImage* paintImg, const QPoint& paintImageTopLeft);

protected:
signals:
    void sendSelectModeExit();

public slots:

private slots:
    void receiveParentSizeChangedSignal();
    void selectExit();
    void selectReset();
    void cropZoomedImage();
    void cropOriginalImage();

private:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);
    void wheelEvent(QWheelEvent* event);
    bool eventFilter(QObject* watched, QEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void saveImage(const QImage* img, QRect rect);
    void fixRectInfo(QRect& rect);
    QRect getRectInImage(const QImage* img, const QPoint& imgTopLeftPos, QRect rect);
    void getEdgeRect();
    int getSelectedAreaSubscript(QPoint cursorPos);
    void selectedRectChangeEvent(int SR_LOCATION, const QPoint& cursorPos);

    QMenu* mMenu = nullptr;
    QAction* mActionReset = nullptr;
    QAction* mActionSaveZoomedImage = nullptr;
    QAction* mActionSaveOriginalImage = nullptr;
    QAction* mActionExit = nullptr;
    enum { SR_NULL = -1, SR_CENTER, SR_TOPLEFT, SR_TOPRIGHT, SR_BOTTOMRIGHT, SR_BOTTOMLEFT, SR_TOP, SR_RIGHT, SR_BOTTOM, SR_LEFT, SR_ENTIRETY, SR_TEST };
    // Widget中选中的范围
    QRect selectedRect[10];
    QRect lastSelectedRect;
    // Image中选中的范围
    QRect fixedRectInImage;
    QPoint paintImageTopLeft = QPoint(-1, -1);
    QPoint mouseLeftClickedPos = QPoint(0, 0);
    int mouseStatus;
    QImage* inputImg = nullptr;
    QImage* paintImg = nullptr;
    bool isLoadImage = false;
    bool isSelectedRectStable = false;
    bool isSelectedRectExisted = false;
    int cursorPosInSelectedArea = SR_NULL;
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
    void createSelectRectInWidget();
    void selectModeExit();
    // R1
    void updateImageWidget();

protected:
private:
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

    // status flags
    bool isSelectMode = false;
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
    QAction* mActionSelect = nullptr;
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
    QPoint getCursorPosInImage(const QImage& originalImage, const QImage& zoomedImage, const QPoint& imageLeftTopPos, const QPoint& cursorPos);
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
