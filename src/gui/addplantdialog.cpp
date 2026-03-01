#include "addplantdialog.h"
#include "ui_addplantdialog.h"
#include "src/gui/addbatchdialog.h"
#include "src/gui/addgroupdialog.h"
#include "src/utils/constants.h"
#include "src/database/databasemanager.h"
#include <QDate>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QToolButton>
#include <QSqlQuery>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>

AddPlantDialog::AddPlantDialog(int plantId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddPlantDialog)
    , _plantId(plantId)
{
    ui->setupUi(this);
    QSettings settings("MyGardenApp", "GardenManager");
    QString dateFormat = settings.value("date_format", "dd/MM/yyyy").toString();
    ui->dateEditStart->setDisplayFormat(dateFormat);
    setupGroupsCombo();
    setupSeedsCombo();
    setupStatusCombo();
    ui->dateEditStart->setDate(QDate::currentDate());
    if(_plantId != -1){
        this->setWindowTitle(tr("Edit plant"));
        ui->spinQty->setVisible(false);
        ui->lblQuantity->setVisible(false);
        loadRecord();
    }
    else{
        ui->cmbStatus->setVisible(false);
        ui->lblStatus->setVisible(false);
        ui->btnAddStatus->setVisible(false);
    }
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddPlantDialog::validateAndSave);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::rejected);
    connect(ui->btnAddGroup, &QPushButton::clicked, this, &AddPlantDialog::onButtonAddGroupClicked);
    connect(ui->btnAddSeed, &QPushButton::clicked, this, &AddPlantDialog::onButtonAddSeedClicked);
    connect(ui->btnAddStatus, &QPushButton::clicked, this, &AddPlantDialog::onButtonAddStatusClicked);
}

AddPlantDialog::~AddPlantDialog()
{
    delete ui;
}

void AddPlantDialog::setupGroupsCombo()
{
    ui->cmbGroup->clear();
    QMap<QString, int> orderedGroups = DatabaseManager::getHierarchicalGroups();
    for (const QString &path : orderedGroups.keys())
        ui->cmbGroup->addItem(path, orderedGroups.value(path));
}

void AddPlantDialog::setupSeedsCombo()
{
    ui->cmbSeed->clear();
    QMap<int, QString> seeds = DatabaseManager::getAvailableSeeds();
    for (auto seed = seeds.begin(); seed != seeds.end(); ++seed)
        ui->cmbSeed->addItem(seed.value(), seed.key());
}

void AddPlantDialog::setupStatusCombo(){
    ui->cmbStatus->clear();
    const char *defaultStatuses[] = {
        QT_TR_NOOP("Not started yet"),
        QT_TR_NOOP("Planted"),
        QT_TR_NOOP("Seedling"),
        QT_TR_NOOP("Growing"),
        QT_TR_NOOP("Flowering"),
        QT_TR_NOOP("Fruiting"),
        QT_TR_NOOP("Vegetative rest"),
        QT_TR_NOOP("Harvested"),
        QT_TR_NOOP("Dead")
    };
    QSqlQuery query("SELECT id, name FROM plant_statuses ORDER BY id ASC");
    while(query.next()){
        QString originalName = query.value("name").toString();
        int id = query.value("id").toInt();
        QString translatedName = tr(originalName.toUtf8().constData());
        ui->cmbStatus->addItem(translatedName, id);
    }
}

void AddPlantDialog::onButtonAddGroupClicked()
{
    AddGroupDialog dlg(-1, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        setupGroupsCombo();
        ui->cmbGroup->setCurrentIndex(ui->cmbGroup->count() - 1);
    }
}

void AddPlantDialog::onButtonAddSeedClicked()
{
    AddBatchDialog dlg(-1, this);
    dlg.preselectCategory(CategoryID::Seeds);
    if (dlg.exec() == QDialog::Accepted)
    {
        setupSeedsCombo();
        ui->cmbSeed->setCurrentIndex(ui->cmbSeed->count() - 1);
    }
}

void AddPlantDialog::onButtonAddStatusClicked(){
    bool ok;
    QString newStatus = QInputDialog::getText(this, tr("New state"), tr("Insert new state's name:"), QLineEdit::Normal, "", &ok);
    if(ok && !newStatus.trimmed().isEmpty()){
        int existingIndex = ui->cmbStatus->findText(newStatus.trimmed(), Qt::MatchFixedString | Qt::MatchCaseSensitive);
        if(existingIndex != -1){
            QMessageBox::information(this, tr("Info"), tr("This status already exists."));
            ui->cmbStatus->setCurrentIndex(existingIndex);
            return;
        }
        QSqlQuery query;
        query.prepare("INSERT INTO plant_statuses (name) VALUES (:n)");
        query.bindValue(":n", newStatus.trimmed());
        if(query.exec()){
            setupStatusCombo();
            ui->cmbStatus->setCurrentText(newStatus.trimmed());
        }
        else QMessageBox::critical(this, tr("Error"), tr("Database error: ") + query.lastError().text());
    }
}

