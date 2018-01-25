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


class RectInfo
{
public :
    RectInfo(int x, int y, int width, int height)
    {
        x1 = x;
        y1 = y;
        w = width;
        h = height;
    }
    RectInfo()
    {

    }
    ~RectInfo()
    {

    }
    int x1 = 0;
    int y1 = 0;
    int w = 0;
    int h = 0;
    int x2() const
    {
        return x1+w;
    }
    int y2() const
    {
        return y1+h;
    }
    void clear()
    {
        x1 = y1 = w = h = 0;
    }
};

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
    void saveImage(const QImage *img, RectInfo rect);
    void fixRectInfo(RectInfo &rect);
    RectInfo calculateRectInfoInImage(const QImage *img, const QPoint &leftTopPos, RectInfo rect);
    bool isCursorPosInSelectedArea(const RectInfo &rect, QPoint cursorPos);

    QMenu *mMenu = NULL;
    QAction *mActionReset = NULL;
    QAction *mActionSaveZoomedImage = NULL;
    QAction *mActionSaveOriginalImage = NULL;
    QAction *mActionExit = NULL;
    // Widget中选中的范围
    RectInfo selectedRectInfo;
    RectInfo lastSelectedRectInfo;
    // Image中选中的范围
    RectInfo fixedRectInfoInImage;
    int mouseLeftClickedPosX = 0;
    int mouseLeftClickedPosY = 0;
    int mouseStatus;
    QImage* image = NULL;
    QImage *zoomedImage = NULL;
    bool isLoadImage = false;
    bool isCursorInSelectedArea = false;
};

#endif // SELECTRECT_H
