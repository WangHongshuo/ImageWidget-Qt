// UTF-8 with BOM

// Avoid gibberish when use MSVC
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMenu>
#include <QWidget>

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#ifndef SELECTRECT_H
#define SELECTERCT_H

class SelectRect : public QWidget {
    Q_OBJECT
public:
    SelectRect(QWidget* parent = nullptr);
    ~SelectRect();

    void setImage(QImage* img, QImage* zoomedImg, QPoint imageLeftTop)
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
    enum {
        SR_NULL = -1,
        SR_CENTER,
        SR_TOPLEFT,
        SR_TOPRIGHT,
        SR_BOTTOMRIGHT,
        SR_BOTTOMLEFT,
        SR_TOP,
        SR_RIGHT,
        SR_BOTTOM,
        SR_LEFT,
        SR_ENTIRETY
    };
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

    void setImageWithData(QImage& img);
    void setImageWithPointer(QImage* img);
    void setImageWithFilePath(QString& path);
    void setEnableOnlyShowImage(bool flag = false);

    // 发送点击位置坐标信号默认关闭，使用前需要开启
    void setEnableSendLeftClickedPosInWidget(bool flag = false);
    void setEnableSendLeftClickedPosInImage(bool flag = false);
    QPoint getDrawImageTopLeftPos() const;

signals:
    void sendParentWidgetSizeChangedSignal();
    void sendLeftClickedPosInWidget(int x, int y);
    void sendLeftClickedPosInImage(int x, int y);

public slots:
    void clear();
    void setEnableDragImage(bool flag = true);
    void setEnableZoomImage(bool flag = true);
    void setEnableImageFitWidget(bool flag = true);
    void setEnableRecordLastParameters(bool flag = false);

private slots:
    void resetImageWidget();
    void save();
    void select();
    void selectModeExit();

protected:
    void getImageLeftTopRelativePosInWidget(const int x, const int y, double& returnX, double& returnY);

private:
    void updateZoomedImage();
    void imageZoomOut();
    void imageZoomIn();
    void initializeContextmenu();
    void emitLeftClickedSignals(QMouseEvent* e);
    QPoint getCursorPosInImage(const QImage* originalImage, const QImage* zoomedImage, const QPoint& imageLeftTopPos, QPoint cursorPos);
    QPoint getCursorPosInZoomedImage(QPoint cursorPos);
    void setDefaultParameters();
    QPoint getPutImageInCenterPos(const QImage* showImage, const QWidget* ImageWidget);

    void wheelEvent(QWheelEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);
    void contextMenuEvent(QContextMenuEvent* e);
    void resizeEvent(QResizeEvent* e);

    QImage* qImageContainer = nullptr;
    QImage* qImageZoomedImage = nullptr;

    QSize lastZoomedImageSize = QSize(0, 0);

    double imageTopLeftRelativePosInWdigetX = 0.0;
    double imageTopLeftRelativePosInWdigetY = 0.0;

    double zoomScale = 1.0;

    QPoint mouseLeftClickedPos = QPoint(0, 0);
    QPoint drawImageTopLeftLastPos = QPoint(0, 0);
    QPoint drawImageTopLeftPos = QPoint(0, 0);

    // status flags
    bool isImageLoaded = false;
    bool isSelectMode = false;
    bool isImageCloned = false;
    bool isImageDragged = false;
    bool isImageDragging = false;
    bool isZoomedParametersChanged = false;

    bool isEnableOnlyShowImage = false;
    bool isEnableDragImage = true;
    bool isEnableZoomImage = true;
    bool isEnableFitWidget = true;
    bool isEnableRecordLastParameters = false;
    bool isEnableSendLeftClickedPos = false;
    bool isEnableSendLeftClickedPosInImage = false;

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
    QAction* mActionImageFitWidget = nullptr;
    QAction* mActionRecordLastParameters = nullptr;
};

#endif // IMAGEWIDGET_H
