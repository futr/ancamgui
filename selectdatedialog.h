#ifndef SELECTDATEDIALOG_H
#define SELECTDATEDIALOG_H

#include <QDialog>
#include <QCalendarWidget>
#include <QDateTime>

namespace Ui {
class SelectDateDialog;
}

class SelectDateDialog : public QDialog
{
    Q_OBJECT
    enum {
        DateTimeClear = 3,
    };

public:
    explicit SelectDateDialog(QWidget *parent = nullptr);
    ~SelectDateDialog();

    QDateTime getSelectedDate();
    void setTitle( QString title );
    void setDate( QDateTime date );
    void setMinimumDateTime( QDateTime min );
    void setMaximumDateTime( QDateTime max );

private:
    Ui::SelectDateDialog *ui;
};

#endif // SELECTDATEDIALOG_H
