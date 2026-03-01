#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QMap>
#include <QString>

class DatabaseManager{
private:
    DatabaseManager();
    ~DatabaseManager();
    const QString DB_NAME = "garden_manager.db";
    QSqlDatabase _db;

public:
    static DatabaseManager &instance();
    static QMap<QString, int> getHierarchicalGroups();
    static QMap<int, QString> getAvailableSeeds();
    bool connectToDatabase();
    void closeConnection();
};