// UTF-8 with BOM

// Avoid gibberish when use MSVC
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "selectrect.h"
#include <iostream>
#include <QDebug>
#include <QCoreApplication>

SelectRect::SelectRect(QWidget *parent) : QWidget(parent)
{
    mouseStatus = MOUSE_NO;

    // 初始化右键菜单
    mMenu = new QMenu(this);
    mActionReset = mMenu->addAction(tr("重选"));// Reset
    mActionSaveZoomedImage = mMenu->addAction(tr("另存为(缩放图像)"));// Save as (From the zoomed image)
    mActionSaveOriginalImage = mMenu->addAction(tr("另存为(实际图像)"));// Save as (From the original image)
    mActionExit = mMenu->addAction(tr("退出"));// Exit

    connect(mActionExit,SIGNAL(triggered()),this,SLOT(selectExit()));
    connect(mActionSaveZoomedImage,SIGNAL(triggered()),this,SLOT(cropZoomedImage()));
    connect(mActionSaveOriginalImage,SIGNAL(triggered()),this,SLOT(cropOriginalImage()));
    connect(mActionReset,SIGNAL(triggered()),this,SLOT(selectReset()));
    // 关闭后释放资源
    this->setAttribute(Qt::WA_DeleteOnClose);
}

SelectRect::~SelectRect()
{
//    disconnect(this->parent(),SIGNAL(parent_widget_size_changed(int,int)),this,SLOT(receive_parent_size_changed_value(int,int)));
//    disconnect(this,SIGNAL(select_mode_exit()),this->parent(),SLOT(is_select_mode_exit()));
    image = NULL;
    zoomedImage = NULL;
    mMenu = NULL;
}

void SelectRect::paintEvent(QPaintEvent *event)
{
    // 背景
    QPainterPath mask;
    // 选中的范围
    QPainterPath selectArea;
    // 椭圆
//    QRect boundingRectangle(rect.x,rect.y,rect.w,rect.h);
//    select_area.addEllipse(boundingRectangle);
    selectArea.addRect(selectedRectInfo.x1,selectedRectInfo.y1,selectedRectInfo.w,selectedRectInfo.h);
    mask.addRect(this->geometry());
    QPainterPath drawMask =mask.subtracted(selectArea);
    QPainter painter(this);
    painter.setPen(QPen(QColor(0, 140, 255, 255), 1));
    painter.fillPath(drawMask,QBrush(QColor(0,0,0,160)));
    painter.drawRect(QRect(selectedRectInfo.x1,selectedRectInfo.y1,selectedRectInfo.w,selectedRectInfo.h));
}

void SelectRect::mousePressEvent(QMouseEvent *event)
{
        switch(event->button())
        {
        case Qt::LeftButton:
            mouseStatus = MOUSE_LEFT;
            mouseLeftClickedPosX = event->x();
            mouseLeftClickedPosY = event->y();
//            qDebug() << first_mousePosX << first_mousePosX;
            break;
        case Qt::RightButton:
            mouseStatus = MOUSE_RIGHT;
            break;
        case Qt::MiddleButton:
            mouseStatus = MOUSE_MID;
            break;
        default:
            mouseStatus = MOUSE_NO;
    }
}

void SelectRect::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseStatus == MOUSE_LEFT )
    {
        // 限定在mask内
        selectedRectInfo.x1 = mouseLeftClickedPosX;
        selectedRectInfo.y1 = mouseLeftClickedPosY;
        int x = event->x();
        int y = event->y();
        if(x < 0)
            x = 0;
        else if(x > this->width())
            x = this->width();
        if(y < 0)
            y = 0;
        else if (y > this->height())
            y = this->height();
        selectedRectInfo.w = x - selectedRectInfo.x1;
        selectedRectInfo.h = y - selectedRectInfo.y1;

    }
    if(mouseStatus == MOUSE_NO)
    {

    }
    update();
}

void SelectRect::mouseReleaseEvent(QMouseEvent *event)
{
    // 修正RectInfo::w和RectInfo::h为正
    fixRectInfo(selectedRectInfo);

    mouseStatus = MOUSE_NO;
}

void SelectRect::contextMenuEvent(QContextMenuEvent *event)
{
    mMenu->exec(QCursor::pos());
    mouseStatus = MOUSE_NO;
}

