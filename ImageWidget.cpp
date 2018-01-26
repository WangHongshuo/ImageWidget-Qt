// UTF-8 with BOM

// Avoid gibberish when use MSVC
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

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
    if(isImageCloned)
        delete qImageContainer;
    if(isLoadImage)
        qImageContainer = NULL;
    delete qImageZoomedImage;
    qImageZoomedImage = NULL;
}

void ImageWidget::setImageWithData(QImage img)
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
        *qImageZoomedImage = img.copy();
        updateZoomedImage();
        if(!isEnableRecordLastParameters)
            setDefaultParameters();
        update();
    }
}

void ImageWidget::setImageWithPointer(QImage *img)
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
        *qImageZoomedImage = img->copy();
        updateZoomedImage();
        if(!isEnableRecordLastParameters)
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
        {
            delete qImageContainer;
            qImageContainer = NULL;
        }
        else
            qImageContainer = NULL;
        resetImageWidget();
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
}

void ImageWidget::setEnableRecordLastParameters(bool flag)
{
    isEnableRecordLastParameters = flag;
    mActionRecordLastParameters->setChecked(isEnableRecordLastParameters);
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
        updateZoomedImage();
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
            mouseStatus = Qt::LeftButton;
            mouseLeftClickedPos = e->pos();
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
    if(e->button() == Qt::LeftButton)
        emitLeftClickedSignals(e);

}

void ImageWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(isLoadImage && !isOnlyShowImage && isEnableDragImage)
    {
        if(mouseStatus == Qt::LeftButton)
        {
            // 记录上次图像顶点
            drawImageTopLeftLastPos = drawImageTopLeftPos;
            calculateImageLeftTopRelativePosInWidget(drawImageTopLeftPos.x(),drawImageTopLeftPos.y(),
                                                     imageLeftTopRelativePosInWdigetX,
                                                     imageLeftTopRelativePosInWdigetY);
            // 释放后鼠标状态置No
            mouseStatus = Qt::NoButton;
        }
    }
}


void ImageWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(isLoadImage && !isOnlyShowImage && isEnableDragImage)
    {
        if(mouseStatus == Qt::LeftButton )
        {
            // e->pos()为当前鼠标坐标 转换为相对移动距离
            //        qDebug() << e->x() << e->y();
            getDrawImageTopLeftPos(e->pos()-mouseLeftClickedPos);
        }
    }
}

void ImageWidget::getDrawImageTopLeftPos(QPoint xy)
{
    if(isEnableDragImage)
    {
        drawImageTopLeftPos = drawImageTopLeftLastPos+xy;
        update();
    }
}

void ImageWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setBrush(QBrush(QColor(200,200,200)));
    painter.drawRect(0,0,this->width(),this->height());
    if(!qImageZoomedImage->isNull())
        painter.drawImage(drawImageTopLeftPos,*qImageZoomedImage);
}

void ImageWidget::contextMenuEvent(QContextMenuEvent *e)
{
    if(!isOnlyShowImage)
    {
        mMenu->exec(QCursor::pos());
        // 右键菜单弹出后 鼠标状态置No
        mouseStatus = Qt::NoButton;
    }
}

void ImageWidget::resizeEvent(QResizeEvent *event)
{
    if(isEnableFitWidget && isLoadImage)
    {
        *qImageZoomedImage = qImageContainer->scaled(this->width()*zoomScaleX,this->height()*zoomScaleY,Qt::KeepAspectRatio);
        // 如果图像没有被拖拽或者缩放过 置中
        if((drawImageTopLeftPos.x() == 0 || drawImageTopLeftPos.y() == 0) &&
                qImageZoomedImage->width() <= this->width() &&
                qImageZoomedImage->height() <= this->height())
        {
            setImageInCenter();
        }
        else
        {
            drawImageTopLeftPos.setX(int(double(this->width())*imageLeftTopRelativePosInWdigetX));
            drawImageTopLeftPos.setY(int(double(this->height())*imageLeftTopRelativePosInWdigetY));
            drawImageTopLeftLastPos = drawImageTopLeftPos;
        }
    }
    if(isSelectMode && isLoadImage)
        emit parentWidgetSizeChanged(this->width(),this->height(),drawImageTopLeftPos.x(),drawImageTopLeftPos.y());
}

void ImageWidget::resetImageWidget()
{
    setDefaultParameters();
    updateZoomedImage();
    update();
}

