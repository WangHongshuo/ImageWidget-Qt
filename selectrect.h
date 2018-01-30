// UTF-8 with BOM

// Avoid gibberish when use MSVC
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#ifndef SELECTRECT_H
#define SELECTRECT_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QWheelEvent>
#include <QMenu>
#include <QCursor>
#include <QFileDialog>
#include <QSplitter>
#include "ImageWidget.h"

class SelectRect : public QWidget
{
    Q_OBJECT
public:

    SelectRect(QWidget *parent);
    ~SelectRect();
    void setImage(QImage *img, QImage *zoomedImg, int x,int y)
    {
        image = img;
        zoomedImage = zoomedImg;
        drawImageTopLeftPos.setX(x);
        drawImageTopLeftPos.setY(y);
        isLoadImage = true;
    }
    QPoint drawImageTopLeftPos = QPoint(-1,-1);

protected:

signals:
    void sendSelectModeExit();
public slots:


private slots:
    void receiveParentSizeChangedValue(int width, int height, int imageLeftTopPosX, int imageLeftTopPosY);
    void selectExit();
    void selectReset();
    void cropZoomedImage();
    void cropOriginalImage();

private:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void saveImage(const QImage *img, QRect rect);
    void fixRectInfo(QRect &rect);
    QRect getRectInImage(const QImage *img, const QPoint &imgTopLeftPos, QRect rect);
    void getEdgeRect();
    int getSelectedAreaSubscript(QPoint cursorPos);
    void selectedRectChangeEvent(int SR_LOCATION, QPoint &cursorPos);

    QMenu *mMenu = NULL;
    QAction *mActionReset = NULL;
    QAction *mActionSaveZoomedImage = NULL;
    QAction *mActionSaveOriginalImage = NULL;
    QAction *mActionExit = NULL;
    enum{SR_NULL=-1,SR_CENTER,SR_TOPLEFT,SR_TOPRIGHT,SR_BOTTOMRIGHT,SR_BOTTOMLEFT,
         SR_TOP,SR_RIGHT,SR_BOTTOM,SR_LEFT,SR_ENTIRETY};
    // Widget中选中的范围
    QRect selectedRect[10];
    QRect lastSelectedRect;
    // Image中选中的范围
    QRect fixedRectInImage;

    QPoint mouseLeftClickedPos = QPoint(0,0);
    int mouseStatus;
    QImage* image = NULL;
    QImage *zoomedImage = NULL;
    bool isLoadImage = false;
    bool isSelectedRectStable = false;
    int cursorPosInSelectedArea = SR_NULL;
};

#endif // SELECTRECT_H
