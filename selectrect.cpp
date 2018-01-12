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
    connect(subActionSave,SIGNAL(triggered()),this,SLOT(cropImage()));
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
}

void SelectRect::paintEvent(QPaintEvent *event)
{
    // 背景
    QPainterPath mask;
    // 选中的范围
    QPainterPath select_area;
    // 椭圆
//    QRect boundingRectangle(rect.x,rect.y,rect.w,rect.h);
//    select_area.addEllipse(boundingRectangle);
    select_area.addRect(rect.x,rect.y,rect.w,rect.h);
    mask.addRect(this->geometry());
    QPainterPath drawMask =mask.subtracted(select_area);

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
        rect.x = mouseLeftClickedPosX;
        rect.y = mouseLeftClickedPosY;
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
        rect.w = x - rect.x;
        rect.h = y - rect.y;

    }
    update();
}

void SelectRect::mouseReleaseEvent(QMouseEvent *event)
{

}

void SelectRect::contextMenuEvent(QContextMenuEvent *event)
{
    subMenu->exec(QCursor::pos());
}

void SelectRect::selectExit()
{
    emit sendSelectModeExit();
    this->close();
}

void SelectRect::selectReset()
{
    rect.w = 0;
    rect.h = 0;
    update();
}

void SelectRect::cropImage()
{
    // 接受到截取框信息
    if(isImageLoad)
    {
        rect_info data;
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
        if(data.w == 0 && data.h == 0)
        {
            QMessageBox msgBox(QMessageBox::Critical,tr("错误"),tr("未选取区域！"));
            msgBox.exec();
        }
        else
        {
            // 计算相对于图像内的坐标
            int x,y,w,h;
            x = data.x - drawImageTopLeftPosX;
            y = data.y - drawImageTopLeftPosY;
            w = data.w;
            h = data.h;
            // 限定截取范围在图像内
            // 修正顶点
            if(x < 0)
            {
                w = data.w + x;
                x = 0;
            }
            if (y < 0)
            {
                h = data.h + y;
                y = 0;
            }
            if(x + data.w > image->width())
                w = image->width() - x;
            if(y + data.h > image->height())
                h = image->height() - y;
//            qDebug() << x << y << w << h;
            if(w > 0 && h > 0)
            {
                QImage saveImageTemp = image->copy(x,y,w,h);
                QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                                QCoreApplication::applicationDirPath(),
                                                                tr("Images (*.png *.xpm *.jpg *.tiff *.bmp)"));
                if(!filename.isEmpty() || !filename.isNull())
                    saveImageTemp.save(filename);
            }
            else
            {
                QMessageBox msgBox(QMessageBox::Critical,tr("错误"),tr("未选中图像！"));
                msgBox.exec();
            }
        }
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Critical,tr("错误"),tr("未选中图像！"));
        msgBox.exec();
    }
}

void SelectRect::receiveParentSizeChangedValue(int width, int height)
{
    this->setGeometry(0,0,width,height);
    update();
}
