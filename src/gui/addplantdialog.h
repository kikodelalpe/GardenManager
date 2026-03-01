#pragma once

#include <QDialog>


namespace Ui {
class AddPlantDialog;
}

class AddPlantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddPlantDialog(int plantId = -1, QWidget *parent = nullptr);
    ~AddPlantDialog();

private slots:
    void onButtonAddGroupClicked();
    void onButtonAddSeedClicked();
    void onButtonAddStatusClicked();

private:
    Ui::AddPlantDialog *ui;
    int _plantId;
    void setupGroupsCombo();
    void setupSeedsCombo();
    void setupStatusCombo();
    void loadRecord();
    void validateAndSave();
};
