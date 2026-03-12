#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    populateCombos();
    loadCurrentSettings();
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::saveSettings);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::populateCombos()
{
    ui->cmbLanguage->addItem("Italiano", "it");
    ui->cmbLanguage->addItem("English", "en");
    ui->cmbSystem->addItem(tr("Metric"), "metric");
    ui->cmbSystem->addItem(tr("Imperial"), "imperial");
    ui->cmbCurrency->addItem("Euro (€)", "EUR");
    ui->cmbCurrency->addItem("Dollar ($)", "USD");
    ui->cmbCurrency->addItem("Pound (£)", "GBP");
    ui->cmbDate->addItem("28/02/2026 (dd/MM/yyyy)", "dd/MM/yyyy");
    ui->cmbDate->addItem("02/28/2026 (MM/dd/yyyy)", "MM/dd/yyyy");
    ui->cmbDate->addItem("2026-02-28 (yyyy-MM-dd)", "yyyy-MM-dd");
    ui->cmbTime->addItem("15:30 (24h)", "HH:mm");
    ui->cmbTime->addItem("03:30 PM (12h)", "hh:mm AP");
}

void SettingsDialog::loadCurrentSettings()
{
    QSettings settings;
    QString savedLang = settings.value("language", "it").toString();
    QString savedSystem = settings.value("unit_system", "metric").toString();
    QString savedCurrency = settings.value("currency", "EUR").toString();
    QString savedDate = settings.value("date_format", "dd/MM/yyyy").toString();
    QString savedTime = settings.value("time_format", "HH:mm").toString();
    int idx;
    idx = ui->cmbLanguage->findData(savedLang);
    if (idx != -1)
        ui->cmbLanguage->setCurrentIndex(idx);
    idx = ui->cmbSystem->findData(savedSystem);
    if (idx != -1)
        ui->cmbSystem->setCurrentIndex(idx);
    idx = ui->cmbCurrency->findData(savedCurrency);
    if (idx != -1)
        ui->cmbCurrency->setCurrentIndex(idx);
    idx = ui->cmbDate->findData(savedDate);
    if (idx != -1)
        ui->cmbDate->setCurrentIndex(idx);
    idx = ui->cmbTime->findData(savedTime);
    if (idx != -1)
        ui->cmbTime->setCurrentIndex(idx);
}

void SettingsDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("language", ui->cmbLanguage->currentData().toString());
    settings.setValue("unit_system", ui->cmbSystem->currentData().toString());
    settings.setValue("currency", ui->cmbCurrency->currentData().toString());
    settings.setValue("date_format", ui->cmbDate->currentData().toString());
    settings.setValue("time_format", ui->cmbTime->currentData().toString());
    QMessageBox::information(this, tr("Success"), tr("Settings saved correctly.\nLanguage will be updated at the next start."));
    emit settingsChanged();
    accept();
}