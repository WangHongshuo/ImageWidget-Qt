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
    isImageLoad = false;

    // 初始化右键菜单
    subMenu = new QMenu();
    subActionReset = subMenu->addAction(tr("重选"));
    subActionSave = subMenu->addAction(tr("另存为"));
    subActionExit = subMenu->addAction(tr("退出"));
    connect(subActionExit,SIGNAL(triggered()),this,SLOT(selectExit()));
    connect(subActionSave,SIGNAL(triggered()),this,SLOT(crop()));
    connect(subActionReset,SIGNAL(triggered()),this,SLOT(selectReset()));
    // 关闭后释放资源
    this->setAttribute(Qt::WA_DeleteOnClose);
}

SelectRect::~SelectRect()
{
//    disconnect(this->parent(),SIGNAL(parent_widget_size_changed(int,int)),this,SLOT(receive_parent_size_changed_value(int,int)));
//    disconnect(this,SIGNAL(select_mode_exit()),this->parent(),SLOT(is_select_mode_exit()));
    image = NULL;
    delete subMenu;
    subMenu = NULL;
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
    selectArea.addRect(selectedRectInfo.x,selectedRectInfo.y,selectedRectInfo.w,selectedRectInfo.h);
    mask.addRect(this->geometry());
    QPainterPath drawMask =mask.subtracted(selectArea);
    QPainter paint(this);
    paint.setOpacity(0.5);
    paint.fillPath(drawMask,QBrush(Qt::black));

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
        selectedRectInfo.x = mouseLeftClickedPosX;
        selectedRectInfo.y = mouseLeftClickedPosY;
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
        selectedRectInfo.w = x - selectedRectInfo.x;
        selectedRectInfo.h = y - selectedRectInfo.y;

    }
    update();
}

void SelectRect::mouseReleaseEvent(QMouseEvent *event)
{
    fixedRectInfoInImage = fixRectInfoInImage(selectedRectInfo);
}

void SelectRect::contextMenuEvent(QContextMenuEvent *event)
{
    subMenu->exec(QCursor::pos());
}

rectInfo SelectRect::fixRectInfoInImage(rectInfo rect)
{
    rectInfo data;
    rectInfo returnRectInfo;
    data = rect;
    // 修正矩形坐标
    if(data.w < 0)
    {
        data.x += data.w;
        data.w = -data.w;
    }
    if(data.h < 0)
    {
        data.y += data.h;
        data.h = -data.h;
    }
    //        qDebug() << data.x << data.y << data.w << data.h;
    // 计算相对于图像内的坐标
    returnRectInfo.x = data.x - drawImageTopLeftPosX;
    returnRectInfo.y = data.y - drawImageTopLeftPosY;
    returnRectInfo.w = data.w;
    returnRectInfo.h = data.h;
    // 限定截取范围在图像内
    // 修正顶点
    if(returnRectInfo.x < 0)
    {
        returnRectInfo.w = data.w + returnRectInfo.x;
        returnRectInfo.x = 0;
    }
    if (returnRectInfo.y < 0)
    {
        returnRectInfo.h = data.h + returnRectInfo.y;
        returnRectInfo.y = 0;
    }
    if(returnRectInfo.x + data.w > image->width())
        returnRectInfo.w = image->width() - returnRectInfo.x;
    if(returnRectInfo.y + data.h > image->height())
        returnRectInfo.h = image->height() - returnRectInfo.y;
    return returnRectInfo;
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

void SelectRect::crop()
{
    cropImage(fixedRectInfoInImage);
}

void SelectRect::cropImage(rectInfo rect)
{
    // 接受到截取框信息
    if(isImageLoad)
    {
        if(rect.w > 0 && rect.h > 0)
        {
            QImage saveImageTemp = image->copy(rect.x,rect.y,rect.w,rect.h);
            QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                            QCoreApplication::applicationDirPath(),
                                                            tr("Images (*.png *.xpm *.jpg *.tiff *.bmp)"));
            if(!filename.isEmpty() || !filename.isNull())
                saveImageTemp.save(filename);
        }
        else
        {
            QMessageBox msgBox(QMessageBox::Critical,tr("错误"),tr("未选中图像!"));
            msgBox.exec();
        }
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Critical,tr("错误"),tr("未选中图像!"));
        msgBox.exec();
    }
}

void SelectRect::receiveParentSizeChangedValue(int width, int height, int imageLeftTopPosX, int imageLeftTopPosY)
{
    this->setGeometry(0,0,width,height);
    drawImageTopLeftPosX = imageLeftTopPosX;
    drawImageTopLeftPosY = imageLeftTopPosY;
    update();
}

