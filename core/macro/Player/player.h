#ifndef PLAYER_H
#define PLAYER_H

#pragma once

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>

#include <Scriptio/scriptio.h>

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

struct _XDisplay;
typedef struct _XDisplay Display;

class Player : public QObject
{
    Q_OBJECT

public:
    explicit Player(QObject *parent = nullptr);
    ~Player() override;

    void load(const QVector<InputEvent> &events);
    void setLoop(bool loop) { m_loop = loop; }
    bool isPlaying() const  { return m_playing; }
    int  currentLoop() const { return m_loopCount; }

    public slots:
        void play();
    void stop();
    void pause();
    void resume();

    signals:
        void started(int totalEvents);
    void eventPlayed(int index, const InputEvent &ev);
    void loopCompleted(int loopNumber);
    void finished();
    void errorOccurred(const QString &msg);

    private slots:
        void onTimer();

private:
    bool openDisplay();
    void closeDisplay();
    void injectEvent(const InputEvent &ev);
    void scheduleNext();

    Display *m_dpy    = nullptr;
    QTimer  *m_timer  = nullptr;

    QVector<InputEvent> m_events;
    int                 m_index      = 0;
    bool                m_loop       = false;
    bool                m_playing    = false;
    bool                m_paused     = false;
    int                 m_loopCount  = 0;

    QElapsedTimer m_elapsed;
    qint64        m_pauseOffset = 0;
};

#endif