#include "process.h"
#include <QFile>
#include <QFileInfo>

Process::Process(QStringList &ref_filenames, QString& inpath, QString& outpath, bool if_file_exists,
                bool del_file, qint64 value, bool is_call_once, bool& timer_done, QMutex& mtx)
    : m_ref_filenames(ref_filenames), m_inpath(inpath), m_outpath(outpath),
      m_if_file_exists(if_file_exists), m_del_file(del_file), m_value(value),
      m_is_call_once(is_call_once), m_timer_done(timer_done), m_mtx(mtx)
{

}

bool Process::fileIsOpen(const QString& filepath) const
{
    QString tempFilePath = filepath + ".temp_check";
    if (QFile::rename(filepath, tempFilePath)) {
        QFile::rename(tempFilePath, filepath);
        return true;
    } else {
        return false;
    }
}


void Process::process()
{
    while(!m_timer_done)
    {
        for (const QString& filename : m_ref_filenames)
        {
            QMutexLocker l(&m_mtx);
            qDebug() << filename;

            QString filepath = QString("%1/%2").arg(m_inpath).arg(filename);
            bool file_is_open = fileIsOpen(filepath);

            if(!file_is_open)
                continue;

            // если выбрано создавать копию
            if(m_if_file_exists)
            {
                QFileInfo f_info(filepath);
                QString new_filepath = QString("%1/%2").arg(m_outpath).arg(filename);
                int count = 1;
                while(QFile::exists(new_filepath))
                {
                    new_filepath = QString("%1/%2_%3.%4").arg(m_outpath)
                                                    .arg(f_info.baseName())
                                                    .arg(count)
                                                    .arg(f_info.suffix());
                    count++;
                }
                QFile::copy(filepath, new_filepath);
                if(!m_del_file)
                    QFile::remove(filepath);
                filepath = new_filepath;
            }
            // если выбрана перезапись
            else
            {   // если in != out, то просто перезаписываем outfile и если надо удаляем infile
                if(m_inpath != m_outpath)
                {
                    QString new_filepath = QString("%1/%2").arg(m_outpath).arg(filename);
                    QFile::copy(filepath, new_filepath);
                    if(!m_del_file)
                        QFile::remove(filepath);
                    filepath = new_filepath;
                }
                // если путь in == out и выбрано удаление файлов, то удаление не выполняется
                // модифицируется исходный файл
            }

            QFile file(filepath);
            bool is_open = file.open(QIODevice::ReadWrite);

            if(!is_open){
                file.close();
                continue;
            }

            qint64 file_size = file.size();
            if(file_size == 0){
                file.close();
                continue;
            }

            QByteArray byte_arr(reinterpret_cast<const char*>(&m_value), sizeof(qint64));
            QByteArray read_arr;

            if(file_size < sizeof(quint64))
            {
                read_arr = file.read(sizeof(qint64) - file_size);
                file.seek(0);
            }
            else
            {
                file.seek(file_size-sizeof(qint64));
                read_arr = file.read(sizeof(qint64));
                file.seek(file_size-sizeof(qint64));
            }
            int min = file_size > sizeof(qint64) ? sizeof(qint64) : file_size;
            for(int i = min; i > 0; i--)
            {
                file.seek(file_size - i);
                char get;
                file.getChar(&get);
                char put = get ^ byte_arr[i-1];
                file.seek(file_size - i);
                file.putChar(put);
            }
            file.close();
            m_ref_filenames.takeFirst();
        }
        if(m_is_call_once){
            break;
        }
    }
    emit workDone();
}
