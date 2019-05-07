#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sqlite3.h>
#include <QStringListModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Glue model and view together
    this->ui->lview_places->setModel(&(this->model_locations));
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::reloadLocations (SqliteConnection *conn) {
    // Clear list
    QStringList newList;
    this->model_locations.setStringList(newList);


    // Make data
    newList.append("CArlos");

    // SELECT path, obs FROM locations ORDER BY lastUse DESC
    // foreach(item) list.push(item.path + item.obs)
    // selet first item (if any)
}

void MainWindow::on_pbut_addPlace_clicked()
{
    // TODO: Verifica se a pasta existe e confirma caso não exista

    SqliteConnection *conn = SqliteConnection::create(this->ui->ledit_db->text().toUtf8().constData());
    if (conn == nullptr) return;
    SqliteStatement* query;

    query = conn->prepare("CREATE TABLE IF NOT EXISTS locations (id NUMBER, obs TEXT, path TEXT, lastUse TEXT)");
    if (!query->execute()) {
        delete query;
        delete conn;
        return;
    }

    query = conn->prepare("SELECT MAX(id) FROM locations");
    if (query->step() != SQLITE_ROW) {
        delete query;
        delete conn;
        return;
    }
    int maxId = query->getIntValue(0);
    delete query;

    time_t now;
    time(&now);
    char isoTime[50];
    strftime(isoTime, sizeof isoTime, "%FT%TZ", gmtime(&now));
    //strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));

    char hostname[1024];
    gethostname(hostname, 1024);

    query = conn->prepare("INSERT INTO locations (id, obs, path, lastUse) VALUES (:id, :obs, :path, :lastUse)");
    query->setParameter(":id", maxId + 1);
    query->setParameter(":obs", hostname);
    query->setParameter(":path", this->ui->ledit_newPlace->text().toUtf8().constData());
    query->setParameter(":lastUse", isoTime);
    if (!query->execute()) {
        delete query;
        delete conn;
        return;
    }
    delete query;

    this->reloadLocations(conn);
    delete conn;
}

void MainWindow::on_pbut_loadPlaces_clicked()
{


    // Open database
    // call reloadLocations()
}

void MainWindow::on_pbut_scan_clicked()
{

}
