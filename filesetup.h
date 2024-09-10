#ifndef FILESETUP_H
#define FILESETUP_H

#include <QWidget>
#include <unordered_set>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "process.h"

class FileSetup : public QWidget
{
    Q_OBJECT
public:
    explicit FileSetup(QWidget *parent = nullptr);

public slots:
    void startProcess();
    void selectOutPath();
    void selectOutPath(const QString& path);
    void selectInPath();
    void selectInPath(const QString& path);
    void ifFileExists(bool if_file_exists);
    void isCallOnce(bool is_call_once);
    void setValue(const QString& value);
    void setMaskFile(const QString& mask_file);
    void setDelFile(bool is_no);
    void availabilityFiles();
    void timerFinished();

private:
    void setFileNames();

signals:
    void chosenFile(const QString& filepath);
    void chosenOutPath(const QString& outpath);
    void chosenInPath(const QString& inpath);
    void workDone();
private:
    bool m_del_file;
    bool m_is_call_once;
    bool m_if_file_exists;
    QString m_mask_file;
    QString m_inpath;
    QString m_outpath;
    qint64 m_value;

    QMutex m_mtx;
    bool m_timer_done;
    QStringList m_filenames; // список имен файлов
    std::unordered_set<quint64> m_hashnames; // хешированные имена файлов для исключения повторений

};

#endif // FILESETUP_H
