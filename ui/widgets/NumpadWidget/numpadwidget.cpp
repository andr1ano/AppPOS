#include <NumpadWidget/numpadwidget.h>

NumPadWidget::NumPadWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("numPad");

    auto *root = new QVBoxLayout(this);
    root->setSpacing(6);
    root->setContentsMargins(10, 10, 10, 10);

    m_display = new QLabel("0.00", this);
    m_display->setObjectName("numDisplay");
    m_display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_display->setMinimumHeight(54);
    root->addWidget(m_display);

    auto *grid = new QGridLayout();
    grid->setSpacing(6);

    grid->addWidget(makeButton("7"), 0, 0);
    grid->addWidget(makeButton("8"), 0, 1);
    grid->addWidget(makeButton("9"), 0, 2);

    grid->addWidget(makeButton("4"), 1, 0);
    grid->addWidget(makeButton("5"), 1, 1);
    grid->addWidget(makeButton("6"), 1, 2);

    grid->addWidget(makeButton("1"), 2, 0);
    grid->addWidget(makeButton("2"), 2, 1);
    grid->addWidget(makeButton("3"), 2, 2);

    auto *dotBtn = makeButton(".",  "special");
    auto *bsBtn  = makeButton("<-", "special");
    connect(dotBtn, &QPushButton::clicked, this, &NumPadWidget::onDot);
    connect(bsBtn,  &QPushButton::clicked, this, &NumPadWidget::onBackspace);

    grid->addWidget(dotBtn,          3, 0);
    grid->addWidget(makeButton("0"), 3, 1);
    grid->addWidget(bsBtn,           3, 2);

    root->addLayout(grid);

    auto *botRow = new QHBoxLayout();
    botRow->setSpacing(6);

    auto *clrBtn  = makeButton("Очистити",      "action-clear");
    auto *confBtn = makeButton("Підтвердити",   "action-confirm");
    clrBtn ->setFixedHeight(46);
    confBtn->setFixedHeight(46);

    connect(clrBtn,  &QPushButton::clicked, this, &NumPadWidget::onClear);
    connect(confBtn, &QPushButton::clicked, this, &NumPadWidget::onConfirm);

    botRow->addWidget(clrBtn);
    botRow->addWidget(confBtn);
    root->addLayout(botRow);
}

void NumPadWidget::setTargetField(QLineEdit *field)
{
    m_targetField = field;
    m_buffer.clear();
    syncDisplay();
}

QPushButton *NumPadWidget::makeButton(const QString &text,
                                      const QString &styleClass)
{
    auto *btn = new QPushButton(text, this);
    btn->setProperty("class", styleClass);
    btn->setFixedHeight(52);
    btn->setFocusPolicy(Qt::NoFocus);

    if (styleClass == "digit")
        connect(btn, &QPushButton::clicked, this, &NumPadWidget::onDigitClicked);

    return btn;
}

void NumPadWidget::syncDisplay()
{
    m_display->setText(m_buffer.isEmpty() ? "0.00" : m_buffer);

    if (m_targetField)
        m_targetField->setText(m_buffer);
}

void NumPadWidget::appendChar(const QString &ch)
{
    if (m_buffer.length() >= kMaxLen) return;

    if (ch == "." && m_buffer.contains('.')) return;

    const int dotPos = m_buffer.indexOf('.');
    if (dotPos != -1 &&
        (m_buffer.length() - dotPos - 1) >= kMaxDecimals) return;

    if (ch != "." && m_buffer == "0")
        m_buffer = ch;
    else
        m_buffer += ch;

    syncDisplay();
}

void NumPadWidget::onDigitClicked()
{
    if (auto *btn = qobject_cast<QPushButton *>(sender()))
        appendChar(btn->text());
}

void NumPadWidget::onDot()
{
    if (m_buffer.isEmpty())
        m_buffer = "0";

    appendChar(".");
}

void NumPadWidget::onBackspace()
{
    if (!m_buffer.isEmpty()) {
        m_buffer.chop(1);
        syncDisplay();
    }
}

void NumPadWidget::onClear()
{
    m_buffer.clear();
    syncDisplay();
}

void NumPadWidget::onConfirm()
{
    emit valueConfirmed(m_buffer);
}
