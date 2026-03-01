#pragma once

#include <QDialog>

namespace Ui
{
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

signals:
    void settingsChanged();

private slots:
    void saveSettings();

private:
    Ui::SettingsDialog *ui;
    void populateCombos();
    void loadCurrentSettings();
};