#include "filesortfilterproxymodel.h"

FileSortFilterProxyModel::FileSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel( parent ), filterFilteType( DataEntry::AllType )
{

}

void FileSortFilterProxyModel::setFileTypeFilter(DataEntry::FileType type)
{
    filterFilteType = type;
    invalidateFilter();
}

void FileSortFilterProxyModel::setDateTimeFilter(QDateTime start, QDateTime end)
{
    if ( start < end ) {
        QDateTime bend = end;
        end = start;
        start = bend;
    }

    startDateTime = start;
    endDateTime   = end;

    invalidateFilter();
}

void FileSortFilterProxyModel::setStartDateTimeFilter(QDateTime start)
{
    startDateTime = start;
    invalidateFilter();
}

void FileSortFilterProxyModel::setEndDateTimeFilter(QDateTime end)
{
    endDateTime = end;
    invalidateFilter();
}

QDateTime FileSortFilterProxyModel::getStartDateTime() const
{
    return startDateTime;
}

QDateTime FileSortFilterProxyModel::getEndDateTime() const
{
    return endDateTime;
}

bool FileSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if ( !source_parent.isValid() || source_parent.internalPointer() != nullptr ) {
        // Camera name or root or error
        return true;
    }

    QModelIndex fileIndex = sourceModel()->index( source_row, 0, source_parent );

    DataEntry *file = static_cast<DataEntry *>(fileIndex.data( CameraDataModel::DataPointerRole ).value<void *>());

    // File type filter
    if ( !( file->fileType & filterFilteType ) ) {
        return false;
    }

    if ( !startDateTime.isValid() && !endDateTime.isValid() ) {
        return true;
    }

    // If datetime is not valid, update datetime
    if ( !file->dateTime.isValid() ) {
        file->updateDateTime();
    }

    if ( startDateTime.isValid() && file->dateTime < startDateTime ) {
        return false;
    }

    if ( endDateTime.isValid() && file->dateTime > endDateTime ) {
        return false;
    }

    return true;
}
