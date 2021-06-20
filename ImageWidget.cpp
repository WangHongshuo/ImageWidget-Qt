// UTF-8 with BOM

// Avoid gibberish when use MSVC
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "ImageWidget.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <math.h>
#include <string>

// static const var
const char* ImageMarquees::ERR_MSG_NULL_IMAGE = "No Image Selected!";
const char* ImageMarquees::ERR_MSG_INVALID_FILE_PATH = "Invalid File Path!";
const ImageMarquees::CROPRECT ImageMarquees::CROPRECTGRP[3][3] = {
    { ImageMarquees::CR_TOPLEFT, ImageMarquees::CR_TOP, ImageMarquees::CR_TOPRIGHT },
    { ImageMarquees::CR_LEFT, ImageMarquees::CR_CENTER, ImageMarquees::CR_RIGHT },
    { ImageMarquees::CR_BOTTOMLEFT, ImageMarquees::CR_BOTTOM, ImageMarquees::CR_BOTTOMRIGHT }
};

const std::unordered_map<ImageMarquees::CROPRECT, Qt::CursorShape> ImageMarquees::CURSORCRPS = std::unordered_map<ImageMarquees::CROPRECT, Qt::CursorShape> {
    { ImageMarquees::CR_NULL, Qt::ArrowCursor }, { ImageMarquees::CR_CENTER, Qt::SizeAllCursor }, { ImageMarquees::CR_TOPLEFT, Qt::SizeFDiagCursor },
    { ImageMarquees::CR_TOPRIGHT, Qt::SizeBDiagCursor }, { ImageMarquees::CR_BOTTOMRIGHT, Qt::SizeFDiagCursor },
    { ImageMarquees::CR_BOTTOMLEFT, Qt::SizeBDiagCursor }, { ImageMarquees::CR_ENTIRETY, Qt::SizeAllCursor }, { ImageMarquees::CR_TOP, Qt::SizeVerCursor },
    { ImageMarquees::CR_RIGHT, Qt::SizeHorCursor }, { ImageMarquees::CR_BOTTOM, Qt::SizeVerCursor }, { ImageMarquees::CR_LEFT, Qt::SizeHorCursor }
};

const QImage ImageWidget::NULL_QIMAGE = QImage();
const QPoint ImageWidget::NULL_POINT = QPoint(0, 0);
const QSize ImageWidget::NULL_SIZE = QSize(0, 0);
const QRect ImageWidget::NULL_RECT = QRect(0, 0, 0, 0);
const QColor ImageWidget::BACKGROUD_COLOR = QColor(200, 200, 200);


ImageMarquees::ImageMarquees(QWidget* parent, int marqueesEdgeWidtht)
    : QWidget(parent)
{
    mouseStatus = Qt::NoButton;
    this->marqueesEdgeWidth = marqueesEdgeWidtht;

    // 初始化右键菜单
    mMenu = new QMenu(this);
    mActionReset = mMenu->addAction(tr("重选")); // Reset
    mActionSavePaintImage = mMenu->addAction(tr("另存为(缩放图像)")); // Save as (From the zoomed image)
    mActionSaveOriginalImage = mMenu->addAction(tr("另存为(实际图像)")); // Save as (From the original image)
    mActionExit = mMenu->addAction(tr("退出")); // Exit

    connect(mActionExit, SIGNAL(triggered()), this, SLOT(exit()));
    connect(mActionSavePaintImage, SIGNAL(triggered()), this, SLOT(cropPaintImage()));
    connect(mActionSaveOriginalImage, SIGNAL(triggered()), this, SLOT(cropOriginalImage()));
    connect(mActionReset, SIGNAL(triggered()), this, SLOT(reset()));
    // 关闭后释放资源
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setFocusPolicy(Qt::StrongFocus);
    cropRects = std::unordered_map<CROPRECT, QRect> { { CR_CENTER, QRect() }, { CR_TOPLEFT, QRect() }, { CR_TOPRIGHT, QRect() }, { CR_BOTTOMRIGHT, QRect() },
        { CR_BOTTOMLEFT, QRect() }, { CR_ENTIRETY, QRect() } };
    cropRectCorners = std::unordered_map<CROPRECT, QRect*> { { CR_TOPLEFT, &cropRects[CR_TOPLEFT] }, { CR_TOPRIGHT, &cropRects[CR_TOPRIGHT] },
        { CR_BOTTOMLEFT, &cropRects[CR_BOTTOMLEFT] }, { CR_BOTTOMRIGHT, &cropRects[CR_BOTTOMRIGHT] } };
}

ImageMarquees::~ImageMarquees()
{
    inputImg = nullptr;
    paintImg = nullptr;
    paintImageRect = nullptr;
    mMenu = nullptr;
}

