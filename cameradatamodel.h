#ifndef CAMERADATAMODEL_H
#define CAMERADATAMODEL_H

#include <QObject>
#include <QVector>
#include <QAbstractItemModel>
#include <QDebug>
#include <QIcon>
#include "cameradataaccess.h"

/*
 * NULLptr == CameraNameEntry
 * Valid pointer is a FileEntry
 *
 */

class CameraDataModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum class EntryTypeID {
        CameraNameEntry = 1,
        FileEntry,
    };

    enum class FileColumnTypeID {
        FileName,
        DateTime,
        Size,
        Local,
        Decrypt,
        // Progress,
        Online,
        FileColumnTypeCount,
    };

    enum FileRoles {
        LocalFullPathRole = Qt::UserRole + 1,
        LocalDecryptedFullPathRole,
        RemoteFullPathRole,
        UnixTimeRole,
        DateTimeRole,
        DateTimeTextRole,
        DecryptedRole,
        DownloadedRole,
        ProgressRole,
        DataRole,
        DataPointerRole,
        SortRole,
    };

public:
    explicit CameraDataModel( CameraDataAccess *cameraData, QObject *parent = nullptr );

    CameraDataAccess *getCameraData() const;
private:
    CameraDataAccess *cameraData;

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex index( QString camName ) const;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

public slots:
    void beginResetAllData();
    void endResetAllData();
    void fileUpdated( QString camName, quint32 fileIndex ); // DEPRECATED
    void fileUpdated( QString camName, DataEntry *file );
    void fileUpdatedList( QString camName, QList<DataEntry *> list );
};

#endif // CAMERADATAMODEL_H
