#ifndef IMAGEVIEWERWIDGET_H
#define IMAGEVIEWERWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QFileInfo>

class ImageViewerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageViewerWidget(QWidget *parent = nullptr);

signals:

public slots:
    void setFileName( QString fileName );

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage img;
    QImage resizedImg;
};

#endif // IMAGEVIEWERWIDGET_H
