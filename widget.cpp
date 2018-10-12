#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    // Initialize vars
#if QT_VERSION >= 0x050700
    versionNumber = QVersionNumber( 0, 2, 1 );
#endif
    versionStr = "0.2.1";
    m_workFinished = true;
    m_disableSelectServer = false;
    currentServer = nullptr;
    currentServerModel = nullptr;

    // Read settings
    settings   = new QSettings( QSettings::IniFormat, QSettings::UserScope, "anzencamera", "anzencameraGUI", this );
    settings->setFallbacksEnabled( false );
    standardError( tr( "READ SETTINGS FROM : " ) + settings->fileName() );

    // Instance
    m_muninPortForwardingProccess = new QProcess( this );
    deleteFileDialog = new FileDeleteDialog( this );

    // Create server manager
    serverManager = new ServerManager( settings );
    serverManager->setWorkerThread( &workerThread );
    serverManager->loadSettings();

    // Start worker thread
    workerThread.start();

    // Create Model
    //cameraDataModel = new CameraDataModel( cameraData, this );
    proxyModel = new FileSortFilterProxyModel( this );
    proxyModel->setSortRole( CameraDataModel::SortRole );

    // Setup server selector
    ui->serverComboBox->setModel( serverManager->managerModel() );

    // Reset filter
    filterChanged( static_cast<int>( ui->filetypeCombobox->currentIndex() ) );

    // Setup preview Widget
    ui->previewWidget->layout()->setMargin( 1 );

    // Action initialize
    actDeleteFiles = new QAction( QIcon( ":/img/icons/user-trash.png" ), tr( "Delete files" ), this );
    connect( actDeleteFiles, &QAction::triggered, this, &Widget::deleteSelectedFiles );
    actSaveFiles = new QAction( QIcon( ":/img/icons/document-save.png" ), tr( "Copy files" ), this );
    actDownloadAndDecrypt = new QAction( QIcon( ":/img/icons/go-down.png" ), tr( "Download and Decrypt" ), this );
    actDownloadAndDecrypt->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    actDownloadAndDecrypt->setShortcut( QKeySequence( tr( "Ctrl+D" ) ) );
    addAction( actDownloadAndDecrypt );
    connect( actDownloadAndDecrypt, &QAction::triggered, this, &Widget::downloadAndDecryptSelectedFiles );

    // Create Menu
    fileViewMenu = new QMenu( this );
    fileViewMenu->addAction( actDownloadAndDecrypt );
    // fileViewMenu->addAction( actSaveFiles );
    fileViewMenu->addSeparator();
    fileViewMenu->addAction( actDeleteFiles );

    // Connect menu
    ui->fileView->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( ui->fileView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(fileViewContextMenuRequested(QPoint)) );

    // Setup Button icons
    ui->updateFileListButton->setIcon( QIcon( ":/img/icons/view-refresh.png" ) );
    ui->downloadButton->setIcon( QIcon( ":/img/icons/go-down.png" ) );
    ui->clearSelectionButton->setIcon( QIcon( ":/img/icons/edit-clear.png" ) );
    ui->openMuninButton->setIcon( QIcon( ":/img/icons/utilities-system-monitor.png" ) );
    ui->quitButton->setIcon( style()->standardIcon( QStyle::SP_DialogCloseButton ) );
    ui->cancelButton->setIcon( QIcon( ":/img/icons/process-stop.png" ) );
    ui->configButton->setIcon( QIcon( ":/img/icons/emblem-system.png" ) );
    ui->clearDateTimeFilterButton->setIcon( QIcon( ":/img/icons/edit-clear.png" ) );
    ui->selectFromButton->setIcon( QIcon( ":/img/icons/x-office-calendar.png" ) );
    ui->selectToButton->setIcon( QIcon( ":/img/icons/x-office-calendar.png" ) );
    ui->serverSettingButton->setIcon( QIcon( ":/img/icons/system-file-manager.png" ) );
    ui->aboutButton->setIcon( style()->standardIcon( QStyle::SP_FileDialogInfoView ) );

    // Setup splitters
    ui->horizontalSplitter->setStretchFactor( 0, 0 );
    ui->horizontalSplitter->setStretchFactor( 1, 1 );

    // Init labels
    lastUpdateTimeChanged( QDateTime() );

    // UI interaction
    connect( ui->fileView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openPreview(QModelIndex)) );
    connect( ui->filetypeCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(filterChanged(int)) );
    connect( ui->serverComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(serverIndexChanged(int)) );

    // Select a server
    if ( serverManager->serverCount() > 0 ) {
        int idx = settings->value( "local/serverIndex", 0 ).toInt();

        if ( idx >= serverManager->serverCount() ) idx = 0;

        if ( idx == ui->serverComboBox->currentIndex() ) {
            serverIndexChanged( idx );
        } else {
            ui->serverComboBox->setCurrentIndex( idx );
        }
    }

    // Unlock progress GUI mode
    unlockProgressGUI();
}

