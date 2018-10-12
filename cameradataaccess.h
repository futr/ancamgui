#ifndef CAMERADATAACCESS_H
#define CAMERADATAACCESS_H

/*
 * 更新ファイル通知をファイルごとに出すと名スレッドで詰まってしまう。
 * 複数ファイルまとめて出すようにすれば大丈夫だと思う。
 * Model/View自体は、複数ファイルに対応している。
 */

#include <QObject>
#include <QProcess>
#include <QTextStream>
#include <QDebug>
#include <QMap>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include <QRegExp>
#include <QDateTime>

class CameraDataAccess;

class DataEntry {
public:
    enum FileType {
        Video      = 0x01,
        VideoThumb = 0x02,
        Snapshot   = 0x04,
        AllType    = 0xFF,
    };

public:
    QString name;
    QString plainName;
    QString path;
    QString camera;
    quint64 fileSize;
    quint32 index;
    quint64 numStrDateTime;
    QDateTime dateTime;
    QString strDateTime;
    double unixTime;
    FileType fileType;

    bool downloaded;
    bool decrypted;
    bool online;

    CameraDataAccess *dataAccess;

public:
    QString getRemoteFullPath();
    QString getLocalFullPath();
    QString getLocalDecryptedFullPath();

    bool isDownloaded();
    bool isDecrypted();
    bool isOnline();
    void updateFlags();
    bool updateDlFlag();
    bool updateDecryptFlag();
    bool deleteLocalCryptoFile();
    bool deleteLocalDecryptedFile();
    void updateDateTime();

    bool operator < ( const DataEntry &l ) const;
    bool operator > ( const DataEntry &l ) const;
};
Q_DECLARE_METATYPE(DataEntry)

typedef QMap< QString, QList<DataEntry> > DataTree;
typedef QList<DataEntry> CameraDataList;

class CameraDataAccess : public QObject
{
    Q_OBJECT
public:
    explicit CameraDataAccess(QObject *parent = nullptr);

    void setupServerInfo( QString serverName, QString serverPort, QString rootDir );
    void setupLocalInfo( QString keyID, QString downDir, QString plainDir, QString sshKeyPath );
    void setProgramPath( QString sshPath, QString gpgPath, QString rsyncPath );

    QStringList getFileList() const;
    QStringList getCameraNames() const;
    QList<DataEntry> *getFileEntryListByCameraName( QString camera );
    QStringList getOnlineFileNameListByCameraName( QString camName );
    int cameraCount() const;
    QString getCameraNameByCameraIndex( int idx ) const;
    int getFileEntryCountByCameraName( QString camName );

    QString getDownDir() const;
    QString getPlainDir() const;
    void setDownDir(const QString &downDir);
    void setPlainDir(const QString &plainDir);

    // bool isRunging();
    // bool isSucceeded();

    void sortFilesByCameraName( QString camName );
    void sortAllFiles();
    void resetFileIndex( QList<DataEntry> *fileList );
    void resetAllFileIndex();

    QString getServerName() const;
    void setServerName(const QString &serverName);
    QString getServerPort() const;
    void setServerPort(const QString &serverPort);
    QString getName() const;
    void setName(const QString &name);
    QString getSshKeyPath() const;
    void setSshKeyPath(const QString &sshKeyPath);
    QString getSshPath() const;
    void setSshPath(const QString &sshPath);
    QString getGpgPath() const;
    void setGpgPath(const QString &gpgPath);
    QString getRsyncPath() const;
    void setRsyncPath(const QString &rsyncPath);
    QString getMuninPort() const;
    void setMuninPort(const QString &muninPort);
    QDateTime getLastUpdated() const;

public slots:
    bool updateFileList( void );
    void parseFileList( void );
    bool downloadFile( DataEntry *entry );
    bool decryptFile( DataEntry *entry );

    void cancelProcess();

    void enqueueDownload( DataEntry *file );
    void enqueueDecrypt( DataEntry *file );
    void clearDownloadQueue();
    void clearDecryptQueue();
    bool execDownloadQueue();
    bool execDecryptQueue();

private:
    void startProcess();
    void finishProcess( bool succeeded );
    void addLocalOnlyFilesOfCamera( QString camName );
    void addAllLocalOnlyFiles();

private:
    QString m_name;
    QString m_serverName, m_serverPort, m_rootDir;
    QString m_keyID, m_downDir, m_plainDir;
    QString m_sshKeyPath;
    QString m_sshPath, m_gpgPath, m_rsyncPath;
    QString m_muninPort;
    int m_maxArgLenght;

    QQueue<DataEntry *> m_dlQueue;
    QQueue<DataEntry *> m_decQueue;

    QStringList m_fileList;
    DataTree m_dataTree;

    QDateTime m_lastUpdated;

    bool m_running;
    bool m_succeeded;
    bool m_cancelled;

private slots:
    void readAndPassMessageFromProcess();
    void readAndPassErrorMessageFromProcess();

signals:
    void fileListIsUpdated( QDateTime dateTime );
    void standardError( QString str );
    void workFinished( bool succeeded );
    void setProgress( int progress, int max );
    void beginResetAllData();
    void endResetAllData();
    void fileUpdated( QString camName, quint32 fileIndex ); // DEPRE
    void fileUpdated( QString camName, DataEntry *file );
    void killProcess();

public slots:
};

#endif // CAMERADATAACCESS_H
