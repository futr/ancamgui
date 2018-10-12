#ifndef FILESORTFILTERPROXYMODEL_H
#define FILESORTFILTERPROXYMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <QDateTime>
#include "cameradatamodel.h"

class FileSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    FileSortFilterProxyModel( QObject *parent = nullptr );

public slots:
    void setFileTypeFilter( DataEntry::FileType type );
    void setDateTimeFilter( QDateTime start, QDateTime end );
    void setStartDateTimeFilter( QDateTime start );
    void setEndDateTimeFilter( QDateTime end );

    QDateTime getStartDateTime() const;
    QDateTime getEndDateTime() const;

    // QSortFilterProxyModel interface
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    DataEntry::FileType filterFilteType;
    QDateTime startDateTime;
    QDateTime endDateTime;
};

#endif // FILESORTFILTERPROXYMODEL_H
