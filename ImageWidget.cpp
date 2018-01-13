#include "ImageWidget.h"
#include <iostream>
#include <QDebug>
#include <QCoreApplication>
#include "selectrect.h"

ImageWidget::ImageWidget(QWidget *parent):QWidget(parent)
{
    isLoadImage = false;
    isSelectMode = false;
    qImageZoomedImage = new QImage;
    initializeContextmenu();
}

ImageWidget::~ImageWidget()
{
    if(isLoadImage)
        qImageContainer = NULL;
    if(isImageCloned)
        delete qImageContainer;
    delete  mMenu;
    delete qImageZoomedImage;
}

void ImageWidget::setImageWithData(QImage img, bool resetImageWhenLoaded)
{
    if(!isImageCloned)
    {
        qImageContainer = new QImage;
        isImageCloned = true;
    }
    if(img.isNull())
    {
        isLoadImage = false;
        return;
    }
    else
    {
        *qImageContainer = img.copy();
//        qDebug() << qImageContainer->bits() << img.bits();
        isLoadImage = true;
        if(resetImageWhenLoaded)
            setDefaultParameters();
        update();
    }
}

void ImageWidget::setImageWithPointer(QImage *img, bool resetImageWhenLoaded)
{
    if(img->isNull())
    {
        isLoadImage = false;
        return;
    }
    else
    {
        qImageContainer = img;
        isLoadImage = true;
        if(resetImageWhenLoaded)
            setDefaultParameters();
        update();
    }
}

void ImageWidget::clear()
{
    if(isLoadImage)
    {
        isLoadImage = false;
        if(isImageCloned)
            delete qImageContainer;
        else
            qImageContainer = NULL;
        update();
    }
}


void ImageWidget::setOnlyShowImage(bool flag)
{
    isOnlyShowImage = flag;
}

void ImageWidget::setEnableDragImage(bool flag)
{
    isEnableDragImage = flag;
    mActionEnableDrag->setChecked(isEnableDragImage);
}

void ImageWidget::setEnableZoomImage(bool flag)
{
    isEnableZoomImage = flag;
    mActionEnableZoom->setChecked(isEnableZoomImage);
}

void ImageWidget::setEnableImageFitWidget(bool flag)
{
    isEnableFitWidget = flag;
    mActionImageFitWidget->setChecked(isEnableFitWidget);
    resetImageWidget();
    update();
}

void ImageWidget::setEnableSendLeftClickedPos(bool flag)
{
    isEnableSendLeftClickedPos = flag;
}

void ImageWidget::setEnableSendLeftClickedPosInImage(bool flag)
{
    isEnableSendLeftClickedPosInImage = flag;
}


void ImageWidget::wheelEvent(QWheelEvent *e)
{
    if(isLoadImage && !isSelectMode && !isOnlyShowImage && isEnableZoomImage)
    {
        int numDegrees = e->delta();
        if(numDegrees > 0)
        {
            imageZoomOut();
        }
        if(numDegrees < 0)
        {
            imageZoomIn();
        }
        update();
    }
}

void ImageWidget::mousePressEvent(QMouseEvent *e)
{
    if(isLoadImage && !isOnlyShowImage)
    {
        switch(e->button())
        {
        case Qt::LeftButton:
            mouseStatus = MOUSE_LEFT;
            mouseLeftClickedPosX = e->x();
            mouseLeftClickedPosY = e->y();
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
    if(e->button() == Qt::LeftButton)
        emitLeftClickedSignals(e);

}

void ImageWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(isLoadImage && !isOnlyShowImage && isEnableDragImage)
    {
        if(mouseStatus == MOUSE_LEFT)
        {
            // 记录上次图像顶点
            drawImageTopLeftLastPosX = drawImageTopLeftPosX;
            drawImageTopLeftLastPosY = drawImageTopLeftPosY;
            // 释放后鼠标状态置No
            mouseStatus = MOUSE_NO;
        }
    }
}


void ImageWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(isLoadImage && !isOnlyShowImage && isEnableDragImage)
    {
        if(mouseStatus == MOUSE_LEFT )
        {
            // e->x()和e->y()为当前鼠标坐标 转换为相对移动距离
            //        qDebug() << e->x() << e->y();
            getDrawImageTopLeftPos(e->x()-mouseLeftClickedPosX,
                                   e->y()-mouseLeftClickedPosY);
        }
    }
}

void ImageWidget::getDrawImageTopLeftPos(int x,int y)
{
    if(isEnableDragImage)
    {
        drawImageTopLeftPosX = drawImageTopLeftLastPosX+x;
        drawImageTopLeftPosY = drawImageTopLeftLastPosY+y;
        update();
    }
}

void ImageWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setBrush(QBrush(QColor(200,200,200)));
    painter.drawRect(0,0,this->width(),this->height());
    if(!isLoadImage)
        return;
    if(isEnableFitWidget)
        *qImageZoomedImage = qImageContainer->scaled(this->width()*zoomScaleX,this->height()*zoomScaleY,Qt::KeepAspectRatio);
    else
        *qImageZoomedImage = qImageContainer->scaled(qImageContainer->width()*zoomScaleX,qImageContainer->height()*zoomScaleY,Qt::KeepAspectRatio);
    painter.drawImage(QPoint(drawImageTopLeftPosX,drawImageTopLeftPosY),*qImageZoomedImage);
    //    qDebug() << zoomScaleX;
}