void ImageWidget::imageZoomOut()
{
    if(zoomScaleX <= 8 && zoomScaleY <= 8)
    {
        zoomScaleX *= 1.1;
        zoomScaleY *= 1.1;
        isZoomedParametersChanged = true;
    }  
}

void ImageWidget::imageZoomIn()
{
    if(zoomScaleX >= 0.05 && zoomScaleY >= 0.05)
    {
        zoomScaleX *= 1.0/1.1;
        zoomScaleY *= 1.0/1.1;
        isZoomedParametersChanged = true;
    }  
}

void ImageWidget::select()
{
    if(isLoadImage)
    {
        isSelectMode = true;
        SelectRect* m = new SelectRect(this);
        m->setGeometry(0,0,this->geometry().width(),this->geometry().height());
        connect(m,SIGNAL(sendSelectModeExit()),this,SLOT(selectModeExit()));
        connect(this,SIGNAL(parentWidgetSizeChanged(int,int,int,int)),
                m,SLOT(receiveParentSizeChangedValue(int,int,int,int)));
        m->setImage(qImageContainer,qImageZoomedImage,drawImageTopLeftPos.x(),drawImageTopLeftPos.y());
        m->show();
    }
}


void ImageWidget::initializeContextmenu()
{
    mMenu = new QMenu(this);
    mMenuAdditionalFunction = new QMenu(mMenu);

    mActionResetParameters = mMenu->addAction(tr("重置"));// Reset
    mActionSave = mMenu->addAction(tr("另存为")); // Save As
    mActionSelect = mMenu->addAction(tr("截取")); // Crop
    mMenuAdditionalFunction = mMenu->addMenu(tr("更多功能")); // More Function
    mActionEnableDrag = mMenuAdditionalFunction->addAction(tr("启用拖拽")); // Enable Drag
    mActionEnableZoom = mMenuAdditionalFunction->addAction(tr("启用缩放"));// Enable Zoom
    mActionImageFitWidget = mMenuAdditionalFunction->addAction(tr("自适应大小"));// Enable Image Fit Widget
    mActionRecordLastParameters = mMenuAdditionalFunction->addAction(tr("记住上次参数"));// Record Last Parameters (such as Scale, Image Position)

    mActionEnableDrag->setCheckable(true);
    mActionEnableZoom->setCheckable(true);
    mActionImageFitWidget->setCheckable(true);
    mActionRecordLastParameters->setCheckable(true);

    mActionEnableDrag->setChecked(isEnableDragImage);
    mActionEnableZoom->setChecked(isEnableZoomImage);
    mActionImageFitWidget->setChecked(isEnableFitWidget);
    mActionRecordLastParameters->setChecked(isEnableRecordLastParameters);

    connect(mActionResetParameters,SIGNAL(triggered()),this,SLOT(resetImageWidget()));
    connect(mActionSave,SIGNAL(triggered()),this,SLOT(save()));
    connect(mActionSelect,SIGNAL(triggered()),this,SLOT(select()));
    connect(mActionEnableDrag,SIGNAL(toggled(bool)),this,SLOT(setEnableDragImage(bool)));
    connect(mActionEnableZoom,SIGNAL(toggled(bool)),this,SLOT(setEnableZoomImage(bool)));
    connect(mActionImageFitWidget,SIGNAL(toggled(bool)),this,SLOT(setEnableImageFitWidget(bool)));
    connect(mActionRecordLastParameters,SIGNAL(toggled(bool)),this,SLOT(setEnableRecordLastParameters(bool)));
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
            QPoint cursorPosInImage = calculateCursorPosInImage(qImageContainer,qImageZoomedImage,drawImageTopLeftPos,e->pos());
            // 如果光标不在图像上则返回-1
            if(cursorPosInImage.x() < 0 || cursorPosInImage.y() < 0 ||
               cursorPosInImage.x() > qImageContainer->width()-1 ||
               cursorPosInImage.y() > qImageContainer->height()-1)
            {
                cursorPosInImage.setX(-1);
                cursorPosInImage.setY(-1);
            }
            emit sendLeftClickedPosInImage(cursorPosInImage.x(),cursorPosInImage.y());
        }
    }
}

