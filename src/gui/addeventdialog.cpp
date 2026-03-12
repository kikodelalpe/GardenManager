#include "addeventdialog.h"
#include "ui_addeventdialog.h"
#include "src/gui/addbatchdialog.h"
#include <QToolButton>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTableWidget>
#include <QInputDialog>
#include <QDateTime>
#include <QSettings>

addEventDialog::addEventDialog(int groupId, int plantId, int eventId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addEventDialog)
    , _groupId(groupId)
    , _plantId(plantId)
    , _eventId(eventId)
{
    ui->setupUi(this);
    QSettings settings;
    QString dateFormat = settings.value("date_format", "dd/MM/yyyy").toString();
    QString timeFormat = settings.value("time_format", "HH:mm").toString();
    ui->dateTimeEvent->setDisplayFormat(dateFormat + " " + timeFormat);
    setupLists();
    ui->dateTimeEvent->setDateTime(QDateTime::currentDateTime());
    ui->groupBatchesUsed->setVisible(false);
    ui->groupTechnuiques->setVisible(false);
    if(_eventId != -1){
        this->setWindowTitle(tr("Edit diary entry"));
        ui->listPlants->setVisible(false);
        loadRecord();
    }
    else if(_plantId != -1) ui->listPlants->setVisible(false);
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accepted);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &addEventDialog::validateAndSave);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    connect(ui->btnAddProduct, &QToolButton::clicked, this, &addEventDialog::onButtonAddProductClicked);
    connect(ui->btnAddToList, &QToolButton::clicked, this, &addEventDialog::onButtonAddToListClicked);
    connect(ui->listEventTypes, &QListWidget::itemChanged, this, &addEventDialog::onEventTypeChanged);
    setupProductsCombo();
    connect(ui->cmbProduct, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &addEventDialog::onProductChanged);
}

addEventDialog::~addEventDialog()
{
    delete ui;
}

void addEventDialog::onButtonAddProductClicked(){
    AddBatchDialog dlg(-1, this);
    if(dlg.exec() == QDialog::Accepted){
        setupProductsCombo();
        ui->cmbProduct->setCurrentIndex(ui->cmbProduct->count() - 1);
    }
}

void addEventDialog::onButtonAddToListClicked(){
    double qty = ui->spinQty->value();
    if(qty <= 0){
        QMessageBox::warning(this, tr("Error"), tr("Qunatity is not valid."));
        return;
    }
    int batchId = ui->cmbProduct->currentData().toInt();
    if(batchId <= 0) return;
    QString unit = ui->cmbUnit->currentText();
    QString productName = ui->cmbProduct->currentText();
    int row = ui->tableUsage->rowCount();
    ui->tableUsage->insertRow(row);
    QTableWidgetItem *itemProd = new QTableWidgetItem(productName);
    itemProd->setData(Qt::UserRole, batchId);
    ui->tableUsage->setItem(row, 0, itemProd);
    ui->tableUsage->setItem(row, 1, new QTableWidgetItem(QString::number(qty)));
    ui->tableUsage->setItem(row, 2, new QTableWidgetItem(unit));
    QPushButton *btnRemove = new QPushButton(tr("Remove"));
    ui->tableUsage->setCellWidget(row, 3, btnRemove);
    connect(btnRemove, &QPushButton::clicked, this, [this, btnRemove](){
        for(int i=0; i < ui->tableUsage->rowCount(); i++){
            if(ui->tableUsage->cellWidget(i, 3) == btnRemove){
                ui->tableUsage->removeRow(i);
                break;
            }
        }
    });
    ui->spinQty->setValue(0);
}

void addEventDialog::onEventTypeChanged(QListWidgetItem *item){
    bool showProductsBox = false;
    bool showTechniquesBox = false;
    for(int i=0; i < ui->listEventTypes->count(); ++i){
        QListWidgetItem *currentItem = ui->listEventTypes->item(i);
        if(currentItem->checkState() == Qt::Checked){
            QString eventName = currentItem->text();
            if(eventName == "Fertilization" || eventName == "Plant" || eventName == "Transplant") { showProductsBox = true;  }
            if(eventName == "Training") showTechniquesBox = true;
        }
    }
    ui->groupBatchesUsed->setVisible(showProductsBox);
    ui->groupTechnuiques->setVisible(showTechniquesBox);
}