void ImageWidget::contextMenuEvent(QContextMenuEvent *e)
{
    if(!isOnlyShowImage)
    {
        mMenu->exec(QCursor::pos());
        // 右键菜单弹出后 鼠标状态置No
        mouseStatus = MOUSE_NO;
    }
}

void ImageWidget::resizeEvent(QResizeEvent *event)
{
    if(isSelectMode && isLoadImage)
        emit parentWidgetSizeChanged(this->width(),this->height());
}

void ImageWidget::resetImageWidget()
{
    setDefaultParameters();
    update();
}

void ImageWidget::imageZoomOut()
{
    if(zoomScaleX <= 8 && zoomScaleY <= 8)
    {
        zoomScaleX *= 1.1;
        zoomScaleY *= 1.1;
    }
    update();
}

void ImageWidget::imageZoomIn()
{
    if(zoomScaleX >= 0.05 && zoomScaleY >= 0.05)
    {
        zoomScaleX *= 1.0/1.1;
        zoomScaleY *= 1.0/1.1;
    }
    update();
}

void ImageWidget::select()
{
    if(isLoadImage)
    {
        isSelectMode = true;
        SelectRect* m = new SelectRect(this);
        m->setGeometry(0,0,this->geometry().width(),this->geometry().height());
        connect(m,SIGNAL(sendSelectModeExit()),this,SLOT(selectModeExit()));
        connect(this,SIGNAL(parentWidgetSizeChanged(int,int)),
                m,SLOT(receiveParentSizeChangedValue(int,int)));
        m->setImage(qImageZoomedImage,drawImageTopLeftPosX,drawImageTopLeftPosY);
        m->show();
    }
}


void ImageWidget::initializeContextmenu()
{
    mMenu = new QMenu();
    mActionResetPos = mMenu->addAction(tr("重置"));
    mActionSave = mMenu->addAction(tr("另存为"));
    mActionSelect = mMenu->addAction(tr("截取"));
    mActionEnableDrag = mMenu->addAction(tr("启用拖拽"));
    mActionEnableZoom = mMenu->addAction(tr("启用缩放"));
    mActionImageFitWidget = mMenu->addAction(tr("自适应大小"));

    mActionEnableDrag->setCheckable(true);
    mActionEnableZoom->setCheckable(true);
    mActionImageFitWidget->setCheckable(true);

    mActionEnableDrag->setChecked(isEnableDragImage);
    mActionEnableZoom->setChecked(isEnableZoomImage);
    mActionImageFitWidget->setChecked(isEnableFitWidget);

    connect(mActionResetPos,SIGNAL(triggered()),this,SLOT(resetImageWidget()));
    connect(mActionSave,SIGNAL(triggered()),this,SLOT(save()));
    connect(mActionSelect,SIGNAL(triggered()),this,SLOT(select()));
    connect(mActionEnableDrag,SIGNAL(toggled(bool)),this,SLOT(setEnableDragImage(bool)));
    connect(mActionEnableZoom,SIGNAL(toggled(bool)),this,SLOT(setEnableZoomImage(bool)));
    connect(mActionImageFitWidget,SIGNAL(toggled(bool)),this,SLOT(setEnableImageFitWidget(bool)));
}

void ImageWidget::emitLeftClickedSignals(QMouseEvent *e)
{
    if(isEnableSendLeftClickedPos)
        emit sendLeftClickedPos(e->x(),e->y());

    if(isEnableSendLeftClickedPosInImage)
    {
        if(!isLoadImage)
            emit sendLeftClickedPosInImage(-1,-1);
        else
        {
            int leftClickedPosXInZoomedImage = e->x()-drawImageTopLeftPosX;
            int leftClickedPosYInZoomedImage = e->y()-drawImageTopLeftPosY;
            // 点不在图像上
            if(leftClickedPosXInZoomedImage < 0 ||
                    leftClickedPosYInZoomedImage < 0 ||
                    leftClickedPosXInZoomedImage > qImageZoomedImage->width() ||
                    leftClickedPosYInZoomedImage > qImageZoomedImage->height())
            {
                emit sendLeftClickedPosInImage(-1,-1);
                return;
            }
            // 点在图像上，根据缩放计算出点在原图上的位置
            else
            {
                double xDivZoomedImageW = double(leftClickedPosXInZoomedImage)/double(qImageZoomedImage->width());
                double yDivZoomedImageH = double(leftClickedPosYInZoomedImage)/double(qImageZoomedImage->height());
                int posXInImage = int(double(qImageContainer->width())*xDivZoomedImageW);
                int posYInImage = int(double(qImageContainer->height())*yDivZoomedImageH);
                emit sendLeftClickedPosInImage(posXInImage,posYInImage);
            }
        }
    }
}

void ImageWidget::save()
{
    if(isLoadImage)
    {
        QImage temp = qImageContainer->copy();
        QString filename = QFileDialog::getSaveFileName(this, tr("Open File"),
                                                        QCoreApplication::applicationDirPath(),
                                                        tr("Images (*.png *.xpm *.jpg *.tiff *.bmp)"));
        if(!filename.isEmpty() || !filename.isNull())
            temp.save(filename);
    }
}

void ImageWidget::selectModeExit()
{
    isSelectMode = false;
}

void ImageWidget::setDefaultParameters()
{
    zoomScaleX = zoomScaleY = 1.0;
    drawImageTopLeftPosX = drawImageTopLeftLastPosX = 0;
    drawImageTopLeftPosY = drawImageTopLeftLastPosY = 0;
}
