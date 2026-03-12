#include <Scriptio/scriptio.h>

static const char *typeName(InputEvent::Type t)
{
    switch (t) {
    case InputEvent::MouseMove:    return "MOUSE_MOVE";
    case InputEvent::MousePress:   return "MOUSE_PRESS";
    case InputEvent::MouseRelease: return "MOUSE_RELEASE";
    case InputEvent::KeyPress:     return "KEY_PRESS";
    case InputEvent::KeyRelease:   return "KEY_RELEASE";
    }
    return "UNKNOWN";
}

QString InputEvent::toScript() const
{
    QString line = QString("T=%1 %2").arg(timestamp).arg(typeName(type));

    switch (type) {
    case MouseMove:
        line += QString(" X=%1 Y=%2").arg(x).arg(y);
        break;
    case MousePress:
    case MouseRelease:
        line += QString(" BTN=%1 X=%2 Y=%3").arg(button).arg(x).arg(y);
        break;
    case KeyPress:
    case KeyRelease:
        line += QString(" CODE=%1 SYM=%2").arg(keycode).arg(keyName.isEmpty() ? "?" : keyName);
        break;
    }
    return line;
}

bool InputEvent::fromScript(const QString &rawLine, InputEvent &out)
{
    QString line = rawLine.trimmed();
    if (line.isEmpty() || line.startsWith('#'))
        return false;

    auto tokens = line.split(' ', Qt::SkipEmptyParts);
    if (tokens.size() < 2) return false;

    QMap<QString, QString> kv;
    for (const auto &tok : tokens) {
        int eq = tok.indexOf('=');
        if (eq > 0)
            kv[tok.left(eq).toUpper()] = tok.mid(eq + 1);
        else
            kv[tok.toUpper()] = QString();
    }

    if (!kv.contains("T")) return false;
    out.timestamp = kv["T"].toLongLong();

    QString typeStr;
    for (auto it = kv.begin(); it != kv.end(); ++it) {
        if (it.key() == "MOUSE_MOVE" || it.key() == "MOUSE_PRESS" ||
            it.key() == "MOUSE_RELEASE" || it.key() == "KEY_PRESS" ||
            it.key() == "KEY_RELEASE") {
            typeStr = it.key();
            break;
        }
    }
    if (typeStr.isEmpty() && tokens.size() >= 2)
        typeStr = tokens[1].toUpper();

    if      (typeStr == "MOUSE_MOVE")    out.type = InputEvent::MouseMove;
    else if (typeStr == "MOUSE_PRESS")   out.type = InputEvent::MousePress;
    else if (typeStr == "MOUSE_RELEASE") out.type = InputEvent::MouseRelease;
    else if (typeStr == "KEY_PRESS")     out.type = InputEvent::KeyPress;
    else if (typeStr == "KEY_RELEASE")   out.type = InputEvent::KeyRelease;
    else return false;

    out.x       = kv.value("X", "0").toInt();
    out.y       = kv.value("Y", "0").toInt();
    out.button  = kv.value("BTN", "0").toInt();
    out.keycode = kv.value("CODE", "0").toInt();
    out.keyName = kv.value("SYM");

    return true;
}

bool ScriptIO::save(const QString &path, const QVector<InputEvent> &events,
                    qint64 durationMs)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&f);
    out << "# MacroRecorder Script v1.0\n";
    out << "# Created: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    out << "# Events:  " << events.size() << "\n";
    out << "# Duration: " << durationMs << " ms\n";
    out << "#\n";
    out << "# Format: T=<ms> TYPE [X=x Y=y] [BTN=n] [CODE=n SYM=name]\n";
    out << "#\n";

    for (const auto &ev : events)
        out << ev.toScript() << "\n";

    return true;
}

bool ScriptIO::load(const QString &path, QVector<InputEvent> &events,
                    qint64 &durationMs, QString &error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = "Cannot open file: " + path;
        return false;
    }

    events.clear();
    durationMs = 0;

    QTextStream in(&f);
    int lineNo = 0;
    while (!in.atEnd()) {
        ++lineNo;
        QString line = in.readLine();

        if (line.startsWith("# Duration:")) {
            durationMs = line.mid(line.indexOf(':') + 1).trimmed()
                             .split(' ').first().toLongLong();
            continue;
        }

        InputEvent ev;
        if (InputEvent::fromScript(line, ev))
            events.append(ev);
    }

    if (events.isEmpty()) {
        error = "No events found in script.";
        return false;
    }

    if (durationMs == 0 && !events.isEmpty())
        durationMs = events.last().timestamp;

    return true;
}