void addEventDialog::setupLists(){
    ui->listEventTypes->clear();
    ui->listTechniques->clear();
    ui->listPlants->clear();
    const char *dbEventTypes[] = {
        QT_TR_NOOP("Dead"),
        QT_TR_NOOP("Fertilization"),
        QT_TR_NOOP("Harvest"),
        QT_TR_NOOP("Note"),
        QT_TR_NOOP("Plant"),
        QT_TR_NOOP("Training"),
        QT_TR_NOOP("Transplant")
    };
    QSqlQuery queryTypes("SELECT id, name FROM event_types ORDER BY name ASC");
    while(queryTypes.next()){
        QString originalName = queryTypes.value("name").toString();
        int id = queryTypes.value("id").toInt();
        QString translatedName = tr(originalName.toUtf8().constData());
        QListWidgetItem *item = new QListWidgetItem(translatedName, ui->listEventTypes);
        item->setData(Qt::UserRole, id);
        item->setData(Qt::UserRole + 1, originalName);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
    const char *dbEventTechniques[] = {
        QT_TR_NOOP("Pruning"),
        QT_TR_NOOP("Topping"),
        QT_TR_NOOP("Fimming"),
        QT_TR_NOOP("Lollipopping"),
        QT_TR_NOOP("Defoliation"),
        QT_TR_NOOP("Branches removing"),
        QT_TR_NOOP("LST"),
        QT_TR_NOOP("Mainlining"),
        QT_TR_NOOP("SOG"),
        QT_TR_NOOP("ScrOG")
    };
        QSqlQuery queryTech("SELECT id, name FROM techniques ORDER BY name ASC");
    while(queryTech.next()){
        QString originalName = queryTech.value("name").toString();
        int id = queryTech.value("id").toInt();
        QString translatedName = tr(originalName.toUtf8().constData());
        QListWidgetItem *item = new QListWidgetItem(translatedName, ui->listTechniques);
        item->setData(Qt::UserRole, id);
        item->setData(Qt::UserRole + 1, originalName);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
    QSqlQuery queryPlants;
    queryPlants.prepare("SELECT id, name FROM plants WHERE group_id = :gid");
    queryPlants.bindValue(":gid", _groupId);
    if(queryPlants.exec()){
        while(queryPlants.next()){
            int pId = queryPlants.value("id").toInt();
            QString pName = queryPlants.value("name").toString();
            QListWidgetItem *item = new QListWidgetItem(pName, ui->listPlants);
            item->setData(Qt::UserRole, pId);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void addEventDialog::setupProductsCombo(){
    ui->cmbProduct->blockSignals(true);
    ui->cmbProduct->clear();
    QSqlQuery query("SELECT ib.id, p.name || ' (Lotto: ' || ib.lot_number || ')' "
                    "FROM inventory_batches ib "
                    "JOIN products p ON ib.product_id = p.id "
                    "LEFT JOIN units u ON ib.unit = u.abbreviation "
                    "WHERE ib.is_active = 1");
    while(query.next()){
        int batchId = query.value(0).toInt();
        QString name = query.value(1).toString();
        ui->cmbProduct->addItem(name, batchId);
    }
    ui->cmbProduct->blockSignals(false);
    if(ui->cmbProduct->count() > 0) onProductChanged(ui->cmbProduct->currentIndex());
}

void addEventDialog::onProductChanged(int index){
    ui->cmbUnit->clear();
    if(index < 0) return;
    int batchId = ui->cmbProduct->itemData(index).toInt();
    QSqlQuery queryBatch;
    queryBatch.prepare("SELECT unit FROM inventory_batches WHERE id = :bid");
    queryBatch.bindValue(":bid", batchId);
    QString batchUnit = "";
    if(queryBatch.exec() && queryBatch.next()) batchUnit = queryBatch.value(0).toString().trimmed();
    if(batchUnit.isEmpty()) return;
    QString unitType = "";
    QString correctAbbreviation = batchUnit;
    QSqlQuery queryAll("SELECT unit_type, abbreviation, name FROM units");
    while(queryAll.next()){
        QString uType = queryAll.value(0).toString();
        QString abbr = queryAll.value(1).toString();
        QString name = queryAll.value(2).toString();
        if(abbr.compare(batchUnit, Qt::CaseInsensitive) == 0 || name.compare(batchUnit, Qt::CaseInsensitive) == 0){
            unitType = uType;
            correctAbbreviation = abbr;
            break;
        }
    }
    if(!unitType.isEmpty()){
        QSqlQuery query;
        query.prepare("SELECT abbreviation FROM units WHERE unit_type = :type ORDER BY multiplier_to_base ASC");
        query.bindValue(":type", unitType);
        if(query.exec()) while(query.next()) ui->cmbUnit->addItem(query.value(0).toString());
    }
    else ui->cmbUnit->addItem(correctAbbreviation);
    ui->cmbUnit->setCurrentText(correctAbbreviation);
}

void addEventDialog::loadRecord(){
    QSqlQuery query;
    query.prepare("SELECT timestamp, notes FROM events WHERE id = :id");
    query.bindValue(":id", _eventId);
    if(query.exec() && query.next()){
        ui->dateTimeEvent->setDateTime(QDateTime::fromString(query.value("timestamp").toString(), "yyyy-MM-dd HH:mm:ss"));
        ui->txtNotes->setPlainText(query.value("notes").toString());
    }
    query.prepare("SELECT event_type_id FROM event_event_types WHERE event_id = :id");
    query.bindValue(":id", _eventId);
    if(query.exec()){
        while(query.next()){
            int tid = query.value(0).toInt();
            for(int i=0; i < ui->listEventTypes->count(); i++){
                if(ui->listEventTypes->item(i)->data(Qt::UserRole).toInt() == tid) ui->listEventTypes->item(i)->setCheckState(Qt::Checked);
            }
        }
        onEventTypeChanged(nullptr);
    }
    query.prepare("SELECT technique_id FROM event_techniques WHERE event_id = :id");
    query.bindValue(":id", _eventId);
    if(query.exec()) while(query.next()){
        int tid = query.value(0).toInt();
        for(int i=0; i < ui->listTechniques->count(); i++){
            if(ui->listTechniques->item(i)->data(Qt::UserRole).toInt() == tid) ui->listTechniques->item(i)->setCheckState(Qt::Checked);
        }
    }
    query.prepare(
        "SELECT eu.batch_id, eu.quantity_used, p.name, ib.unit FROM event_usage eu "
        "JOIN inventory_batches ib ON eu.batch_id = ib.id "
        "JOIN products p ON ib.product_id = p.id WHERE eu.event_id = :id"
    );
    query.bindValue(":id", _eventId);
    if(query.exec()) while(query.next()){
        int batchId = query.value(0).toInt();
        double qty = query.value(1).toDouble();
        QString pName = query.value(2).toString();
        QString unit = query.value(3).toString();
        int row = ui->tableUsage->rowCount();
        ui->tableUsage->insertRow(row);
        QTableWidgetItem *itemProd = new QTableWidgetItem(pName);
        itemProd->setData(Qt::UserRole, batchId);
        ui->tableUsage->setItem(row, 0, itemProd);
        ui->tableUsage->setItem(row, 1, new QTableWidgetItem(QString::number(qty)));
        ui->tableUsage->setItem(row, 2, new QTableWidgetItem(unit));
        QPushButton *btnRemove = new QPushButton(tr("Remove"));
        ui->tableUsage->setCellWidget(row, 3, btnRemove);
        connect(btnRemove, &QPushButton::clicked, this, [this, btnRemove](){
            for(int i=0; i < ui->tableUsage->rowCount(); i++){
                if(ui->tableUsage->cellWidget(i, 3) == btnRemove){
                    ui->tableUsage->removeRow(i);
                    break;
                }
            }
        });
    }
}

void addEventDialog::validateAndSave(){
    int stateChangesCount = 0;
    int newStateId = 0;
    QList<int> selectedEventTypes;
    for(int i=0; i < ui->listEventTypes->count(); i++){
        QListWidgetItem *item = ui->listEventTypes->item(i);
        if(item->checkState() == Qt::Checked){
            int eventTypeId = item->data(Qt::UserRole).toInt();
            selectedEventTypes.append(eventTypeId);
            QSqlQuery checkQuery;
            checkQuery.prepare("SELECT changes_to_status_id FROM event_types WHERE id = :id");
            checkQuery.bindValue(":id", eventTypeId);
            if(checkQuery.exec() && checkQuery.next()){
                QVariant statusVar = checkQuery.value(0);
                if(!statusVar.isNull()){
                    stateChangesCount++;
                    newStateId = statusVar.toInt();
                }
            }
        }
    }
    if(stateChangesCount > 1){
        QMessageBox::warning(this, tr("Error"), tr("More than one event type changes the plant status"));
        return;
    }
    if(selectedEventTypes.isEmpty()){
        QMessageBox::warning(this, tr("Error"), tr("Select at least an event type."));
        return;
    }
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    int targetEventId = _eventId;
    if(_eventId == -1){
        QSqlQuery queryEvent;
        if(_plantId == -1){
            queryEvent.prepare("INSERT INTO events (timestamp, group_id, notes) VALUES (:ts, :group, :notes)");
            queryEvent.bindValue(":group", _groupId);
        }
        else{
            queryEvent.prepare("INSERT INTO events (timestamp, group_id, plant_id, notes) VALUES (:ts, :group, :plant, :notes)");
            queryEvent.bindValue(":group", _groupId);
            queryEvent.bindValue(":plant", _plantId);
        }
        queryEvent.bindValue(":ts", ui->dateTimeEvent->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
        queryEvent.bindValue(":notes", ui->txtNotes->toPlainText());
        if(!queryEvent.exec()) {
            db.rollback();
            return;
        }
        targetEventId = queryEvent.lastInsertId().toInt();
    }
    else{
        QSqlQuery queryRestore;
        queryRestore.prepare("SELECT batch_id, quantity_used FROM event_usage WHERE event_id = :eid");
        queryRestore.bindValue(":eid", _eventId);
        if(queryRestore.exec()){
            QSqlQuery queryUpdateWarehouse;
            queryUpdateWarehouse.prepare("UPDATE inventory_batches SET quantity_current = quantity_current + :qty, is_active = 1 WHERE id = :bid");
            while(queryRestore.next()){
                queryUpdateWarehouse.bindValue(":bid", queryRestore.value(0).toInt());
                queryUpdateWarehouse.bindValue(":qty", queryRestore.value(1).toDouble());
                if(!queryUpdateWarehouse.exec()) { db.rollback(); return; }
            }
        }
        QSqlQuery queryClean;
        queryClean.prepare("DELETE FROM event_usage WHERE event_id = :eid");queryClean.bindValue(":eid", _eventId); queryClean.exec();
        queryClean.prepare("DELETE FROM event_event_types WHERE event_id = :eid");queryClean.bindValue(":eid", _eventId); queryClean.exec();
        queryClean.prepare("DELETE FROM event_techniques WHERE event_id = :eid");queryClean.bindValue(":eid", _eventId); queryClean.exec();
        QSqlQuery queryEvent;
        queryEvent.prepare("UPDATE events SET timestamp = :ts, notes = :notes WHERE id = :eid");
        queryEvent.bindValue(":ts", ui->dateTimeEvent->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
        queryEvent.bindValue(":notes", ui->txtNotes->toPlainText());
        queryEvent.bindValue(":eid", _eventId);
        if(!queryEvent.exec()) { db.rollback(); return; }
    }

    QSqlQuery queryJoinTypes;
    queryJoinTypes.prepare("INSERT INTO event_event_types (event_id, event_type_id) VALUES (:eid, :tid)");
    for (int typeId : selectedEventTypes) {
        queryJoinTypes.bindValue(":eid", targetEventId);
        queryJoinTypes.bindValue(":tid", typeId);
        if (!queryJoinTypes.exec()){
            db.rollback();
            return;
        }
    }
    QSqlQuery queryTech;
    queryTech.prepare("INSERT INTO event_techniques (event_id, technique_id) VALUES (:eid, :tid)");
    for (int i = 0; i < ui->listTechniques->count(); ++i) {
        QListWidgetItem *item = ui->listTechniques->item(i);
        if (item->checkState() == Qt::Checked) {
            queryTech.bindValue(":eid", targetEventId);
            queryTech.bindValue(":tid", item->data(Qt::UserRole).toInt());
            if (!queryTech.exec()){
                db.rollback();
                return;
            }
        }
    }
    QSqlQuery queryUsage;
    queryUsage.prepare("INSERT INTO event_usage (event_id, batch_id, quantity_used) VALUES (:eid, :bid, :qty)");
    QSqlQuery queryInventoryUpdate;
    queryInventoryUpdate.prepare("UPDATE inventory_batches SET quantity_current = quantity_current - :qty WHERE id = :bid");
    for (int i = 0; i < ui->tableUsage->rowCount(); ++i) {
        int batchId = ui->tableUsage->item(i, 0)->data(Qt::UserRole).toInt();
        double enteredQty = ui->tableUsage->item(i, 1)->text().toDouble();
        QString enteredUnit = ui->tableUsage->item(i, 2)->text();
        double enteredMultiplier = 1.0;
        QSqlQuery queryEnteredUnit;
        queryEnteredUnit.prepare("SELECT multiplier_to_base FROM units WHERE abbreviation = :abbr");
        queryEnteredUnit.bindValue(":abbr", enteredUnit);
        if(queryEnteredUnit.exec() && queryEnteredUnit.next()) enteredMultiplier = queryEnteredUnit.value(0).toDouble();
        double batchMultiplier = 1.0;
        QSqlQuery queryBatch;
        queryBatch.prepare("SELECT unit FROM inventory_batches WHERE id = :bid");
        queryBatch.bindValue(":bid", batchId);
        if(queryBatch.exec() && queryBatch.next()){
            QString batchUnit = queryBatch.value(0).toString().trimmed();
            QSqlQuery queryUnit("SELECT abbreviation, name, multiplier_to_base FROM units");
            while(queryUnit.next()){
                if(queryUnit.value(0).toString().compare(batchUnit, Qt::CaseInsensitive) == 0 || queryUnit.value(1).toString().compare(batchUnit, Qt::CaseInsensitive) == 0){
                    batchMultiplier = queryUnit.value(2).toDouble();
                    break;
                }
            }
        }
        double finalQtyToDeduct = enteredQty * (enteredMultiplier / batchMultiplier);
        queryUsage.bindValue(":eid", targetEventId);
        queryUsage.bindValue(":bid", batchId);
        queryUsage.bindValue(":qty", finalQtyToDeduct);
        if (!queryUsage.exec()){
            db.rollback();
            QMessageBox::critical(this, tr("Database Error"), tr("Unable to save products: ") + queryUsage.lastError().text());
            return;
        }
        queryInventoryUpdate.bindValue(":qty", finalQtyToDeduct);
        queryInventoryUpdate.bindValue(":bid", batchId);
        if (!queryInventoryUpdate.exec()){
            db.rollback();
            QMessageBox::critical(this, tr("Database Error"), tr("Unable to update warehouse quantity: ") + queryInventoryUpdate.lastError().text());
            return;
        }
    }
    if(newStateId != 0){
        QSqlQuery queryStateUpdate;
        if(_plantId == -1){
            queryStateUpdate.prepare("UPDATE plants SET status_id = :sid WHERE group_id = :gid");
            queryStateUpdate.bindValue(":gid", _groupId);
        }
        else{
            queryStateUpdate.prepare("UPDATE plants SET status_id = :sid WHERE id = :pid");
            queryStateUpdate.bindValue(":pid", _plantId);
        }
        queryStateUpdate.bindValue(":sid", newStateId);
        if (!queryStateUpdate.exec()){
            db.rollback();
            return;
        }
    }
    int noteTypeId = -1;
    QSqlQuery queryType("SELECT id FROM event_types WHERE name = 'Note' LIMIT 1");
    if(queryType.exec() && queryType.next()) noteTypeId = queryType.value(0).toInt();
    db.commit();
    for(int i=0; i < ui->listPlants->count(); i++){
        QListWidgetItem *item = ui->listPlants->item(i);
        if(item->checkState() == Qt::Checked){
            int pId = item->data(Qt::UserRole).toInt();
            QString plantName = item->text();
            QMessageBox::information(this, tr("Plant specific entry"), tr("Adding an entry to the plant:") + "\n\n" + plantName);
            addEventDialog plantDlg(_groupId, pId, -1, this->parentWidget());
            plantDlg.setWindowTitle(tr("Specific entry for: ") + plantName);
            plantDlg.exec();
        }
    }
    QMessageBox::information(this, tr("Success"), tr("Diary entry correctly added."));
    accept();
}
