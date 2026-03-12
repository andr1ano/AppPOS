#include <MacroPanel/macropanel.h>

namespace
{
    struct BadgeInfo
    {
          QString text;
          QString stateKey;
    };

    const QMap<MacroPanel::State, BadgeInfo> &stateInfo()
    {
        static const QMap<MacroPanel::State, BadgeInfo> map = {
            { MacroPanel::State::Idle,      { "МАКРО: Очікування",   "idle" } },
            { MacroPanel::State::Countdown, { "МАКРО: Відлік...",    "countdown" } },
            { MacroPanel::State::Recording, { "МАКРО: ЗАПИС",        "recording" } },
            { MacroPanel::State::Playing,   { "МАКРО: ВІДТВОРЕННЯ",  "playing" } },
            { MacroPanel::State::Paused,    { "МАКРО: ПАУЗА",        "paused" } },
        };
        return map;
    }
}

MacroPanel::MacroPanel(QWidget *parent)
    : QWidget(parent)
{
    m_recorder       = new Recorder(this);
    m_player         = new Player(this);
    m_countdownTimer = new QTimer(this);
    m_countdownTimer->setInterval(1000);

    buildLayout();
    updateUi();

    connect(m_recorder, &Recorder::eventRecorded,
            this, &MacroPanel::onEventRecorded);
    connect(m_recorder, &Recorder::recordingStopped,
            this, &MacroPanel::onRecordingStopped);
    connect(m_recorder, &Recorder::errorOccurred,
            this, &MacroPanel::onRecorderError);

    connect(m_player, &Player::eventPlayed,
            this, &MacroPanel::onEventPlayed);
    connect(m_player, &Player::loopCompleted,
            this, &MacroPanel::onLoopCompleted);
    connect(m_player, &Player::finished,
            this, &MacroPanel::onPlayFinished);
    connect(m_player, &Player::errorOccurred,
            this, &MacroPanel::onPlayerError);

    connect(m_countdownTimer, &QTimer::timeout,
            this, &MacroPanel::onRecordingCountdown);

    connect(m_chkLoop, &QCheckBox::toggled,
            m_spnLoops, &QSpinBox::setEnabled);
}

void MacroPanel::buildLayout()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 14, 14, 14);
    root->setSpacing(10);

    auto *title = new QLabel("MACRO RECORDER", this);
    title->setObjectName("macroTitle");
    root->addWidget(title);

    buildRecordGroup(this, root);
    buildPlayGroup  (this, root);
    buildFileGroup  (this, root);
    buildLog        (this, root);
}

void MacroPanel::buildRecordGroup(QWidget *parent, QVBoxLayout *layout)
{
    auto *grp = new QGroupBox("Запис", parent);
    grp->setObjectName("macroGroup");
    auto *grpL = new QVBoxLayout(grp);
    grpL->setSpacing(6);

    auto *delayRow = new QHBoxLayout();
    delayRow->addWidget(new QLabel("Затримка старту (с):", parent));
    m_spnDelay = new QSpinBox(parent);
    m_spnDelay->setRange(0, 10);
    m_spnDelay->setValue(3);
    m_spnDelay->setFixedWidth(56);
    delayRow->addWidget(m_spnDelay);
    delayRow->addStretch();
    grpL->addLayout(delayRow);

    m_lblCountdown = new QLabel(parent);
    m_lblCountdown->setObjectName("macroCountdown");
    m_lblCountdown->setAlignment(Qt::AlignCenter);
    m_lblCountdown->hide();
    grpL->addWidget(m_lblCountdown);

    m_btnRecord = new QPushButton("Почати запис", parent);
    m_btnRecord->setObjectName("btnMacroRecord");
    m_btnRecord->setMinimumHeight(40);
    grpL->addWidget(m_btnRecord);

    m_btnStopRecord = new QPushButton("Зупинити запис", parent);
    m_btnStopRecord->setObjectName("btnMacroStop");
    m_btnStopRecord->setMinimumHeight(40);
    grpL->addWidget(m_btnStopRecord);

    auto *infoGrid = new QGridLayout();
    infoGrid->addWidget(new QLabel("Подій:",     parent), 0, 0);
    m_lblEventCount = new QLabel("0",      parent);
    m_lblEventCount->setObjectName("macroInfoVal");
    infoGrid->addWidget(m_lblEventCount, 0, 1);

    infoGrid->addWidget(new QLabel("Тривалість:", parent), 1, 0);
    m_lblDuration = new QLabel("0 мс", parent);
    m_lblDuration->setObjectName("macroInfoVal");
    infoGrid->addWidget(m_lblDuration, 1, 1);
    grpL->addLayout(infoGrid);

    layout->addWidget(grp);

    connect(m_btnRecord,     &QPushButton::clicked, this, &MacroPanel::onMacroRecord);
    connect(m_btnStopRecord, &QPushButton::clicked, this, &MacroPanel::onMacroStopRecord);
}

