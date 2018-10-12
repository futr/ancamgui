#include "configwidget.h"
#include "ui_configwidget.h"

ConfigWidget::ConfigWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigWidget)
{
    ui->setupUi(this);
}

ConfigWidget::~ConfigWidget()
{
    delete ui;
}

void ConfigWidget::readSettings(QSettings *settings)
{
    ui->sshPathEdit->setText( settings->value( "local/sshPath", "/path/to/ssh" ).toString() );
    ui->gpgPathEdit->setText( settings->value( "local/gpgPath", "/path/to/gpg2" ).toString() );
    ui->rsyncPathEdit->setText( settings->value( "local/rsyncPath", "/path/to/rsync" ).toString() );

    this->settings = settings;
}

void ConfigWidget::writeSettings()
{
    settings->setValue( "local/sshPath", ui->sshPathEdit->text() );
    settings->setValue( "local/gpgPath", ui->gpgPathEdit->text() );
    settings->setValue( "local/rsyncPath", ui->rsyncPathEdit->text() );
}
