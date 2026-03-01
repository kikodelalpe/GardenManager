#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/gui/addbatchdialog.h"
#include "src/gui/addvarietydialog.h"
#include "src/gui/addplantdialog.h"
#include "src/gui/addeventdialog.h"
#include "src/gui/addgroupdialog.h"
#include "src/gui/settingsdialog.h"
#include <QSqlRelationalDelegate>
#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>
#include <QPushButton>
#include <QDialog>
#include <QMessageBox>
#include <QItemSelection>
#include <QCheckBox>
#include <QFileDialog>
#include <QDate>
#include <QDateTime>
#include <QSettings>
#include <QStyledItemDelegate>

class CurrencyDelegate : public QStyledItemDelegate{
public:
    CurrencyDelegate(const QString &symbol, QObject *parent = nullptr) : QStyledItemDelegate(parent), currencySymbol(symbol) {}
    QString displayText(const QVariant &value, const QLocale &locale) const override{
        bool isNumber;
        double price = value.toDouble(&isNumber);
        if(isNumber) return QString::number(price, 'f', 2) + " " + currencySymbol;
        return QStyledItemDelegate::displayText(value, locale);
    }

private:
    QString currencySymbol;
};

class DateDelegate : public QStyledItemDelegate{
public:
    DateDelegate(const QString &format, QObject *parent = nullptr) : QStyledItemDelegate(parent), dateFormat(format) {}
    QString displayText(const QVariant &value, const QLocale &locale) const override{
        if(value.isNull() || value.toString().isEmpty()) return "";
        if(value.type() == QVariant::DateTime) return value.toDateTime().toString(dateFormat);
        if(value.type() == QVariant::Date) return value.toDate().toString(dateFormat);
        QString strValue = value.toString();
        if(strValue.contains(":")){
            QDateTime dateTime = QDateTime::fromString(strValue, "yyyy-MM-dd HH:mm:ss");
            if(!dateTime.isValid()) dateTime = QDateTime::fromString(strValue, Qt::ISODate);
            if(dateTime.isValid()) return dateTime.toString(dateFormat);
        }
        else{
            QDate date = QDate::fromString(strValue, "yyyy-MM-dd");
            if(!date.isValid()) date = value.toDate();
            if(date.isValid()) return date.toString(dateFormat);
        }
        return QStyledItemDelegate::displayText(value, locale);
    }

private:
    QString dateFormat;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupWarehouse();
    setupCatalog();
    setupDiariesTab();
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabSelectedChanged);
    connect(ui->btnAddPlant, &QPushButton::clicked, this, &MainWindow::onBtnAddPlantClicked);
    connect(ui->btnAddEvent, &QPushButton::clicked, this, &MainWindow::onBtnAddEventClicked);
    connect(ui->btnAddGroup, &QPushButton::clicked, this, &MainWindow::onBtnAddGroupClicked);
    connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, &MainWindow::onTreeSelectionChanged);
    connect(ui->btnEditDiaries, &QPushButton::clicked, this, &MainWindow::onBtnEditItemClicked);
    connect(ui->btnDeleteDiaries, &QPushButton::clicked, this, &MainWindow::onBtnDeleteItemClicked);
    connect(ui->btnAddBatch, &QPushButton::clicked, this, &MainWindow::onBtnAddBatchClicked);
    connect(ui->btnEditWarehouse, &QPushButton::clicked, this, &MainWindow::onBtnEditWarehouseClicked);
    connect(ui->btnDeleteWarehouse, &QPushButton::clicked, this, &MainWindow::onBtnDeleteWarehouseClicked);
    connect(ui->btnAddVariety, &QPushButton::clicked, this, &MainWindow::onBtnAddVarietyClicked);
    connect(ui->btnEditCatalog, &QPushButton::clicked, this, &MainWindow::onBtnEditCatalogClicked);
    connect(ui->btnDeleteCatalog, &QPushButton::clicked, this, &MainWindow::onBtnDeleteCatalogClicked);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::onActionSettingsTriggered);
    connect(ui->actionBackup_database, &QAction::triggered, this, &MainWindow::onActionBackupTriggered);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTabSelectedChanged(int index){
    if(ui->tabWidget->widget(index) == ui->tabDiaries){
        setupDiariesTab();
    }
    else if(ui->tabWidget->widget(index) == ui->tabWarehouse){
        if(_warehouseModel) _warehouseModel->select();
    }
    else if(ui->tabWidget->widget(index) == ui->tabCatalog){
        if(_catalogModel) _catalogModel->select();
    }
}

