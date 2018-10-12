#include "cameradataaccess.h"

CameraDataAccess::CameraDataAccess(QObject *parent) : QObject(parent)
{
    m_running = false;
    m_maxArgLenght = 31000; // The maximum lenght of a arg of CreateProcess is less than 32767 on Windows systems
}

void CameraDataAccess::setupServerInfo(QString serverName, QString serverPort, QString rootDir)
{
    m_serverName = serverName;
    m_serverPort = serverPort;
    m_rootDir    = rootDir;
}

void CameraDataAccess::setupLocalInfo(QString keyID, QString downDir, QString plainDir, QString sshKeyPath)
{
    m_keyID    = keyID;
    m_downDir  = downDir;
    m_plainDir = plainDir;
    m_sshKeyPath = sshKeyPath;
}

void CameraDataAccess::setProgramPath(QString sshPath, QString gpgPath, QString rsyncPath )
{
    m_sshPath = sshPath;
    m_gpgPath = gpgPath;
    m_rsyncPath = rsyncPath;
}

bool CameraDataAccess::updateFileList()
{
    // Get a file list from server
    QString program;
    QStringList args;
    QProcess process;

    m_succeeded = false;
    m_running = true;

    // Logging
    emit standardError( tr( "Start updating file entries from %1, Please wait..." ).arg( m_serverName ) );

    // Progress
    emit setProgress( 0, 0 );

    program = m_sshPath;
    args << "-p" << m_serverPort << "-i" << m_sshKeyPath << m_serverName << "/usr/local/anzencamera/bin/listAllFiles.sh";

    // qDebug() << program << args;

    connect( this, &CameraDataAccess::killProcess, &process, &QProcess::kill, Qt::DirectConnection );
    process.setProgram( program );
    process.setArguments( args );
    process.start();

    // block
    process.waitForFinished( 60000 );

    emit standardError( tr( "Receiving file list is completed" ) );
    emit standardError( QString::fromLocal8Bit( process.readAllStandardError() ) );

    if ( process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0 ) {
        emit standardError( tr( "Updating file list is failed" ) );
        emit setProgress( 1, 1 );

        finishProcess( false );
        return false;
    }

    // Read stdout into QStringList
    QTextStream stream( process.readAllStandardOutput() );

    m_fileList.clear();

    while ( !stream.atEnd() ) {
        m_fileList << stream.readLine();
    }

    // Progress
    emit setProgress( 1, 1 );

    if ( m_fileList.size() < 3 ) {
        finishProcess( false );
        return false;
    }

    // Logging
    emit standardError( tr( "%1 file entries has been received" ).arg( m_fileList.count() ) );

    // notify time
    m_lastUpdated = QDateTime::currentDateTime();
    emit fileListIsUpdated( m_lastUpdated );

    finishProcess( true );
    return true;
}

QStringList CameraDataAccess::getFileList() const
{
    return m_fileList;
}