Widget::~Widget()
{
    workerThread.quit();
    workerThread.wait();

    delete ui;
}

void Widget::filterChanged(int index , bool updateView)
{
    // Check filter index
    switch ( index ) {
    case 0: // All
        proxyModel->setFileTypeFilter( DataEntry::AllType );
        break;
    case 1: // Image only
        proxyModel->setFileTypeFilter( DataEntry::FileType( DataEntry::Snapshot | DataEntry::VideoThumb ) );
        break;
    case 2: // First frame
        proxyModel->setFileTypeFilter( DataEntry::VideoThumb );
        break;
    case 3: // Video and First frame
        proxyModel->setFileTypeFilter( DataEntry::FileType( DataEntry::Video | DataEntry::VideoThumb ) );
        break;
    case 4: // Video only
        proxyModel->setFileTypeFilter( DataEntry::Video );
        break;
    default:
        proxyModel->setFileTypeFilter( DataEntry::AllType );
        break;
    }

    if ( updateView ) setFocusAndActivateCurrent( true );
}

void Widget::updateFileTreeViewByCameraName(QString camera)
{
    if ( currentServer == nullptr ) return;

    ui->fileView->clearSelection();
    ui->fileView->setRootIndex( proxyModel->mapFromSource( currentServerModel->index( camera ) ) );
}

void Widget::openPreview(const QModelIndex &idx)
{
    // File is double-clicked (M/V)
    DataEntry *entry = getDataEntryFromIndex( idx );

    if ( !entry ) return;

    if ( !entry->updateDecryptFlag() ) downloadAndDecryptFiles( QModelIndexList( {idx} ) );
    if ( entry->updateDecryptFlag() ) ui->previewWidget->setFileName( entry->getLocalDecryptedFullPath() );
}

void Widget::itemSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    QItemSelectionModel *sm = qobject_cast<QItemSelectionModel *>(sender());

    setSelectionStatusLabel( sm->selectedRows().count() );
}

void Widget::downloadAndDecryptFiles(QModelIndexList list)
{
    if ( currentServer == nullptr ) return;
    if ( !m_workFinished ) return;
    if ( !list.count() ) return;

    // Lock GUI
    lockProgressGUI( true );

    // Progress
    setProgress( 0, 0 );

    // Clear queue
    currentServer->clearDownloadQueue();
    currentServer->clearDecryptQueue();

    // Queuing
    for ( int i = 0; i < list.count(); i++ ) {
        QModelIndex source = proxyModel->mapToSource( list[i] );
        DataEntry *e = static_cast<DataEntry *>(source.data( CameraDataModel::DataPointerRole ).value<void *>());

        if ( !e->isDownloaded() ) currentServer->enqueueDownload( e );
        if ( !e->isDecrypted() )  currentServer->enqueueDecrypt( e );
    }

    // Start downloading
    startWork();

    emit executeDownloadQueue();

    while ( !workIsFinished() ) {
        QApplication::processEvents();
    }

    if ( workIsCanceled() ) {
        // Progress
        setProgress( list.count(), list.count() );

        // Unlock GUI
        unlockProgressGUI();

        return;
    }

    // Start decrypting
    startWork();

    emit executeDecryptQueue();

    while ( !workIsFinished() ) {
        QApplication::processEvents();
    }

    // Progress
    setProgress( list.count(), list.count() );

    // Unlock GUI
    unlockProgressGUI();

    // Set focust to view
    ui->fileView->setFocus();
}