void ImageMarquees::setImage(QImage* inputImg, QImage* paintImg, QRect* paintImageRect)
{
    if (!inputImg || !paintImg || !paintImageRect) {
        return;
    }
    if (inputImg->isNull() || paintImg->isNull() || paintImageRect->isNull()) {
        return;
    }
    this->inputImg = inputImg;
    this->paintImg = paintImg;
    this->paintImageRect = paintImageRect;
    isLoadImage = true;
}

void ImageMarquees::setMarqueesEdgeWidth(int width) { this->marqueesEdgeWidth = width; }

void ImageMarquees::recvParentWidgetSizeChangeSignal()
{
    ImageWidget* parentWidget = static_cast<ImageWidget*>(this->parent());
    this->setGeometry(0, 0, parentWidget->width(), parentWidget->height());
    update();
}

void ImageMarquees::paintEvent(QPaintEvent* event)
{
    QPainterPath transparentArea, cropArea;
    transparentArea.addRect(this->geometry());
    cropArea.addRect(cropRects[CR_CENTER]);
    transparentArea = transparentArea.subtracted(cropArea);
    QPainter painter(this);
    painter.fillPath(transparentArea, QBrush(QColor(0, 0, 0, 160)));
    if (isCropRectStable) {
        painter.setPen(QPen(QColor(0, 140, 255, 255), 1));
        painter.drawRect(cropRects[CR_CENTER]);
        cropArea.clear();
        for (auto r:cropRectCorners)
            cropArea.addRect(*(r.second));
        painter.fillPath(cropArea, QBrush(QColor(0, 140, 255, 255)));
    } else {
        painter.setPen(QPen(QColor(255, 0, 0, 255), 1));
        painter.drawRect(cropRects[CR_CENTER]);
    }
}

void ImageMarquees::mousePressEvent(QMouseEvent* event)
{
    switch (event->button()) {
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

void ImageMarquees::mouseMoveEvent(QMouseEvent* event)
{
    if (mouseStatus == Qt::LeftButton) {
        isCropRectStable = false;
        isCropRectExisted = true;
        cropRectChangeEvent(cursorPosInCropRect, event->pos());
        update();
    }
    // 判断鼠标是否在矩形框内
    if (mouseStatus == Qt::NoButton && isCropRectStable) {
        if (cropRects[CR_ENTIRETY].contains(event->pos())) {
            cursorPosInCropRect = getSubRectInCropRect(event->pos());
        } else {
            cursorPosInCropRect = CR_NULL;
        }
        this->setCursor(CURSORCRPS.at(cursorPosInCropRect));
    }
}

void ImageMarquees::mouseReleaseEvent(QMouseEvent* event)
{
    if (mouseStatus == Qt::LeftButton) {
        // 修正width,height为正
        cropRects[CR_CENTER] = cropRects[CR_CENTER].normalized();
        // 限定初次画选取框在widget内
        if (cursorPosInCropRect == CR_NULL) {
            cropRects[CR_CENTER] = this->rect().intersected(cropRects[CR_CENTER]);
        }
        // 备份
        prevCropRect = cropRects[CR_CENTER];
        mouseStatus = Qt::NoButton;
        calcMarqueesEdgeRect();
        isCropRectStable = true;
        isCropRectExisted = true;
        update();
        // 开启鼠标追踪
        this->setMouseTracking(true);
        mouseStatus = Qt::NoButton;
    }
}

void ImageMarquees::contextMenuEvent(QContextMenuEvent* event)
{
    mMenu->exec(QCursor::pos());
    mouseStatus = Qt::NoButton;
}

void ImageMarquees::wheelEvent(QWheelEvent* event) { eventFilter(this, event); }

bool ImageMarquees::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
            // 支持ESC键退出
        case Qt::Key_Escape:
            return keyEscapePressEvent();
        default:
            break;
        }
    }
    // 截断wheelEvent
    if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        wheelEvent->accept();
        return true;
    }
    return false;
}

void ImageMarquees::keyPressEvent(QKeyEvent* event) { eventFilter(this, event); }

QRect ImageMarquees::getCropRectInImage(const QRect& paintImageRect, const QRect& cropRect)
{
    QRect result = paintImageRect.intersected(cropRect);
    result.translate(-paintImageRect.topLeft());
    return result;
}