void CameraDataAccess::parseFileList()
{
    // Notify reset will be start
    emit beginResetAllData();

    m_dataTree.clear();

    // reset queues
    m_dlQueue.clear();
    m_decQueue.clear();

    m_succeeded = false;
    m_running = true;

    // Progress
    emit setProgress( 0, 0 );

    int processed = 0;

    for ( int i = 0; i < m_fileList.count(); i++ ) {
        QString line = m_fileList[i];
        QStringList elem = line.split( " " );

        if ( elem.count() < 6 ) {
            break;
        }

        int depth = elem[0].toInt();
        QString mode = elem[1];
        QString path = elem[2];
        QString name = elem[3];
        // QDateTime dateTime = QDateTime::fromTime_t( elem[4].toDouble() );
        QString fileSize = elem[5];

        switch ( depth ) {
        case 1:
            // Camera name directory
            m_dataTree[name] = QList<DataEntry>();
            break;
        case 2:
            // file entry
            QStringList namesplit = name.split( "-" );
            QString strDateTime = namesplit[1].mid( 0, 14 );
            //QDateTime nameDateTime = QDateTime::fromString( strDateTime, "yyyyMMddHHmmss" );    // TOTTEMO OMOI Very heavy process

            DataEntry entry;
            entry.index = static_cast<quint32>( i );
            //entry.dateTime = nameDateTime;
            entry.dateTime = QDateTime();
            entry.strDateTime = strDateTime;
            entry.numStrDateTime = strDateTime.toULongLong();
            entry.name = name;
            entry.plainName = QFileInfo( name ).completeBaseName();
            entry.path = path;
            entry.camera = QFileInfo( path ).fileName();
            entry.dataAccess = this;
            entry.fileSize = fileSize.toULongLong();
            entry.online = true;

            // Decide file type
            if ( entry.name.contains( "jpg" ) && entry.name.contains( "snapshot" ) ) {
                entry.fileType = DataEntry::Snapshot;
            } else if ( entry.name.contains( "jpg" ) ) {
                entry.fileType = DataEntry::VideoThumb;
            } else {
                entry.fileType = DataEntry::Video;
            }

            // If you need to sort by check status, Uncomment the below
            // entry.updateFlags();

            m_dataTree[entry.camera] << entry;
            break;
        }

        processed++;

        // Progress
        if ( !( i % 1000 ) ) {
            emit standardError( tr( "Parsing... %1 / %2" ).arg( i ).arg( m_fileList.count() ) );
            emit setProgress( i, m_fileList.count() );
        }
    }

    // Reset progress
    emit setProgress( 0, 0 );

    // Add local files
    emit standardError( tr( "Searching local files" ) );
    addAllLocalOnlyFiles();

    // sort
    emit standardError( tr( "Sorting the file list" ) );
    sortAllFiles();

    // Set item index
    emit standardError( tr( "Calculating file indexes" ) );
    resetAllFileIndex();

    // Progress
    emit setProgress( processed, processed );

    finishProcess( true );

    // Notify reset process has been completed
    emit endResetAllData();

    emit standardError( tr( "Parsing the file list is completed (%1 entries)" ).arg( processed ) );
}

QStringList CameraDataAccess::getCameraNames() const
{
    return m_dataTree.keys();
}

QList<DataEntry> *CameraDataAccess::getFileEntryListByCameraName(QString camera)
{
    return &m_dataTree[camera];
}

QStringList CameraDataAccess::getOnlineFileNameListByCameraName(QString camName)
{
    // Return a file name list for camera
    QStringList files;
    const int count = m_dataTree[camName].count();

    for ( int i = 0; i < count; i++ ) {
        if ( m_dataTree[camName][i].isOnline() ) {
            files << m_dataTree[camName][i].name;
        }
    }

    return files;
}

int CameraDataAccess::cameraCount() const
{
    return m_dataTree.keys().count();
}

QString CameraDataAccess::getCameraNameByCameraIndex(int idx) const
{
    if ( idx >= cameraCount() ) {
        return "";
    }

    return getCameraNames()[idx];
}

int CameraDataAccess::getFileEntryCountByCameraName(QString camName)
{
    return getFileEntryListByCameraName( camName )->count();
}

