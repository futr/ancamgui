#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>
#include <QSettings>
#include <QThread>
#include "cameradataaccess.h"
#include "cameradatamodel.h"
#include "servermanagermodel.h"

class ServerManager : public QObject
{
    Q_OBJECT
public:
    explicit ServerManager( QSettings *settings, QObject *parent = nullptr);
    void setWorkerThread( QThread *thread );

    int serverCount();
    int serverIndex( CameraDataAccess *server );
    ServerManagerModel *managerModel() const;
    CameraDataAccess *getServer( int idx );
    CameraDataModel *getServerModel( int idx );

signals:
    void serverListUpdated();
    void serverSelected( CameraDataModel *model );

public slots:
    void loadSettings();
    void saveSettings();
    void updateGlobalSetting();
    void selectServer( int idx );
    int addServer(int idx, bool emitUpdate = true );
    void removeServer( int idx );

private:
    void setSettings( QSettings *setting );
    void deleteAllServers();
    CameraDataModel *serverIndexToModel( int idx );

private:
    QSettings *setting;
    QList<CameraDataAccess *> servers;
    QList<CameraDataModel *> models;
    ServerManagerModel *manModel;
    QThread *workerThread;
    QString sshPath;
    QString gpgPath;
    QString rsyncPath;
};

#endif // SERVERMANAGER_H
