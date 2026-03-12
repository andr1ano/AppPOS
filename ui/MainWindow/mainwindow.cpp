#include <MainWindow/mainwindow.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Оплата чека — Каса №1");
    setStyleSheet(AppStyles::global());

    m_cart = new Cart;

    auto *central    = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    setCentralWidget(central);

    auto *header = new QWidget(central);
    header->setObjectName("header");
    header->setFixedHeight(60);
    auto *hdrLayout = new QHBoxLayout(header);
    hdrLayout->setContentsMargins(20, 0, 20, 0);
    buildHeader(header, hdrLayout);
    rootLayout->addWidget(header);

    buildSplitter(central);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    showMaximized();

    connect(m_macroPanel, &MacroPanel::statusChanged,
            this, [this](const QString &text, const QString &stateKey) {
                m_macroStateBadge->setText(text);
                m_macroStateBadge->setProperty("macroState", stateKey);
                // Force QSS re-evaluation after property change
                m_macroStateBadge->style()->unpolish(m_macroStateBadge);
                m_macroStateBadge->style()->polish(m_macroStateBadge);
            });
}

void MainWindow::buildHeader(QWidget *parent, QHBoxLayout *layout)
{
    auto *logo = new QLabel("MART POS", parent);
    logo->setObjectName("logoLabel");

    auto *dt = new QLabel(
        QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm"), parent);
    dt->setObjectName("dtLabel");
    auto *timer = new QTimer(parent);
    connect(timer, &QTimer::timeout, dt, [dt]() {
        dt->setText(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));
    });
    timer->start(1000);

    auto *cashier = new QLabel("Касир: Іванова О.В.", parent);
    cashier->setObjectName("cashierLabel");

    m_macroStateBadge = new QLabel("МАКРО: Очікування", parent);
    m_macroStateBadge->setObjectName("macroStateBadge");

    auto *closeBtn = new QPushButton("✕", parent);
    closeBtn->setObjectName("closeButton");
    closeBtn->setFixedSize(36, 36);
    connect(closeBtn, &QPushButton::clicked, this, &MainWindow::close);

    layout->addWidget(logo);
    layout->addStretch();
    layout->addWidget(m_macroStateBadge);
    layout->addSpacing(20);
    layout->addWidget(dt);
    layout->addSpacing(16);
    layout->addWidget(cashier);
    layout->addSpacing(16);
    layout->addWidget(closeBtn);
}

void MainWindow::buildSplitter(QWidget *central)
{
    auto *splitter = new QSplitter(Qt::Horizontal, central);
    splitter->setObjectName("mainSplitter");
    splitter->setHandleWidth(4);

    m_posPanel = new PosPanel(m_cart, splitter);
    m_posPanel->setObjectName("posWidget");
    splitter->addWidget(m_posPanel);

    m_macroPanel = new MacroPanel(splitter);
    m_macroPanel->setObjectName("macroWidget");
    splitter->addWidget(m_macroPanel);

    splitter->setSizes({ 820, 380 });
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);

    auto *rootLayout = qobject_cast<QVBoxLayout *>(central->layout());
    rootLayout->addWidget(splitter, 1);
}
