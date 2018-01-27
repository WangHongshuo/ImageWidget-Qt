// UTF-8 with BOM

// Avoid gibberish when use MSVC
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "selectrect.h"
#include <iostream>
#include <QCoreApplication>

#include <QDebug>

SelectRect::SelectRect(QWidget *parent) : QWidget(parent)
{
    mouseStatus = Qt::NoButton;

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
    selectArea.addRect(selectedRect.x(),selectedRect.y(),selectedRect.width(),selectedRect.height());
    mask.addRect(this->geometry());
    QPainterPath drawMask =mask.subtracted(selectArea);
    QPainter painter(this);
    painter.setPen(QPen(QColor(0, 140, 255, 255), 1));
    painter.fillPath(drawMask,QBrush(QColor(0,0,0,160)));
    painter.drawRect(selectedRect);
}

void SelectRect::mousePressEvent(QMouseEvent *event)
{
        switch(event->button())
        {
        case Qt::LeftButton:
            mouseStatus = Qt::LeftButton;
            mouseLeftClickedPos = event->pos();
            if(isCursorPosInSelectedAreaFlag)
                this->setCursor(Qt::ClosedHandCursor);
            // 关闭鼠标追踪 节省资源
            this->setMouseTracking(false);
            break;
        case Qt::RightButton:
            mouseStatus = Qt::RightButton;
            break;
        case Qt::MiddleButton:
            mouseStatus = Qt::MiddleButton;
            break;
        default:
            mouseStatus = Qt::NoButton;
    }
}

void SelectRect::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseStatus == Qt::LeftButton )
    {
        if(isCursorPosInSelectedAreaFlag)
        {
            selectedRect.moveTo(lastSelectedRect.topLeft()+event->pos()-mouseLeftClickedPos);
        }
        else
        {
            // 限定在mask内
            selectedRect.setTopLeft(mouseLeftClickedPos);
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
            selectedRect.setWidth(x-selectedRect.x());
            selectedRect.setHeight(y-selectedRect.y());
        }
        update();
    }
    // 判断鼠标是否在矩形框内
    if(mouseStatus == Qt::NoButton)
    {
        if(selectedRect.contains(event->pos()))
        {
            this->setCursor(Qt::OpenHandCursor);
            isCursorPosInSelectedAreaFlag = true;
        }
        else
        {
            this->setCursor(Qt::ArrowCursor);
            isCursorPosInSelectedAreaFlag = false;
        }
    }
}

void SelectRect::mouseReleaseEvent(QMouseEvent *event)
{
    // 修正RectInfo::w和RectInfo::h为正
    fixRectInfo(selectedRect);
    // 备份
    lastSelectedRect = selectedRect;
    mouseStatus = Qt::NoButton;
    // 开启鼠标追踪
    this->setMouseTracking(true);
    this->setCursor(Qt::OpenHandCursor);
}

void SelectRect::contextMenuEvent(QContextMenuEvent *event)
{
    mMenu->exec(QCursor::pos());
    mouseStatus = Qt::NoButton;
}

QRect SelectRect::calculateRectInImage(const QImage *img, const QPoint &imgTopLeftPos, QRect rect)
{
    QRect returnRect;

    // 计算相对于图像内的坐标
    returnRect.setTopLeft(rect.topLeft()-imgTopLeftPos);
    returnRect.setSize(rect.size());

    // 限定截取范围在图像内 修正顶点
    if(returnRect.x() < 0)
    {
        returnRect.setWidth(rect.width()+returnRect.x());
        returnRect.setX(0);
    }
    if (returnRect.y() < 0)
    {
        returnRect.setHeight(rect.height()+returnRect.y());
        returnRect.setY(0);
    }
    if(returnRect.bottomRight().x() > img->width())
        returnRect.setLeft(img->width());
    if(returnRect.bottomRight().y() > img->height())
        returnRect.setBottom(img->height());
    return returnRect;
}

void SelectRect::selectExit()
{
    emit sendSelectModeExit();
    this->close();
}

void SelectRect::selectReset()
{
    selectedRect = QRect(0,0,0,0);
    lastSelectedRect = QRect(0,0,0,0);
    // 关闭鼠标追踪 节省资源
    this->setMouseTracking(false);
    this->setCursor(Qt::ArrowCursor);
    isCursorPosInSelectedAreaFlag = false;
    update();
}

void SelectRect::cropZoomedImage()
{
    fixedRectInImage = calculateRectInImage(zoomedImage,drawImageTopLeftPos,selectedRect);
    saveImage(zoomedImage,fixedRectInImage);
}

void SelectRect::cropOriginalImage()
{
    fixedRectInImage = calculateRectInImage(zoomedImage,drawImageTopLeftPos,selectedRect);
    int x2 = fixedRectInImage.bottomRight().x();
    int y2 = fixedRectInImage.bottomRight().y();
    fixedRectInImage.setX(int(round(double(fixedRectInImage.x())/double(zoomedImage->width())*double(image->width()))));
    fixedRectInImage.setY(int(round(double(fixedRectInImage.y())/double(zoomedImage->height())*double(image->height()))));
    fixedRectInImage.setWidth(int(round(double(x2)/double(zoomedImage->width())*double(image->width())))-fixedRectInImage.x());
    fixedRectInImage.setHeight(int(round(double(y2)/double(zoomedImage->height())*double(image->height())))-fixedRectInImage.y());
    saveImage(image,fixedRectInImage);
}

void SelectRect::saveImage(const QImage *img, QRect rect)
{
    // 接受到截取框信息
    if(isLoadImage)
    {
        if(rect.width() > 0 && rect.height() > 0)
        {
            QImage saveImageTemp = img->copy(rect);
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

void SelectRect::fixRectInfo(QRect &rect)
{
    if(rect.width() < 0)
    {
        rect.setWidth(-rect.width());
        rect.setX(rect.x()-rect.width());
    }
    if(rect.height() < 0)
    {
        rect.setHeight(-rect.height());
        rect.setY(rect.y()-rect.height());
    }
}

void SelectRect::receiveParentSizeChangedValue(int width, int height, int imageLeftTopPosX, int imageLeftTopPosY)
{
    this->setGeometry(0,0,width,height);
    drawImageTopLeftPos.setX(imageLeftTopPosX);
    drawImageTopLeftPos.setY(imageLeftTopPosY);
    update();
}