bool CameraDataAccess::downloadFile(DataEntry *entry)
{
    // Download
    QString program;
    QStringList args;
    QString sshOptions;
    QString fileName;
    QString targetDir;
    QProcess process;

    // -rtv -e "ssh -p21" test@127.0.0.1:/var/safetycamera/data/cam1/\*-20160205\*.pgp ./

    m_succeeded = false;
    m_running = true;

    emit standardError( tr( "Start downloading a file" ) );

    // Progress
    //emit setProgress( 0, 0 );

    // Windowsの場合rsyncに渡すパスをいじる必要がある
    // Windowsではしょうがないのでscpを使う
    if ( QSysInfo::windowsVersion() ) {
        program = m_rsyncPath;
        fileName = m_serverName + ":" + entry->path + "/" + entry->name;
        // QString mingwDir = "/" + m_downDir;
        // mingwDir.remove( ":" );
        // targetDir = mingwDir + "/" + entry->camera + "/" + entry->name;
        targetDir =  QDir::toNativeSeparators( m_downDir + "/" + entry->camera );

        args << "-P" << m_serverPort << "-i" << QDir::toNativeSeparators( m_sshKeyPath ) << fileName << targetDir;
    } else {
        program = m_rsyncPath;
        sshOptions = m_sshPath + " -p" + m_serverPort;
        fileName = m_serverName + ":" + entry->path + "/" + entry->name;
        targetDir = m_downDir + "/" + entry->camera + "/";

        args << "-rtv" << "-e" << sshOptions << fileName << targetDir;
    }

    QDir dir;
    dir.mkpath( targetDir );

    process.setProgram( program );
    process.setArguments( args );
    process.start();

    // block
    process.waitForFinished( 120000 );

    qDebug() << process.arguments();
    emit standardError( QString::fromLocal8Bit( process.readAllStandardError() ) );
    emit standardError( QString::fromLocal8Bit( process.readAllStandardOutput() ) );

    entry->updateFlags();

    // Progress
    //emit setProgress( 1, 1 );

    if ( process.exitStatus() != QProcess::NormalExit ) {
        emit standardError( tr( "Downloading failed" ) );

        finishProcess( false );
        return false;
    }

    finishProcess( true );

    emit fileUpdated( entry->camera, entry );

    return true;
}

bool CameraDataAccess::decryptFile(DataEntry *entry)
{
    // Decrypt
    QString program;
    QStringList args;
    QString fileName;
    QString targetName;
    QProcess process;
    QDir targetDir;

    startProcess();
    emit standardError( tr( "Start decrypting a file" ) );

    // Progress
    //emit setProgress( 0, 0 );

    targetDir.mkpath( m_plainDir + "/" + entry->camera );

    program = m_gpgPath;
    fileName =  QDir::toNativeSeparators( entry->getLocalFullPath() );
    targetName = QDir::toNativeSeparators( entry->getLocalDecryptedFullPath() );

    args << "--output" << targetName << "--batch" << "--yes" << "-d" << fileName;

    process.setArguments( args );
    process.setProgram( program );
    process.start( QIODevice::ReadOnly );

    // blockするけど，タイムアウトしないようにした
    process.waitForFinished( -1 );

    //qDebug() << process.readAllStandardError();
    emit standardError( QString::fromLocal8Bit( process.readAllStandardError() ) );

    entry->updateFlags();

    // Progress
    //emit setProgress( 1, 1 );

    if ( process.exitStatus() != QProcess::NormalExit ) {
        finishProcess( false );
        return false;
    }

    finishProcess( true );

    emit fileUpdated( entry->camera, entry );

    return true;
}

void CameraDataAccess::cancelProcess()
{
    m_cancelled = true;

    emit killProcess();
}

void CameraDataAccess::enqueueDownload(DataEntry *file)
{
    if ( !file->online ) return;

    m_dlQueue.enqueue( file );
}

void CameraDataAccess::enqueueDecrypt(DataEntry *file)
{
    if ( !file->online ) return;

    m_decQueue.enqueue( file );
}

void CameraDataAccess::clearDownloadQueue()
{
    m_dlQueue.clear();
}

void CameraDataAccess::clearDecryptQueue()
{
    m_decQueue.clear();
}

