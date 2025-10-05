#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableWidget->setWordWrap(true);
    ui->label_resultAll->setVisible(0);
}

MainWindow::~MainWindow()
{
    for (auto info: processes) {
        if (info->worker) {
            disconnect(info->worker, nullptr, nullptr, nullptr);
            info->worker->stop();
            info->worker->wait();
            info->worker->deleteLater();
        }
        delete info;
    }
    delete ui;
}

void MainWindow::on_pB_add_clicked()
{
    ui->label_resultAll->setVisible(0);
    ProcessInfo* info = new ProcessInfo(ui->cB_pickFunc->currentText(), ui->dSB_num->value());

    int rowCount = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(rowCount + 1);

    ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(info->funcName));
    ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(QString::number(info->value)));

    QProgressBar* bar = new QProgressBar();
    bar->setRange(0, 100);
    bar->setValue(0);
    ui->tableWidget->setCellWidget(rowCount, 2, bar);

    ui->tableWidget->setItem(rowCount, 3, new QTableWidgetItem(info->status));
    processes.append(info);

    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->resizeColumnsToContents();
}


void MainWindow::on_pB_start_clicked()
{
    ui->label_resultAll->setVisible(0);
    int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Процесс не выбран");
        return;
    }

    ProcessInfo* info = processes[row];
    if (info->worker != nullptr) {
        QMessageBox::warning(this, "Ошибка", "Процесс уже запущен");
        return;
    }

    info->worker = new Worker(info->funcName, info->value);

    connect(info->worker, &Worker::progressUpdated, this, [=](int value) {
        if (QProgressBar* bar = qobject_cast<QProgressBar*>(ui->tableWidget->cellWidget(row, 2)))
            bar->setValue(value);
    });

    connect(info->worker, &Worker::statusUpdated, this, [=](QString status) {
        info->status = status;
        ui->tableWidget->item(row, 3)->setText(status);
    });

    connect(info->worker, &Worker::finished, info->worker, &QObject::deleteLater);

    connect(info->worker, &Worker::finished, this, [=]() {
        info->worker = nullptr;
    });

    connect(info->worker, &Worker::finished, this, [=](long long resultN) {
        QMutexLocker locker(&mutex);
        resultSum += resultN;
        ui->label_resultAll->setText("Сумма: " + QString::number(resultSum));
        info->worker = nullptr;
    });

    info->worker->start();
}


void MainWindow::on_pB_pause_clicked()
{
    ui->label_resultAll->setVisible(0);
    int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Процесс не выбран");
        return;
    }

    ProcessInfo* info = processes[row];
    if (info->worker != nullptr) {
        info->isPaused = !info->isPaused;
        info->worker->pause(info->isPaused);
        info->status = info->isPaused ? "На паузе" : "Запущен";
        ui->tableWidget->item(row, 3)->setText(info->status);
    }
}


void MainWindow::on_pB_stop_clicked()
{
    ui->label_resultAll->setVisible(0);
    int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Процесс не выбран");
        return;
    }

    ProcessInfo* info = processes[row];
    if (info->worker != nullptr) {
        info->worker->stop();
        info->status = "Остановлен";
        ui->tableWidget->item(row, 3)->setText(info->status);
    }
}


void MainWindow::on_pB_delete_clicked()
{
    ui->label_resultAll->setVisible(0);
    int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Процесс не выбран");
        return;
    }

    ProcessInfo* info = processes[row];

    if (info->worker != nullptr) {
        disconnect(info->worker, nullptr, nullptr, nullptr);
        info->worker->stop();
        info->worker->wait();
        info->worker->deleteLater();
        info->worker = nullptr;
    }

    delete info;

    processes.remove(row);
    ui->tableWidget->removeRow(row);
}


void MainWindow::on_pB_startAll_clicked()
{
    ui->label_resultAll->setVisible(1);
    ui->label_resultAll->setText("Сумма: 0");

    {QMutexLocker locker(&mutex);
        resultSum = 0;}

    QSet<int> exceptions; //уже запущенные
    for (int i = 0; i < processes.size(); i++) {
        ProcessInfo* info = processes[i];
        if (info->worker != nullptr) {
            exceptions.insert(i);
            continue;
        }


        info->worker = new Worker(info->funcName, info->value);

        connect(info->worker, &Worker::progressUpdated, this, [=](int value) {
            if (QProgressBar* bar = qobject_cast<QProgressBar*>(ui->tableWidget->cellWidget(i, 2)))
                bar->setValue(value);
        });

        connect(info->worker, &Worker::statusUpdated, this, [=](QString status) {
            info->status = status;
            ui->tableWidget->item(i, 3)->setText(status);
        });

        connect(info->worker, &Worker::finished, info->worker, &QObject::deleteLater);

        connect(info->worker, &Worker::finished, this, [=](long long resultN) {
            QMutexLocker locker(&mutex);
            resultSum += resultN;
            ui->label_resultAll->setText("Сумма: " + QString::number(resultSum));
            info->worker = nullptr;
        });
    }

    for (int i = 0; i < processes.size(); i++) {
        if (exceptions.contains(i))
            continue;
        processes[i]->worker->start();
    }
}