void ImageMarquees::calcMarqueesEdgeRect()
{
    // TODO: 减少计算量或QRect数量
    cropRects[CR_TOPLEFT].setTopLeft(cropRects[CR_CENTER].topLeft() + QPoint(-marqueesEdgeWidth, -marqueesEdgeWidth));
    cropRects[CR_TOPLEFT].setBottomRight(cropRects[CR_CENTER].topLeft() + QPoint(-1, -1));

    cropRects[CR_TOPRIGHT].setTopRight(cropRects[CR_CENTER].topRight() + QPoint(marqueesEdgeWidth, -marqueesEdgeWidth));
    cropRects[CR_TOPRIGHT].setBottomLeft(cropRects[CR_CENTER].topRight() + QPoint(1, -1));

    cropRects[CR_BOTTOMRIGHT].setTopLeft(cropRects[CR_CENTER].bottomRight() + QPoint(1, 1));
    cropRects[CR_BOTTOMRIGHT].setBottomRight(cropRects[CR_CENTER].bottomRight() + QPoint(marqueesEdgeWidth, marqueesEdgeWidth));

    cropRects[CR_BOTTOMLEFT].setTopRight(cropRects[CR_CENTER].bottomLeft() + QPoint(-1, 1));
    cropRects[CR_BOTTOMLEFT].setBottomLeft(cropRects[CR_CENTER].bottomLeft() + QPoint(-marqueesEdgeWidth, marqueesEdgeWidth));

    cropRects[CR_ENTIRETY].setTopLeft(cropRects[CR_TOPLEFT].topLeft());
    cropRects[CR_ENTIRETY].setBottomRight(cropRects[CR_BOTTOMRIGHT].bottomRight());

    cropRectPoints[0][0] = cropRects[CR_TOPLEFT].left();
    cropRectPoints[0][1] = cropRects[CR_TOPLEFT].right();
    cropRectPoints[0][2] = cropRects[CR_TOPRIGHT].left();
    cropRectPoints[0][3] = cropRects[CR_TOPRIGHT].right();

    cropRectPoints[1][0] = cropRects[CR_TOPLEFT].top();
    cropRectPoints[1][1] = cropRects[CR_TOPLEFT].bottom();
    cropRectPoints[1][2] = cropRects[CR_BOTTOMLEFT].top();
    cropRectPoints[1][3] = cropRects[CR_BOTTOMLEFT].bottom();
}

ImageMarquees::CROPRECT ImageMarquees::getSubRectInCropRect(QPoint cursorPos)
{
    int col = -1, row = -1;
    if (cursorPos.x() >= cropRectPoints[0][0] && cursorPos.x() <= cropRectPoints[0][1]) {
        col = 0;
    } else if (cursorPos.x() >= cropRectPoints[0][2] && cursorPos.x() <= cropRectPoints[0][3]) {
        col = 2;
    } else {
        col = 1;
    }
    if (cursorPos.y() >= cropRectPoints[1][0] && cursorPos.y() <= cropRectPoints[1][1]) {
        row = 0;
    } else if (cursorPos.y() >= cropRectPoints[1][2] && cursorPos.y() <= cropRectPoints[1][3]) {
        row = 2;
    } else {
        row = 1;
    }
    if (col == -1 || row == -1) {
        return CR_NULL;
    }
    return CROPRECTGRP[row][col];
}

void ImageMarquees::cropRectChangeEvent(int SR_LOCATION, const QPoint& cursorPos)
{
    switch (SR_LOCATION) {
    case CR_NULL:
        // 限定在mask内
        cropRects[CR_CENTER].setTopLeft(mouseLeftClickedPos);
        cropRects[CR_CENTER].setWidth(cursorPos.x() - mouseLeftClickedPos.x());
        cropRects[CR_CENTER].setHeight(cursorPos.y() - mouseLeftClickedPos.y());
        break;
    case CR_CENTER:
        cropRects[CR_CENTER].moveTo(prevCropRect.topLeft() + (cursorPos - mouseLeftClickedPos));
        break;
    case CR_TOPLEFT:
        cropRects[CR_CENTER].setTopLeft(prevCropRect.topLeft() + (cursorPos - mouseLeftClickedPos));
        break;
    case CR_TOP:
        cropRects[CR_CENTER].setTop(prevCropRect.top() + (cursorPos.y() - mouseLeftClickedPos.y()));
        break;
    case CR_TOPRIGHT:
        cropRects[CR_CENTER].setTopRight(prevCropRect.topRight() + (cursorPos - mouseLeftClickedPos));
        break;
    case CR_RIGHT:
        cropRects[CR_CENTER].setRight(prevCropRect.right() + (cursorPos.x() - mouseLeftClickedPos.x()));
        break;
    case CR_BOTTOMRIGHT:
        cropRects[CR_CENTER].setBottomRight(prevCropRect.bottomRight() + (cursorPos - mouseLeftClickedPos));
        break;
    case CR_BOTTOM:
        cropRects[CR_CENTER].setBottom(prevCropRect.bottom() + (cursorPos.y() - mouseLeftClickedPos.y()));
        break;
    case CR_BOTTOMLEFT:
        cropRects[CR_CENTER].setBottomLeft(prevCropRect.bottomLeft() + (cursorPos - mouseLeftClickedPos));
        break;
    case CR_LEFT:
        cropRects[CR_CENTER].setLeft(prevCropRect.left() + (cursorPos.x() - mouseLeftClickedPos.x()));
        break;
    default:
        break;
    }
}

