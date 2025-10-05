#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QVector>
#include <QMessageBox>
#include <QDebug>
#include <QMutex>
#include "processinfo.h"


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
    void on_pB_add_clicked();

    void on_pB_start_clicked();

    void on_pB_pause_clicked();

    void on_pB_stop_clicked();

    void on_pB_delete_clicked();

    void on_pB_startAll_clicked();

private:
    Ui::MainWindow *ui;
    QVector<ProcessInfo*> processes;
    long long resultSum;
    QMutex mutex;
};
#endif // MAINWINDOW_H
