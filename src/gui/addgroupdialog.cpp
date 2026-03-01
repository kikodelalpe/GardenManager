#include "addgroupdialog.h"
#include "ui_addgroupdialog.h"
#include "src/database/databasemanager.h"
#include <QMessageBox>

AddGroupDialog::AddGroupDialog(int groupId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddGroupDialog)
    , _groupId(groupId)
{
    ui->setupUi(this);
    setupGroupsCombo();
    if(_groupId != -1){
        this->setWindowTitle(tr("Edit group"));
        loadRecord();
    }
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddGroupDialog::validateAndSave);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::rejected);
}

AddGroupDialog::~AddGroupDialog()
{
    delete ui;
}

void AddGroupDialog::setupGroupsCombo(){
    ui->cmbGroups->clear();
    ui->cmbGroups->addItem(tr("Root"), -1);
    QMap<QString, int> orderedGroups = DatabaseManager::getHierarchicalGroups();
    for(const QString &path : orderedGroups.keys()) ui->cmbGroups->addItem(path, orderedGroups.value(path));
}

void AddGroupDialog::loadRecord(){
    QSqlQuery query;
    query.prepare("SELECT name, parent_id FROM groups WHERE id = :id");
    query.bindValue(":id", _groupId);
    if(query.exec() && query.next()){
        ui->txtName->setText(query.value("name").toString());
        int parentId = -1;
        if(!query.value("parent_id").isNull()) parentId = query.value("parent_id").toInt();
        int idx = ui->cmbGroups->findData(parentId);
        if(idx != -1) ui->cmbGroups->setCurrentIndex(idx);
    }
}

void AddGroupDialog::validateAndSave(){
    QString name = ui->txtName->text();
    int parentId = ui->cmbGroups->currentData().toInt();
    if(name.isEmpty()){
        QMessageBox::warning(this, tr("Error"), tr("Name can't be empty."));
        return;
    }
    if(_groupId != -1 && _groupId == parentId){
        QMessageBox::warning(this, tr("Error"), tr("A group can't be his own subgroup."));
        return;
    }
    QSqlQuery query;
    if(_groupId == -1) query.prepare("INSERT INTO groups (name, parent_id) VALUES (:n, :parent)");
    else{
        query.prepare("UPDATE groups SET name = :n, parent_id = :parent WHERE id = :id");
        query.bindValue(":id", _groupId);
    }
    query.bindValue(":n", name);
    if(parentId <= 0) query.bindValue(":parent", QVariant());
    else query.bindValue(":parent", parentId);
    if(query.exec()){
        QMessageBox::information(this, tr("Success"), tr("Group correctly saved."));
        accept();
    }
    else{
        QString errorMsg = query.lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("Database Error: ") + errorMsg);
        return;
    }
}