bool ImageMarquees::keyEscapePressEvent()
{
    if (isCropRectExisted) {
        reset();
    } else {
        this->exit();
    }
    return true;
}

void ImageMarquees::showErrorMsgBox(const char* errMsg)
{
    QMessageBox msgBox(QMessageBox::Critical, tr("ERROR"), tr(errMsg));
    msgBox.exec();
}

void ImageMarquees::exit()
{
    emit sendExitSignal();
    this->close();
}

void ImageMarquees::reset()
{
    cropRects[CR_CENTER] = QRect(0, 0, 0, 0);
    prevCropRect = QRect(0, 0, 0, 0);

    isCropRectStable = false;
    isCropRectExisted = false;
    // 关闭鼠标追踪 节省资源
    this->setMouseTracking(false);
    this->setCursor(Qt::ArrowCursor);
    cursorPosInCropRect = CR_NULL;
    mouseStatus = Qt::NoButton;
    update();
}

void ImageMarquees::cropPaintImage()
{
    cropRectInImage = getCropRectInImage(*paintImageRect, cropRects[CR_CENTER]);
    saveImage(paintImg, cropRectInImage);
}

void ImageMarquees::cropOriginalImage()
{
    cropRectInImage = getCropRectInImage(*paintImageRect, cropRects[CR_CENTER]);
    QPoint topLeft = mappingCoordinate(inputImg->rect(), paintImg->rect(), cropRectInImage.topLeft(), std::floor);
    QPoint bottomRight = mappingCoordinate(inputImg->rect(), paintImg->rect(), cropRectInImage.bottomRight(), std::ceil) - QPoint(1, 1);
    cropRectInImage.setTopLeft(topLeft);
    cropRectInImage.setBottomRight(bottomRight);
    saveImage(inputImg, cropRectInImage);
}

void ImageMarquees::saveImage(const QImage* img, const QRect& rect)
{
    // 接受到截取框信息
    if (!isLoadImage || !img || img->isNull() || rect.width() <= 0 || rect.height() <= 0) {
        showErrorMsgBox(ERR_MSG_NULL_IMAGE);
        return;
    }
    // TODO: 应该可以像opencv的Mat选取ROI避免拷贝
    QString filename
        = QFileDialog::getSaveFileName(this, tr("Save File"), QCoreApplication::applicationDirPath(), tr("Images(*.bmp *.jpg *.png *.xpm *.tiff )"));
    if (filename.isEmpty() || filename.isNull()) {
        showErrorMsgBox(ERR_MSG_INVALID_FILE_PATH);
        return;
    }
    QImage saveImageTemp = img->copy(rect);
    saveImageTemp.save(filename);
}

ImageWidget::ImageWidget(QWidget* parent)
    : QWidget(parent)
{
    isCropImageMode = false;
    imageWidgetPaintRect = QRect(-PAINT_AREA_OFFEST, -PAINT_AREA_OFFEST, this->width() + 2 * PAINT_AREA_OFFEST, this->height() + 2 * PAINT_AREA_OFFEST);
    initializeContextmenu();
    roi.clear();
}

ImageWidget::~ImageWidget() {}

bool ImageWidget::setImage(const QImage& img, bool isDeepCopy)
{
    if (img.isNull()) {
        return false;
    }
    // 默认使用QImage的浅拷贝，自动管理QImage中data引用，避免使用指针，传入局部变量造成crash
    if (isDeepCopy) {
        inputImg = img.copy();
    } else {
        inputImg = img;
    }
    initShowImage();
    return true;
}

bool ImageWidget::setImage(const QString& filePath) { return loadImageFromPath(filePath); }

bool ImageWidget::setImage(const std::string& filePath) { return loadImageFromPath(QString::fromStdString(filePath)); }

bool ImageWidget::loadImageFromPath(const QString& filePath)
{
    if (filePath.isEmpty() || filePath.isNull()) {
        return false;
    }
    inputImg.load(filePath);
    if (inputImg.isNull()) {
        return false;
    }
    initShowImage();
    return true;
}