void AddPlantDialog::loadRecord(){
    QSqlQuery query;
    query.prepare("SELECT name, group_id, seed_batch_id, start_date, status_id FROM plants WHERE id = :id");
    query.bindValue(":id", _plantId);
    if(query.exec() && query.next()){
        ui->txtName->setText(query.value("name").toString());
        int groupId = query.value("group_id").toInt();
        int idxGroup = ui->cmbGroup->findData(groupId);
        if(idxGroup != -1) ui->cmbGroup->setCurrentIndex(idxGroup);
        int seedId = query.value("seed_batch_id").toInt();
        int idxSeed = ui->cmbSeed->findData(seedId);
        if(idxSeed != -1) ui->cmbSeed->setCurrentIndex(idxSeed);
        ui->dateEditStart->setDate(QDate::fromString(query.value("start_date").toString(), "yyyy-MM-dd"));
        int statusId = query.value("status_id").toInt();
        int idxStatus = ui->cmbStatus->findData(statusId);
        if(idxStatus != -1) ui->cmbStatus->setCurrentIndex(idxStatus);
    }
}

void AddPlantDialog::validateAndSave()
{
    QString name = ui->txtName->text();
    int parentId = ui->cmbGroup->currentData().toInt();
    int seedId = ui->cmbSeed->currentData().toInt();
    QString startDate = ui->dateEditStart->date().toString("yyyy-MM-dd");
    int quantity = ui->spinQty->value();
    if (name.isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("Plant name can't be empty."));
        return;
    }
    if (parentId == -1)
    {
        QMessageBox::warning(this, tr("Error"), tr("You must select a parent group."));
        return;
    }
    if (seedId == -1)
    {
        QMessageBox::warning(this, tr("Error"), tr("You must select a seed."));
        return;
    }
    if (quantity <= 0 && _plantId == -1)
    {
        QMessageBox::warning(this, tr("Error"), tr("Quantity is not valid."));
        return;
    }
    int varietyId = 0;
    QSqlQuery queryVariety;
    queryVariety.prepare("SELECT p.variety_id "
                         "FROM inventory_batches ib "
                         "JOIN products p ON ib.product_id = p.id "
                         "WHERE ib.id = :seedBatchId ");
    queryVariety.bindValue(":seedBatchId", seedId);
    if (queryVariety.exec() && queryVariety.next())
    {
        varietyId = queryVariety.value("variety_id").toInt();
        if (varietyId == 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Selected seed has no variety in warehouse."));
            return;
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Unable to get seed data."));
        return;
    }
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    if(_plantId == -1){
        QSqlQuery queryInsert;
        queryInsert.prepare("INSERT INTO plants (variety_id, group_id, seed_batch_id, name, start_date, status_id) VALUES (:variety, :group, :seed, :n, :date, :status)");
        queryInsert.bindValue(":variety", varietyId);
        queryInsert.bindValue(":group", parentId);
        queryInsert.bindValue(":seed", seedId);
        queryInsert.bindValue(":date", startDate);
        queryInsert.bindValue(":status", 1);
        for (int i = 1; i <= quantity; i++){
            QString plantName = name;
            if (quantity > 1) plantName += " - " + QString::number(i);
            queryInsert.bindValue(":n", plantName);
            if (!queryInsert.exec()){
                db.rollback();
                QMessageBox::critical(this, tr("Error"), tr("Unable to save data: ") + queryInsert.lastError().text());
                return;
            }
        }
    }
    else{
        int statusId = ui->cmbStatus->currentData().toInt();
        QSqlQuery queryUpdate;
        queryUpdate.prepare("UPDATE plants SET variety_id = :variety, group_id = :group, seed_batch_id = :seed, name = :n, start_date = :date, status_id = :status WHERE id = :id");
        queryUpdate.bindValue(":variety", varietyId);
        queryUpdate.bindValue(":group", parentId);
        queryUpdate.bindValue(":seed", seedId);
        queryUpdate.bindValue(":n", name);
        queryUpdate.bindValue(":date", startDate);
        queryUpdate.bindValue(":status", statusId);
        queryUpdate.bindValue(":id", _plantId);
        if(!queryUpdate.exec()){
            db.rollback();
            QMessageBox::critical(this, tr("Error"), tr("Unable to update data: ") + queryUpdate.lastError().text());
            return;
        }
    }
    db.commit();
    QMessageBox::information(this, tr("Success"), tr("Plants correctly saved."));
    accept();
}