void Widget::downloadAndDecryptSelectedFiles()
{
    if ( currentServer == nullptr ) return;

    QModelIndexList selectedIdxs = ui->fileView->selectionModel()->selectedRows();

    downloadAndDecryptFiles( selectedIdxs );
}

void Widget::downloadAndDecryptFile(QModelIndex /*idx*/)
{
    // DEPRECATED
    /*
    DataEntry *entry = getDataEntryFromIndex( idx );

    if ( entry == nullptr ) return;

    if ( !entry->downloaded ) {
        startWork();
        emit startDownload( entry );

        while ( !workIsFinished() ) {
            QApplication::processEvents();
        }
    }

    standardError( tr( "Donwloading process is completed" ) );

    if ( entry->downloaded && !entry->decrypted ) {
        startWork();
        emit startDecrypt( entry );

        while ( !workIsFinished() ) {
            QApplication::processEvents();
        }
    }
    */
}

void Widget::deleteFiles(QModelIndexList list, bool crypt, bool plain)
{
    if ( currentServer == nullptr ) return;
    if ( !m_workFinished ) return;
    if ( !list.count() ) return;

    // Lock GUI
    lockProgressGUI( true );

    // Progress
    setProgress( 0, 0 );

    //
    for ( int i = 0; i < list.count(); i++ ) {
        QModelIndex source = proxyModel->mapToSource( list[i] );
        DataEntry *e = static_cast<DataEntry *>(source.data( CameraDataModel::DataPointerRole ).value<void *>());

        if ( crypt ) {
            e->deleteLocalCryptoFile();
        }

        if ( plain ) {
            e->deleteLocalDecryptedFile();
        }
    }

    // Progress
    setProgress( list.count(), list.count() );

    emit standardError( tr( "%1 files has been deleted" ).arg( list.count() ) );

    // Unlock GUI
    unlockProgressGUI();

    // Set focus to view
    ui->fileView->setFocus();
}

void Widget::deleteSelectedFiles()
{
    if ( currentServer == nullptr ) return;

    deleteFileDialog->setupInfo( ui->fileView->selectionModel()->selectedRows().count(), deleteFileDialog->deletePlain(), deleteFileDialog->deleteCrypto() );

    if ( deleteFileDialog->exec() != QDialog::Accepted ) {
        return;
    }

    deleteFiles( ui->fileView->selectionModel()->selectedRows(), deleteFileDialog->deleteCrypto(), deleteFileDialog->deletePlain() );

    ui->previewWidget->clearPreview();
}

void Widget::lockProgressGUI(bool canCancel)
{
    // Progress
    ui->cancelButton->setEnabled( canCancel );
    ui->cameraNameListView->setEnabled( false );
    ui->fileView->setEnabled( false );
    ui->updateFileListButton->setEnabled( false );
    ui->clearSelectionButton->setEnabled( false );
    ui->downloadButton->setEnabled( false );
    ui->quitButton->setEnabled( false );
    ui->configButton->setEnabled( false );
    ui->filetypeCombobox->setEnabled( false );
    ui->serverComboBox->setEnabled( false );
    ui->serverSettingButton->setEnabled( false );
    ui->selectFromButton->setEnabled( false );
    ui->selectToButton->setEnabled( false );
    ui->clearDateTimeFilterButton->setEnabled( false );
    ui->aboutButton->setEnabled( false );
}

