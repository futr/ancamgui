#ifndef WIDGET_H
#define WIDGET_H

/*
 * ファイルリストのモデルと、カメラ名のモデルは分けるべきだった。
 * 分けてないのでぐちゃぐちゃになってきた。
 *
 *
 */

#include <QWidget>
#include <QProcess>
#include <QListWidget>
#include <QSettings>
#include <QAction>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QThread>
#include <QButtonGroup>
#include <QListView>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QStatusBar>
#include <QLocale>
#include <QTextEdit>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMenu>
#include <QAction>
#if QT_VERSION >= 0x050600
#include <QVersionNumber>
#endif
#include "configwidget.h"
#include "cameradataaccess.h"
#include "previewwidget.h"
#include "cameradatamodel.h"
#include "filesortfilterproxymodel.h"
#include "selectdatedialog.h"
#include "servermanager.h"
#include "serversettingdialog.h"
#include "filedeletedialog.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    void filterChanged(int index, bool updateView = true);
    void updateFileTreeViewByCameraName( QString camera );

    void itemSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void downloadAndDecryptFiles( QModelIndexList list );
    void downloadAndDecryptSelectedFiles();
    void downloadAndDecryptFile( QModelIndex idx ); // DEPRE
    void deleteFiles( QModelIndexList list, bool crypt, bool plain );
    void deleteSelectedFiles();

    void setProgress( int progress, int max );
    void clearProgress();
    void workFinished( bool succeeded );

    void setCurrentServer( int idx );
    void clearCurrentServer();

    // Messages
    void standardError( QString err );

    // UI Interaction
    void fileViewContextMenuRequested( QPoint p );
    void serverIndexChanged( int idx );
    void lastUpdateTimeChanged( QDateTime dateTime );
    void openPreview( const QModelIndex &idx );

    // UI actions
    void selectFromFilter();
    void selectToFilter();
    void clearDateTimeFilter();
    void showDetailStatus();
    void serversSettings();
    void aboutBox();

    // UI locker
    void lockProgressGUI( bool canCancel );
    void unlockProgressGUI();
    //void lockInvalidServerGUI();
    //void unlockInvalidServerGUI();

    // Control file view
    void setFocusAndActivateCurrent( bool activateCurrent = true );

private:
    void startWork();
    bool workIsFinished();
    bool workIsSucceeded();
    void clearCancelFlag();
    bool workIsCanceled();
    void setModelToFileView( QAbstractItemModel *model );   // DEPRE
    void removeModelFromFileView();
    DataEntry *getDataEntryFromIndex(const QModelIndex &index );

private slots:
    void cameraDataModelReseted();
    void cameraNameListSelected(const QModelIndex &index , const QModelIndex &prev);
    void currentFileIndexChanged( const QModelIndex &current, const QModelIndex &previous );
    void setSelectionStatusLabel( int count );
    void on_updateFileListButton_clicked();
    void on_downloadButton_clicked();
    void on_clearSelectionButton_clicked();
    void on_quitButton_clicked();
    void on_openMuninButton_clicked();
    void on_cancelButton_clicked();
    void on_configButton_clicked();
    void on_selectFromButton_clicked();
    void on_selectToButton_clicked();
    void on_clearDateTimeFilterButton_clicked();
    void on_showDetailButton_clicked();
    void on_serverSettingButton_clicked();

    void on_aboutButton_clicked();

signals:
    void cancelProcess( void );
    void executeDownloadQueue();
    void executeDecryptQueue();

    void startUpdateFileList();
    void startDownload(DataEntry*);
    void startDecrypt(DataEntry*);
    void startParseFileList();

private:
    Ui::Widget *ui;
    FileDeleteDialog *deleteFileDialog;

    QAction *actSettings, *actServerSettings, *actDownloadAndDecrypt, *actDeleteFiles, *actSaveFiles;
    QMenu *fileViewMenu;
    
    CameraDataAccess *currentServer;
    CameraDataModel *currentServerModel;
    CameraDataAccess *cameraData;
    CameraDataModel *cameraDataModel;
    QSettings *settings;
    QProcess *m_muninPortForwardingProccess;
    PreviewWidget *preview;
    QThread workerThread;
    FileSortFilterProxyModel *proxyModel;
    QStringList errorList;
    ServerManager *serverManager;

    bool m_workFinished;
    bool m_workSucceeded;
    bool m_isCanceled;
    bool m_disableSelectServer;

#if QT_VERSION >= 0x050700
    QVersionNumber versionNumber;
#endif
    QString versionStr;

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event);
};

#endif // WIDGET_H
