#ifndef SERVERMANAGERMODEL_H
#define SERVERMANAGERMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QDebug>

class ServerManager;

class ServerManagerModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum class ServerSettingsID {
        Name,
        ServerName,
        Port,
        DownloadPath,
        DecryptPath,
        SSHKeyPath,
        MuninPort,
        SettingsIDCount,
    };

public:
    explicit ServerManagerModel( ServerManager *man, QObject *parent = nullptr);

signals:

public slots:

private:
    ServerManager *manager;

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool setData(const QModelIndex &idx, const QVariant &value, int role) override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent) override;
};

#endif // SERVERMANAGERMODEL_H