void ImageWidget::setImageAttributeWithAutoFitFlag(bool enableAutoFit)
{
    if (enableAutoFit) {
        // 根据Widget大小缩放图像
        paintImg = inputImg.scaled(this->width(), this->height(), Qt::KeepAspectRatio);
        // TODO: 存在除数为0风险
        zoomScale = double(paintImg.width()) / double(inputImg.width());
    } else {
        paintImg = inputImg;
        zoomScale = 1.0;
    }
    // 计算图像在Widget中显示的左上坐标
    paintImageRect.setTopLeft(getImageTopLeftPosWhenShowInCenter(paintImg, this));
    paintImageRect.setSize(paintImg.size());
    paintImageLastTopLeft = paintImageRect.topLeft();
}

void ImageWidget::fixPaintImageTopLeft()
{
    if (restrictMode == RM_INNER) {
        fixPaintImageTopLeftInOutterMode(imageWidgetPaintRect, paintImageRect);
    } else {
        fixPaintImageTopLefInInnerMode(imageWidgetPaintRect, paintImageRect);
    }
}

void ImageWidget::initShowImage()
{
    paintImg = inputImg.copy();
    setImageAttributeWithAutoFitFlag(enableAutoFitWidget);
    updateImageWidget();
}

void ImageWidget::clear()
{
    if (inputImg.isNull()) {
        return;
    }
    inputImg = NULL_QIMAGE;
    paintImg = NULL_QIMAGE;
    lastPaintImgSize = NULL_SIZE;
    zoomScale = 1.0;
    mouseLeftKeyPressDownPos = NULL_POINT;
    paintImageLastTopLeft = NULL_POINT;
    paintImageRect = NULL_RECT;
    removeAllROI();
    update();
}

void ImageWidget::setEnableOnlyShowImage(bool flag) { enableOnlyShowImage = flag; }

ImageWidget* ImageWidget::setEnableDrag(bool flag)
{
    enableDragImage = flag;
    mActionEnableDrag->setChecked(enableDragImage);
    return this;
}

ImageWidget* ImageWidget::setEnableZoom(bool flag)
{
    enableZoomImage = flag;
    mActionEnableZoom->setChecked(enableZoomImage);
    return this;
}

ImageWidget* ImageWidget::setEnableAutoFit(bool flag)
{
    enableAutoFitWidget = flag;
    mActionImageAutoFitWidget->setChecked(enableAutoFitWidget);
    resetImageWidget();
    return this;
}

ImageWidget* ImageWidget::setMaxZoomScale(double scale)
{
    MAX_ZOOM_SCALE = scale;
    return this;
}

ImageWidget* ImageWidget::setMinZoomScale(double scale)
{
    MIN_ZOOM_SCALE = scale;
    return this;
}

ImageWidget* ImageWidget::setMaxZoomedImageSize(int width, int height)
{
    MAX_ZOOMED_IMG_SIZE.setWidth(width);
    MAX_ZOOMED_IMG_SIZE.setHeight(height);
    return this;
}

ImageWidget* ImageWidget::setMinZoomedImageSize(int width, int height)
{
    MIN_ZOOMED_IMG_SIZE.setWidth(width);
    MIN_ZOOMED_IMG_SIZE.setHeight(height);
    return this;
}

ImageWidget* ImageWidget::setPaintAreaOffset(int offset)
{
    this->PAINT_AREA_OFFEST = offset;
    return this;
}

ImageWidget* ImageWidget::setPaintImageRestrictMode(ImageWidget::RestrictMode rm)
{
    restrictMode = rm;
    return this;
}

ImageWidget* ImageWidget::setEnableSendLeftClickedPosInWidget(bool flag)
{
    enableSendLeftClickedPosInWidget = flag;
    return this;
}

ImageWidget* ImageWidget::setEnableSendLeftClickedPosInImage(bool flag)
{
    enableSendLeftClickedPosInImage = flag;
    return this;
}

QPoint ImageWidget::getDrawImageTopLeftPos() const { return paintImageRect.topLeft(); }

// roi为原始图像坐标
// TODO: 图像尺寸，位置发生变化时，ROI跟随变化
void ImageWidget::addROI(const QRect &rect, int label)
{
    QRect mappedRoi(mappingCoordinate(paintImageRect, inputImg.rect(), rect.topLeft(), std::floor),
        mappingCoordinate(paintImageRect, inputImg.rect(), rect.bottomRight(), std::ceil));
    roi[label] = std::make_pair(rect, mappedRoi);
    update();
}

void ImageWidget::removeROI(int lable)
{
    roi.erase(lable);
    update();
}

void ImageWidget::removeAllROI()
{
    roi.clear();
    update();
}

void ImageWidget::wheelEvent(QWheelEvent* e)
{
    if (inputImg.isNull() || enableOnlyShowImage || !enableZoomImage) {
        return;
    }
    if (e->angleDelta().x() < 0 || e->angleDelta().y() < 0) {
        imageZoomOut();
    } else {
        imageZoomIn();
    }
    updateImageByWheelEvent();
    updateImageWidget();
}

