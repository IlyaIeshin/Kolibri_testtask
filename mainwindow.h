#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "filesetup.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void setTime(const QString& time);
    void updateTimer();
    void callOnceWork();
    void timerWork();

signals:
    void timerFinished();

private:
    bool m_is_call_once;
    Ui::MainWindow *ui;
    QTimer *timer;
    QTimer *frequency;
    FileSetup *m_filesetup;
};
#endif // MAINWINDOW_H
