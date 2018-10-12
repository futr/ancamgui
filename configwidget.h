#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QDialog>
#include <QSettings>
#include <QFileDialog>

namespace Ui {
class ConfigWidget;
}

class ConfigWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWidget(QWidget *parent = nullptr);
    ~ConfigWidget();

public slots:
    void readSettings( QSettings *settings );
    void writeSettings();

private:
    Ui::ConfigWidget *ui;

    QSettings *settings;
};

#endif // CONFIGWIDGET_H