RectInfo SelectRect::calculateRectInfoInImage(const QImage *img, const QPoint &leftTopPos, RectInfo rect)
{
    RectInfo returnRectInfo;

    // 计算相对于图像内的坐标
    returnRectInfo.x1 = rect.x1 - leftTopPos.x();
    returnRectInfo.y1 = rect.y1 - leftTopPos.y();
    returnRectInfo.w = rect.w;
    returnRectInfo.h = rect.h;
    // 限定截取范围在图像内 修正顶点
    if(returnRectInfo.x1 < 0)
    {
        returnRectInfo.w = rect.w + returnRectInfo.x1;
        returnRectInfo.x1 = 0;
    }
    if (returnRectInfo.y1 < 0)
    {
        returnRectInfo.h = rect.h + returnRectInfo.y1;
        returnRectInfo.y1 = 0;
    }
    if(returnRectInfo.x1 + rect.w > img->width())
        returnRectInfo.w = img->width() - returnRectInfo.x1;
    if(returnRectInfo.y1 + rect.h > img->height())
        returnRectInfo.h = img->height() - returnRectInfo.y1;
    return returnRectInfo;
}

bool SelectRect::isCursorPosInSelectedArea(QPoint cursorPos)
{
    return 0;
}

void SelectRect::selectExit()
{
    emit sendSelectModeExit();
    this->close();
}

void SelectRect::selectReset()
{
    selectedRectInfo.w = 0;
    selectedRectInfo.h = 0;
    update();
}

void SelectRect::cropZoomedImage()
{
    fixedRectInfoInImage = calculateRectInfoInImage(zoomedImage,drawImageTopLeftPos,selectedRectInfo);
    saveImage(zoomedImage,fixedRectInfoInImage);
}

void SelectRect::cropOriginalImage()
{
    fixedRectInfoInImage = calculateRectInfoInImage(zoomedImage,drawImageTopLeftPos,selectedRectInfo);
    int x2 = fixedRectInfoInImage.x1 + fixedRectInfoInImage.w;
    int y2 = fixedRectInfoInImage.y1 + fixedRectInfoInImage.h;
    fixedRectInfoInImage.x1 = int(round(double(fixedRectInfoInImage.x1)/double(zoomedImage->width())*double(image->width())));
    fixedRectInfoInImage.y1 = int(round(double(fixedRectInfoInImage.y1)/double(zoomedImage->height())*double(image->height())));
    fixedRectInfoInImage.w = int(round(double(x2)/double(zoomedImage->width())*double(image->width())))-fixedRectInfoInImage.x1;
    fixedRectInfoInImage.h = int(round(double(y2)/double(zoomedImage->height())*double(image->height())))-fixedRectInfoInImage.y1;
    saveImage(image,fixedRectInfoInImage);
}

void SelectRect::saveImage(const QImage *img,RectInfo rect)
{
    // 接受到截取框信息
    if(isLoadImage)
    {
        if(rect.w > 0 && rect.h > 0)
        {
            QImage saveImageTemp = img->copy(rect.x1,rect.y1,rect.w,rect.h);
            QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                            QCoreApplication::applicationDirPath(),
                                                            tr("Images (*.png *.xpm *.jpg *.tiff *.bmp)"));
            if(!filename.isEmpty() || !filename.isNull())
                saveImageTemp.save(filename);
        }
        else
        {
            QMessageBox msgBox(QMessageBox::Critical,tr("错误!"),tr("未选中图像!"));// Error! No Image Selected!
            msgBox.exec();
        }
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Critical,tr("错误!"),tr("未选中图像!"));// Error! No Image Selected!
        msgBox.exec();
    }
}

void SelectRect::fixRectInfo(RectInfo &rect)
{
    if(rect.w < 0)
    {
        rect.x1 += rect.w;
        rect.w = -rect.w;
    }
    if(rect.h < 0)
    {
        rect.y1 += rect.h;
        rect.h = -rect.h;
    }
}

void SelectRect::receiveParentSizeChangedValue(int width, int height, int imageLeftTopPosX, int imageLeftTopPosY)
{
    this->setGeometry(0,0,width,height);
    drawImageTopLeftPos.setX(imageLeftTopPosX);
    drawImageTopLeftPos.setY(imageLeftTopPosY);
    update();
}

