#include "selectdatedialog.h"
#include "ui_selectdatedialog.h"

SelectDateDialog::SelectDateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectDateDialog)
{
    ui->setupUi(this);
}

SelectDateDialog::~SelectDateDialog()
{
    delete ui;
}

QDateTime SelectDateDialog::getSelectedDate()
{
    return QDateTime( ui->calendarWidget->selectedDate( ) );
}

void SelectDateDialog::setTitle(QString title)
{
    setWindowTitle( title );
}

void SelectDateDialog::setDate(QDateTime date)
{
    ui->calendarWidget->setSelectedDate( date.date() );
}

void SelectDateDialog::setMinimumDateTime(QDateTime min)
{
    ui->calendarWidget->setMinimumDate( min.date() );
}

void SelectDateDialog::setMaximumDateTime(QDateTime max)
{
    ui->calendarWidget->setMaximumDate( max.date() );
}