void MainWindow::onActionSettingsTriggered(){
    SettingsDialog dlg(this);
    connect(&dlg, &SettingsDialog::settingsChanged, this, [this](){
        setupWarehouse();
        setupDiariesTab();
        if(ui->treeWidget->currentItem()){
            int currentGroupId = ui->treeWidget->currentItem()->data(0, Qt::UserRole).toInt();
            loadDiariesForGroup(currentGroupId);
        }
    });
    dlg.exec();
}

void MainWindow::onActionBackupTriggered(){
    QString suggestedName = "GardenManager_Backup_" + QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString destinationPath = QFileDialog::getSaveFileName(
        this,
        tr("Save backup database"),
        suggestedName,
        tr("Database SQLite (*.db)")
    );
    if(destinationPath.isEmpty()) return;
    QString sourcePath = QSqlDatabase::database().databaseName();
    if(QFile::exists(destinationPath)) QFile::remove(destinationPath);
    if(QFile::copy(sourcePath, destinationPath)) QMessageBox::information(this, tr("Success"), tr("Backup correctly saved."));
    else QMessageBox::critical(this, tr("Error"), tr("Unable to save database backup."));
}

//diaries
void MainWindow::onBtnAddPlantClicked(){
    AddPlantDialog dlg(-1, this);
    if(dlg.exec() == QDialog::Accepted) setupDiariesTab();
}

void MainWindow::onBtnAddEventClicked(){
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if(!currentItem) return;
    int groupId = currentItem->data(0, Qt::UserRole).toInt();
    QString groupName = currentItem->text(0);
    int plantId = -1;
    QString targetName = tr("Group: ") + groupName;
    if(ui->tablePlants->selectionModel() && ui->tablePlants->selectionModel()->hasSelection()){
        int row = ui->tablePlants->selectionModel()->selectedRows().first().row();
        plantId = ui->tablePlants->model()->index(row, 0).data().toInt();
        QString variety = ui->tablePlants->model()->index(row, 1).data().toString();
        QString name = ui->tablePlants->model()->index(row, 2).data().toString();
        targetName = tr("Single plant: ") + variety + " (" + name + ")";
    }
    QMessageBox::information(this, tr("New diary entry"), tr("You're adding an entry to:") + "\n\n" + targetName);
    addEventDialog dlg(groupId, plantId, -1, this);
    if(dlg.exec() == QDialog::Accepted){
        setupDiariesTab();
        loadDiariesForGroup(groupId);
    }
}

void MainWindow::onBtnAddGroupClicked(){
    AddGroupDialog dlg(-1, this);
    if(dlg.exec() == QDialog::Accepted) setupDiariesTab();
}

void MainWindow::onTreeSelectionChanged(){
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if(!currentItem){
        updateDynamicButtons();
        return;
    }
    int id = currentItem->data(0, Qt::UserRole).toInt();
    loadPlantsForGroup(id);
    loadDiariesForGroup(id);
    updateDynamicButtons();
}

