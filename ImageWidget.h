// UTF-8 with BOM

// Avoid gibberish when use MSVC
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#pragma once
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QWheelEvent>
#include <QMenu>
#include <QCursor>
#include <QFileDialog>
#include <QSplitter>
#include <QMessageBox>
#include <QDebug>
#include "selectrect.h"

enum MouseDown{MOUSE_NO,MOUSE_LEFT,MOUSE_MID,MOUSE_RIGHT};

class ImageWidget :
	public QWidget
{
	Q_OBJECT
public:
    ImageWidget(QWidget *parent);
	~ImageWidget();

    void setImageWithData(QImage img);
    void setImageWithPointer(QImage* img);

signals:
    void parentWidgetSizeChanged(int width, int height, int imageLeftTopPosX, int imageLeftTopPosY);
    void sendLeftClickedPos(int x, int y);
    void sendLeftClickedPosInImage(int x, int y);

public slots:
    void clear();
    void setOnlyShowImage(bool flag = false);
    void setEnableDragImage(bool flag = true);
    void setEnableZoomImage(bool flag = true);
    void setEnableImageFitWidget(bool flag = true);
    void setEnableRecordLastParameters(bool flag = false);
    // 发送点击位置坐标信号默认关闭，使用前需要开启
    void setEnableSendLeftClickedPos(bool flag = false);
    void setEnableSendLeftClickedPosInImage(bool flag = false);

private slots:
    void resetImageWidget();
    void save();
    void select();
    void selectModeExit();

protected:
    void calculateImageLeftTopRelativePosInWidget(const int x, const int y, double &returnX, double &returnY);

private:
    void updateZoomedImage();
    void imageZoomOut();
    void imageZoomIn();
    void getDrawImageTopLeftPos(int x,int y);
    void initializeContextmenu();
    void emitLeftClickedSignals(QMouseEvent *e);
    QPoint calculateCursorPosInImage(const QImage *originalImage, const QImage *zoomedImage, const QPoint &imageLeftTopPos, QPoint cursorPos);
    QPoint calculateCursorPosInZoomedImage(QPoint cursorPos);
    void setDefaultParameters();
    void setImageInCenter();

    void wheelEvent(QWheelEvent *e);
    void mouseMoveEvent(QMouseEvent * e);
    void mousePressEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);
    void contextMenuEvent(QContextMenuEvent *e);
    void resizeEvent(QResizeEvent *event);

    QImage *qImageContainer = NULL;
    QImage *qImageZoomedImage = NULL;

    int lastZoomedImageWidth;
    int lastZoomedImageHeight;

    double imageLeftTopRelativePosInWdigetX = 0.0;
    double imageLeftTopRelativePosInWdigetY = 0.0;

    double zoomScaleX = 1.0;
    double zoomScaleY = 1.0;

    int mouseLeftClickedPosX;
    int mouseLeftClickedPosY;

    int drawImageTopLeftLastPosX = 0;
    int drawImageTopLeftLastPosY = 0;
    int drawImageTopLeftPosX = 0;
    int drawImageTopLeftPosY = 0;
    // status flags
    bool isLoadImage = false;
    bool isSelectMode = false;
    bool isOnlyShowImage = false;
    bool isImageCloned = false;
    bool isZoomedParametersChanged = false;

    bool isEnableDragImage = true;
    bool isEnableZoomImage = true;
    bool isEnableFitWidget = true;
    bool isEnableRecordLastParameters = false;
    bool isEnableSendLeftClickedPos = false;
    bool isEnableSendLeftClickedPosInImage = false;

    int mouseStatus = MOUSE_NO;
    // 1
    QMenu *mMenu = NULL;
    QAction *mActionResetParameters = NULL;
    QAction *mActionSave = NULL;
    QAction *mActionSelect = NULL;
    // 2
    QMenu *mMenuAdditionalFunction = NULL;
    QAction *mActionEnableDrag = NULL;
    QAction *mActionEnableZoom = NULL;
    QAction *mActionImageFitWidget = NULL;
    QAction *mActionRecordLastParameters = NULL;
};
