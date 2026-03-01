#include "addproductdialog.h"
#include "ui_addproductdialog.h"
#include "src/gui/addvarietydialog.h"
#include "src/database/databasemanager.h"
#include "src/utils/constants.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

AddProductDialog::AddProductDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddProductDialog)
{
    ui->setupUi(this);
    setupCategories();
    setupVarietyCombo();
    resetDynamicFields();
    int defaultCategoryIndex = ui->cmbCategory->currentIndex();
    onCategoryChanged(defaultCategoryIndex);
    connect(ui->cmbCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddProductDialog::onCategoryChanged);
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddProductDialog::validateAndSave);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->btnAddVariety, &QToolButton::clicked, this, &AddProductDialog::onButtonAddVarietyClicked);
}

AddProductDialog::~AddProductDialog()
{
    delete ui;
}

void AddProductDialog::preselectCategory(int categoryId){
    int index = ui->cmbCategory->findData(categoryId);
    if(index != -1){
        ui->cmbCategory->setCurrentIndex(index);
        ui->cmbCategory->setEnabled(false);
    }
}

void AddProductDialog::setupCategories(){
    ui->cmbCategory->clear();
    const char *defaultCategories[] = {
        QT_TR_NOOP("Fertilizers"),
        QT_TR_NOOP("Seeds"),
        QT_TR_NOOP("Substrates"),
        QT_TR_NOOP("Pots"),
        QT_TR_NOOP("Tools"),
        QT_TR_NOOP("Other")
    };
    QSqlQuery query("SELECT id, name FROM product_categories ORDER BY id ASC");
    while(query.next()){
        QString originalName = query.value("name").toString();
        int id = query.value("id").toInt();
        QString translatedName = tr(originalName.toUtf8().constData());
        ui->cmbCategory->addItem(translatedName, id);
    }
}

void AddProductDialog::setupVarietyCombo(){
    ui->cmbVariety->clear();
    QSqlQuery query("SELECT id, name FROM varieties ORDER BY name ASC");
    while(query.next()){
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        ui->cmbVariety->addItem(name, id);
    }
}

void AddProductDialog::resetDynamicFields(){
    ui->groupFertilizer->setVisible(false);
    ui->groupSeeds->setVisible(false);
    ui->groupSubstrates->setVisible(false);
}

void AddProductDialog::onCategoryChanged(int index){
    resetDynamicFields();
    int categoryId = ui->cmbCategory->currentData().toInt();
    switch(categoryId){
        case CategoryID::Fertilizers: ui->groupFertilizer->setVisible(true); break;
        case CategoryID::Seeds: ui->groupSeeds->setVisible(true); break;
        case CategoryID::Substrates: ui->groupSubstrates->setVisible(true); break;
        default: break;
    }
}

void AddProductDialog::onButtonAddVarietyClicked(){
    AddVarietyDialog dlg(-1, this);
    if(dlg.exec() == QDialog::Accepted){
        setupVarietyCombo();
        ui->cmbVariety->setCurrentIndex(ui->cmbVariety->count() - 1);
    }
}

void AddProductDialog::validateAndSave(){
    QString name = ui->txtName->text();
    if(name.isEmpty()){
        QMessageBox::warning(this, tr("Error"), tr("Product name can't be empty."));
        return;
    }
    int catID = ui->cmbCategory->currentData().toInt();
    double n = 0, p = 0, k = 0, ph = 0;
    int isOrganic = 0;
    QString substrateRecipe = "";
    int varietyId = -1;
    if(catID == CategoryID::Fertilizers){
        n = ui->spinN->value();
        p = ui->spinP->value();
        k = ui->spinK->value();
        isOrganic = ui->checkOrganic->isChecked() ? 1 : 0;
    }
    else if(catID == CategoryID::Substrates){
        ph = ui->spinPh->value();
        substrateRecipe = ui->textEditRecipe->toPlainText();
    }
    else if(catID == CategoryID::Seeds){
        varietyId = ui->cmbVariety->currentData().toInt();
    }

    QSqlQuery query;
    query.prepare("INSERT INTO products (name, manufacturer, category_id, description, n_value, p_value, k_value, is_organic, variety_id, ph, recipe) VALUES (:name, :manuf, :cat, :desc, :n, :p, :k, :org, :variety, :ph, :recipe)");
    query.bindValue(":name", name);
    query.bindValue(":manuf", ui->txtManufacturer->text());
    query.bindValue(":cat", catID);
    query.bindValue(":desc", ui->textEditDescription->toPlainText());
    query.bindValue(":n", n);
    query.bindValue(":p", p);
    query.bindValue(":k", k);
    query.bindValue(":org", isOrganic);
    if(catID != CategoryID::Seeds) query.bindValue(":variety", QVariant());
    else query.bindValue(":variety", varietyId);
    query.bindValue(":ph", ph);
    query.bindValue(":recipe", substrateRecipe);
    if(query.exec()){
        QMessageBox::information(this, tr("Success"), tr("Product correctly added."));
        accept();
    }
    else QMessageBox::critical(this, tr("Error"), tr("Database error: ") + query.lastError().text());
}
