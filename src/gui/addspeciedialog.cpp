#include "addspeciedialog.h"
#include "ui_addspeciedialog.h"
#include "src/database/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

AddSpecieDialog::AddSpecieDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddSpecieDialog)
{
    ui->setupUi(this);
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddSpecieDialog::validateAndSave);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AddSpecieDialog::~AddSpecieDialog()
{
    delete ui;
}

void AddSpecieDialog::validateAndSave(){
    QString commonName = ui->txtCommonName->text();
    if(commonName.isEmpty()){
        QMessageBox::warning(this, tr("Error"), tr("Common name can't be empty."));
        return;
    }
    QString scientificName = ui->txtScientificName->text();
    if(scientificName.isEmpty()){
        QMessageBox::warning(this, tr("Error"), tr("Scientific name can't be empty."));
        return;
    }
    QString family = ui->txtFamily->text();
    if(family.isEmpty()){
        QMessageBox::warning(this, tr("Error"), tr("Family can't be empty."));
        return;
    }
    QSqlQuery query;
    query.prepare("INSERT INTO species (scientific_name, common_name, family, coltivation_guide, temp_min, temp_max, rh_min, rh_max, ph_soil_min, ph_soil_max, ph_water_min, ph_water_max) VALUES (:sName, :cName, :family, :guide, :tMin, :tMax, :hMin, :hMax, :phMinS, :phMaxS, :phMinW, :phMaxW)");
    query.bindValue(":sName", scientificName);
    query.bindValue(":cName", commonName);
    query.bindValue(":family", family);
    query.bindValue("guide", ui->txtEditDescription->toPlainText());
    query.bindValue("tMin", ui->spinTempMin->value());
    query.bindValue("tMax", ui->spinTempMax->value());
    query.bindValue("hMin", ui->spinRhMin->value());
    query.bindValue("hMax", ui->spinRhMax->value());
    query.bindValue("phMinS", ui->spinSoilPhMin->value());
    query.bindValue("phMaxS", ui->spinSoilPhMax->value());
    query.bindValue("phMinW", ui->spinWaterPhMin->value());
    query.bindValue("phMaxW", ui->spinWaterPhMax->value());
    if(query.exec()){
        QMessageBox::information(this, tr("Success"), tr("Specie correctly added."));
        accept();
    }
    else QMessageBox::critical(this, tr("Error"), tr("Database error: ") + query.lastError().text());
}