void MainWindow::setupDiariesTab(){
    ui->treeWidget->clear();
    ui->treeWidget->setHeaderLabel(tr("Groups and diaries"));
    QSqlQuery query("SELECT id, name FROM groups WHERE parent_id IS NULL ORDER BY name ASC");
    while(query.next()){
        int groupId = query.value("id").toInt();
        QString groupName = query.value("name").toString();
        QTreeWidgetItem *rootGroupItem = new QTreeWidgetItem(ui->treeWidget);
        rootGroupItem->setText(0, groupName);
        rootGroupItem->setData(0, Qt::UserRole, groupId);
        rootGroupItem->setData(0, Qt::UserRole + 1, "group");
        populateTreeGroup(rootGroupItem, groupId);
    }
    ui->treeWidget->expandAll();
}

void MainWindow::populateTreeGroup(QTreeWidgetItem *parentItem, int parentGroupId){
    QSqlQuery query;
    query.prepare("SELECT id, name FROM groups WHERE parent_id = :parentId ORDER BY name ASC");
    query.bindValue(":parentId", parentGroupId);
    query.exec();
    while(query.next()){
        int subGroupId = query.value("id").toInt();
        QString subGroupName = query.value("name").toString();
        QTreeWidgetItem *subGroupItem = new QTreeWidgetItem(parentItem);
        subGroupItem->setText(0, subGroupName);
        subGroupItem->setData(0, Qt::UserRole, subGroupId);
        subGroupItem->setData(0, Qt::UserRole + 1, "group");
        populateTreeGroup(subGroupItem, subGroupId);
    }
}

void MainWindow::updateDynamicButtons(){
    bool hasEvent = ui->tableEntries->selectionModel() && ui->tableEntries->selectionModel()->hasSelection();
    bool hasPlant = ui->tablePlants->selectionModel() && ui->tablePlants->selectionModel()->hasSelection();
    bool hasGroup = ui->treeWidget->currentItem() != nullptr;
    if(hasEvent){
        ui->btnEditDiaries->setText(tr("Edit diary"));
        ui->btnDeleteDiaries->setText(tr("Delete diary"));
        ui->btnEditDiaries->setEnabled(true);
        ui->btnDeleteDiaries->setEnabled(true);
    }
    else if(hasPlant){
        ui->btnEditDiaries->setText(tr("Edit plant"));
        ui->btnDeleteDiaries->setText(tr("Delete plant"));
        ui->btnEditDiaries->setEnabled(true);
        ui->btnDeleteDiaries->setEnabled(true);
    }
    else if(hasGroup){
        ui->btnEditDiaries->setText(tr("Edit group"));
        ui->btnDeleteDiaries->setText(tr("Delete group"));
        ui->btnEditDiaries->setEnabled(true);
        ui->btnDeleteDiaries->setEnabled(true);
    }
    else{
        ui->btnEditDiaries->setText(tr("Edit..."));
        ui->btnDeleteDiaries->setText(tr("Delete..."));
        ui->btnEditDiaries->setEnabled(false);
        ui->btnDeleteDiaries->setEnabled(false);
    }
}

void MainWindow::onBtnEditItemClicked(){
    if (ui->tableEntries->selectionModel() && ui->tableEntries->selectionModel()->hasSelection()){
        int row = ui->tableEntries->selectionModel()->selectedRows().first().row();
        int eventId = ui->tableEntries->model()->index(row, 0).data().toInt();
        int groupId = ui->treeWidget->currentItem()->data(0, Qt::UserRole).toInt();
        int plantId = -1;
        QSqlQuery query("SELECT plant_id FROM events WHERE id = " + QString::number(eventId));
        if(query.exec() && query.next() && !query.value(0).isNull()) plantId = query.value(0).toInt();
        addEventDialog dlg(groupId, plantId, eventId, this);
        if(dlg.exec() == QDialog::Accepted){
            loadDiariesForGroup(groupId);
            if(_warehouseModel) _warehouseModel->select();
        }
    }
    else if (ui->tablePlants->selectionModel() && ui->tablePlants->selectionModel()->hasSelection()){
        int row = ui->tablePlants->selectionModel()->selectedRows().first().row();
        int plantId = ui->tablePlants->model()->index(row, 0).data().toInt();
        AddPlantDialog dlg(plantId, this);
        if(dlg.exec() == QDialog::Accepted){
            int groupId = ui->treeWidget->currentItem()->data(0, Qt::UserRole).toInt();
            loadPlantsForGroup(groupId);
        }
    }
    else if (ui->treeWidget->currentItem()){
        int groupId = ui->treeWidget->currentItem()->data(0, Qt::UserRole).toInt();
        AddGroupDialog dlg(groupId, this);
        if(dlg.exec() == QDialog::Accepted) setupDiariesTab();
    }
}

