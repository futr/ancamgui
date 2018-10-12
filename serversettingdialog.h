#ifndef SERVERSETTINGDIALOG_H
#define SERVERSETTINGDIALOG_H

#include <QDialog>
#include "servermanager.h"
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QFileDialog>

namespace Ui {
class ServerSettingDialog;
}

class ServerSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServerSettingDialog(ServerManager *man, CameraDataAccess *current ,QWidget *parent = nullptr);
    ~ServerSettingDialog();

public slots:
    void serverRemoved();
    void serverCurrentIndexChanged( int idx );

private slots:
    void selectDownloadDir();
    void selectDecryptDir();

    void on_addServerButton_clicked();

    void on_removeServerButton_clicked();

private:
    Ui::ServerSettingDialog *ui;

    ServerManager *manager;
    QDataWidgetMapper *mapper;
};

#endif // SERVERSETTINGDIALOG_H
