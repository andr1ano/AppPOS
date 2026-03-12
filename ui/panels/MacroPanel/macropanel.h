#ifndef MACROPANEL_H
#define MACROPANEL_H

#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QListWidget>
#include <QTimer>
#include <QVector>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QMap>

#include <Player/player.h>
#include <Recorder/recorder.h>
#include <Scriptio/scriptio.h>

class MacroPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MacroPanel(QWidget *parent = nullptr);
    enum class State { Idle, Countdown, Recording, Playing, Paused };
    State m_state = State::Idle;

signals:
    // Emitted whenever the macro state changes.
    // text  — e.g. "МАКРО: ЗАПИС"
    // color — CSS colour string, e.g. "#EF4444"
    void statusChanged(const QString &text, const QString &color);

private slots:
    // ── Record ───────────────────────────────────────────────────────────
    void onMacroRecord();
    void onMacroStopRecord();
    void onRecordingCountdown();
    void onEventRecorded(const InputEvent &ev);
    void onRecordingStopped(int count, qint64 durationMs);
    void onRecorderError(const QString &msg);

    // ── Playback ─────────────────────────────────────────────────────────
    void onMacroPlay();
    void onMacroStopPlay();
    void onMacroPauseResume();
    void onEventPlayed(int index, const InputEvent &ev);
    void onLoopCompleted(int n);
    void onPlayFinished();
    void onPlayerError(const QString &msg);

    // ── File ─────────────────────────────────────────────────────────────
    void onMacroSave();
    void onMacroOpen();
    void onMacroClear();

private:
    // ── Build helpers ─────────────────────────────────────────────────────
    void buildLayout();
    void buildRecordGroup(QWidget *parent, QVBoxLayout *layout);
    void buildPlayGroup  (QWidget *parent, QVBoxLayout *layout);
    void buildFileGroup  (QWidget *parent, QVBoxLayout *layout);
    void buildLog        (QWidget *parent, QVBoxLayout *layout);

    // ── Runtime helpers ───────────────────────────────────────────────────
    void startCountdown(int secs);
    void updateUi();
    void addLog(const QString &text, const QString &color = "#94A3B8");

    // ── Backend objects ───────────────────────────────────────────────────
    Recorder            *m_recorder       = nullptr;
    Player              *m_player         = nullptr;
    QTimer              *m_countdownTimer = nullptr;
    int                  m_countdownSec   = 0;
    QVector<InputEvent>  m_events;
    qint64               m_durMs          = 0;

    // ── Record group widgets ───────────────────────────────────────────────
    QSpinBox    *m_spnDelay        = nullptr;
    QLabel      *m_lblCountdown    = nullptr;
    QPushButton *m_btnRecord       = nullptr;
    QPushButton *m_btnStopRecord   = nullptr;
    QLabel      *m_lblEventCount   = nullptr;
    QLabel      *m_lblDuration     = nullptr;

    // ── Play group widgets ────────────────────────────────────────────────
    QCheckBox   *m_chkLoop         = nullptr;
    QSpinBox    *m_spnLoops        = nullptr;
    QProgressBar *m_progress       = nullptr;
    QPushButton *m_btnPlay         = nullptr;
    QPushButton *m_btnPause        = nullptr;
    QPushButton *m_btnStopPlay     = nullptr;

    // ── File group widgets ────────────────────────────────────────────────
    QPushButton *m_btnSave         = nullptr;
    QPushButton *m_btnOpen         = nullptr;
    QPushButton *m_btnClear        = nullptr;

    // ── Log ───────────────────────────────────────────────────────────────
    QListWidget *m_log             = nullptr;
};

#endif //MACROPANEL_H
