#include "addbatchdialog.h"
#include "ui_addbatchdialog.h"
#include "src/gui/addproductdialog.h"
#include "src/database/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QPushButton>
#include <QInputDialog>
#include <QSettings>

AddBatchDialog::AddBatchDialog(int batchId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddBatchDialog)
    , _batchId(batchId)
{
    ui->setupUi(this);
    ui->dateEditBuy->setDate(QDate::currentDate());
    ui->dateEditExpire->setDate(QDate::currentDate().addYears(1));
    setupCategoryFilter();
    connect(ui->cmbCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddBatchDialog::onCategoryChanged);
    refreshProductList();
    setupUnits();
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddBatchDialog::validateAndSave);
    connect(ui->btnCreateProduct, &QToolButton::clicked, this, &AddBatchDialog::onBtnCreateProductClicked);
    connect(ui->btnCreateCategory, &QToolButton::clicked, this, &AddBatchDialog::onBtnAddCategoryClicked);
    if(_batchId != -1){
        this->setWindowTitle(tr("Edit product"));
        loadRecord();
    }
}

AddBatchDialog::~AddBatchDialog()
{
    delete ui;
}

void AddBatchDialog::preselectCategory(int categoryId){
    int index = ui->cmbCategory->findData(categoryId);
    if(index != -1){
        ui->cmbCategory->setCurrentIndex(index);
        ui->cmbCategory->setEnabled(false);
    }
}

void AddBatchDialog::setupCategoryFilter(){
    ui->cmbCategory->clear();
    ui->cmbCategory->addItem(tr("All the products"), -1);
    QSqlQuery query("SELECT id, name FROM product_categories ORDER BY name ASC");
    while(query.next()) ui->cmbCategory->addItem(query.value("name").toString(), query.value("id"));
}

void AddBatchDialog::refreshProductList(){
    QVariant currentId = ui->cmbProduct->currentData();
    ui->cmbProduct->clear();
    int categoryId = -1;
    if(ui->cmbCategory->count() > 0) categoryId = ui->cmbCategory->currentData().toInt();
    QSqlQuery query;
    if(categoryId == -1) query.prepare("SELECT id, name FROM products ORDER BY name ASC");
    else{
        query.prepare("SELECT id, name FROM PRODUCTS WHERE category_id = :catId ORDER BY name ASC");
        query.bindValue(":catId", categoryId);
    }
    query.exec();
    while(query.next()){
        ui->cmbProduct->addItem(query.value("name").toString(), query.value("id"));
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->cmbProduct->count() > 0);
    int index = ui->cmbProduct->findData(currentId);
    if(index != -1) ui->cmbProduct->setCurrentIndex(index);
}

void AddBatchDialog::setupUnits(){
    ui->cmbUnits->clear();
    QSettings settings;
    QString unitSystem = settings.value("unit_system", "metric").toString();
    QSqlQuery query;
    query.prepare("SELECT abbreviation FROM units WHERE system IN (:sys, 'universal') ORDER BY id ASC");
    query.bindValue(":sys", unitSystem);
    query.exec();
    while(query.next()) ui->cmbUnits->addItem(query.value(0).toString());
}

void AddBatchDialog::onCategoryChanged(){
    refreshProductList();
}

void AddBatchDialog::onBtnCreateProductClicked(){
    AddProductDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted){
        refreshProductList();
        ui->cmbProduct->setCurrentIndex(ui->cmbProduct->count() - 1);
    }
}

void AddBatchDialog::onBtnAddCategoryClicked(){
    bool ok;
    QString newCategoryName = QInputDialog::getText(this, tr("New Category"), tr("Insert new category's name"), QLineEdit::Normal, "", &ok);
    if(ok && !newCategoryName.trimmed().isEmpty()){
        QSqlQuery query;
        query.prepare("INSERT INTO product_categories (name) VALUES (:name)");
        query.bindValue(":name", newCategoryName.trimmed());
        if(query.exec()){
            int newId = query.lastInsertId().toInt();
            setupCategoryFilter();
            int index = ui->cmbCategory->findData(newId);
            if(index != -1) ui->cmbCategory->setCurrentIndex(index);
            QMessageBox::information(this, tr("Success"), tr("Category correctly added"));
        }
        else QMessageBox::warning(this, tr("Error"), tr("Unable to add new category"));
    }
}

void AddBatchDialog::validateAndSave(){
    if(ui->cmbProduct->currentIndex() == -1){
        QMessageBox::warning(this, tr("Error"), tr("You have to select a product."));
        return;
    }
    double qty = ui->spinQty->value();
    if(qty <= 0 && _batchId == -1){
        QMessageBox::warning(this, tr("Error"), tr("Quantity has to be greater than 0."));
        return;
    }
    int isActive = (qty > 0) ? 1 : 0;
    QSqlQuery query;
    if(_batchId == -1) query.prepare("INSERT INTO inventory_batches (product_id, purchase_date, expiration_date, Quantity_initial, quantity_current, price_paid, lot_number, unit, is_active) VALUES(:prod, :buy, :exp, :qty, :qty, :price, :lot, :unit, :active)");
    else{
        query.prepare("UPDATE inventory_batches SET product_id = :prod, purchase_date = :pdate, lot_number = :lot, price_paid = :price, unit = :unit, quantity_current = :qty, expiration_date = :exp, is_active = :active WHERE id = :id");
        query.bindValue(":id", _batchId);
    }
    query.bindValue(":prod", ui->cmbProduct->currentData().toInt());
    query.bindValue(":buy", ui->dateEditBuy->date().toString("yyyy-MM-dd"));
    query.bindValue(":exp", ui->dateEditExpire->date().toString("yyyy-MM-dd"));
    query.bindValue(":qty", qty);
    query.bindValue(":price", ui->spinPrice->value());
    query.bindValue(":lot", ui->txtBatch->text());
    query.bindValue(":unit", ui->cmbUnits->currentText());
    query.bindValue(":active", isActive);
    if(query.exec()) accept();
    else QMessageBox::critical(this, "Error", "Unable to add the product to warehouse.");
}

void AddBatchDialog::loadRecord(){
    QSqlQuery query;
    query.prepare("SELECT product_id, purchase_date, lot_number, price_paid, unit, quantity_current, expiration_date "
                  "FROM inventory_batches WHERE id = :id");
    query.bindValue(":id", _batchId);
    if(query.exec() && query.next()){
        int prodId = query.value("product_id").toInt();
        int idx = ui->cmbProduct->findData(prodId);
        if(idx!= -1) ui->cmbProduct->setCurrentIndex(idx);
        ui->dateEditBuy->setDate(query.value("purchase_date").toDate());
        ui->dateEditExpire->setDate(query.value("expiration_date").toDate());
        ui->txtBatch->setText(query.value("lot_number").toString());
        ui->spinPrice->setValue(query.value("price_paid").toDouble());
        QString unit = query.value("unit").toString();
        int unitIdx = ui->cmbUnits->findText(unit);
        if(unitIdx != -1) ui->cmbUnits->setCurrentIndex(unitIdx);
        ui->spinQty->setValue(query.value("quantity_current").toDouble());
    }
}