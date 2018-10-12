#include "cameradatamodel.h"

CameraDataModel::CameraDataModel(CameraDataAccess *cameraData, QObject *parent)
    : QAbstractItemModel( parent )
{
    this->cameraData = cameraData;
}

CameraDataAccess *CameraDataModel::getCameraData() const
{
    return cameraData;
}

QModelIndex CameraDataModel::index(int row, int column, const QModelIndex &parent) const
{
    // Obtain a model index
    if ( !parent.isValid() ) {
        // Parent is root
        /*
        if ( column > 0 ) {
            return QModelIndex();
        }
        */
        if ( column >= static_cast<int>(FileColumnTypeID::FileColumnTypeCount) || column < 0 ) {
            return QModelIndex();
        }

        if ( row >= cameraData->getCameraNames().count() || row < 0 ) {
            return QModelIndex();
        }

        return createIndex( row, column );
    } else if ( parent.internalPointer() == nullptr ) {
        // Parent is a camera name entry
        QStringList camNames = cameraData->getCameraNames();
        CameraDataList *files = cameraData->getFileEntryListByCameraName( camNames[parent.row()] );

        if ( row >= files->count() || row < 0 ) {
            return QModelIndex();
        }

        if ( column >= static_cast<int>(FileColumnTypeID::FileColumnTypeCount) || column < 0 ) {
            return QModelIndex();
        }

        return createIndex( row, column, files );
    } else {
        // Error
        return QModelIndex();
    }
}

QModelIndex CameraDataModel::index(QString camName) const
{
    // Obtain a index by the camera name
    QStringList camNames = cameraData->getCameraNames();
    int row = camNames.indexOf( camName );

    if ( row < 0 ) {
        return QModelIndex();
    } else {
        return createIndex( row, 0 );
    }
}

QModelIndex CameraDataModel::parent(const QModelIndex &child) const
{
    if ( !child.isValid() ) {
        return QModelIndex();
    }

    if ( child.internalPointer() == nullptr ) {
        // Camera Name Entry
        return QModelIndex();
    } else {
        // File Entry
        CameraDataList *entry = static_cast<CameraDataList *>(child.internalPointer());

        if ( entry->count() == 0 ) {
            // Invalid Data Entry ( It should not be occurred )
            qWarning( "Invalid Data Entry : row=%d col=%d", child.row(), child.column() );

            return QModelIndex();
        }

        QString camName = (*entry)[0].camera;

        return index( camName );
    }
}

int CameraDataModel::rowCount(const QModelIndex &parent) const
{
    if ( !parent.isValid() ) {
        // Camera count
        return cameraData->cameraCount();
    } else if ( parent.internalPointer() == nullptr ) {
        // Camera Name Entry has file entries
        // File Entry count
        const QString camName = cameraData->getCameraNameByCameraIndex( parent.row() );

        if ( camName == "" ) {
            return 0;
        }

        return cameraData->getFileEntryCountByCameraName( camName );
    } else {
        // File entry doesnt have any children
        return 0;
    }
}

int CameraDataModel::columnCount(const QModelIndex &parent) const
{
    // ルートの場合1を返すように設計すると、ビューに対してカメラをルートとして指定しない限りカラムが１個しか表示されない
    /*
    if ( !parent.isValid() ) {
        // A Camera name entry has a camera name only
        return 1;
    } else if ( parent.internalPointer() == nullptr ) {
        // Camera Name Entry has file entries
        // File enty has "FileColumnTypeCount" columns
        return static_cast<int>(FileColumnTypeID::FileColumnTypeCount);
    } else {
        // File Entry has no children
        return 0;
    }
    */

    // QAbstProxModelがルートを親とした場合のカラム数をヘッダー処理に使ってしまうため、カラム数は固定としなければいけなかった
    Q_UNUSED( parent )
    return static_cast<int>(FileColumnTypeID::FileColumnTypeCount);
}

