#ifndef SCRIPTIO_H
#define SCRIPTIO_H

#pragma once

#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStringList>

// C legacy
#undef KeyPress
#undef KeyRelease

// C legacy
#undef KeyPress
#undef KeyRelease

struct InputEvent {
    enum Type {
        MouseMove    = 0,
        MousePress   = 1,
        MouseRelease = 2,
        KeyPress     = 3,
        KeyRelease   = 4
    };

    Type    type      = MouseMove;
    qint64  timestamp = 0;
    int     x         = 0;
    int     y         = 0;
    int     button    = 0;
    int     keycode   = 0;
    QString keyName;

    QString toScript() const;
    static bool fromScript(const QString &line, InputEvent &out);
};

class ScriptIO {
public:
    static bool save(const QString &path, const QVector<InputEvent> &events,
                     qint64 durationMs);
    static bool load(const QString &path, QVector<InputEvent> &events,
                     qint64 &durationMs, QString &error);
};

#endif