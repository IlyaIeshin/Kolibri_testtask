#ifndef PROCESS_H
#define PROCESS_H

#include <QObject>
#include <QMutex>

class Process : public QObject{
    Q_OBJECT
public:
    Process(QStringList &ref_filenames, QString& inpath, QString& outpath, bool if_file_exists,
            bool del_file, qint64 value, bool is_call_once, bool& timer_done, QMutex& mtx);
    void process();

signals:
    void workDone();

private:
    bool fileIsOpen(const QString& filepath) const;

private:
    QStringList& m_ref_filenames;
    QString& m_inpath;
    QString& m_outpath;
    bool m_if_file_exists;
    bool m_del_file;
    qint64 m_value;
    bool m_is_call_once;

    bool& m_timer_done;
    QMutex& m_mtx;
};

#endif // PROCESS_H
