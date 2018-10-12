#include "servermanagermodel.h"
#include "servermanager.h"

ServerManagerModel::ServerManagerModel(ServerManager *man, QObject *parent)
    : QAbstractItemModel(parent),
      manager(man)
{

}

QModelIndex ServerManagerModel::index(int row, int column, const QModelIndex &parent) const
{
    if ( parent.isValid() || column >= static_cast<int>(ServerSettingsID::SettingsIDCount) || row >= manager->serverCount() || row < 0 || column < 0 ) {
        return QModelIndex();
    }

    return createIndex( row, column );
}

QModelIndex ServerManagerModel::parent(const QModelIndex &/*child*/) const
{
    return QModelIndex();
}

int ServerManagerModel::rowCount(const QModelIndex &parent) const
{
    if ( parent.isValid() ) {
        return 0;
    } else {
        return manager->serverCount();
    }
}

int ServerManagerModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(ServerSettingsID::SettingsIDCount);
}

QVariant ServerManagerModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() ) {
        return  QVariant();
    }

    CameraDataAccess *server = manager->getServer( index.row() );

    if ( server == nullptr ) {
        return QVariant();
    }

    ServerSettingsID setting = static_cast<ServerSettingsID>( index.column() );

    switch ( role ) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch ( setting ) {
        case ServerSettingsID::Name:
            return server->getName();
        case ServerSettingsID::ServerName:
            return server->getServerName();
        case ServerSettingsID::Port:
            return server->getServerPort();
        case ServerSettingsID::DecryptPath:
            return server->getPlainDir();
        case ServerSettingsID::DownloadPath:
            return server->getDownDir();
        case ServerSettingsID::SSHKeyPath:
            return server->getSshKeyPath();
        case ServerSettingsID::MuninPort:
            return server->getMuninPort();
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}

Qt::ItemFlags ServerManagerModel::flags(const QModelIndex &index) const
{
    if ( !index.isValid() ) {
        return  Qt::ItemIsEnabled;
    }

    return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable;
}


bool ServerManagerModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if ( !idx.isValid() ) {
        return false;
    }

    CameraDataAccess *server = manager->getServer( idx.row() );

    if ( server == nullptr ) {
        return false;
    }

    ServerSettingsID setting = static_cast<ServerSettingsID>( idx.column() );

    switch ( role ) {
    case Qt::EditRole:
        switch ( setting ) {
        case ServerSettingsID::Name:
            server->setName( value.toString() );
            break;
        case ServerSettingsID::ServerName:
            server->setServerName( value.toString() );
            break;
        case ServerSettingsID::Port:
            server->setServerPort( value.toString() );
            break;
        case ServerSettingsID::DecryptPath:
            server->setPlainDir( value.toString() );
            break;
        case ServerSettingsID::DownloadPath:
            server->setDownDir( value.toString() );
            break;
        case ServerSettingsID::SSHKeyPath:
            server->setSshKeyPath( value.toString() );
            break;
        case ServerSettingsID::MuninPort:
            server->setMuninPort( value.toString() );
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }

    QModelIndex l = index( idx.row(), 0, idx.parent() );
    QModelIndex r = index( idx.row(), static_cast<int>( ServerSettingsID::SettingsIDCount ) - 1, idx.parent() );

    emit dataChanged( l, r );

    return true;
}

bool ServerManagerModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if ( parent.isValid() ) return false;

    beginInsertRows( parent, row, row + count - 1 );

    for ( int i = 0; i < count; i++ ) {
        manager->addServer( row + i, false );
    }

    endInsertRows();

    return true;
}

bool ServerManagerModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if ( parent.isValid() ) return false;

    beginRemoveRows( parent, row, row + count - 1 );

    for ( int i = 0; i < count; i++ ) {
        manager->removeServer( row );
    }

    endRemoveRows();

    return true;
}
