#include "servermanager.h"
#include "servermanagermodel.h"

ServerManager::ServerManager(QSettings *settings, QObject *parent) : QObject(parent)
{
    setSettings( settings );
    setWorkerThread( nullptr );
    manModel = new ServerManagerModel( this, this );
}

void ServerManager::setWorkerThread(QThread *thread)
{
    workerThread = thread;
}

int ServerManager::serverCount()
{
    return servers.count();
}

int ServerManager::serverIndex(CameraDataAccess *server)
{
    return servers.indexOf( server );
}

ServerManagerModel *ServerManager::managerModel() const
{
    return manModel;
}

CameraDataAccess *ServerManager::getServer(int idx)
{
    if ( idx >= serverCount() || idx < 0 ) {
        return  nullptr;
    }

    return servers[idx];
}

CameraDataModel *ServerManager::getServerModel(int idx)
{
    if ( idx >= serverCount() || idx < 0 ) {
        return  nullptr;
    }

    return models[idx];
}

void ServerManager::setSettings(QSettings *setting)
{
    this->setting = setting;
}

void ServerManager::loadSettings()
{
    if ( !setting ) return;

    // Delete all servers from the current instance lists
    deleteAllServers();

    // Read global settings
    sshPath   = setting->value( "local/sshPath", "/path/to/ssh" ).toString();
    gpgPath   = setting->value( "local/gpgPath", "/path/to/gpg2" ).toString();
    rsyncPath = setting->value( "local/rsyncPath", "/path/to/rsync" ).toString();

    // Start array of servers
    int count = setting->beginReadArray( "Servers" );

    if ( count == 0 ) {
        // There are no servers
        setting->endArray();

        return;
    }

    for ( int i = 0; i < count; i++ ) {
        int id = addServer( serverCount(), false );

        setting->setArrayIndex( i );
        servers[id]->setName( setting->value( "Name", "SampleServer" ).toString() );
        servers[id]->setupServerInfo( setting->value( "ServerName", "user@address" ).toString(), setting->value( "Port", "12345" ).toString(), "" );
        servers[id]->setupLocalInfo( "", setting->value( "DownloadDir", "/path/to/dldir" ).toString(), setting->value( "DecryptDir", "/path/to/plaindir" ).toString(), setting->value( "SSHKey", "~/.ssh/id_rsa" ).toString() );
        servers[id]->setMuninPort( setting->value( "RemoteMuninPort", "80" ).toString() );
    }

    setting->endArray();

    emit serverListUpdated();
}

void ServerManager::saveSettings()
{
    // Write back current server settings into the "setting"
    if ( !setting ) return;

    setting->beginWriteArray( "Servers" );

    for ( int i = 0; i < servers.count(); i++ ) {
        setting->setArrayIndex( i );
        setting->setValue( "Name",        servers[i]->getName() );
        setting->setValue( "ServerName",  servers[i]->getServerName() );
        setting->setValue( "Port",        servers[i]->getServerPort() );
        setting->setValue( "DownloadDir", servers[i]->getDownDir() );
        setting->setValue( "DecryptDir",  servers[i]->getPlainDir() );
        setting->setValue( "SSHKey",      servers[i]->getSshKeyPath() );
        setting->setValue( "RemoteMuninPort", servers[i]->getMuninPort() );
    }

    setting->endArray();
}

void ServerManager::updateGlobalSetting()
{
    if ( !setting ) return;

    // Get program path
    sshPath   = setting->value( "local/sshPath", "/path/to/ssh" ).toString();
    gpgPath   = setting->value( "local/gpgPath", "/path/to/gpg2" ).toString();
    rsyncPath = setting->value( "local/rsyncPath", "/path/to/rsync" ).toString();

    for ( int i = 0; i < servers.count(); i++ ) {
        servers[i]->setProgramPath( sshPath, gpgPath, rsyncPath );
    }
}

void ServerManager::selectServer(int idx)
{
    emit serverSelected( serverIndexToModel( idx ) );
}

int ServerManager::addServer( int idx, bool emitUpdate )
{
    CameraDataAccess *server = new CameraDataAccess();
    CameraDataModel *model = new CameraDataModel( server, this );

    // If worker is valid, move server to worker
    if ( workerThread ) server->moveToThread( workerThread );

    // Connet signals
    connect( server, &CameraDataAccess::beginResetAllData, model, &CameraDataModel::beginResetAllData );
    connect( server, &CameraDataAccess::endResetAllData, model, &CameraDataModel::endResetAllData );
    connect( server, SIGNAL(fileUpdated(QString, DataEntry *)), model, SLOT(fileUpdated(QString, DataEntry *)) );

    servers.insert( idx, server );
    models.insert( idx, model );

    // Set initial entry name
    server->setName( tr( "Unnamed" ) );

    // Set global setting
    server->setProgramPath( sshPath, gpgPath, rsyncPath );

    if ( emitUpdate ) {
        emit serverListUpdated();
    }

    if ( servers.count() <= idx ) {
        return servers.count() - 1;
    }

    if ( idx < 0 ) {
        return 0;
    }

    return idx;
}

void ServerManager::removeServer(int idx)
{
    if ( servers.count() <= idx ) return;

    delete models.takeAt( idx );
    delete servers.takeAt( idx );

    emit serverListUpdated();
}

void ServerManager::deleteAllServers()
{
    for ( auto e : models ) {
        delete e;
    }

    models.clear();

    for ( auto e : servers ) {
        delete e;
    }

    servers.clear();
}

CameraDataModel *ServerManager::serverIndexToModel(int idx)
{
    if ( models.count() <= idx ) return nullptr;

    return models[idx];
}
