#ifndef WORKER_H
#define WORKER_H

#include <QString>
#include <QThread>
#include <QObject>
#include <QMessageBox>

class Worker: public QThread
{
    Q_OBJECT
public:
    Worker(QString func, double value);

    void pause(bool paused);
    void stop();


protected:
    void run() override;


signals:
    void progressUpdated(int percent);
    void statusUpdated(QString status);
    void finished(long long resultN);


private:
    QString func;
    double value;

    bool paused = false;
    bool stopped = false;
};

#endif // WORKER_H
