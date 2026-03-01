#pragma once

#include <QMainWindow>
#include <QSqlRelationalTableModel>
#include <QTreeWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTabSelectedChanged(int index);
    void onActionSettingsTriggered();
    void onActionBackupTriggered();
    //diaries
    void onBtnAddPlantClicked();
    void onBtnAddEventClicked();
    void onBtnAddGroupClicked();
    void onTreeSelectionChanged();
    void updateDynamicButtons();
    void onBtnEditItemClicked();
    void onBtnDeleteItemClicked();
    //warehouse
    void onBtnAddBatchClicked();
    void onBtnEditWarehouseClicked();
    void onBtnDeleteWarehouseClicked();
    //plants catalog
    void onBtnEditCatalogClicked();
    void onBtnDeleteCatalogClicked();
    void onBtnAddVarietyClicked();

private:
    Ui::MainWindow *ui;
    //diaries
    void setupDiariesTab();
    void populateTreeGroup(QTreeWidgetItem *parentItem, int parentGroupId);
    void loadDiariesForGroup(int groupId);
    void loadPlantsForGroup(int groupId);
    //warehouse
    QSqlRelationalTableModel *_warehouseModel;
    void setupWarehouse();
    //plants catalog
    QSqlRelationalTableModel *_catalogModel;
    void setupCatalog();
};