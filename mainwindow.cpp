#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sqlite3.h>
#include <QStringListModel>
#include <dirent.h>

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

    if (conn == nullptr) return false;
    SqliteStatement* query;

    query = conn->prepare("SELECT path, obs FROM locations ORDER BY lastUse DESC");
    int colPath = query->col("path");
    int colObs = query->col("obs");
    while (query->step() == SQLITE_ROW) {
        newList.append(QString::fromUtf8((const char*)query->getTextValue(colPath))); //  + " - " + QString::fromUtf8((const char*)query->getTextValue(colObs))
    }
    if (query->lastReturnCode != SQLITE_DONE) {
        delete query;
        return false;
    }
    delete query;
    this->model_locations.setStringList(newList);
    // selet first item (if any)
    return true;
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
    SqliteConnection *conn = SqliteConnection::create(this->ui->ledit_db->text().toUtf8().constData());
    if (conn == nullptr) return;
    this->reloadLocations(conn);
    delete conn;
}

void MainWindow::on_pbut_scan_clicked()
{
    QModelIndexList selected = this->ui->lview_places->selectionModel()->selectedIndexes();
    if (selected.count() != 1) {
        return;
    }
    std::string path = this->model_locations.data(selected[0]).value<QString>().toUtf8().constData();
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }

    struct dirent* entity;
    for (entity = readdir(dir); entity != nullptr; entity = readdir(dir))
    {
        //find entity type
        if(entity->d_type == DT_DIR) { //it's an direcotry
            //don't process the  '..' and the '.' directories
            if(entity->d_name == std::string(".") || entity->d_name == std::string(".."))
            {
                continue;
            }

            //it's an directory so process it
            fprintf(stderr, ("Directory: " + std::string(entity->d_name) + "\n").c_str());
        }
        else if(entity->d_type == DT_REG) { //regular file
            fprintf(stderr, ("File: " + std::string(entity->d_name) + "\n").c_str());
        }
        else {
            //there are some other types
            //read here http://linux.die.net/man/3/readdir
            fprintf(stderr, ("Other: " + std::string(entity->d_name) + "\n").c_str());
        }
    }
}
