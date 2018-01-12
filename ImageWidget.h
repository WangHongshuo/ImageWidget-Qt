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

    void setImageWithData(QImage img, bool resetImageWhenLoaded = false);
    void setImageWithPointer(QImage* img, bool resetImageWhenLoaded = false);

signals:
    void parentWidgetSizeChanged(int width, int height);
    void sendLeftClickedPos(int x, int y);

public slots:
    void clear();
    void setOnlyShowImage(bool flag = false);
    void setEnableDragImage(bool flag = true);
    void setEnableZoomImage(bool flag = true);
    void setEnableImageFitWidget(bool flag = true);

private slots:
    void resetImageWidget();
    void save();
    void select();
    void selectModeExit();

private:
    void setDefaultParameters();
    void imageZoomOut();
    void imageZoomIn();
    void getDrawImageTopLeftPos(int x,int y);
    void initializeContextmenu();

    void wheelEvent(QWheelEvent *e);
    void mouseMoveEvent(QMouseEvent * e);
    void mousePressEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);
    void contextMenuEvent(QContextMenuEvent *e);
    void resizeEvent(QResizeEvent *event);

    QImage *qImageContainer = NULL;
    QImage *qImageZoomedImage = NULL;

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

    bool isEnableDragImage = true;
    bool isEnableZoomImage = true;
    bool isEnableFitWidget = true;

    int mouseStatus = MOUSE_NO;

    QMenu* mMenu = NULL;
    QAction *mActionResetPos = NULL;
    QAction *mActionSave = NULL;
    QAction *mActionSelect = NULL;
    QAction *mActionEnableDrag = NULL;
    QAction *mActionEnableZoom = NULL;
    QAction *mActionImageFitWidget = NULL; 
};
