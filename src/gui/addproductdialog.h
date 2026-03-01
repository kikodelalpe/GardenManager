#pragma once

#include <QDialog>

namespace Ui {
class AddProductDialog;
}

class AddProductDialog : public QDialog{
    Q_OBJECT

public:
    explicit AddProductDialog(QWidget *parent = nullptr);
    ~AddProductDialog();
    void preselectCategory(int categoryId);

private slots:
    void validateAndSave();
    void onCategoryChanged(int index);
    void onButtonAddVarietyClicked();

private:
    Ui::AddProductDialog *ui;
    void setupCategories();
    void setupVarietyCombo();
    void resetDynamicFields();
};