QPoint ImageWidget::calculateCursorPosInImage(const QImage *originalImage,const QImage *zoomedImage,const QPoint &imageLeftTopPos, QPoint cursorPos)
{
    // 计算当前光标在原始图像坐标系中相对于图像原点的位置
    QPoint returnPoint;
    int distanceX = cursorPos.x()-imageLeftTopPos.x();
    int distanceY = cursorPos.y()-imageLeftTopPos.y();
    double xDivZoomedImageW = double(distanceX)/double(zoomedImage->width());
    double yDivZoomedImageH = double(distanceY)/double(zoomedImage->height());
    returnPoint.setX(int(double(originalImage->width())*xDivZoomedImageW));
    returnPoint.setY(int(double(originalImage->height())*yDivZoomedImageH));
    return returnPoint;
}

QPoint ImageWidget::calculateCursorPosInZoomedImage(QPoint cursorPos)
{
    // 计算当前光标在缩放后图像坐标系中相对于图像原点的位置
    return cursorPos - drawImageTopLeftLastPos;
}

void ImageWidget::calculateImageLeftTopRelativePosInWidget(const int x, const int y, double &returnX, double &returnY)
{
    returnX = double(x)/double(this->width());
    returnY = double(y)/double(this->height());
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

void ImageWidget::updateZoomedImage()
{
    QPoint cursorPosInWidget, cursorPosInImage;
    // 图像为空直接返回
    if(!isLoadImage)
        return;

    if(isZoomedParametersChanged)
    {
        lastZoomedImageWidth = qImageZoomedImage->width();
        lastZoomedImageHeight = qImageZoomedImage->height();
        // 获取当前光标并计算出光标在图像中的位置
       cursorPosInWidget = this->mapFromGlobal(QCursor::pos());
       cursorPosInImage = calculateCursorPosInImage(qImageContainer,qImageZoomedImage,drawImageTopLeftPos,cursorPosInWidget);
    }

    // 减少拖动带来的QImage::scaled
    if(isEnableFitWidget)
        *qImageZoomedImage = qImageContainer->scaled(this->width()*zoomScaleX,this->height()*zoomScaleY,Qt::KeepAspectRatio);
    else
        *qImageZoomedImage = qImageContainer->scaled(qImageContainer->width()*zoomScaleX,qImageContainer->height()*zoomScaleY,Qt::KeepAspectRatio);

    if(isZoomedParametersChanged)
    {
        int zoomedImageChangedWith = lastZoomedImageWidth - qImageZoomedImage->width();
        int zoomedImageChangedHeight = lastZoomedImageHeight - qImageZoomedImage->height();

        // 根据光标在图像的位置进行调整左上绘图点位置
        drawImageTopLeftPos += QPoint(int(double(zoomedImageChangedWith)*double(cursorPosInImage.x())/double(qImageContainer->width()-1)),
                                      int(double(zoomedImageChangedHeight)*double(cursorPosInImage.y())/double(qImageContainer->height()-1)));

        drawImageTopLeftLastPos = drawImageTopLeftPos;

        calculateImageLeftTopRelativePosInWidget(drawImageTopLeftPos.x(),drawImageTopLeftPos.y(),
                                                 imageLeftTopRelativePosInWdigetX,
                                                 imageLeftTopRelativePosInWdigetY);
        isZoomedParametersChanged = false;
    }
}

void ImageWidget::setDefaultParameters()
{
    zoomScaleX = zoomScaleY = 1.0;

    if(isEnableFitWidget && isLoadImage)
    {
        // 首先恢复zoomedImage大小再调整drawPos
        updateZoomedImage();
        setImageInCenter();
        calculateImageLeftTopRelativePosInWidget(drawImageTopLeftPos.x(),drawImageTopLeftPos.y(),
                                                 imageLeftTopRelativePosInWdigetX,
                                                 imageLeftTopRelativePosInWdigetY);
    }
    else
    {
        drawImageTopLeftPos = drawImageTopLeftLastPos = QPoint(0,0);
        imageLeftTopRelativePosInWdigetX = 0.0;
        imageLeftTopRelativePosInWdigetY = 0.0;
    }
}

void ImageWidget::setImageInCenter()
{
    if(this->width()>qImageZoomedImage->width())
        drawImageTopLeftPos.setX(int(double(this->width()-qImageZoomedImage->width())*0.5));
    else
        drawImageTopLeftPos.setX(0);

    if(this->height()>qImageZoomedImage->height())
        drawImageTopLeftPos.setY(int(double(this->height()-qImageZoomedImage->height())*0.5));
    else
        drawImageTopLeftPos.setY(0);
    drawImageTopLeftLastPos = drawImageTopLeftPos;
}