void MainWindow::onBtnDeleteItemClicked(){
    if(ui->tableEntries->selectionModel() && ui->tableEntries->selectionModel()->hasSelection()){
        int row = ui->tableEntries->selectionModel()->selectedRows().first().row();
        int eventId = ui->tableEntries->model()->index(row, 0).data().toInt();
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Confirm"));
        msgBox.setText(tr("Do you really want to delete this diary entry?"));
        QCheckBox *cbRestore = new QCheckBox(tr("Restore materials used in this entry to the warehouse?"));
        cbRestore->setChecked(true);
        msgBox.setCheckBox(cbRestore);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if(msgBox.exec() == QMessageBox::Yes){
            QSqlDatabase db = QSqlDatabase::database();
            db.transaction();
            if(cbRestore->isChecked()){
                QSqlQuery queryRestore;
                queryRestore.prepare("SELECT batch_id, quantity_used From event_usage WHERE event_id = :eid");
                queryRestore.bindValue(":eid", eventId);
                if(queryRestore.exec()){
                    QSqlQuery queryUpdateWarehouse;
                    queryUpdateWarehouse.prepare("UPDATE inventory_batches SET quantity_current = quantity_current + :qty, is_active = 1 WHERE id = :bid");
                    while(queryRestore.next()){
                        queryUpdateWarehouse.bindValue(":bid", queryRestore.value("batch_id").toInt());
                        queryUpdateWarehouse.bindValue(":qty", queryRestore.value("quantity_used").toDouble());
                        if(!queryUpdateWarehouse.exec()){
                            db.rollback();
                            QMessageBox::critical(this, tr("Error"), tr("Unable to update warehouse."));
                            return;
                        }
                    }
                }
            }
            QSqlQuery queryClean;
            queryClean.prepare("DELETE FROM event_usage WHERE event_id = :eid");
            queryClean.bindValue(":eid", eventId);
            if(!queryClean.exec()) { db.rollback(); return; }

            queryClean.prepare("DELETE FROM event_event_types WHERE event_id = :eid");
            queryClean.bindValue(":eid", eventId);
            if(!queryClean.exec()) { db.rollback(); return; }

            queryClean.prepare("DELETE FROM event_techniques WHERE event_id = :eid");
            queryClean.bindValue(":eid", eventId);
            if(!queryClean.exec()) { db.rollback(); return; }

            QSqlQuery queryDelete;
            queryDelete.prepare("DELETE FROM events WHERE id = :eid");
            queryDelete.bindValue(":eid", eventId);

            if(queryDelete.exec()){
                db.commit();
                int groupId = ui->treeWidget->currentItem()->data(0, Qt::UserRole).toInt();
                loadDiariesForGroup(groupId); 
                if(_warehouseModel) _warehouseModel->select(); 
            }
            else{
                db.rollback();
                QMessageBox::critical(this, tr("Error"), tr("Database error: ") + queryDelete.lastError().text());
            }
        }
    }
    else if(ui->tablePlants->selectionModel() && ui->tablePlants->selectionModel()->hasSelection()){
        int row = ui->tablePlants->selectionModel()->selectedRows().first().row();
        int plantId = ui->tablePlants->model()->index(row, 0).data().toInt();
        if(QMessageBox::question(this, tr("Confirm"), tr("Do you really want to delete this plant and all his diaries?")) == QMessageBox::Yes){
            QSqlQuery query;
            query.prepare("DELETE FROM plants WHERE id = :id");
            query.bindValue(":id", plantId);
            if(query.exec()){
                int groupId = ui->treeWidget->currentItem()->data(0, Qt::UserRole).toInt();
                loadPlantsForGroup(groupId);
            }
            else QMessageBox::critical(this, tr("Error"), tr("Database error: ") + query.lastError().text());
        }
    }
    else if(ui->treeWidget->currentItem()){
        int groupId = ui->treeWidget->currentItem()->data(0, Qt::UserRole).toInt();
        if(QMessageBox::question(this, tr("Confirm"), tr("Do you really want to delete the entire group?")) == QMessageBox::Yes){
            QSqlQuery query;
            query.prepare("DELETE FROM groups WHERE id = :id");
            query.bindValue(":id", groupId);
            if(query.exec()) setupDiariesTab();
            else QMessageBox::critical(this, tr("Error"), tr("Database error: ") + query.lastError().text());
        }
    }
}