void Widget::unlockProgressGUI()
{
    ui->cancelButton->setEnabled( false );
    ui->cameraNameListView->setEnabled( true );
    ui->fileView->setEnabled( true );
    ui->updateFileListButton->setEnabled( true );
    ui->clearSelectionButton->setEnabled( true );
    ui->downloadButton->setEnabled( true );
    ui->quitButton->setEnabled( true );
    ui->configButton->setEnabled( true );
    ui->filetypeCombobox->setEnabled( true );
    ui->serverComboBox->setEnabled( true );
    ui->serverSettingButton->setEnabled( true );
    ui->selectFromButton->setEnabled( true );
    ui->selectToButton->setEnabled( true );
    ui->clearDateTimeFilterButton->setEnabled( true );
    ui->aboutButton->setEnabled( true );

    clearProgress();
}

void Widget::setProgress( int progress, int max )
{
    ui->progressBar->setEnabled( true );
    ui->progressBar->setMaximum( max );
    ui->progressBar->setValue( progress );
    ui->progressBar->setVisible( true );
}

void Widget::clearProgress()
{
    ui->progressBar->setEnabled( false );
    ui->progressBar->setValue( 0 );
    ui->progressBar->setMaximum( 100 );
    ui->progressBar->setVisible( false );
}

void Widget::workFinished(bool succeeded)
{
    m_workFinished = true;
    m_workSucceeded = succeeded;
}

void Widget::standardError(QString err)
{
    errorList.append( err );
    // ui->errorBrowser->append( err );

    for ( auto str : err.split( '\n' ) ) {
        if ( str != "" ) ui->errorLabel->setText( str );
    }
}

void Widget::fileViewContextMenuRequested(QPoint p)
{
    if ( ui->fileView->selectionModel()->selectedRows().count() == 0 ) return;

    fileViewMenu->exec( ui->fileView->viewport()->mapToGlobal( p ) );
}

void Widget::setCurrentServer( int idx )
{
    qDebug() <<  "Set current server" << idx;

    // ongoing work should be finished
    if ( !workIsFinished() ) return;

    // Cant change current during server configuration
    if ( m_disableSelectServer ) return;

    qDebug() <<  "Setting current start " << idx;

    // Clear ui status
    setSelectionStatusLabel( 0 );
    lastUpdateTimeChanged( QDateTime() );

    // Rmeove current model from proxy ( This is very important in some enviroment )
    proxyModel->setSourceModel( nullptr );

    // Clear file view
    removeModelFromFileView();

    // Get server instance
    CameraDataAccess *server = serverManager->getServer( idx );
    CameraDataModel *model   = serverManager->getServerModel( idx );

    // Remove curruent connections
    if ( currentServerModel ) {
        currentServerModel->disconnect( this );
    }

    if ( currentServer ) {
        disconnect( currentServer );
        currentServer->disconnect( this );
    }

    // Check if server is valid
    if ( server == nullptr ) {
        // Server will be removed if invalid index ( -1 or greater than or equal to count of server ) is specified ( current-server will be disconnected and set to null )
        clearCurrentServer();
        return;
    }

    currentServerModel = model;
    currentServer = server;

    // Connect ( server ctxs signals )
    connect( this, &Widget::executeDownloadQueue, currentServer, &CameraDataAccess::execDownloadQueue, Qt::UniqueConnection );
    connect( this, &Widget::executeDecryptQueue, currentServer, &CameraDataAccess::execDecryptQueue, Qt::UniqueConnection );
    connect( this, &Widget::cancelProcess, currentServer, &CameraDataAccess::cancelProcess, static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection) );
    connect( currentServer, SIGNAL(setProgress(int,int)), this, SLOT(setProgress(int,int)), Qt::UniqueConnection );
    connect( currentServer, SIGNAL(workFinished(bool)), this, SLOT(workFinished(bool)), Qt::UniqueConnection );
    connect( this, SIGNAL(cancelProcess()), currentServer, SLOT(cancelProcess()), Qt::UniqueConnection );
    connect( this, SIGNAL(startUpdateFileList()), currentServer, SLOT(updateFileList()), Qt::UniqueConnection );
    connect( this, SIGNAL(startParseFileList()), currentServer, SLOT(parseFileList()), Qt::UniqueConnection );
    connect( this, SIGNAL(startDownload(DataEntry*)), currentServer, SLOT(downloadFile(DataEntry*)), Qt::UniqueConnection );
    connect( this, SIGNAL(startDecrypt(DataEntry*)), currentServer, SLOT(decryptFile(DataEntry*)), Qt::UniqueConnection );
    connect( currentServer, SIGNAL(standardError(QString)), this, SLOT(standardError(QString)), Qt::UniqueConnection );
    connect( currentServer, SIGNAL(fileListIsUpdated(QDateTime)), this, SLOT(lastUpdateTimeChanged(QDateTime)), Qt::UniqueConnection );

    // Connect ( models siganls )
    connect( currentServerModel, &CameraDataModel::modelReset, this, &Widget::cameraDataModelReseted );

    // Set current to proxy
    proxyModel->setSourceModel( currentServerModel );
    proxyModel->setSortRole( CameraDataModel::SortRole );

    // Reset filter
    filterChanged( static_cast<int>( ui->filetypeCombobox->currentIndex() ) );

    // Setup Camera Name view
    ui->cameraNameListView->setModel( currentServerModel );
    connect( ui->cameraNameListView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(cameraNameListSelected(QModelIndex, QModelIndex)), Qt::UniqueConnection );

    // Update UI
    lastUpdateTimeChanged( server->getLastUpdated() );
    emit standardError( tr( "Server \"%1\" (%2) is selected" ).arg( currentServer->getName() ).arg( currentServer->getServerName() ) );
}

