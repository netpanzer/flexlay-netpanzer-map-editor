#pragma once
#include <QByteArray>
#include <QFile>
#include <QString>
#include <QTemporaryDir>

// Write `data` to a uniquely-named file in a per-run temporary directory and
// return its path. The directory is static so it outlives the returned path
// and is cleaned up when the test binary exits.
inline QString writeTempFile(const QByteArray& data, const QString& suffix,
                             const QString& prefix = "test")
{
    static QTemporaryDir tmpDir;
    static int counter = 0;
    const QString path = tmpDir.filePath(
        QString("%1_%2%3").arg(prefix).arg(++counter).arg(suffix));
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        return {};
    f.write(data);
    return path;
}
