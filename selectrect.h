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
    int x2()
    {
        return x1+w-1;
    }
    int y2()
    {
        return y1+h-1;
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
        drawImageTopLeftPosX = x;
        drawImageTopLeftPosY = y;
        isLoadImage = true;
    }

    int drawImageTopLeftPosX = -1;
    int drawImageTopLeftPosY = -1;

protected:

signals:
    void sendSelectModeExit();
public slots:


private slots:
    void receiveParentSizeChangedValue(int width, int height, int imageLeftTopPosX, int imageLeftTopPosY);
    void selectExit();
    void selectReset();
    void crop();

private:

    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void cropImage(RectInfo rect);
    RectInfo fixRectInfoInImage(RectInfo rect);
    bool isCursorPosInSelectedArea(QPoint cursorPos);

    QMenu *subMenu = NULL;
    QAction *subActionReset = NULL;
    QAction *subActionSave = NULL;
    QAction *subActionSendRect = NULL;
    QAction *subActionExit = NULL;
    // Widget中选中的范围
    RectInfo selectedRectInfo;
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