void MainWindow::loadDiariesForGroup(int groupId){
    QSqlQueryModel *diaryModel = new QSqlQueryModel(this);
    QSqlQuery query;
    query.prepare(
        "SELECT "
        "   e.id as EventID, "
        "   e.timestamp as Date, "
        "   IFNULL(pl.name, 'Full group') AS Target, "
        "   GROUP_CONCAT(DISTINCT et.name) As Type, "
        "   GROUP_CONCAT(DISTINCT t.name) AS Technique, "
        "   GROUP_CONCAT(DISTINCT p.name || ' (' || eu.quantity_used || ' ' || ib.unit || ')') AS Products, "
        "   e.notes AS Note "
        "FROM events e "
        "LEFT JOIN plants pl ON e.plant_id = pl.id "
        "LEFT JOIN event_event_types eet ON e.id = eet.event_id "
        "LEFT JOIN event_types et ON eet.event_type_id = et.id "
        "LEFT JOIN event_techniques etq ON e.id = etq.event_id "
        "LEFT JOIN techniques t ON etq.technique_id = t.id "
        "LEFT JOIN event_usage eu ON e.id = eu.event_id "
        "LEFT JOIN inventory_batches ib ON eu.batch_id = ib.id "
        "LEFT JOIN products p ON ib.product_id = p.id "
        "WHERE e.group_id = :gid "
        "GROUP BY e.id "
        "ORDER BY e.timestamp DESC"
    );
    query.bindValue(":gid", groupId);
    query.exec();
    diaryModel->setQuery(std::move(query));
    ui->tableEntries->setModel(diaryModel);
    ui->tableEntries->setColumnHidden(0, true);
    QSettings settings("MyGardenApp", "GardenManager");
    QString dateFormat = settings.value("date_format", "dd/MM/yyyy").toString();
    QString timeFormat = settings.value("time_format", "HH:mm").toString();
    QString dateTimeFormat = dateFormat + " " + timeFormat;
    ui->tableEntries->setItemDelegateForColumn(1, new DateDelegate(dateTimeFormat, this));
    ui->tableEntries->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableEntries->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableEntries->resizeColumnsToContents();
    ui->tableEntries->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    connect(ui->tableEntries->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](){
        if(ui->tableEntries->selectionModel()->hasSelection()) ui->tablePlants->clearSelection();
        updateDynamicButtons();
    });
}

void MainWindow::loadPlantsForGroup(int groupId){
    QSqlQueryModel *plantsModel = new QSqlQueryModel(this);
    QSqlQuery query;
    query.prepare(
        "SELECT p.id as PlantID, "
        "v.name as Variety, "
        "p.name as Name, "
        "s.name as State "
        "FROM plants p "
        "LEFT JOIN varieties v ON p.variety_id = v.id "
        "LEFT JOIN plant_statuses s ON p.status_id = s.id "
        "WHERE p.group_id = :gid"
    );
    query.bindValue(":gid", groupId);
    query.exec();
    plantsModel->setQuery(std::move(query));
    ui->tablePlants->setModel(plantsModel);
    ui->tablePlants->setColumnHidden(0, true);
    ui->tablePlants->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablePlants->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tablePlants->resizeColumnsToContents();
    ui->tablePlants->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(ui->tablePlants->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](){
        if(ui->tablePlants->selectionModel()->hasSelection()) ui->tableEntries->clearSelection();
        updateDynamicButtons();
    });
}

