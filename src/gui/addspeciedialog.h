#pragma once

#include <QDialog>

namespace Ui {
class AddSpecieDialog;
}

class AddSpecieDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddSpecieDialog(QWidget *parent = nullptr);
    ~AddSpecieDialog();

private slots:
    void validateAndSave();

private:
    Ui::AddSpecieDialog *ui;
};