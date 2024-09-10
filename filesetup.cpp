#include "filesetup.h"
#include <QLayout>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>

FileSetup::FileSetup(QWidget *parent)
    : QWidget(parent), m_filenames()
{
    m_del_file = true;
    m_if_file_exists = true;
    m_is_call_once = true;
    m_timer_done = false;
}

void FileSetup::startProcess()
{

    Process* process = new Process(m_filenames, m_inpath, m_outpath, m_if_file_exists,
                                   m_del_file, m_value, m_is_call_once, m_timer_done, m_mtx);
    QThread* process_thread = new QThread;
    m_timer_done = false;
    process->moveToThread(process_thread);

    connect(process_thread, &QThread::started, process, &Process::process);
    connect(process, &Process::workDone, process_thread, &QThread::quit);
    connect(process, &Process::workDone, process, &Process::deleteLater);
    connect(process_thread, &QThread::finished, process_thread, &QThread::deleteLater);

    connect(process, &Process::workDone, this, [this](){
        m_filenames.clear();
        m_hashnames.clear();
        emit workDone();
    });
    setFileNames();

    process_thread->start();
}

void FileSetup::selectOutPath(const QString &path)
{
    QDir outdir(path);
    if(!outdir.exists())
        QMessageBox::warning(nullptr, "Ошибка!", "Указанной директории не существует!");
    m_outpath = path;
}

void FileSetup::selectOutPath()
{
    m_outpath = QFileDialog::getExistingDirectory(this, tr("Выберите путь для сохранения нового файла"),
                                                        "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!m_outpath.isEmpty()) {
        emit chosenOutPath(m_outpath);
    }
}

void FileSetup::selectInPath(const QString &path)
{
    QDir indir(path);
    if(!indir.exists())
        QMessageBox::warning(nullptr, "Ошибка!", "Указанной директории не существует!");
    m_inpath = path;
}

void FileSetup::selectInPath()
{
    m_inpath = QFileDialog::getExistingDirectory(this, tr("Выберите директорию для входящих файлов"),
                                                       "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!m_inpath.isEmpty()) {
        emit chosenInPath(m_inpath);
    }
}

void FileSetup::ifFileExists(bool if_file_exists)
{
    m_if_file_exists = if_file_exists;
}

void FileSetup::isCallOnce(bool is_call_once)
{
    m_is_call_once = is_call_once;
}

void FileSetup::setValue(const QString &value)
{
    m_value = value.toInt();
    qint64 temp = m_value;
    m_value = m_value << 31;
    m_value += temp;
}

void FileSetup::setMaskFile(const QString &mask_file)
{
    m_mask_file = mask_file;
}

void FileSetup::setDelFile(bool del_file)
{
    m_del_file = del_file;
}

void FileSetup::setFileNames()
{
    QDirIterator iter_dir(m_inpath, QStringList(m_mask_file), QDir::Files);
    QString temp_filename{};

    do {
        iter_dir.next();
        temp_filename = iter_dir.fileName();
        auto check = m_hashnames.insert(qHash(temp_filename));
        if(check.second)
            m_filenames.append(temp_filename);
    } while(iter_dir.hasNext());
}

void FileSetup::availabilityFiles()
{
    QMutexLocker l(&m_mtx);
    setFileNames();
}

void FileSetup::timerFinished()
{
    QMutexLocker l(&m_mtx);
    m_timer_done = true;
}