bool CameraDataAccess::execDownloadQueue()
{
    // Download
    startProcess();

    if ( m_dlQueue.isEmpty() ) {
        emit standardError( tr( "The queue for downloading is empty (The file has already been downloaded, or ERROR?)" ) );
        finishProcess( false );
        return false;
    }

    emit standardError( tr( "Start downloading files (QUEUE)" ) );

    while ( !m_dlQueue.isEmpty() ) {
        // Create list of files to download
        QString program;
        QStringList args;
        QString sshOptions;
        QString fileName;
        QString fileNameList;
        QString targetDir;
        QProcess process;
        QList<DataEntry *> processedFiles;
        DataEntry *first;

        first = m_dlQueue.dequeue();
        processedFiles << first;

        fileName = m_serverName + ":" + first->path + "/";

        fileNameList = first->name;
        int fileCount = 1;

        while ( fileName.length() < m_maxArgLenght && !m_dlQueue.isEmpty() ) {
            // if camera of head is different from camera of first, Stop dequeuing
            if ( m_dlQueue.head()->camera != first->camera ) {
                break;
            }

            DataEntry *f = m_dlQueue.dequeue();

            processedFiles << f;

            fileNameList += ",";
            fileNameList += f->name;
            fileCount++;
        }

        if ( fileCount > 1 ) {
            fileName += "{" + fileNameList + "}";
        } else {
            fileName += fileNameList;
        }

        if ( QSysInfo::windowsVersion() ) {
            program = m_rsyncPath;
            targetDir =  QDir::toNativeSeparators( m_downDir + "/" + first->camera );

            args << "-P" << m_serverPort << "-i" << QDir::toNativeSeparators( m_sshKeyPath ) << fileName << targetDir;

            connect( &process, &QProcess::readyReadStandardError, this, &CameraDataAccess::readAndPassErrorMessageFromProcess );
        } else {
            program = m_rsyncPath;
            sshOptions = m_sshPath + " -p" + m_serverPort + " -i " + m_sshKeyPath;
            targetDir = m_downDir + "/" + first->camera + "/";

            args << "-rthv" << "--progress" << "-e" << sshOptions << fileName << targetDir;
        }

        QDir dir;
        dir.mkpath( targetDir );

        connect( this, &CameraDataAccess::killProcess, &process, &QProcess::kill, Qt::DirectConnection );
        connect( &process, &QProcess::readyReadStandardOutput, this, &CameraDataAccess::readAndPassMessageFromProcess );
        process.setProgram( program );
        process.setArguments( args );
        process.start();

        // block
        process.waitForFinished( 30000 * processedFiles.count() );

        qDebug() << process.arguments();
        emit standardError( QString::fromLocal8Bit( process.readAllStandardError() ) );
        emit standardError( QString::fromLocal8Bit( process.readAllStandardOutput() ) );

        for ( auto f : processedFiles ) {
            // We should update flags before emit result
            // Although downloading process has been failed, Some files may be downloaded.
            f->updateFlags();
            //emit fileUpdated( f->camera, f );
        }

        if ( process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0 ) {
            emit standardError( tr( "Downloading failed (QUEUE)" ) );
            finishProcess( false );
            return false;
        }
    }

    emit standardError( tr( "Downloading is succeeded (QUEUE)" ) );
    finishProcess( true );

    return true;
}

bool CameraDataAccess::execDecryptQueue()
{
    // Execute decryption queue
    startProcess();

    if ( m_decQueue.isEmpty() ) {
        emit standardError( tr( "The queue for decryption is empty (The file has already been decrypted, or ERROR?)" ) );
        finishProcess( false );
        return false;
    }

    int queueSize = m_decQueue.size();
    int i = 0;

    emit standardError( tr( "Start decrypting files (QUEUE)" ) );

    while ( !m_decQueue.isEmpty() ) {
        // Create list of files to decrypt
        QString program;
        QStringList args;
        QString fileName;
        QString targetName;
        QProcess process;
        QDir targetDir;
        DataEntry *first;

        first = m_decQueue.dequeue();

        if ( first->decrypted ) {
            // The file has already been decrypted
            emit setProgress( i, queueSize );
            i++;

            continue;
        }

        emit standardError( tr( "Start dcerypting %1" ).arg( first->name ) );

        targetDir.mkpath( m_plainDir + "/" + first->camera );

        program = m_gpgPath;
        fileName =  QDir::toNativeSeparators( first->getLocalFullPath() );
        targetName = QDir::toNativeSeparators( first->getLocalDecryptedFullPath() );

        args << "-vv" << "--output" << targetName << "--batch" << "--yes" << "-d" << fileName;

        connect( this, &CameraDataAccess::killProcess, &process, &QProcess::kill, Qt::DirectConnection );
        connect( &process, &QProcess::readyReadStandardOutput, this, &CameraDataAccess::readAndPassMessageFromProcess );
        process.setArguments( args );
        process.setProgram( program );
        process.start( QIODevice::ReadOnly );

        // blockするけど，タイムアウトしないようにした
        process.waitForFinished( -1 );

        //qDebug() << process.readAllStandardError();
        emit standardError( QString::fromLocal8Bit( process.readAllStandardError() ) );

        emit setProgress( i, queueSize );
        i++;

        if ( process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0 ) {
            emit standardError( tr( "Decrypting failed (QUEUE)" ) );

            finishProcess( false );
            return false;
        }

        first->updateFlags();

        // Notify updated
        emit fileUpdated( first->camera, first );

        emit standardError( tr( "Dcerypting %1 is succeeded" ).arg( first->name ) );
    }

    emit standardError( tr( "Dcerypting is succeeded (QUEUE)" ) );
    finishProcess( true );

    return true;
}

