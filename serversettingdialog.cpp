#include "serversettingdialog.h"
#include "ui_serversettingdialog.h"

ServerSettingDialog::ServerSettingDialog(ServerManager *man, CameraDataAccess *current, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerSettingDialog)
{
    ui->setupUi(this);

    manager = man;
    // manager->loadSettings();
    mapper = new QDataWidgetMapper( this );
    ui->serverListView->setModel( manager->managerModel() );
    ui->serverListView->setEditTriggers( QListView::NoEditTriggers );
    ui->groupBox->setEnabled( false );

    connect( this, &ServerSettingDialog::accepted, mapper, &QDataWidgetMapper::submit );
    connect( this, &ServerSettingDialog::accepted, manager, &ServerManager::saveSettings );
    connect( ui->serverListView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), mapper, SLOT(setCurrentModelIndex(QModelIndex)) );
    connect( mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(serverCurrentIndexChanged(int)) );
    connect( manager->managerModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(serverRemoved()) );

    mapper->setModel( manager->managerModel() );
    mapper->setSubmitPolicy( QDataWidgetMapper::AutoSubmit );
    mapper->addMapping( ui->nameEdit,       static_cast<int>(ServerManagerModel::ServerSettingsID::Name) );
    mapper->addMapping( ui->serverNameEdit, static_cast<int>(ServerManagerModel::ServerSettingsID::ServerName) );
    mapper->addMapping( ui->portEdit,       static_cast<int>(ServerManagerModel::ServerSettingsID::Port) );
    mapper->addMapping( ui->dlDirEdit,      static_cast<int>(ServerManagerModel::ServerSettingsID::DownloadPath) );
    mapper->addMapping( ui->decryptDirEdit, static_cast<int>(ServerManagerModel::ServerSettingsID::DecryptPath) );
    mapper->addMapping( ui->muninPortEdit,  static_cast<int>(ServerManagerModel::ServerSettingsID::MuninPort) );
    mapper->addMapping( ui->sshKeyEdit,     static_cast<int>(ServerManagerModel::ServerSettingsID::SSHKeyPath) );
    // mapper->toFirst();

    ui->serverListView->selectionModel()->setCurrentIndex( ui->serverListView->model()->index( 0, 0 ), QItemSelectionModel::SelectCurrent );

    // UI
    connect( ui->selectDlDirButton, SIGNAL(clicked(bool)), this, SLOT(selectDownloadDir()) );
    connect( ui->selectDecryptDirButton, SIGNAL(clicked(bool)), this, SLOT(selectDecryptDir()) );

    // Setup current
    ui->serverListView->setCurrentIndex( manager->managerModel()->index( manager->serverIndex( current ), 0, QModelIndex() ) );
}

ServerSettingDialog::~ServerSettingDialog()
{
    delete ui;
}

void ServerSettingDialog::serverRemoved()
{
    if ( manager->serverCount() == 0 ) {
        ui->groupBox->setEnabled( false );
        mapper->revert();
    }
}

void ServerSettingDialog::serverCurrentIndexChanged(int idx)
{
    Q_UNUSED(idx)
    ui->groupBox->setEnabled( true );
}

void ServerSettingDialog::selectDownloadDir()
{
    QString dir = QFileDialog::getExistingDirectory( this, "Select a directory for download", ui->dlDirEdit->text() );

    if ( dir != "" ) {
        ui->dlDirEdit->setFocus();
        ui->dlDirEdit->setText( dir );
    }
}

void ServerSettingDialog::selectDecryptDir()
{
    QString dir = QFileDialog::getExistingDirectory( this, "Select a directory for decrypt", ui->decryptDirEdit->text() );

    if ( dir != "" ) {
        ui->decryptDirEdit->setFocus();
        ui->decryptDirEdit->setText( dir );
    }
}

void ServerSettingDialog::on_addServerButton_clicked()
{
    mapper->submit();
    manager->managerModel()->insertRow( manager->serverCount() );
    ui->serverListView->selectionModel()->setCurrentIndex( ui->serverListView->model()->index( manager->serverCount() - 1, 0 ), QItemSelectionModel::ClearAndSelect );
}

void ServerSettingDialog::on_removeServerButton_clicked()
{
    if ( !ui->serverListView->currentIndex().isValid() ) return;

    if ( QMessageBox::question( this, tr( "Remove a server" ), tr( "Are you sure you want to remove \"%1\"?" ).arg( ui->serverListView->currentIndex().data().toString() ) ) == QMessageBox::Yes ) {
        manager->managerModel()->removeRow( ui->serverListView->currentIndex().row() );
    }
}
