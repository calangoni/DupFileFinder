#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <sqlite3.h>
#include <QStringListModel>
#include "dbwrap.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pbut_addPlace_clicked();

    void on_pbut_loadPlaces_clicked();

    void on_pbut_scan_clicked();

private:
    Ui::MainWindow *ui;
    QStringListModel model_locations;

    bool reloadLocations (SqliteConnection *conn);
};

#endif // MAINWINDOW_H
