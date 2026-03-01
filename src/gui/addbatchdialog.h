#pragma once

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class AddBatchDialog;
}

class AddBatchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddBatchDialog(int batchId = -1, QWidget *parent = nullptr);
    ~AddBatchDialog();
    void preselectCategory(int categoryId);

private slots:
    void onBtnCreateProductClicked();
    void onBtnAddCategoryClicked();
    void validateAndSave();
    void onCategoryChanged();

private:
    Ui::AddBatchDialog *ui;
    int _batchId;
    void loadRecord();
    void setupCategoryFilter();
    void refreshProductList();
    void setupUnits();
};
