#include <Recorder/recorder.h>

// C legacy
#undef KeyPress
#undef KeyRelease

static QString keycodeToName(Display *dpy, unsigned int keycode)
{
    KeySym sym = XkbKeycodeToKeysym(dpy, keycode, 0, 0);
    if (sym == NoSymbol) return QString::number(keycode);
    const char *name = XKeysymToString(sym);
    return name ? QString::fromLatin1(name) : QString::number(keycode);
}

static inline int16_t readInt16(const unsigned char *p, int offset)
{
    int16_t v;
    std::memcpy(&v, p + offset, 2);
    return v;
}

Recorder::Recorder(QObject *parent) : QThread(parent) {}

Recorder::~Recorder()
{
    stopRecording();
    wait(3000);
}

void Recorder::startRecording()
{
    if (isRunning()) return;

    m_dpyCtrl = XOpenDisplay(nullptr);
    if (!m_dpyCtrl) {
        emit errorOccurred("Cannot open X control display. Is DISPLAY set?");
        return;
    }

    int evBase, errBase;
    if (!XRecordQueryVersion(m_dpyCtrl, &evBase, &errBase)) {
        emit errorOccurred("XRecord extension not available.");
        XCloseDisplay(m_dpyCtrl); m_dpyCtrl = nullptr;
        return;
    }

    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange *range = XRecordAllocRange();
    if (!range) {
        emit errorOccurred("XRecordAllocRange failed.");
        XCloseDisplay(m_dpyCtrl); m_dpyCtrl = nullptr;
        return;
    }
    std::memset(range, 0, sizeof(XRecordRange));
    range->device_events.first = 2;
    range->device_events.last  = 6;

    m_ctx = XRecordCreateContext(m_dpyCtrl, 0, &clients, 1, &range, 1);
    XFree(range);

    if (!m_ctx) {
        emit errorOccurred("XRecordCreateContext failed.");
        XCloseDisplay(m_dpyCtrl); m_dpyCtrl = nullptr;
        return;
    }
    XFlush(m_dpyCtrl);

    {
        QMutexLocker lk(&m_mutex);
        m_events.clear();
    }
    m_lastMoveTs = -1000;
    m_lastMX = m_lastMY = -1;

    m_timer.start();
    start();
}

void Recorder::stopRecording()
{
    if (m_dpyCtrl && m_ctx) {
        XRecordDisableContext(m_dpyCtrl, m_ctx);
        XFlush(m_dpyCtrl);
    }
}

QVector<InputEvent> Recorder::takeEvents()
{
    QMutexLocker lk(&m_mutex);
    return std::move(m_events);
}

qint64 Recorder::elapsedMs() const
{
    return m_timer.isValid() ? m_timer.elapsed() : 0;
}

void Recorder::xRecordCallback(char *priv, void *rawData)
{
    reinterpret_cast<Recorder *>(priv)->handleXEvent(rawData);
}

void Recorder::handleXEvent(void *rawData)
{
    auto *data = reinterpret_cast<XRecordInterceptData *>(rawData);
    if (!data) return;

    if (data->category != XRecordFromServer) {
        XRecordFreeData(data);
        return;
    }

    const unsigned char *d    = reinterpret_cast<const unsigned char *>(data->data);
    unsigned char        type = d[0] & 0x7F;

    qint64 now = m_timer.elapsed();
    InputEvent ev;
    ev.timestamp = now;

    switch (type) {

    case 2:
    case 3: {
        unsigned char kc = d[1];
        ev.type    = (type == 2) ? InputEvent::KeyPress : InputEvent::KeyRelease;
        ev.keycode = kc;
        ev.keyName = keycodeToName(m_dpyData, kc);
        break;
    }

    case 4:
    case 5: {
        unsigned char btn = d[1];
        if (btn == 4 || btn == 5) { XRecordFreeData(data); return; }
        ev.type   = (type == 4) ? InputEvent::MousePress : InputEvent::MouseRelease;
        ev.button = btn;
        ev.x      = readInt16(d, 20);
        ev.y      = readInt16(d, 22);
        break;
    }

    case 6: {
        int mx = readInt16(d, 20);
        int my = readInt16(d, 22);
        if (now - m_lastMoveTs < kMoveThrottleMs &&
            qAbs(mx - m_lastMX) < 3 && qAbs(my - m_lastMY) < 3) {
            XRecordFreeData(data); return;
        }
        m_lastMoveTs = now; m_lastMX = mx; m_lastMY = my;
        ev.type = InputEvent::MouseMove;
        ev.x = mx; ev.y = my;
        break;
    }

    default:
        XRecordFreeData(data);
        return;
    }

    {
        QMutexLocker lk(&m_mutex);
        m_events.append(ev);
    }
    emit eventRecorded(ev);
    XRecordFreeData(data);
}

void Recorder::run()
{
    m_dpyData = XOpenDisplay(nullptr);
    if (!m_dpyData) {
        emit errorOccurred("Cannot open X data display in recording thread.");
        if (m_dpyCtrl) {
            if (m_ctx) { XRecordFreeContext(m_dpyCtrl, m_ctx); m_ctx = 0; }
            XCloseDisplay(m_dpyCtrl); m_dpyCtrl = nullptr;
        }
        return;
    }

    XRecordEnableContext(m_dpyData, m_ctx,
        reinterpret_cast<XRecordInterceptProc>(xRecordCallback),
        reinterpret_cast<XPointer>(this));

    qint64 duration = m_timer.elapsed();
    int count;
    {
        QMutexLocker lk(&m_mutex);
        count = m_events.size();
    }

    XCloseDisplay(m_dpyData);
    m_dpyData = nullptr;

    XRecordFreeContext(m_dpyCtrl, m_ctx);
    m_ctx = 0;
    XCloseDisplay(m_dpyCtrl);
    m_dpyCtrl = nullptr;

    emit recordingStopped(count, duration);
}
