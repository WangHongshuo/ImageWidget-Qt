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
    QRect calculateRectInImage(const QImage *img, const QPoint &imgTopLeftPos, QRect rect);

    QMenu *mMenu = NULL;
    QAction *mActionReset = NULL;
    QAction *mActionSaveZoomedImage = NULL;
    QAction *mActionSaveOriginalImage = NULL;
    QAction *mActionExit = NULL;
    // Widget中选中的范围
    QRect selectedRect;
    QRect lastSelectedRect;
    // Image中选中的范围
    QRect fixedRectInImage;

    QPoint mouseLeftClickedPos = QPoint(0,0);
    int mouseStatus;
    QImage* image = NULL;
    QImage *zoomedImage = NULL;
    bool isLoadImage = false;
    bool isCursorPosInSelectedAreaFlag = false;
};

#endif // SELECTRECT_H
