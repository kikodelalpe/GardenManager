#include "databasemanager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QVariant>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

DatabaseManager::DatabaseManager(){
    if(!QSqlDatabase::contains("qt_sql_default_connection")) _db = QSqlDatabase::addDatabase("QSQLITE");
    else _db = QSqlDatabase::database("qt_sql_default_connection");
}
DatabaseManager::~DatabaseManager() { closeConnection(); }

DatabaseManager &DatabaseManager::instance(){
    static DatabaseManager instance;
    return instance;
}

QMap<QString, int> DatabaseManager::getHierarchicalGroups(){
    QMap<QString, int> result;
    QMap<int, QString> names;
    QMap<int, int> parents;
    QSqlQuery query("SELECT id, name, parent_id FROM groups");
    while(query.next()){
        int id = query.value("id").toInt();
        names[id] = query.value("name").toString();
        parents[id] = query.value("parent_id").isNull() ? 0 : query.value("parent_id").toInt();
    }
    for(auto it = names.begin(); it != names.end(); it++){
        int id = it.key();
        QString path = it.value();
        int currentParent = parents[id];
        while(currentParent > 0){
            if(names.contains(currentParent)){
                path = names[currentParent] + " > " + path;
                currentParent = parents[currentParent];
            }
            else break;
        }
        result.insert(path, id);
    }
    return result;
}

QMap<int, QString> DatabaseManager::getAvailableSeeds(){
    QMap<int, QString> seeds;
    QSqlQuery query("SELECT ib.id, p.name || '(Lotto: '|| ib.lot_number ||')' "
                    "FROM inventory_batches ib "
                    "JOIN products p ON ib.product_id = p.id "
                    "WHERE p.category_id = 2 AND ib.is_active = 1 ");
    while(query.next()){
        int batchId = query.value(0).toInt();
        QString displayName = query.value(1).toString();
        seeds.insert(batchId, displayName);
    }
    return seeds;
}

bool DatabaseManager::connectToDatabase(){
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if(!dir.exists()) dir.mkpath(dataDir);
    QString dbPath = dir.absoluteFilePath("garden_manager.db");
    qInfo() << "Searching database in: " << dbPath;
    if(!QFileInfo::exists(dbPath)){
        qInfo() << "First start, copying database in: " << dbPath;
        QString cleanDbPath = ":/data/db/garden_manager.db";
        if(QFile::copy(cleanDbPath, dbPath)){
            QFile::setPermissions(dbPath, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser);
            qInfo() << "Database initialized correctly.";
        }
        else{
            qCritical() << "CRITICAL ERROR: unable to initialize the database.";
            return false;
        }
    }
    _db.setDatabaseName(dbPath);
    if(!_db.open()){
        qCritical() << "ERROR: Cant open the database." << _db.lastError().text();
        return false;
    }
    QSqlQuery query;
    if(!query.exec("PRAGMA foreign_keys = ON")) qWarning() << "Unable to enable foreign keys.";
    qInfo() << "Connected to the database.";
    return true;
}

void DatabaseManager::closeConnection(){
    if(_db.isOpen()){
        _db.close();
        qInfo() << "Database disconnected correctly.";
    }
}
