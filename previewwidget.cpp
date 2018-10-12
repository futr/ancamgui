#include "previewwidget.h"
#include "ui_previewwidget.h"

PreviewWidget::PreviewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PreviewWidget)
{
    ui->setupUi(this);

    player = new QMediaPlayer( this );

    player->setVideoOutput( ui->videoWidget );

    connect( player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)) );
    connect( player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)) );
    connect( ui->videoSlider, SIGNAL(sliderMoved(int)), this, SLOT(setPosition(int)) );
    connect( ui->videoSlider, SIGNAL(valueChanged(int)), this, SLOT(setPosition(int)) );
    connect( player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(mediaStateChanged(QMediaPlayer::State)) );

    // Set play button icon
    ui->playButton->setIcon( style()->standardIcon( QStyle::SP_MediaPlay ) );
    ui->openDefaultButton->setIcon( QIcon( ":/img/icons/emblem-symbolic-link.png" ) );

    ui->videoWidget->setVisible( false );

    // Stay on Top
    setWindowFlags(Qt::WindowStaysOnTopHint);
}

PreviewWidget::~PreviewWidget()
{
    delete ui;
}

void PreviewWidget::setFileName( QString str )
{
    m_name = str;

    player->stop();

    if ( str == "" ) {
        ui->videoWidget->setVisible( false );
        ui->imageViewer->setVisible( true );
        ui->playButton->setEnabled( false );
        durationChanged( 0 );
        ui->imageViewer->setFileName( str );
    } else if ( QFileInfo( str ).suffix() == "jpg" ) {
        ui->videoWidget->setVisible( false );
        ui->imageViewer->setVisible( true );
        ui->playButton->setEnabled( false );

        durationChanged( 0 );

        ui->imageViewer->setFileName( str );
    } else {
        ui->imageViewer->setVisible( false );
        ui->videoWidget->setVisible( true );
        ui->playButton->setEnabled( true );

        player->setMedia( QUrl::fromLocalFile( str ) );

        // Auto play video
        player->play();
    }

    ui->nameLabel->setText( str );
}

void PreviewWidget::play()
{
    if ( player->state() == QMediaPlayer::PlayingState ) {
        player->pause();
    } else {
        player->play();

        // Fall back
        /*
        if ( !player->isVideoAvailable() ) {
            player->stop();
            QDesktopServices::openUrl( QUrl::fromLocalFile( m_name ) );
        }
        */
    }
}

void PreviewWidget::positionChanged(qint64 position)
{
    ui->videoSlider->setValue( static_cast<int>(position) );
    QString pos      = QString( "%1:%2" ).arg( ui->videoSlider->value() / 1000 / 60 ).arg( ui->videoSlider->value() / 1000 % 60, 2, 10, QLatin1Char( '0' ) );
    QString duration = QString( "%1:%2" ).arg( ui->videoSlider->maximum() / 1000 / 60 ).arg( ui->videoSlider->maximum() / 1000 % 60, 2, 10, QLatin1Char( '0' ) );

    ui->posLabel->setText( pos + " / " + duration );
}

void PreviewWidget::durationChanged(qint64 duration)
{
    ui->videoSlider->setRange( 0, static_cast<int>(duration) );
    ui->videoSlider->setPageStep( ui->videoSlider->maximum() / 10 );
    ui->videoSlider->setSingleStep( ui->videoSlider->maximum() / 100 );
}

void PreviewWidget::setPosition(int position)
{
    player->setPosition( position );
}

void PreviewWidget::disableLabel()
{
    ui->nameLabel->setVisible( false );
}

void PreviewWidget::clearPreview()
{
    setFileName( "" );
}

void PreviewWidget::on_playButton_clicked()
{
    play();
}

void PreviewWidget::mediaStateChanged(QMediaPlayer::State state)
{
    switch ( state ) {
    case QMediaPlayer::PlayingState:
        ui->playButton->setIcon( style()->standardIcon( QStyle::SP_MediaPause ) );
        break;
    default:
        ui->playButton->setIcon( style()->standardIcon( QStyle::SP_MediaPlay ) );
        break;
    }
}

void PreviewWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent( event );
}

void PreviewWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if ( !isFullScreen() ) {
        showFullScreen();
    } else{
        showNormal();
    }
}

void PreviewWidget::on_openDefaultButton_clicked()
{
    QDesktopServices::openUrl( QUrl::fromLocalFile( m_name ) );
}