void ImageWidget::mousePressEvent(QMouseEvent* e)
{
    if (inputImg.isNull() || enableOnlyShowImage) {
        return;
    }
    switch (e->button()) {
    case Qt::LeftButton:
        mouseStatus = Qt::LeftButton;
        mouseLeftKeyPressDownPos = e->pos();
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

void ImageWidget::mouseReleaseEvent(QMouseEvent* e)
{
    // 单击事件
    if (mouseStatus == Qt::LeftButton && !isImageDragging) {
        sendLeftClickedSignals(e);
        mouseStatus = Qt::NoButton;
    }
    if (inputImg.isNull() || enableOnlyShowImage || !enableDragImage) {
        return;
    }
    if (mouseStatus == Qt::LeftButton) {
        // 记录上次图像顶点
        paintImageLastTopLeft = paintImageRect.topLeft();
        // 释放后鼠标状态置No
        mouseStatus = Qt::NoButton;
        isImagePosChanged = true;
        isImageDragging = false;
        updateImageWidget();
    }
}

void ImageWidget::fixPaintImageTopLeftInOutterMode(const QRect& imageWidgetRect, QRect& qImgZoomedRect)
{
    // 计算坐标偏差并修正
    if (qImgZoomedRect.right() < imageWidgetRect.left()) {
        qImgZoomedRect.moveRight(imageWidgetRect.left());
    }
    if (qImgZoomedRect.left() > imageWidgetRect.right()) {
        qImgZoomedRect.moveLeft(imageWidgetRect.right());
    }
    if (qImgZoomedRect.bottom() < imageWidgetRect.top()) {
        qImgZoomedRect.moveBottom(imageWidgetRect.top());
    }
    if (qImgZoomedRect.top() > imageWidgetRect.bottom()) {
        qImgZoomedRect.moveTop(imageWidgetRect.bottom());
    }
}

void ImageWidget::fixPaintImageTopLefInInnerMode(const QRect& imageWidgetRect, QRect& paintImageRect)
{
    // 计算坐标偏差并修正
    if (paintImageRect.width() <= imageWidgetRect.width()) {
        if (paintImageRect.left() < imageWidgetRect.left()) {
            paintImageRect.moveLeft(imageWidgetRect.left());
        }
        if (paintImageRect.right() > imageWidgetRect.right()) {
            paintImageRect.moveRight(imageWidgetRect.right());
        }
    }
    if (paintImageRect.height() <= imageWidgetRect.height()) {
        if (paintImageRect.top() < imageWidgetRect.top()) {
            paintImageRect.moveTop(imageWidgetRect.top());
        }
        if (paintImageRect.bottom() > imageWidgetRect.bottom()) {
            paintImageRect.moveBottom(imageWidgetRect.bottom());
        }
    }
    if (paintImageRect.width() > imageWidgetRect.width()) {
        if (paintImageRect.left() > imageWidgetRect.left()) {
            paintImageRect.moveLeft(imageWidgetRect.left());
        }
        if (paintImageRect.right() < imageWidgetRect.right()) {
            paintImageRect.moveRight(imageWidgetRect.right());
        }
    }
    if (paintImageRect.height() > imageWidgetRect.height()) {
        if (paintImageRect.top() > imageWidgetRect.top()) {
            paintImageRect.moveTop(imageWidgetRect.top());
        }
        if (paintImageRect.bottom() < imageWidgetRect.bottom()) {
            paintImageRect.moveBottom(imageWidgetRect.bottom());
        }
    }
}

void ImageWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (inputImg.isNull() || enableOnlyShowImage || !enableDragImage) {
        return;
    }
    // TODO: 在图像移动出Widget后，限定drawImageTopLeftPos，防止图像飞出Widget太远
    if (mouseStatus == Qt::LeftButton) {
        // e->pos()为当前鼠标坐标 转换为相对移动距离
        paintImageRect.moveTopLeft(paintImageLastTopLeft + (e->pos() - mouseLeftKeyPressDownPos));
        isImageDragging = true;
        updateImageWidget();
    }
}

void ImageWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setBrush(QBrush(BACKGROUD_COLOR));
    painter.drawRect(0, 0, this->width(), this->height());
    if (inputImg.isNull()) {
        return;
    }
    painter.drawImage(paintImageRect.topLeft(), paintImg);
    if (roi.size() == 0) {
        return;
    }
    QPainterPath roiRect;
    // todo: 绘制中心透明的矩形，临时方案，后续减少Path数量
    for (auto it = roi.begin(); it != roi.end(); ++it) {
        roiRect.addRect(it->second.second);
        roiRect.addRect(it->second.second);
    }
    painter.setPen(QPen(QColor(255, 0, 0, 255), 1));
    painter.drawPath(roiRect);
}

