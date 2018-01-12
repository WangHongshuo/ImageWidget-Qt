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


typedef struct rect_info
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

}rect_info;
Q_DECLARE_METATYPE(rect_info)

class SelectRect : public QWidget
{
    Q_OBJECT
public:

    SelectRect(QWidget *parent);
    ~SelectRect();
    void setImage(QImage *img,int x,int y)
    {
        image = img;
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
    void receiveParentSizeChangedValue(int width, int height);
    void selectExit();
    void selectReset();
    void cropImage();

private:

    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void fixRectInfoInImage();

    QMenu* subMenu;
    QAction* subActionReset;
    QAction* subActionSave;
    QAction* subActionSendRect;
    QAction* subActionExit;
    rect_info rect;
    int mouseLeftClickedPosX;
    int mouseLeftClickedPosY;
    int mouseStatus;
    QImage* image = NULL;
    bool isImageLoad;
};

#endif // SELECTRECT_H
