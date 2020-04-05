// UTF-8 with BOM

// Avoid gibberish when use MSVC
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMenu>
#include <QWidget>
#include <QPainter>

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#ifndef SELECTRECT_H
#define SELECTERCT_H

// Common function

class SelectRect : public QWidget {
    Q_OBJECT
public:
    SelectRect(QWidget* parent = nullptr);
    ~SelectRect();

    void setImage(QImage* img, QImage* zoomedImg, const QPoint& imageLeftTop)
    {
        image = img;
        zoomedImage = zoomedImg;
        drawImageTopLeftPos = imageLeftTop;
        isLoadImage = true;
    }
    QPoint drawImageTopLeftPos = QPoint(-1, -1);

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
    enum { SR_NULL = -1, SR_CENTER, SR_TOPLEFT, SR_TOPRIGHT, SR_BOTTOMRIGHT, SR_BOTTOMLEFT, SR_TOP, SR_RIGHT, SR_BOTTOM, SR_LEFT, SR_ENTIRETY };
    // Widget中选中的范围
    QRect selectedRect[10];
    QRect lastSelectedRect;
    // Image中选中的范围
    QRect fixedRectInImage;

    QPoint mouseLeftClickedPos = QPoint(0, 0);
    int mouseStatus;
    QImage* image = nullptr;
    QImage* zoomedImage = nullptr;
    bool isLoadImage = false;
    bool isSelectedRectStable = false;
    bool isSelectedRectExisted = false;
    int cursorPosInSelectedArea = SR_NULL;
};

#endif

class ImageWidget : public QWidget {
    Q_OBJECT
public:
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
    ImageWidget* setEnableLoadImageWithDefaultConfig(bool flag = false);

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
    static const QImage VOID_QIMAGE;
    // 原始图像
    QImage qImgContainer;
    // 缩放后的图像
    QImage qImgZoomedContainer;

    QSize lastZoomedImageSize = QSize(0, 0);

    double zoomScale = 1.0;

    QPoint mouseLeftKeyPressDownPos = QPoint(0, 0);
    QPoint drawImageTopLeftLastPos = QPoint(0, 0);
    QPoint drawImageTopLeftPos = QPoint(0, 0);

    // status flags
    bool isSelectMode = false;
    bool isImageDragged = false;
    bool isImageDragging = false;
    bool isZoomedParametersChanged = false;

    bool enableOnlyShowImage = false;
    bool enableDragImage = true;
    bool enableZoomImage = true;
    bool enableAutoFitWidget = true;
    bool enableLoadImageWithDefaultConfig = false;
    bool enableSendLeftClickedPosInWidget = false;
    bool enableSendLeftClickedPosInImage = false;

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
    QAction* mActionLoadImageWithDefaultConfig = nullptr;

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

    void wheelEvent(QWheelEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);
    void contextMenuEvent(QContextMenuEvent* e);
    void resizeEvent(QResizeEvent* e);
};

#endif // IMAGEWIDGET_H
