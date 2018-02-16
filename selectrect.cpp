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
    this->setFocusPolicy(Qt::StrongFocus);
}

SelectRect::~SelectRect()
{
//    disconnect(this->parent(),SIGNAL(parent_widget_size_changed(int,int)),this,SLOT(receive_parent_size_changed_value(int,int)));
//    disconnect(this,SIGNAL(select_mode_exit()),this->parent(),SLOT(is_select_mode_exit()));
    image = NULL;
    zoomedImage = NULL;
    mMenu = NULL;
}

void SelectRect::receiveParentSizeChangedSignal()
{
    ImageWidget *parentWidget = static_cast<ImageWidget *>(this->parent());
    this->setGeometry(0,0,parentWidget->width(),parentWidget->height());
    qDebug() << this->geometry();
    drawImageTopLeftPos = parentWidget->getDrawImageTopLeftPos();
    update();
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
    selectArea.addRect(selectedRect[SR_CENTER].x(),selectedRect[SR_CENTER].y(),selectedRect[SR_CENTER].width(),selectedRect[SR_CENTER].height());
    mask.addRect(this->geometry());
    QPainterPath drawMask =mask.subtracted(selectArea);
    QPainter painter(this);
    painter.setPen(QPen(QColor(255, 0, 0, 255), 1));
    painter.fillPath(drawMask,QBrush(QColor(0,0,0,160)));
    painter.drawRect(selectedRect[SR_CENTER]);
    if(isSelectedRectStable)
    {
        painter.setPen(QPen(QColor(0, 140, 255, 255), 1));
        painter.drawRect(selectedRect[SR_CENTER]);
    for(int i=1;i<5;i++)
        painter.fillRect(selectedRect[i],QBrush(QColor(0,140,255,255)));
    }
}

void SelectRect::mousePressEvent(QMouseEvent *event)
{
        switch(event->button())
        {
        case Qt::LeftButton:
            mouseStatus = Qt::LeftButton;
            mouseLeftClickedPos = event->pos();
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
        isSelectedRectStable = false;
        isSelectedRectExisted = true;
        selectedRectChangeEvent(cursorPosInSelectedArea,event->pos());
        update();
    }
    // 判断鼠标是否在矩形框内
    if(mouseStatus == Qt::NoButton && isSelectedRectStable)
    {
        if(selectedRect[SR_ENTIRETY].contains(event->pos()))
        {
            cursorPosInSelectedArea = getSelectedAreaSubscript(event->pos());
        }
        else
        {
            cursorPosInSelectedArea = SR_NULL;
            this->setCursor(Qt::ArrowCursor);
        }
    }
}

void SelectRect::mouseReleaseEvent(QMouseEvent *event)
{
    if(mouseStatus == Qt::LeftButton)
    {
        // 修正RectInfo::w和RectInfo::h为正
        fixRectInfo(selectedRect[SR_CENTER]);
        // 备份
        lastSelectedRect = selectedRect[SR_CENTER];
        mouseStatus = Qt::NoButton;
        getEdgeRect();
        isSelectedRectStable = true;
        isSelectedRectExisted = true;
        update();
        // 开启鼠标追踪
        this->setMouseTracking(true);
        mouseStatus = Qt::NoButton;
    }
}

void SelectRect::contextMenuEvent(QContextMenuEvent *event)
{
    mMenu->exec(QCursor::pos());
    mouseStatus = Qt::NoButton;
}

void SelectRect::wheelEvent(QWheelEvent *event)
{
   eventFilter(this,event);
}

bool SelectRect::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type()==QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape)
        {
            // 如果存在选中框则删除选中框 不存在则退出
            if(isSelectedRectExisted)
            {
                selectReset();
                return true;
            }
            else
                this->selectExit();
        }

    }
    // 截断wheelEvent
    if(event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        wheelEvent->accept();
        return true;
    }
    return false;
}

void SelectRect::keyPressEvent(QKeyEvent *event)
{
    eventFilter(this,event);
}