void Widget::clearCurrentServer()
{
    proxyModel->setSourceModel( nullptr );
    removeModelFromFileView();
    ui->cameraNameListView->setModel( nullptr );

    currentServer = nullptr;
    currentServerModel = nullptr;
}

void Widget::serverIndexChanged(int idx)
{
    setCurrentServer( idx );
}

void Widget::lastUpdateTimeChanged(QDateTime dateTime)
{
    ui->lastUpdateLabel->setText( tr( "Last update : " ) + dateTime.toString( QLocale().dateTimeFormat( QLocale::NarrowFormat ) ) );
}

void Widget::selectFromFilter()
{
    SelectDateDialog *dialog = new SelectDateDialog( this );
    dialog->setTitle( tr( "Filtering files created after selected date" ) );

    if ( proxyModel->getStartDateTime().isValid() ) {
        dialog->setDate( proxyModel->getStartDateTime() );
    }

    if ( proxyModel->getEndDateTime().isValid() ) {
        dialog->setMaximumDateTime( proxyModel->getEndDateTime() );
    }

    if ( dialog->exec() != QDialog::Accepted ) {
        return;
    }

    // Set text to ui
    ui->selectFromButton->setText( dialog->getSelectedDate().toString( QLocale().dateTimeFormat( QLocale::NarrowFormat ) ) );

    // Update filter
    proxyModel->setStartDateTimeFilter( dialog->getSelectedDate() );

    setFocusAndActivateCurrent();
}

void Widget::selectToFilter()
{
    SelectDateDialog *dialog = new SelectDateDialog( this );
    dialog->setTitle( tr( "Filtering files created before selected date" ) );

    if ( proxyModel->getEndDateTime().isValid() ) {
        dialog->setDate( proxyModel->getEndDateTime() );
    }

    if ( proxyModel->getStartDateTime().isValid() ) {
        dialog->setMinimumDateTime( proxyModel->getStartDateTime() );
    }

    if ( dialog->exec() != QDialog::Accepted ) {
        return;
    }

    QDateTime selected = dialog->getSelectedDate();
    selected.setTime( QTime( 23, 59, 59, 999 ) );

    // Set text to ui
    ui->selectToButton->setText( selected.toString( QLocale().dateTimeFormat( QLocale::NarrowFormat ) ) );

    // Update filter
    proxyModel->setEndDateTimeFilter( selected );

    setFocusAndActivateCurrent();
}

void Widget::clearDateTimeFilter()
{
    proxyModel->setDateTimeFilter( QDateTime(), QDateTime() );
    ui->selectFromButton->setText( "" );
    ui->selectToButton->setText( "" );

    setFocusAndActivateCurrent();
}