void ImageWidget::contextMenuEvent(QContextMenuEvent* e)
{
    if (!enableOnlyShowImage) {
        mMenu->exec(QCursor::pos());
        // 右键菜单弹出后 鼠标状态置No
        mouseStatus = Qt::NoButton;
    }
}

void ImageWidget::resizeEvent(QResizeEvent* e)
{
    imageWidgetPaintRect = QRect(-PAINT_AREA_OFFEST, -PAINT_AREA_OFFEST, this->width() + 2 * PAINT_AREA_OFFEST, this->height() + 2 * PAINT_AREA_OFFEST);
    if (inputImg.isNull()) {
        return;
    }
    // 如果图像没有被拖拽或者缩放过 置中缩放
    if (!isImagePosChanged && enableAutoFitWidget) {
        paintImg = inputImg.scaled(this->width(), this->height(), Qt::KeepAspectRatio);
        paintImageRect.moveTopLeft(getImageTopLeftPosWhenShowInCenter(paintImg, this));
        paintImageRect.setSize(paintImg.size());
        paintImageLastTopLeft = paintImageRect.topLeft();
    } else {
        // TODO: 拖动后的图像在ImageWidget尺寸发生变化后如何调整
    }
    if (isCropImageMode)
        emit sendParentWidgetSizeChangedSignal();
    fixPaintImageTopLeft();
    paintImageLastTopLeft = paintImageRect.topLeft();
}

void ImageWidget::resetImageWidget()
{
    setImageAttributeWithAutoFitFlag(enableAutoFitWidget);
    isImagePosChanged = false;
    updateImageWidget();
}

void ImageWidget::imageZoomOut()
{
    // TODO: 使用scale和size共同限定
    if (zoomScale < MIN_ZOOM_SCALE) {
        return;
    }
    zoomScale *= 1.0 / 1.1;
    isZoomedParametersChanged = true;
}

void ImageWidget::imageZoomIn()
{
    // TODO: 使用scale和size共同限定
    if (zoomScale > MAX_ZOOM_SCALE) {
        return;
    }
    zoomScale *= 1.1;
    isZoomedParametersChanged = true;
}

void ImageWidget::enterCropImageMode()
{
    if (inputImg.isNull()) {
        return;
    }
    isCropImageMode = true;
    ImageMarquees* m = new ImageMarquees(this);
    m->setGeometry(0, 0, this->geometry().width(), this->geometry().height());
    connect(m, SIGNAL(sendExitSignal()), this, SLOT(exitCropImageMode()));
    connect(this, SIGNAL(sendParentWidgetSizeChangedSignal()), m, SLOT(recvParentWidgetSizeChangeSignal()));
    m->setImage(&inputImg, &paintImg, &paintImageRect);
    m->show();
}