//warehouse
void MainWindow::onBtnAddBatchClicked(){
    AddBatchDialog dlg(-1, this);
    if(dlg.exec() == QDialog::Accepted) if(_warehouseModel) _warehouseModel->select();
}

void MainWindow::onBtnEditWarehouseClicked(){
    QItemSelectionModel *select = ui->tableWarehouse->selectionModel();
    if(!select->hasSelection()) return;
    int row = select->selectedRows().first().row();
    int idColumn = _warehouseModel->fieldIndex("id");
    int batchId = _warehouseModel->data(_warehouseModel->index(row, idColumn)).toInt();
    AddBatchDialog dlg(batchId, this);
    if(dlg.exec() == QDialog::Accepted) _warehouseModel->select();
}

void MainWindow::onBtnDeleteWarehouseClicked(){
    QItemSelectionModel *select = ui->tableWarehouse->selectionModel();
    if(!select->hasSelection()) return;
    int row = select->selectedRows().first().row();
    if(QMessageBox::question(this, tr("Confirm"), tr("Are you sure to delete the selected item?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
        _warehouseModel->removeRow(row);
        if(_warehouseModel->submitAll()) _warehouseModel->select();
        else{
            QMessageBox::critical(this, tr("Error"), tr("You can't delete this item, it is used in a diary."));
            _warehouseModel->revertAll();
        }
    }
}

void MainWindow::setupWarehouse(){
    QSettings settings("MyGardenApp", "GardenManager");
    QString currencyCode = settings.value("currency", "EUR").toString();
    QString currencySymbol = "€";
    if(currencyCode == "USD") currencySymbol = "$";
    if(currencyCode == "GBP") currencySymbol = "£";
    QString dateFormat = settings.value("date_format", "dd/MM/yyyy").toString();
    _warehouseModel = new QSqlRelationalTableModel(this, QSqlDatabase::database());
    _warehouseModel->setTable("inventory_batches");
    int cProduct = _warehouseModel->fieldIndex("product_id");
    _warehouseModel->setRelation(cProduct, QSqlRelation("products", "id", "name"));
    _warehouseModel->setFilter("inventory_batches.is_active = 1");
    _warehouseModel->select();
    _warehouseModel->setHeaderData(cProduct, Qt::Horizontal, tr("Product"));
    _warehouseModel->setHeaderData(_warehouseModel->fieldIndex("quantity_current"), Qt::Horizontal, tr("Quantity"));
    _warehouseModel->setHeaderData(_warehouseModel->fieldIndex("expiration_date"), Qt::Horizontal, tr("Expires"));
    ui->tableWarehouse->setModel(_warehouseModel);
    ui->tableWarehouse->setColumnHidden(_warehouseModel->fieldIndex("id"), true);
    ui->tableWarehouse->setItemDelegate(new QSqlRelationalDelegate(ui->tableWarehouse));
    QHeaderView *header = ui->tableWarehouse->horizontalHeader();
    int cQtyCurr = _warehouseModel->fieldIndex("quantity_current");
    int cUnit = _warehouseModel->fieldIndex("unit");
    int cQtyInit = _warehouseModel->fieldIndex("quantity_initial");
    int cPrice = _warehouseModel->fieldIndex("price_paid");
    ui->tableWarehouse->setItemDelegateForColumn(cPrice, new CurrencyDelegate(currencySymbol, this));
    int cPurchaseDate = _warehouseModel->fieldIndex("purchase_date");
    ui->tableWarehouse->setItemDelegateForColumn(cPurchaseDate, new DateDelegate(dateFormat, this));
    int cExpirationDate = _warehouseModel->fieldIndex("expiration_date");
    ui->tableWarehouse->setItemDelegateForColumn(cExpirationDate, new DateDelegate(dateFormat, this));
    int cLot = _warehouseModel->fieldIndex("lot_number");
    ui->tableWarehouse->setColumnHidden(_warehouseModel->fieldIndex("is_active"), true);
    header->moveSection(cProduct, 0);
    header->moveSection(cQtyCurr, 1);
    header->moveSection(cUnit, 2);
    header->moveSection(cQtyInit, 3);
    header->moveSection(cPrice, 4);
    header->moveSection(cPurchaseDate, 5);
    header->moveSection(cExpirationDate, 6);
    header->moveSection(cLot, 7);
    ui->tableWarehouse->resizeColumnsToContents();
    ui->tableWarehouse->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWarehouse->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->btnEditWarehouse->setEnabled(false);
    ui->btnDeleteWarehouse->setEnabled(false);
    connect(ui->tableWarehouse->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](){
        bool hasSel = ui->tableWarehouse->selectionModel()->hasSelection();
        ui->btnEditWarehouse->setEnabled(hasSel);
        ui->btnDeleteWarehouse->setEnabled(hasSel);
    });
}

//plants catalog
void MainWindow::onBtnAddVarietyClicked(){
    AddVarietyDialog dlg(-1, this);
    if(dlg.exec() == QDialog::Accepted) if(_catalogModel) _catalogModel->select();
}

void MainWindow::onBtnEditCatalogClicked(){
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection()) return;
    int row = select->selectedRows().first().row();
    int idColumn = _catalogModel->fieldIndex("id");
    int varietyId = _catalogModel->data(_catalogModel->index(row, idColumn)).toInt();
    AddVarietyDialog dlg(varietyId, this);
    if(dlg.exec() == QDialog::Accepted) _catalogModel->select();
}

