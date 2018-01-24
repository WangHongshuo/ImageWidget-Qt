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


typedef struct rectInfo
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

}rectInfo;
Q_DECLARE_METATYPE(rectInfo)

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
        isImageLoad = true;
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
    void cropImage(rectInfo rect);
    rectInfo fixRectInfoInImage(rectInfo rect);

    QMenu* subMenu;
    QAction* subActionReset;
    QAction* subActionSave;
    QAction* subActionSendRect;
    QAction* subActionExit;
    // Widget中选中的范围
    rectInfo selectedRectInfo;
    // Image中选中的范围
    rectInfo fixedRectInfoInImage;
    int mouseLeftClickedPosX;
    int mouseLeftClickedPosY;
    int mouseStatus;
    QImage* image = NULL;
    QImage *zoomedImage = NULL;
    bool isImageLoad;
};

#endif // SELECTRECT_H
