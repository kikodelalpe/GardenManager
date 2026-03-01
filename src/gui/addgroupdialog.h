#pragma once

#include <QDialog>

namespace Ui {
class AddGroupDialog;
}

class AddGroupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddGroupDialog(int groupId = -1, QWidget *parent = nullptr);
    ~AddGroupDialog();

private slots:
    void validateAndSave();
    
private:
    Ui::AddGroupDialog *ui;
    int _groupId;
    void setupGroupsCombo();
    void loadRecord();
};