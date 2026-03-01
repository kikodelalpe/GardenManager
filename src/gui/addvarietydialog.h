#pragma once

#include <QDialog>

namespace Ui {
class AddVarietyDialog;
}

class AddVarietyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddVarietyDialog(int varietyId = -1, QWidget *parent = nullptr);
    ~AddVarietyDialog();

private slots:
    void onBtnAddSpecieClicked();
    void validateAndSave();
    
    private:
    Ui::AddVarietyDialog *ui;
    int _varietyId;
    void loadRecord();
    void refreshSpeciesList();
};