QVariant CameraDataModel::data(const QModelIndex &index, int role) const
{
    // MENDOKUSAI
    if ( !index.isValid() ) {
        return QVariant();
    }

    if ( index.internalId() == 0 ) {
        // Camera name entry
        QString camName = cameraData->getCameraNameByCameraIndex( index.row() );

        switch ( role ) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            if ( index.column() == 0 ) {
                return camName;
            }
            break;
        case Qt::DecorationRole:
            return QIcon( ":/img/icons/camera-video.png" );
        default:
            // qWarning( "Unsupported role has been given for Camera Name %d", role );
            break;
        }

        return QVariant();
    } else {
        // File entry
        FileColumnTypeID columnType = static_cast<FileColumnTypeID>(index.column());
        CameraDataList *files = static_cast<CameraDataList *>(index.internalPointer());
        DataEntry *pFile = &(*files)[index.row()];

        switch ( role ) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            switch ( columnType ) {
            case FileColumnTypeID::FileName:
                // return pFile->name;
                return pFile->plainName;
            case FileColumnTypeID::DateTime:
                if ( !pFile->dateTime.isValid() ) {
                    // Cache constructed datetime object
                    pFile->updateDateTime();
                }
                return pFile->dateTime;
            case FileColumnTypeID::Size: {
#if QT_VERSION >= 0x050A00
                return QLocale::system().formattedDataSize( static_cast<qint64>(pFile->fileSize) );
#else
                return pFile->fileSize;
#endif
            }
            case FileColumnTypeID::Decrypt:
            case FileColumnTypeID::Local:
            default:
                break;
            }
            break;
        case SortRole:
            switch ( columnType ) {
            case FileColumnTypeID::FileName:
                // return pFile->name;
                return pFile->plainName;
            case FileColumnTypeID::DateTime:
                return pFile->numStrDateTime;
            case FileColumnTypeID::Size:
                return pFile->fileSize;
            case FileColumnTypeID::Decrypt:
                // return pFile->decrypted;
                return pFile->isDecrypted();
            case FileColumnTypeID::Local:
                // return pFile->downloaded;
                return pFile->isDownloaded();
            case FileColumnTypeID::Online:
                return pFile->online;
            default:
                break;
            }
            break;
        case Qt::CheckStateRole:
            switch ( columnType ) {
            case FileColumnTypeID::Decrypt:
                return pFile->isDecrypted();
            case FileColumnTypeID::Local:
                return pFile->isDownloaded();
            case FileColumnTypeID::Online:
                return pFile->online;
            default:
                break;
            }
            break;
        case Qt::DecorationRole:
            switch ( columnType ) {
            case FileColumnTypeID::FileName:
                if ( pFile->fileType == DataEntry::Video )      return QIcon( ":/img/icons/video-x-generic.png" );
                if ( pFile->fileType == DataEntry::VideoThumb ) return QIcon( ":/img/icons/applications-multimedia.png" );
                if ( pFile->fileType == DataEntry::Snapshot )   return QIcon( ":/img/icons/camera-photo.png" );
                break;
            default:
                break;
            }
            break;
        case DataRole: {
            QVariant ret;
            ret.setValue( *pFile );
            return ret;
        }
        case DataPointerRole: {
            QVariant ret;
            ret.setValue( static_cast<void *>(pFile) );
            return ret;
        }
        default:
            break;
        }

        return QVariant();
    }
}

bool CameraDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED( index )
    Q_UNUSED( value )
    Q_UNUSED( role )
    // TORIAEZU
    return true;
}

QVariant CameraDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role == Qt::DisplayRole ) {
        if ( orientation == Qt::Horizontal ) {
            FileColumnTypeID column = static_cast<FileColumnTypeID>(section);
            switch ( column ) {
            case FileColumnTypeID::FileName:
                return tr( "Name" );
            case FileColumnTypeID::DateTime:
                return tr( "DateTime" );
            case FileColumnTypeID::Size:
#if QT_VERSION >= 0x050A00
                return tr( "Size" );
#else
                return tr( "Size[Bytes]" );
#endif
            case FileColumnTypeID::Decrypt:
                return tr( "Decrypt" );
            case FileColumnTypeID::Local:
                return tr( "DL" );
            case FileColumnTypeID::Online:
                return tr( "Online" );
            default:
                break;
            }
        } else {
            return QVariant( section );
        }
    }

    return QVariant();
}

Qt::ItemFlags CameraDataModel::flags(const QModelIndex &index) const
{
    if ( !index.isValid() ) {
        Qt::ItemFlags();
        return nullptr;
    }

    return QAbstractItemModel::flags( index );
}

void CameraDataModel::beginResetAllData()
{
    beginResetModel();
}

void CameraDataModel::endResetAllData()
{
    endResetModel();
}

void CameraDataModel::fileUpdated(QString camName, quint32 fileIndex)
{
    int row = static_cast<int>(fileIndex);
    QModelIndex p = index( camName );
    QModelIndex l = index( row, 0, p );
    QModelIndex r = index( row, static_cast<int>(FileColumnTypeID::FileColumnTypeCount) - 1, p );

    QVector<int> roles( static_cast<int>(Qt::CheckStateRole) );

    emit dataChanged( l, r, roles );
}

void CameraDataModel::fileUpdated(QString camName, DataEntry *file)
{
    int row = static_cast<int>(file->index);
    QModelIndex p = index( camName );
    QModelIndex l = index( row, 0, p );
    QModelIndex r = index( row, static_cast<int>(FileColumnTypeID::FileColumnTypeCount) - 1, p );

    QVector<int> roles( static_cast<int>(Qt::CheckStateRole) );

    emit dataChanged( l, r, roles );
}

void CameraDataModel::fileUpdatedList(QString /*camName*/, QList<DataEntry *> /*list*/)
{

}
