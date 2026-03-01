#include "addvarietydialog.h"
#include "ui_addvarietydialog.h"
#include "src/gui/addspeciedialog.h"
#include "src/database/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QPushButton>
#include <QInputDialog>

AddVarietyDialog::AddVarietyDialog(int varietyId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddVarietyDialog)
    , _varietyId(varietyId)
{
    ui->setupUi(this);
    refreshSpeciesList();
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddVarietyDialog::validateAndSave);
    connect(ui->btnAddSpecie, &QToolButton::clicked, this, &AddVarietyDialog::onBtnAddSpecieClicked);
    if(_varietyId != -1){
        this->setWindowTitle(tr("Edit variety"));
        loadRecord();
    }
}

AddVarietyDialog::~AddVarietyDialog()
{
    delete ui;
}

void AddVarietyDialog::refreshSpeciesList(){
    QVariant currentId = ui->cmbSpecie->currentData();
    ui->cmbSpecie->clear();
    int categoryId = -1;
    QSqlQuery query("SELECT id, common_name FROM species ORDER BY common_name ASC");
    query.exec();
    while(query.next()){
        ui->cmbSpecie->addItem(query.value("common_name").toString(), query.value("id"));
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->cmbSpecie->count() > 0);
    int index = ui->cmbSpecie->findData(currentId);
    if(index != -1) ui->cmbSpecie->setCurrentIndex(index);
}

void AddVarietyDialog::onBtnAddSpecieClicked(){
    AddSpecieDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted){
        refreshSpeciesList();
        ui->cmbSpecie->setCurrentIndex(ui->cmbSpecie->count() - 1);
    }
}

void AddVarietyDialog::validateAndSave(){
    if(ui->cmbSpecie->currentIndex() == -1){
        QMessageBox::warning(this, tr("Error"), tr("You have to select a plant specie."));
        return;
    }
    if(ui->txtName->text() == ""){
        QMessageBox::warning(this, tr("Error"), tr("Name can't be empty."));
        return;
    }
    QSqlQuery query;
    if(_varietyId == -1) query.prepare("INSERT INTO varieties (species_id, name, days_to_maturity, variety_description) VALUES (:specie, :n, :days, :desc)");
    else{
        query.prepare("UPDATE varieties SET species_id = :specie, name = :n, days_to_maturity = :days, variety_description = :desc WHERE id = :id");
        query.bindValue(":id", _varietyId);
    }
    query.bindValue(":specie", ui->cmbSpecie->currentData().toInt());
    query.bindValue(":n", ui->txtName->text());
    query.bindValue(":days", ui->spinDaysToMaturity->value());
    query.bindValue(":desc", ui->txtEditDescription->toPlainText());
    if(query.exec()) accept();
    else QMessageBox::critical(this, tr("Error"), tr("Unable to add the variety to plants catalog."));
}

void AddVarietyDialog::loadRecord(){
    QSqlQuery query;
    query.prepare("SELECT species_id, name, days_to_maturity, variety_description FROM varieties WHERE id = :id");
    query.bindValue(":id", _varietyId);
    if(query.exec() && query.next()){
        ui->txtName->setText(query.value("name").toString());
        ui->spinDaysToMaturity->setValue(query.value("days_to_maturity").toInt());
        ui->txtEditDescription->setPlainText(query.value("variety_description").toString());
        int specId = query.value("species_id").toInt();
        int idx = ui->cmbSpecie->findData(specId);
        if(idx != -1) ui->cmbSpecie->setCurrentIndex(idx);
    }
}