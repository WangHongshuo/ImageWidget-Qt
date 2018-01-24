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
    zoomedImage = NULL;
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
    selectArea.addRect(selectedRectInfo.x1,selectedRectInfo.y1,selectedRectInfo.w,selectedRectInfo.h);
    mask.addRect(this->geometry());
    QPainterPath drawMask =mask.subtracted(selectArea);

    QPainter painter(this);
    painter.setPen(QPen(QColor(0, 140, 255, 255), 2));
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
    fixedRectInfoInImage = fixRectInfoInImage(selectedRectInfo);
//    qDebug() << fixedRectInfoInImage.x1 << fixedRectInfoInImage.w << fixedRectInfoInImage.x2();
//    qDebug() << fixedRectInfoInImage.y1 << fixedRectInfoInImage.h << fixedRectInfoInImage.y2();
    mouseStatus = MOUSE_NO;
}

void SelectRect::contextMenuEvent(QContextMenuEvent *event)
{
    subMenu->exec(QCursor::pos());
    mouseStatus = MOUSE_NO;
}

RectInfo SelectRect::fixRectInfoInImage(RectInfo rect)
{
    RectInfo data;
    RectInfo returnRectInfo;
    data = rect;
    // 修正矩形坐标
    if(data.w < 0)
    {
        data.x1 += data.w;
        data.w = -data.w;
    }
    if(data.h < 0)
    {
        data.y1 += data.h;
        data.h = -data.h;
    }
    //        qDebug() << data.x << data.y << data.w << data.h;
    // 计算相对于图像内的坐标
    returnRectInfo.x1 = data.x1 - drawImageTopLeftPosX;
    returnRectInfo.y1 = data.y1 - drawImageTopLeftPosY;
    returnRectInfo.w = data.w;
    returnRectInfo.h = data.h;
    // 限定截取范围在图像内
    // 修正顶点
    if(returnRectInfo.x1 < 0)
    {
        returnRectInfo.w = data.w + returnRectInfo.x1;
        returnRectInfo.x1 = 0;
    }
    if (returnRectInfo.y1 < 0)
    {
        returnRectInfo.h = data.h + returnRectInfo.y1;
        returnRectInfo.y1 = 0;
    }
    if(returnRectInfo.x1 + data.w > zoomedImage->width())
        returnRectInfo.w = zoomedImage->width() - returnRectInfo.x1;
    if(returnRectInfo.y1 + data.h > zoomedImage->height())
        returnRectInfo.h = zoomedImage->height() - returnRectInfo.y1;
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

void SelectRect::crop()
{
    cropImage(fixedRectInfoInImage);
}

void SelectRect::cropImage(RectInfo rect)
{
    // 接受到截取框信息
    if(isLoadImage)
    {
//        qDebug() << rect.x1 << rect.w << rect.x2;
        if(rect.w > 0 && rect.h > 0)
        {

            QImage saveImageTemp = zoomedImage->copy(rect.x1,rect.y1,rect.w,rect.h);
            QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                            QCoreApplication::applicationDirPath(),
                                                            tr("Images (*.png *.xpm *.jpg *.tiff *.bmp)"));
            if(!filename.isEmpty() || !filename.isNull())
                saveImageTemp.save(filename);
        }
        else
        {
            qDebug() << "1";
            QMessageBox msgBox(QMessageBox::Critical,tr("错误"),tr("未选中图像!"));
            msgBox.exec();
        }
    }
    else
    {
        qDebug() << "2";
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