void MainWindow::onBtnDeleteCatalogClicked(){
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection()) return;
    int row = select->selectedRows().first().row();
    if(QMessageBox::question(this, tr("Confirm"), tr("Are you sure to delete the selected plant variety?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
        _catalogModel->removeRow(row);
        if(_catalogModel->submitAll()) _catalogModel->select();
        else{
            QMessageBox::critical(this, tr("Error"), tr("You can't delete this plant, it is used in a diary."));
            _catalogModel->revertAll();
        }
    }
}

void MainWindow::setupCatalog(){
    _catalogModel = new QSqlRelationalTableModel(this, QSqlDatabase::database());
    _catalogModel->setTable("varieties");
    int cSpecies = _catalogModel->fieldIndex("species_id");
    _catalogModel->setRelation(cSpecies, QSqlRelation("species", "id", "common_name"));
    _catalogModel->select();
    _catalogModel->setHeaderData(cSpecies, Qt::Horizontal, tr("Specie"));
    _catalogModel->setHeaderData(_catalogModel->fieldIndex("name"), Qt::Horizontal, tr("Variety"));
    _catalogModel->setHeaderData(_catalogModel->fieldIndex("days_to_maturity"), Qt::Horizontal, tr("Days to maturiry"));
    ui->tableView->setModel(_catalogModel);
    ui->tableView->setColumnHidden(_catalogModel->fieldIndex("id"), true);
    ui->tableView->setColumnHidden(_catalogModel->fieldIndex("variety_description"), true);
    ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));
    ui->tableView->resizeColumnsToContents();
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->btnEditCatalog->setEnabled(false);
    ui->btnDeleteCatalog->setEnabled(false);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](){
        bool hasSel = ui->tableView->selectionModel()->hasSelection();
        ui->btnEditCatalog->setEnabled(hasSel);
        ui->btnDeleteCatalog->setEnabled(hasSel);
    });
}