void CameraDataAccess::startProcess()
{
    // m_cancelled = false;
    // If set to false here, main thread may emit cancel already;
    m_succeeded = false;
    m_running = true;
}

void CameraDataAccess::finishProcess(bool succeeded)
{
    m_running = false;
    m_succeeded = succeeded;
    emit workFinished( m_succeeded );
}

void CameraDataAccess::addLocalOnlyFilesOfCamera(QString camName)
{
    // TODO : This function hasnt been checked
    // Search and add local only files of camera to entries
    QDir camDir( getDownDir() + "/" + camName + "/" );
    QDir plainDir( getPlainDir() + "/" + camName + "/" );

    QFileInfoList fileList = camDir.entryInfoList();
    QStringList onlineFileNameList = getOnlineFileNameListByCameraName( camName );

    for ( auto file : fileList ) {
        QString name = file.fileName();

        // It may be veeeeery slow process
        if ( onlineFileNameList.contains( name ) ) continue;
        if ( !name.contains( "pgp" ) ) continue;

        // file entry
        QStringList namesplit = name.split( "-" );
        QString strDateTime = namesplit[1].mid( 0, 14 );

        DataEntry entry;
        entry.index = static_cast<quint32>( getFileEntryCountByCameraName( camName ) );
        entry.strDateTime = strDateTime;
        entry.dateTime = QDateTime();
        entry.numStrDateTime = strDateTime.toULongLong();
        entry.name = name;
        entry.plainName = QFileInfo( name ).completeBaseName();
        entry.path = file.path();
        entry.camera = QFileInfo( file.path() ).fileName();
        entry.dataAccess = this;
        entry.fileSize = static_cast<quint64>(file.size());
        entry.online = false;

        // Decide file type
        if ( entry.name.contains( "jpg" ) && entry.name.contains( "snapshot" ) ) {
            entry.fileType = DataEntry::Snapshot;
        } else if ( entry.name.contains( "jpg" ) ) {
            entry.fileType = DataEntry::VideoThumb;
        } else {
            entry.fileType = DataEntry::Video;
        }

        // entry.updateFlags();

        m_dataTree[entry.camera] << entry;
    }
}

void CameraDataAccess::addAllLocalOnlyFiles()
{
    for ( auto camName : getCameraNames() ) {
        addLocalOnlyFilesOfCamera( camName );
    }
}

QDateTime CameraDataAccess::getLastUpdated() const
{
    return m_lastUpdated;
}

QString CameraDataAccess::getMuninPort() const
{
    return m_muninPort;
}

void CameraDataAccess::setMuninPort(const QString &muninPort)
{
    m_muninPort = muninPort;
}

QString CameraDataAccess::getRsyncPath() const
{
    return m_rsyncPath;
}

void CameraDataAccess::setRsyncPath(const QString &rsyncPath)
{
    m_rsyncPath = rsyncPath;
}

QString CameraDataAccess::getGpgPath() const
{
    return m_gpgPath;
}

void CameraDataAccess::setGpgPath(const QString &gpgPath)
{
    m_gpgPath = gpgPath;
}

QString CameraDataAccess::getSshPath() const
{
    return m_sshPath;
}

void CameraDataAccess::setSshPath(const QString &sshPath)
{
    m_sshPath = sshPath;
}

QString CameraDataAccess::getSshKeyPath() const
{
    return m_sshKeyPath;
}

