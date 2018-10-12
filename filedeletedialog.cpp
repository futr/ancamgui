#include "filedeletedialog.h"
#include "ui_filedeletedialog.h"

FileDeleteDialog::FileDeleteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileDeleteDialog)
{
    ui->setupUi(this);
}

FileDeleteDialog::~FileDeleteDialog()
{
    delete ui;
}

void FileDeleteDialog::setupInfo(int count, bool deletePlain, bool deleteCrypto)
{
    ui->infoLabel->setText( tr( "Are you sure you want to delete selected files?\n%1 files being selected\n\nNote that you can not delete files on server\n\nPlease select which files you want to delete" ).arg( count ) );

    ui->plainCheckBox->setChecked( deletePlain );
    ui->cryptoCheckBox->setChecked( deleteCrypto );
}

bool FileDeleteDialog::deletePlain() const
{
    return ui->plainCheckBox->isChecked();
}

bool FileDeleteDialog::deleteCrypto() const
{
    return ui->cryptoCheckBox->isChecked();
}