QRect SelectRect::getRectInImage(const QImage *img, const QPoint &imgTopLeftPos, QRect rect)
{
    QRect returnRect;
    // QRect::width(height) = QRect::left(top)-QRect::right(bottom)
    // 计算相对于图像内的坐标
    returnRect.setTopLeft(rect.topLeft()-imgTopLeftPos);
    returnRect.setSize(rect.size());
    // 限定截取范围在图像内 修正顶点
    if(returnRect.x() < 0)
    {
        // QRect::setX change the width
        returnRect.setX(0);
    }
    if (returnRect.y() < 0)
    {
        // QRect::setY change the height
        returnRect.setY(0);
    }
    if(returnRect.bottomRight().x() >= img->width())
        // QRect::setRight change the width
        returnRect.setRight(img->width()-1);
    if(returnRect.bottomRight().y() >= img->height())
        // Qrect::setBottom change the height
        returnRect.setBottom(img->height()-1);
    return returnRect;
}

void SelectRect::getEdgeRect()
{
    // 边框宽
    int w = 5;

    selectedRect[SR_TOPLEFT].setTopLeft(selectedRect[SR_CENTER].topLeft()+QPoint(-w,-w));
    selectedRect[SR_TOPLEFT].setBottomRight(selectedRect[SR_CENTER].topLeft()+QPoint(-1,-1));

    selectedRect[SR_TOPRIGHT].setTopRight(selectedRect[SR_CENTER].topRight()+QPoint(w,-w));
    selectedRect[SR_TOPRIGHT].setBottomLeft(selectedRect[SR_CENTER].topRight()+QPoint(1,-1));

    selectedRect[SR_BOTTOMRIGHT].setTopLeft(selectedRect[SR_CENTER].bottomRight()+QPoint(1,1));
    selectedRect[SR_BOTTOMRIGHT].setBottomRight(selectedRect[SR_CENTER].bottomRight()+QPoint(w,w));

    selectedRect[SR_BOTTOMLEFT].setTopRight(selectedRect[SR_CENTER].bottomLeft()+QPoint(-1,1));
    selectedRect[SR_BOTTOMLEFT].setBottomLeft(selectedRect[SR_CENTER].bottomLeft()+QPoint(-w,w));

    selectedRect[SR_TOP].setTopLeft(selectedRect[SR_TOPLEFT].topRight()+QPoint(1,0));
    selectedRect[SR_TOP].setBottomRight(selectedRect[SR_TOPRIGHT].bottomLeft()+QPoint(-1,0));

    selectedRect[SR_RIGHT].setTopLeft(selectedRect[SR_TOPRIGHT].bottomLeft()+QPoint(0,1));
    selectedRect[SR_RIGHT].setBottomRight(selectedRect[SR_BOTTOMRIGHT].topRight()+QPoint(0,-1));

    selectedRect[SR_BOTTOM].setTopLeft(selectedRect[SR_BOTTOMLEFT].topRight()+QPoint(1,0));
    selectedRect[SR_BOTTOM].setBottomRight(selectedRect[SR_BOTTOMRIGHT].bottomLeft()+QPoint(-1,0));

    selectedRect[SR_LEFT].setTopLeft(selectedRect[SR_TOPLEFT].bottomLeft()+QPoint(0,1));
    selectedRect[SR_LEFT].setBottomRight(selectedRect[SR_BOTTOMLEFT].topRight()+QPoint(0,-1));

    selectedRect[SR_ENTIRETY].setTopLeft(selectedRect[SR_TOPLEFT].topLeft());
    selectedRect[SR_ENTIRETY].setBottomRight(selectedRect[SR_BOTTOMRIGHT].bottomRight());
}

int SelectRect::getSelectedAreaSubscript(QPoint cursorPos)
{
    // 可用树结构减少if
    if(selectedRect[SR_CENTER].contains(cursorPos))
    {
        this->setCursor(Qt::SizeAllCursor);
        return SR_CENTER;
    }
    if(selectedRect[SR_TOPLEFT].contains(cursorPos))
    {
        this->setCursor(Qt::SizeFDiagCursor);
        return SR_TOPLEFT;
    }
    if(selectedRect[SR_TOP].contains(cursorPos))
    {
        this->setCursor(Qt::SizeVerCursor);
        return SR_TOP;
    }
    if(selectedRect[SR_TOPRIGHT].contains(cursorPos))
    {
        this->setCursor(Qt::SizeBDiagCursor);
        return SR_TOPRIGHT;
    }
    if(selectedRect[SR_RIGHT].contains(cursorPos))
    {
        this->setCursor(Qt::SizeHorCursor);
        return SR_RIGHT;
    }
    if(selectedRect[SR_BOTTOMRIGHT].contains(cursorPos))
    {
        this->setCursor(Qt::SizeFDiagCursor);
        return SR_BOTTOMRIGHT;
    }
    if(selectedRect[SR_BOTTOM].contains(cursorPos))
    {
        this->setCursor(Qt::SizeVerCursor);
        return SR_BOTTOM;
    }
    if(selectedRect[SR_BOTTOMLEFT].contains(cursorPos))
    {
        this->setCursor(Qt::SizeBDiagCursor);
        return SR_BOTTOMLEFT;
    }
    if(selectedRect[SR_LEFT].contains(cursorPos))
    {
        this->setCursor(Qt::SizeHorCursor);
        return SR_LEFT;
    }
    return SR_NULL;
}

