#ifndef RECORDER_H
#define RECORDER_H

#pragma once

#include <QThread>
#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>

#include <cstring>

#include <Scriptio/scriptio.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/extensions/record.h>

// C legacy
#undef KeyPress
#undef KeyRelease

struct _XDisplay;
typedef struct _XDisplay Display;
typedef unsigned long XRecordContext;

class Recorder : public QThread
{
    Q_OBJECT

public:
    explicit Recorder(QObject *parent = nullptr);
    ~Recorder() override;

    void startRecording();
    void stopRecording();

    QVector<InputEvent> takeEvents();
    qint64 elapsedMs() const;

    signals:
        void eventRecorded(const InputEvent &ev);
    void recordingStopped(int count, qint64 durationMs);
    void errorOccurred(const QString &msg);

protected:
    void run() override;

private:
    static void xRecordCallback(char *priv, void *data);
    void        handleXEvent(void *interceptData);

    Display        *m_dpyData    = nullptr;
    Display        *m_dpyCtrl    = nullptr;
    XRecordContext  m_ctx        = 0;

    QElapsedTimer       m_timer;
    mutable QMutex      m_mutex;
    QVector<InputEvent> m_events;

    static constexpr qint64 kMoveThrottleMs = 16;
    qint64 m_lastMoveTs = -1000;
    int    m_lastMX = -1, m_lastMY = -1;
};

#endif