void CameraDataAccess::setSshKeyPath(const QString &sshKeyPath)
{
    m_sshKeyPath = sshKeyPath;
}

void CameraDataAccess::setPlainDir(const QString &plainDir)
{
    m_plainDir = plainDir;
}

void CameraDataAccess::setDownDir(const QString &downDir)
{
    m_downDir = downDir;
}

QString CameraDataAccess::getName() const
{
    return m_name;
}

void CameraDataAccess::setName(const QString &name)
{
    m_name = name;
}

QString CameraDataAccess::getServerPort() const
{
    return m_serverPort;
}

void CameraDataAccess::setServerPort(const QString &serverPort)
{
    m_serverPort = serverPort;
}

QString CameraDataAccess::getServerName() const
{
    return m_serverName;
}

void CameraDataAccess::setServerName(const QString &serverName)
{
    m_serverName = serverName;
}

void CameraDataAccess::readAndPassMessageFromProcess()
{
    QProcess *p= qobject_cast<QProcess *>( sender() );
    emit standardError( QString::fromLocal8Bit( p->readAllStandardOutput() ) );
}

void CameraDataAccess::readAndPassErrorMessageFromProcess()
{
    QProcess *p= qobject_cast<QProcess *>( sender() );
    emit standardError( QString::fromLocal8Bit( p->readAllStandardError() ) );
}

QString CameraDataAccess::getDownDir() const
{
    return m_downDir;
}

QString CameraDataAccess::getPlainDir() const
{
    return m_plainDir;
}

void CameraDataAccess::sortFilesByCameraName(QString camName)
{
    QList<DataEntry> *fileList = getFileEntryListByCameraName( camName );

    std::sort( (*fileList).begin(), (*fileList).end(), std::greater<DataEntry>() );
}

void CameraDataAccess::sortAllFiles()
{
    for ( QString camName : getCameraNames() ) {
        sortFilesByCameraName( camName );
    }
}

void CameraDataAccess::resetFileIndex(QList<DataEntry> *fileList)
{
    for ( int i = 0; i < (*fileList).count(); i++ ) {
        (*fileList)[i].index = static_cast<quint32>(i);
    }
}

void CameraDataAccess::resetAllFileIndex()
{
    for ( QString camName : getCameraNames() ) {
        resetFileIndex( getFileEntryListByCameraName( camName ) );
    }
}

QString DataEntry::getRemoteFullPath()
{
    return path + "/" + name;
}

QString DataEntry::getLocalFullPath()
{
    return dataAccess->getDownDir() + "/" + camera + "/" + name;
}

QString DataEntry::getLocalDecryptedFullPath()
{
    return dataAccess->getPlainDir() + "/" + camera + "/" + plainName;
}

bool DataEntry::isDownloaded()
{
    return updateDlFlag();
}

bool DataEntry::isDecrypted()
{
    return updateDecryptFlag();
}

bool DataEntry::isOnline()
{
    return online;
}

void DataEntry::updateFlags()
{
    // Files exist
    updateDlFlag();
    updateDecryptFlag();
}

bool DataEntry::updateDlFlag()
{
    downloaded = QFileInfo( getLocalFullPath() ).exists();
    return  downloaded;
}

bool DataEntry::updateDecryptFlag()
{
    decrypted  = QFileInfo( getLocalDecryptedFullPath() ).exists();
    return decrypted;
}

bool DataEntry::deleteLocalCryptoFile()
{
    // Delete local file
    QFileInfo info( getLocalFullPath() );
    return info.dir().remove( name );
}

bool DataEntry::deleteLocalDecryptedFile()
{
    // Delete local file
    QFileInfo info( getLocalDecryptedFullPath() );
    return info.dir().remove( QFileInfo( name ).completeBaseName() );
}

void DataEntry::updateDateTime()
{
    dateTime = QDateTime::fromString( strDateTime, "yyyyMMddHHmmss" );
}

bool DataEntry::operator <(const DataEntry &l) const
{
    return numStrDateTime < l.numStrDateTime;
}

bool DataEntry::operator >(const DataEntry &l) const
{
    return numStrDateTime > l.numStrDateTime;
}
