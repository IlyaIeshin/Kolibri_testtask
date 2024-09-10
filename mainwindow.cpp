#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QRadioButton>
#include <QStyle>
#include <QRegularExpression>
#include <QValidator>
#include <QTimer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_is_call_once(true)
    , timer(new QTimer(this))
    , frequency(new QTimer(this))
    , m_filesetup(new FileSetup(this))
{
    ui->setupUi(this);
    // маска для файлов
    connect(ui->file_path, &QLineEdit::editingFinished, [this](){
        m_filesetup->setMaskFile(ui->file_path->text());
    });

    // установка иконок для кнопок открытия директорий
    ui->choice_outpath->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    ui->choice_inpath->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    // для выбора пути директорий
    connect(ui->output_path, &QLineEdit::editingFinished, [this](){
        m_filesetup->selectOutPath(ui->output_path->text());
    });
    connect(ui->input_path, &QLineEdit::editingFinished, [this](){
        m_filesetup->selectInPath(ui->input_path->text());
    });
    connect(ui->choice_outpath, &QPushButton::clicked, m_filesetup, qOverload<>(&FileSetup::selectOutPath));
    connect(ui->choice_inpath, &QPushButton::clicked, m_filesetup, qOverload<>(&FileSetup::selectInPath));
    // установка пути в label
    connect(m_filesetup, &FileSetup::chosenOutPath, [this](const QString& outpath){
        ui->output_path->setText(outpath);
    });
    connect(m_filesetup, &FileSetup::chosenInPath,[this](const QString& inpath){
        ui->input_path->setText(inpath);
    });

    // установка создать копию если файл уже существует
    ui->create_copy_btn->setChecked(true);
    connect(ui->create_copy_btn, &QRadioButton::toggled, m_filesetup, &FileSetup::ifFileExists);

    // установка value
    connect(ui->value_box, &QSpinBox::editingFinished, m_filesetup, [this](){
        m_filesetup->setValue(ui->value_box->text());
    });

    // установка "Нет" по умолчанию
    ui->no_button->setChecked(true);
    connect(ui->no_button, &QRadioButton::toggled, m_filesetup, &FileSetup::setDelFile);

    // отключить функциональность таймера по умолчанию
    ui->one_time_job_btn->setChecked(true);
    ui->timer_print->setEnabled(false);
    ui->running_time->setEnabled(false);
    ui->survey_freq->setEnabled(false);

    // одноразовый запуск
    connect(ui->one_time_job_btn, &QRadioButton::toggled, this, [this](bool is_call_once){
        m_filesetup->isCallOnce(is_call_once);
        ui->running_time->setEnabled(false);
        ui->timer_print->setEnabled(false);
        ui->survey_freq->setEnabled(false);
    });

    // функциональность таймера
    connect(ui->timer_job_btn, &QRadioButton::toggled, this, [this](bool is_call_once){
        m_filesetup->isCallOnce(!is_call_once);
        ui->running_time->setEnabled(true);
        ui->timer_print->setEnabled(true);
        ui->survey_freq->setEnabled(true);
    });

    //установка ограничений
    ui->running_time->setMinimum(1);
    ui->running_time->setMaximum(24*60*60-1);
    ui->survey_freq->setMinimum(1);
    ui->survey_freq->setMaximum(24*60*60-1);
    ui->timer_print->setReadOnly(true);
    //установить таймер после ввода пользователя
    connect(ui->running_time, &QSpinBox::editingFinished, this, [this](){
        setTime(ui->running_time->text());
    });
    // переодичность опроса файлов
    connect(ui->survey_freq, &QSpinBox::editingFinished, this, [this](){
        frequency->setInterval(ui->survey_freq->text().toInt() * 1000);
    });

    //начало работы
    connect(ui->begin_btn, &QPushButton::clicked, [this](){
        if(ui->one_time_job_btn->isChecked())
            callOnceWork();
        else {
            timer->start(1000);
            frequency->start();
            timerWork();
        }
        m_filesetup->startProcess();
        setEnabled(false);
    });
    connect(m_filesetup, &FileSetup::workDone, this, [this](){
        QMessageBox::information(this, "Уведомление", "Работа завершена!");
        setEnabled(true);
        setTime(ui->running_time->text());
        frequency->setInterval(ui->survey_freq->text().toInt() * 1000);
        this->setTime(ui->running_time->text());
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::callOnceWork()
{
    disconnect(timer, &QTimer::timeout, this, &MainWindow::updateTimer);
    disconnect(frequency, &QTimer::timeout, m_filesetup, &FileSetup::availabilityFiles);
}

void MainWindow::timerWork()
{
    // обновлять таймер
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTimer);
    //  опрашивать на наличие новых файлов
    connect(frequency, &QTimer::timeout, m_filesetup, &FileSetup::availabilityFiles);
    // таймер закончил работу
    connect(this, &MainWindow::timerFinished, m_filesetup, &FileSetup::timerFinished);

    qDebug() << " MainWindow::timerWork i work";
}


void MainWindow::setTime(const QString &time)
{
    int remaining_time = time.toInt();
    int h = remaining_time / 3600;
    int m = (remaining_time % 3600) / 60;
    int s = remaining_time % 60;
    ui->timer_print->setTime(QTime(h,m,s));
    qDebug() << "MainWindow::setTime i work";
}

void MainWindow::updateTimer()
{
    qDebug() << "MainWindow::updateTimer i work";
    QTime t = ui->timer_print->time();
    if(t > QTime(0,0,0))
        ui->timer_print->setTime(t.addSecs(-1));
    else{
        timer->stop();
        frequency->stop();
        disconnect(timer, &QTimer::timeout, this, &MainWindow::updateTimer);
        disconnect(frequency, &QTimer::timeout, m_filesetup, &FileSetup::availabilityFiles);
        emit timerFinished();
    }
}