void SelectRect::selectedRectChangeEvent(int SR_LOCATION, const QPoint &cursorPos)
{
    int x = cursorPos.x();
    int y = cursorPos.y();
    switch (SR_LOCATION)
    {
    case SR_NULL:
        // 限定在mask内
        selectedRect[SR_CENTER].setTopLeft(mouseLeftClickedPos);
        if(x < 0)
            x = 0;
        else if(x > this->width())
            x = this->width();
        if(y < 0)
            y = 0;
        else if (y > this->height())
            y = this->height();
        selectedRect[SR_CENTER].setWidth(x-selectedRect[SR_CENTER].x());
        selectedRect[SR_CENTER].setHeight(y-selectedRect[SR_CENTER].y());
        fixRectInfo(selectedRect[SR_CENTER]);
        break;
    case SR_CENTER:
        selectedRect[SR_CENTER].moveTo(lastSelectedRect.topLeft()+(cursorPos-mouseLeftClickedPos));
        break;
    case SR_TOPLEFT:
        selectedRect[SR_CENTER].setTopLeft(lastSelectedRect.topLeft()+(cursorPos-mouseLeftClickedPos));
        break;
    case SR_TOP:
        selectedRect[SR_CENTER].setTop(lastSelectedRect.top()+(cursorPos.y()-mouseLeftClickedPos.y()));
        break;
    case SR_TOPRIGHT:
        selectedRect[SR_CENTER].setTopRight(lastSelectedRect.topRight()+(cursorPos-mouseLeftClickedPos));
        break;
    case SR_RIGHT:
        selectedRect[SR_CENTER].setRight(lastSelectedRect.right()+(cursorPos.x()-mouseLeftClickedPos.x()));
        break;
    case SR_BOTTOMRIGHT:
        selectedRect[SR_CENTER].setBottomRight(lastSelectedRect.bottomRight()+(cursorPos-mouseLeftClickedPos));
        break;
    case SR_BOTTOM:
        selectedRect[SR_CENTER].setBottom(lastSelectedRect.bottom()+(cursorPos.y()-mouseLeftClickedPos.y()));
        break;
    case SR_BOTTOMLEFT:
        selectedRect[SR_CENTER].setBottomLeft(lastSelectedRect.bottomLeft()+(cursorPos-mouseLeftClickedPos));
        break;
    case SR_LEFT:
        selectedRect[SR_CENTER].setLeft(lastSelectedRect.left()+(cursorPos.x()-mouseLeftClickedPos.x()));
        break;
    default:
        break;
    }
}

void SelectRect::selectExit()
{
    emit sendSelectModeExit();
    this->close();
}

void SelectRect::selectReset()
{
    selectedRect[SR_CENTER] = QRect(0,0,0,0);
    lastSelectedRect = QRect(0,0,0,0);

    isSelectedRectStable = false;
    isSelectedRectExisted = false;
    // 关闭鼠标追踪 节省资源
    this->setMouseTracking(false);
    this->setCursor(Qt::ArrowCursor);
    cursorPosInSelectedArea = SR_NULL;
    mouseStatus = Qt::NoButton;
    update();
}

void SelectRect::cropZoomedImage()
{
    fixedRectInImage = getRectInImage(zoomedImage,drawImageTopLeftPos,selectedRect[SR_CENTER]);
    saveImage(zoomedImage,fixedRectInImage);
}

void SelectRect::cropOriginalImage()
{
    fixedRectInImage = getRectInImage(zoomedImage,drawImageTopLeftPos,selectedRect[SR_CENTER]);
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
    QPoint topLeft = rect.topLeft();
    int width = rect.width();
    int height = rect.height();
    if(width < 0)
    {
        topLeft.setX(topLeft.x()+width);
        width = -width;
    }
    if(height < 0)
    {
        topLeft.setY(topLeft.y()+height);
        height = -height;
    }
    rect.setTopLeft(topLeft);
    rect.setSize(QSize(width,height));
}