void Widget::showDetailStatus()
{
    QTextEdit *edit = new QTextEdit( this );
    edit->setWindowFlags( Qt::Dialog );
    edit->setAttribute( Qt::WA_DeleteOnClose, true );
    edit->setReadOnly( true );
    edit->setText( errorList.join( '\n' ) );
    edit->show();
}

void Widget::serversSettings()
{
    CameraDataAccess *beforeCurrent = currentServer;

    clearCurrentServer();

    ServerSettingDialog *dialog = new ServerSettingDialog( serverManager, beforeCurrent, this );

    m_disableSelectServer = true;
    dialog->exec();
    m_disableSelectServer = false;

    // Re-select server
    serverIndexChanged( ui->serverComboBox->currentIndex() );
}

void Widget::aboutBox()
{
    QMessageBox::about( this, tr( "About AnzenAnshinCamera" ), tr( "AnzenAnshinCamera GUI\nM/V Test version %1\n\n(c) 2018 Masato Takahashi\n\nRunning on Qt %2\n\nThis software is using Tango Icon Library" ).arg( /*versionNumber.toString()*/versionStr ).arg( qVersion() ) );
}

void Widget::setFocusAndActivateCurrent(bool activateCurrent)
{
    ui->fileView->setFocus();

    if ( activateCurrent && ui->fileView->currentIndex().isValid() ) {
        ui->fileView->scrollTo( ui->fileView->currentIndex() );
    }
}

void Widget::startWork()
{
    m_workFinished = false;
    clearCancelFlag();
}

bool Widget::workIsFinished()
{
    return m_workFinished;
}

bool Widget::workIsSucceeded()
{
    return m_workSucceeded;
}

void Widget::clearCancelFlag()
{
    m_isCanceled = false;
}

bool Widget::workIsCanceled()
{
    return m_isCanceled;
}

void Widget::setModelToFileView(QAbstractItemModel *model)
{
    ui->fileView->setModel( model );
    ui->fileView->setUniformRowHeights( true ); // This is veeeerrrryyy important for view widget performance!
    ui->fileView->setAllColumnsShowFocus( true );
    ui->fileView->setSortingEnabled( true );
    ui->fileView->sortByColumn( static_cast<int>( CameraDataModel::FileColumnTypeID::DateTime ), Qt::DescendingOrder );
    ui->fileView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    ui->fileView->header()->setStretchLastSection( false );
    ui->fileView->header()->setSectionResizeMode( 0, QHeaderView::Stretch );
    for ( int i = 1; i < ui->fileView->header()->count(); i++ ) {
        ui->fileView->header()->setSectionResizeMode( i, QHeaderView::ResizeToContents );
    }

    // We must connect selection singals here because selection model may be changed when setModel has been called
    // We use Qt::UniqueConnection because selection model may not be changed, though
    connect( ui->fileView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &Widget::currentFileIndexChanged, Qt::UniqueConnection );
    connect( ui->fileView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(itemSelectionChanged(QItemSelection, QItemSelection)), Qt::UniqueConnection );
}

void Widget::removeModelFromFileView()
{
    QItemSelectionModel *selmodel = ui->fileView->selectionModel();
    ui->fileView->setModel( nullptr );
    delete selmodel;
}

DataEntry *Widget::getDataEntryFromIndex(const QModelIndex &index)
{
    if ( !index.isValid() || index.internalPointer() == nullptr ) {
        return nullptr;
    }

    return  static_cast<DataEntry *>(index.data( CameraDataModel::DataPointerRole ).value<void *>());
}

void Widget::cameraDataModelReseted()
{
    // Reset selection count label
    setSelectionStatusLabel( 0 );
}

void Widget::cameraNameListSelected(const QModelIndex &index, const QModelIndex &prev)
{
    Q_UNUSED(prev)
    qDebug() << "Camera Selected";
    if ( !ui->fileView->model() ) {
        setModelToFileView( proxyModel );
    }

    updateFileTreeViewByCameraName( index.data().toString() );
    ui->previewWidget->clearPreview();

    // Reset filter
    filterChanged( static_cast<int>( ui->filetypeCombobox->currentIndex() ), false );
}