void MacroPanel::buildPlayGroup(QWidget *parent, QVBoxLayout *layout)
{
    auto *grp = new QGroupBox("Відтворення", parent);
    grp->setObjectName("macroGroup");
    auto *grpL = new QVBoxLayout(grp);
    grpL->setSpacing(6);

    auto *loopRow = new QHBoxLayout();
    m_chkLoop = new QCheckBox("Зациклити", parent);
    m_spnLoops = new QSpinBox(parent);
    m_spnLoops->setRange(0, 9999);
    m_spnLoops->setValue(0);
    m_spnLoops->setSpecialValueText("∞");
    m_spnLoops->setFixedWidth(56);
    m_spnLoops->setEnabled(false);
    loopRow->addWidget(m_chkLoop);
    loopRow->addStretch();
    loopRow->addWidget(new QLabel("Разів:"));
    loopRow->addWidget(m_spnLoops);
    grpL->addLayout(loopRow);

    m_progress = new QProgressBar(parent);
    m_progress->setObjectName("macroProgress");
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_progress->setTextVisible(false);
    m_progress->setFixedHeight(5);
    grpL->addWidget(m_progress);

    m_btnPlay = new QPushButton("Відтворити", parent);
    m_btnPlay->setObjectName("btnMacroPlay");
    m_btnPlay->setMinimumHeight(40);
    grpL->addWidget(m_btnPlay);

    auto *row2 = new QHBoxLayout();
    m_btnPause = new QPushButton("Пауза", parent);
    m_btnPause->setObjectName("btnMacroPause");
    m_btnPause->setMinimumHeight(36);

    m_btnStopPlay = new QPushButton("Стоп", parent);
    m_btnStopPlay->setObjectName("btnMacroStopPlay");
    m_btnStopPlay->setMinimumHeight(36);

    row2->addWidget(m_btnPause);
    row2->addWidget(m_btnStopPlay);
    grpL->addLayout(row2);

    layout->addWidget(grp);

    connect(m_btnPlay,     &QPushButton::clicked, this, &MacroPanel::onMacroPlay);
    connect(m_btnPause,    &QPushButton::clicked, this, &MacroPanel::onMacroPauseResume);
    connect(m_btnStopPlay, &QPushButton::clicked, this, &MacroPanel::onMacroStopPlay);
}

void MacroPanel::buildFileGroup(QWidget *parent, QVBoxLayout *layout)
{
    auto *grp = new QGroupBox("Файл скрипту", parent);
    grp->setObjectName("macroGroup");
    auto *grpL = new QVBoxLayout(grp);
    grpL->setSpacing(6);

    m_btnSave = new QPushButton("Зберегти скрипт", parent);
    m_btnSave->setObjectName("btnMacroFile");
    m_btnSave->setMinimumHeight(36);

    m_btnOpen = new QPushButton("Відкрити скрипт", parent);
    m_btnOpen->setObjectName("btnMacroFile");
    m_btnOpen->setMinimumHeight(36);

    m_btnClear = new QPushButton("Очистити", parent);
    m_btnClear->setObjectName("btnMacroClear");
    m_btnClear->setMinimumHeight(36);

    grpL->addWidget(m_btnSave);
    grpL->addWidget(m_btnOpen);
    grpL->addWidget(m_btnClear);
    layout->addWidget(grp);

    connect(m_btnSave,  &QPushButton::clicked, this, &MacroPanel::onMacroSave);
    connect(m_btnOpen,  &QPushButton::clicked, this, &MacroPanel::onMacroOpen);
    connect(m_btnClear, &QPushButton::clicked, this, &MacroPanel::onMacroClear);
}

void MacroPanel::buildLog(QWidget *parent, QVBoxLayout *layout)
{
    auto *logTitle = new QLabel("Журнал подій", parent);
    logTitle->setObjectName("macroLogTitle");
    layout->addWidget(logTitle);

    m_log = new QListWidget(parent);
    m_log->setObjectName("macroLog");
    m_log->setSelectionMode(QAbstractItemView::NoSelection);
    layout->addWidget(m_log, 1);
}