void ImageWidget::initializeContextmenu()
{
    mMenu = new QMenu(this);
    mMenuAdditionalFunction = new QMenu(mMenu);

    mActionResetParameters = mMenu->addAction(tr("重置")); // Reset
    mActionSave = mMenu->addAction(tr("另存为")); // Save As
    mActionCrop = mMenu->addAction(tr("截取")); // Crop
    mMenuAdditionalFunction = mMenu->addMenu(tr("更多功能")); // More Function
    mActionEnableDrag = mMenuAdditionalFunction->addAction(tr("启用拖拽")); // Enable Drag
    mActionEnableZoom = mMenuAdditionalFunction->addAction(tr("启用缩放")); // Enable Zoom
    mActionImageAutoFitWidget = mMenuAdditionalFunction->addAction(tr("启动自适应大小")); // Enable Image Fit Widget

    mActionEnableDrag->setCheckable(true);
    mActionEnableZoom->setCheckable(true);
    mActionImageAutoFitWidget->setCheckable(true);

    mActionEnableDrag->setChecked(enableDragImage);
    mActionEnableZoom->setChecked(enableZoomImage);
    mActionImageAutoFitWidget->setChecked(enableAutoFitWidget);

    connect(mActionResetParameters, SIGNAL(triggered()), this, SLOT(resetImageWidget()));
    connect(mActionSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(mActionCrop, SIGNAL(triggered()), this, SLOT(enterCropImageMode()));
    connect(mActionEnableDrag, SIGNAL(toggled(bool)), this, SLOT(setEnableDrag(bool)));
    connect(mActionEnableZoom, SIGNAL(toggled(bool)), this, SLOT(setEnableZoom(bool)));
    connect(mActionImageAutoFitWidget, SIGNAL(toggled(bool)), this, SLOT(setEnableAutoFit(bool)));
}

void ImageWidget::sendLeftClickedSignals(QMouseEvent* e)
{
    if (enableSendLeftClickedPosInWidget){
        emit sendLeftClickedPosInWidgetSignal(e->position().x(), e->position().y());
    }

    if (enableSendLeftClickedPosInImage) {
        if (inputImg.isNull()) {
            emit sendLeftClickedPosInImageSignal(-1, -1);
        } else {
            QPoint cursorPosInImage = mappingCoordinate(inputImg.rect(), paintImageRect, e->pos(), std::floor);
            // 如果光标不在图像上则返回-1
            if (cursorPosInImage.x() < 0 || cursorPosInImage.y() < 0 || cursorPosInImage.x() > inputImg.width() - 1
                || cursorPosInImage.y() > inputImg.height() - 1) {
                cursorPosInImage.setX(-1);
                cursorPosInImage.setY(-1);
            }
            emit sendLeftClickedPosInImageSignal(cursorPosInImage.x(), cursorPosInImage.y());
        }
    }
}

// 图像坐标映射
QPoint mappingCoordinate(const QRect& dstImgRect, const QRect& srcImgRect, const QPoint& srcPoint, double (*procFunc)(double))
{
    QPoint resPoint;
    if (procFunc == nullptr) {
        return resPoint;
    }
    int distanceX = srcPoint.x() - srcImgRect.x();
    int distanceY = srcPoint.y() - srcImgRect.y();
    double xDivZoomedImageW = double(distanceX) / double(srcImgRect.width());
    double yDivZoomedImageH = double(distanceY) / double(srcImgRect.height());
    resPoint.setX(dstImgRect.x() + int(procFunc(dstImgRect.width() * xDivZoomedImageW)));
    resPoint.setY(dstImgRect.y() + int(procFunc(dstImgRect.height() * yDivZoomedImageH)));
    return resPoint;
}

void ImageWidget::save()
{
    if (!inputImg.isNull()) {
        QImage temp = inputImg.copy();
        QString filename
            = QFileDialog::getSaveFileName(this, tr("Open File"), QCoreApplication::applicationDirPath(), tr("Images (*.png *.xpm *.jpg *.tiff *.bmp)"));
        if (!filename.isEmpty() || !filename.isNull())
            temp.save(filename);
    }
}

void ImageWidget::exitCropImageMode() { isCropImageMode = false; }

void ImageWidget::updateImageWidget()
{
    fixPaintImageTopLeft();
    update();
}

void ImageWidget::updateImageByWheelEvent()
{
    // 图像为空直接返回
    if (inputImg.isNull() || !isZoomedParametersChanged) {
        return;
    }
    lastPaintImgSize = paintImg.size();

    // 减少拖动带来的QImage::scaled
    paintImg = inputImg.scaled(inputImg.width() * zoomScale, inputImg.height() * zoomScale, Qt::KeepAspectRatio);

    // TODO: 缩放过程中图像位置应不变
    QSize zoomedImageChanged = lastPaintImgSize - paintImg.size();
    // 获取当前光标并计算出光标在图像中的位置
    QPoint cursorPosInWidget = this->mapFromGlobal(QCursor::pos());
    QPoint cursorPosInImage = mappingCoordinate(inputImg.rect(), paintImageRect, cursorPosInWidget, std::floor);
    if (cursorPosInImage.x() < 0) {
        cursorPosInImage.setX(0);
    }
    if (cursorPosInImage.y() < 0) {
        cursorPosInImage.setY(0);
    }
    if (cursorPosInImage.x() > inputImg.width()) {
        cursorPosInImage.setX(inputImg.width() - 1);
    }
    if (cursorPosInImage.y() > inputImg.height()) {
        cursorPosInImage.setY(inputImg.height() - 1);
    }
    // 根据光标在图像的位置进行调整左上绘图点位置 保持鼠标悬停点为缩放中心点
    paintImageRect.moveTopLeft(paintImageLastTopLeft
        + QPoint(int(double(zoomedImageChanged.width()) * double(cursorPosInImage.x()) / double(inputImg.width() - 1)),
            int(double(zoomedImageChanged.height()) * double(cursorPosInImage.y()) / double(inputImg.height() - 1))));
    paintImageRect.setSize(paintImg.size());

    if (paintImageRect.topLeft() != paintImageLastTopLeft) {
        isImagePosChanged = true;
    }
    paintImageLastTopLeft = paintImageRect.topLeft();

    isZoomedParametersChanged = false;
}

QPoint ImageWidget::getImageTopLeftPosWhenShowInCenter(const QImage& img, const QWidget* iw)
{
    return QPoint((iw->width() - img.width()) / 2, (iw->height() - img.height()) / 2);
}