void Widget::currentFileIndexChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    if ( !current.isValid() || current.internalPointer() == nullptr ) return;

    DataEntry *entry = getDataEntryFromIndex( current );

    if ( entry->updateDecryptFlag() ) {
        ui->previewWidget->setFileName( entry->getLocalDecryptedFullPath() );
    } else {
        ui->previewWidget->clearPreview();
    }
}

void Widget::setSelectionStatusLabel(int count)
{
    ui->itemCountLabel->setText( QString::number( count ) + tr( " items being selected" ) );
}

void Widget::on_updateFileListButton_clicked()
{
    if ( currentServer == nullptr ) return;

    // Clear view
    removeModelFromFileView();

    // Lock GUI
    lockProgressGUI( true );

    // Execute
    startWork();
    emit startUpdateFileList();

    while ( !workIsFinished() ) {
        QApplication::processEvents();
    }

    if ( !workIsSucceeded() ) {
        unlockProgressGUI();
        return;
    }

    // Lock GUI
    lockProgressGUI( false );

    startWork();
    emit startParseFileList();

    while ( !workIsFinished() ) {
        QApplication::processEvents();
    }

    // Unlock GUI
    unlockProgressGUI();
}

void Widget::on_downloadButton_clicked()
{
    // Download
    // Model/View
    downloadAndDecryptSelectedFiles();

    ui->fileView->clearSelection();
}

void Widget::on_clearSelectionButton_clicked()
{
    ui->fileView->clearSelection();
}

void Widget::on_quitButton_clicked()
{
    close();
}

void Widget::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    if ( m_muninPortForwardingProccess->isOpen() ) {
        m_muninPortForwardingProccess->close();
        m_muninPortForwardingProccess->waitForFinished();
    }

    // Save current server index
    settings->setValue( "local/serverIndex", ui->serverComboBox->currentIndex() );
}

void Widget::on_openMuninButton_clicked()
{
    // Open munin
    // Munin opening button currently supports only one connection at once
    if ( currentServer == nullptr ) return;

    QString program;
    QStringList args;
    QString portOption;
    QString localPort;
    QString remotePort;

    program = settings->value( "local/sshPath", "ssh" ).toString();
    localPort = settings->value( "local/muninLocalPort", "8181" ).toString();
    remotePort = currentServer->getMuninPort();
    portOption = "-p" + currentServer->getServerPort();
    args << portOption << "-i" << currentServer->getSshKeyPath() << "-N" << "-T" << "-L" + localPort + ":localhost:" + remotePort << currentServer->getServerName();

    if ( m_muninPortForwardingProccess->isOpen() ) {
        m_muninPortForwardingProccess->close();
        m_muninPortForwardingProccess->waitForFinished( 5000 );
        m_muninPortForwardingProccess->kill();
    }

    m_muninPortForwardingProccess->setProgram( program );
    m_muninPortForwardingProccess->setArguments( args );
    m_muninPortForwardingProccess->start();
    m_muninPortForwardingProccess->waitForStarted();

    QDesktopServices::openUrl( QUrl( "http://localhost:" + localPort + "/munin" ) );
}

void Widget::on_cancelButton_clicked()
{
    // Cancel button
    m_isCanceled = true;
    emit cancelProcess();
}

void Widget::on_configButton_clicked()
{
    ConfigWidget *config = new ConfigWidget( this );

    config->readSettings( settings );

    if ( config->exec() != QDialog::Accepted ) {
        return;
    }

    config->writeSettings();
    serverManager->updateGlobalSetting();

    delete config;
}

void Widget::on_selectFromButton_clicked()
{
    selectFromFilter();
}

void Widget::on_selectToButton_clicked()
{
    selectToFilter();
}

void Widget::on_clearDateTimeFilterButton_clicked()
{
    clearDateTimeFilter();
}

void Widget::on_showDetailButton_clicked()
{
    showDetailStatus();
}

void Widget::on_serverSettingButton_clicked()
{
    serversSettings();
}

void Widget::on_aboutButton_clicked()
{
    aboutBox();
}
