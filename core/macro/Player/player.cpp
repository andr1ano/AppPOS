#include <Player/player.h>

// C legacy
#undef KeyPress
#undef KeyRelease

Player::Player(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &Player::onTimer);
}

Player::~Player()
{
    stop();
    closeDisplay();
}

void Player::load(const QVector<InputEvent> &events)
{
    stop();
    m_events = events;
}

void Player::play()
{
    if (m_events.isEmpty()) {
        emit errorOccurred("No events loaded. Record or open a script first.");
        return;
    }
    if (!openDisplay()) return;

    m_index      = 0;
    m_loopCount  = 0;
    m_paused     = false;
    m_playing    = true;
    m_pauseOffset = 0;
    m_elapsed.start();

    emit started(m_events.size());
    scheduleNext();
}

void Player::stop()
{
    m_timer->stop();
    m_playing = false;
    m_paused  = false;
    closeDisplay();
}

void Player::pause()
{
    if (!m_playing || m_paused) return;
    m_timer->stop();
    m_paused = true;
    m_pauseOffset = m_elapsed.elapsed();
}

void Player::resume()
{
    if (!m_playing || !m_paused) return;
    m_paused = false;
    m_elapsed.restart();
    scheduleNext();
}

void Player::onTimer()
{
    if (!m_playing || m_paused) return;
    if (m_index >= m_events.size()) return;

    const InputEvent &ev = m_events[m_index];
    injectEvent(ev);
    emit eventPlayed(m_index, ev);

    ++m_index;

    if (m_index >= m_events.size()) {
        ++m_loopCount;
        emit loopCompleted(m_loopCount);

        if (m_loop) {
            m_index = 0;
            m_pauseOffset = 0;
            m_elapsed.restart();
            scheduleNext();
        } else {
            m_playing = false;
            closeDisplay();
            emit finished();
        }
        return;
    }

    scheduleNext();
}

void Player::scheduleNext()
{
    if (m_index >= m_events.size()) return;

    qint64 nextTs   = m_events[m_index].timestamp;
    qint64 elapsed  = m_elapsed.elapsed() + m_pauseOffset;
    qint64 delay    = nextTs - elapsed;

    m_timer->start(static_cast<int>(qMax<qint64>(0, delay)));
}

bool Player::openDisplay()
{
    if (m_dpy) return true;
    m_dpy = XOpenDisplay(nullptr);
    if (!m_dpy) {
        emit errorOccurred("Cannot open X display for playback.");
        return false;
    }
    int evBase, errBase, major, minor;
    if (!XTestQueryExtension(m_dpy, &evBase, &errBase, &major, &minor)) {
        emit errorOccurred("XTest extension not available. "
                           "Install libxtst and run with X11.");
        XCloseDisplay(m_dpy);
        m_dpy = nullptr;
        return false;
    }
    return true;
}

void Player::closeDisplay()
{
    if (m_dpy) {
        XCloseDisplay(m_dpy);
        m_dpy = nullptr;
    }
}

void Player::injectEvent(const InputEvent &ev)
{
    if (!m_dpy) return;

    switch (ev.type) {

    case InputEvent::MouseMove:
        XTestFakeMotionEvent(m_dpy, -1, ev.x, ev.y, 0);
        break;

    case InputEvent::MousePress:
        XTestFakeMotionEvent(m_dpy, -1, ev.x, ev.y, 0);
        XTestFakeButtonEvent(m_dpy, static_cast<unsigned int>(ev.button), True, 0);
        break;

    case InputEvent::MouseRelease:
        XTestFakeButtonEvent(m_dpy, static_cast<unsigned int>(ev.button), False, 0);
        break;

    case InputEvent::KeyPress:
        XTestFakeKeyEvent(m_dpy, static_cast<unsigned int>(ev.keycode), True, 0);
        break;

    case InputEvent::KeyRelease:
        XTestFakeKeyEvent(m_dpy, static_cast<unsigned int>(ev.keycode), False, 0);
        break;
    }

    XFlush(m_dpy);
}
