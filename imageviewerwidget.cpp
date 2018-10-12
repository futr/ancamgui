#include "imageviewerwidget.h"

ImageViewerWidget::ImageViewerWidget(QWidget *parent) : QWidget(parent)
{

}

void ImageViewerWidget::setFileName(QString fileName)
{
    if ( fileName != "" && QFileInfo::exists( fileName ) ) {
        img.load( fileName );
        resizedImg = img;
    } else {
        img = QImage();
    }

    update();
}

void ImageViewerWidget::paintEvent(QPaintEvent */*event*/)
{
    QPainter p( this );

    if ( img.isNull() ) {
        return;
    }

    if ( resizedImg.size() != size() ) {
        resizedImg = img.scaled( size(), Qt::KeepAspectRatio );
    }

    p.drawImage( width() / 2 - resizedImg.width() / 2, height() / 2 - resizedImg.height() / 2, resizedImg );
}