void MacroPanel::onMacroRecord()
{
    if (m_state != State::Idle) return;

    m_events.clear();
    m_log->clear();
    m_progress->setValue(0);
    m_lblEventCount->setText("0");
    m_lblDuration->setText("0 мс");

    startCountdown(m_spnDelay->value());
}

void MacroPanel::onMacroStopRecord()
{
    if (m_state == State::Countdown) {
        m_countdownTimer->stop();
        m_lblCountdown->hide();
        m_state = State::Idle;
        updateUi();
        return;
    }
    if (m_state == State::Recording)
        m_recorder->stopRecording();
}

void MacroPanel::startCountdown(int secs)
{
    if (secs <= 0) {
        m_recorder->startRecording();
        m_state = State::Recording;
        updateUi();
        addLog("Запис розпочато", "#7C3AED");
        return;
    }
    m_countdownSec = secs;
    m_state = State::Countdown;
    m_lblCountdown->setText(QString::number(m_countdownSec));
    m_lblCountdown->show();
    updateUi();
    m_countdownTimer->start();
}

void MacroPanel::onRecordingCountdown()
{
    if (--m_countdownSec <= 0) {
        m_countdownTimer->stop();
        m_lblCountdown->hide();
        m_recorder->startRecording();
        m_state = State::Recording;
        updateUi();
        addLog("Запис розпочато", "#7C3AED");
    } else {
        m_lblCountdown->setText(QString::number(m_countdownSec));
    }
}

void MacroPanel::onEventRecorded(const InputEvent &ev)
{
    m_durMs = ev.timestamp;
    m_lblDuration->setText(QString("%1 мс").arg(m_durMs));

    if (ev.type == InputEvent::MouseMove) return;

    QString icon, detail, color = "#64748B";
    switch (ev.type) {
    case InputEvent::MousePress:
        detail = QString("BTN%1 ↓ (%2,%3)").arg(ev.button).arg(ev.x).arg(ev.y);
        color  = "#F59E0B";
        break;
    case InputEvent::MouseRelease:
        detail = QString("BTN%1 ↑").arg(ev.button);
        break;
    case InputEvent::KeyPress:
        detail = QString("[%1] ↓").arg(ev.keyName);
        color  = "#34D399";
        break;
    case InputEvent::KeyRelease:
        detail = QString("[%1] ↑").arg(ev.keyName);
        break;
    default:
        break;
    }
    addLog(QString("T+%1  %2").arg(ev.timestamp, 6).arg(detail), color);
}

void MacroPanel::onRecordingStopped(int count, qint64 durationMs)
{
    m_events = m_recorder->takeEvents();
    m_durMs  = durationMs;

    m_lblEventCount->setText(QString::number(m_events.size()));
    m_lblDuration  ->setText(QString("%1 мс").arg(durationMs));

    m_state = State::Idle;
    updateUi();
    addLog(QString("Записано %1 подій (%2 мс)").arg(count).arg(durationMs), "#A78BFA");
}

void MacroPanel::onRecorderError(const QString &msg)
{
    m_state = State::Idle;
    updateUi();
    QMessageBox::critical(this, "Помилка запису", msg);
}

void MacroPanel::onMacroPlay()
{
    if (m_events.isEmpty()) {
        QMessageBox::information(this, "Немає даних",
            "Запишіть дії або відкрийте скрипт.");
        return;
    }
    m_player->setLoop(m_chkLoop->isChecked());
    m_player->load(m_events);
    m_progress->setRange(0, m_events.size());
    m_progress->setValue(0);
    m_state = State::Playing;
    updateUi();
    m_log->clear();
    addLog("Відтворення...", "#059669");
    m_player->play();
}

void MacroPanel::onMacroStopPlay()
{
    m_player->stop();
    m_state = State::Idle;
    updateUi();
    addLog("Зупинено", "#EF4444");
}

void MacroPanel::onMacroPauseResume()
{
    if (m_state == State::Playing) {
        m_player->pause();
        m_state = State::Paused;
        m_btnPause->setText("Продовжити");
        addLog("⏸ Пауза", "#D97706");
    } else if (m_state == State::Paused) {
        m_player->resume();
        m_state = State::Playing;
        m_btnPause->setText("⏸  Пауза");
        addLog("Продовження", "#059669");
    }
    updateUi();
}

