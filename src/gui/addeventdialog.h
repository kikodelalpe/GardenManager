#pragma once

#include <QDialog>
#include <QListWidget>

namespace Ui {
class addEventDialog;
}

class addEventDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addEventDialog(int groupId, int plantId, int eventId, QWidget *parent = nullptr);
    ~addEventDialog();

private slots:
    void onButtonAddProductClicked();
    void onButtonAddToListClicked();
    void onEventTypeChanged(QListWidgetItem *item);
    void onProductChanged(int index);

private:
    Ui::addEventDialog *ui;
    int _groupId;
    int _plantId;
    int _eventId;
    void setupLists();
    void setupProductsCombo();
    void loadRecord();
    void validateAndSave();
};