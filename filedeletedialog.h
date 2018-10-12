#ifndef FILEDELETEDIALOG_H
#define FILEDELETEDIALOG_H

#include <QDialog>

namespace Ui {
class FileDeleteDialog;
}

class FileDeleteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileDeleteDialog(QWidget *parent = nullptr);
    ~FileDeleteDialog();

    void setupInfo( int count, bool deletePlain, bool deleteCrypto );

    bool deletePlain() const;
    bool deleteCrypto() const;

private:
    Ui::FileDeleteDialog *ui;
};

#endif // FILEDELETEDIALOG_H