void MacroPanel::onEventPlayed(int index, const InputEvent &ev)
{
    m_progress->setValue(index + 1);

    if (ev.type == InputEvent::MouseMove) return;

    QString icon, detail;
    switch (ev.type) {
    case InputEvent::MousePress:
        detail = QString("BTN%1 (%2,%3)").arg(ev.button).arg(ev.x).arg(ev.y);
        break;
    case InputEvent::MouseRelease:
        detail = QString("BTN%1↑").arg(ev.button);
        break;
    case InputEvent::KeyPress:
        detail = QString("[%1]↓").arg(ev.keyName);
        break;
    case InputEvent::KeyRelease:
        detail = QString("[%1]↑").arg(ev.keyName);
        break;
    default:
        break;
    }
    addLog(QString("  %1 %2").arg(detail));
}

void MacroPanel::onLoopCompleted(int n)
{
    m_progress->setValue(0);
    addLog(QString("Цикл %1").arg(n), "#7C3AED");

    const int max = m_spnLoops->value();
    if (max > 0 && n >= max) {
        m_player->stop();
        m_state = State::Idle;
        updateUi();
        addLog(QString("%1 циклів завершено").arg(n), "#059669");
    }
}

void MacroPanel::onPlayFinished()
{
    m_state = State::Idle;
    updateUi();
    addLog("Відтворення завершено", "#059669");
}

void MacroPanel::onPlayerError(const QString &msg)
{
    m_state = State::Idle;
    updateUi();
    QMessageBox::critical(this, "Помилка відтворення", msg);
}

void MacroPanel::onMacroSave()
{
    if (m_events.isEmpty()) {
        QMessageBox::information(this, "Немає даних", "Немає записаних подій.");
        return;
    }
    const QString path = QFileDialog::getSaveFileName(
        this, "Зберегти скрипт", "macro.mrs",
        "Macro Script (*.mrs);;Text (*.txt);;All (*)");
    if (path.isEmpty()) return;

    if (ScriptIO::save(path, m_events, m_durMs))
        addLog(path, "#06B6D4");
    else
        QMessageBox::critical(this, "Помилка", "Не вдалось зберегти:\n" + path);
}

void MacroPanel::onMacroOpen()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Відкрити скрипт", {},
        "Macro Script (*.mrs);;Text (*.txt);;All (*)");
    if (path.isEmpty()) return;

    QVector<InputEvent> loaded;
    qint64 dur = 0;
    QString err;

    if (!ScriptIO::load(path, loaded, dur, err)) {
        QMessageBox::critical(this, "Помилка", err);
        return;
    }

    m_events = loaded;
    m_durMs  = dur;
    m_log->clear();
    m_progress->setValue(0);
    m_lblEventCount->setText(QString::number(m_events.size()));
    m_lblDuration  ->setText(QString("%1 мс").arg(m_durMs));
    addLog(QString("%1 (%2 подій)").arg(path).arg(m_events.size()), "#06B6D4");
    updateUi();
}

void MacroPanel::onMacroClear()
{
    if (m_state != State::Idle) return;
    if (QMessageBox::question(this, "Очистити",
            "Видалити всі записані події?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No) != QMessageBox::Yes)
        return;

    m_events.clear();
    m_durMs = 0;
    m_log->clear();
    m_progress->setValue(0);
    m_lblEventCount->setText("0");
    m_lblDuration  ->setText("0 мс");
    updateUi();
}

void MacroPanel::addLog(const QString &text, const QString &color)
{
    auto *item = new QListWidgetItem(text, m_log);
    item->setForeground(QColor(color));
    m_log->addItem(item);

    while (m_log->count() > 1000)
        delete m_log->takeItem(0);

    m_log->scrollToBottom();
}

void MacroPanel::updateUi()
{
    const bool idle      = (m_state == State::Idle);
    const bool recording = (m_state == State::Recording);
    const bool countdown = (m_state == State::Countdown);
    const bool playing   = (m_state == State::Playing);
    const bool paused    = (m_state == State::Paused);
    const bool hasEvents = !m_events.isEmpty();

    m_btnRecord    ->setEnabled(idle);
    m_btnStopRecord->setEnabled(recording || countdown);
    m_btnPlay      ->setEnabled(idle && hasEvents);
    m_btnPause     ->setEnabled(playing || paused);
    m_btnStopPlay  ->setEnabled(playing || paused);
    m_btnSave      ->setEnabled(idle && hasEvents);
    m_btnOpen      ->setEnabled(idle);
    m_btnClear     ->setEnabled(idle && hasEvents);
    m_spnDelay     ->setEnabled(idle);
    m_chkLoop      ->setEnabled(idle);
    m_spnLoops     ->setEnabled(idle && m_chkLoop->isChecked());

    if (!playing && !paused)
        m_btnPause->setText("Пауза");

    const auto &info = stateInfo().value(m_state);
    emit statusChanged(info.text, info.stateKey);
}
