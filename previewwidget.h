#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include <QtGui/QMovie>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtMultimedia/QMediaPlayer>
#include <QFileInfo>
#include <QImage>
#include <QDesktopServices>
#include <QPainter>
#include <QStyle>
#include "imageviewerwidget.h"

namespace Ui {
class PreviewWidget;
}

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget();

private:
    Ui::PreviewWidget *ui;

    QString m_name;

    QMediaPlayer *player;
    QImage img;

public:
    void setFileName(QString str);

public slots:
    void play();

    void positionChanged( qint64 position );
    void durationChanged( qint64 duration );
    void setPosition(int position );
    void disableLabel();
    void clearPreview();
private slots:
    void on_playButton_clicked();
    void mediaStateChanged( QMediaPlayer::State state );

    // QWidget interface
    void on_openDefaultButton_clicked();

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // PREVIEWWIDGET_H
