#include <POSPanel/pospanel.h>

PosPanel::PosPanel(Cart *cart, QWidget *parent)
    : QWidget(parent)
    , m_cart(cart)
{
    buildLayout();

    m_lblTotal ->setText("0.00 ₴");
    m_lblCash  ->setText("— ₴");
    m_lblChange->setText("— ₴");
    setPaymentSectionEnabled(false);
}

void PosPanel::buildLayout()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(14, 14, 8, 14);
    root->setSpacing(14);

    root->addWidget(buildTablePanel(this), 3);
    root->addWidget(buildPayPanel(this),   2);
}


QWidget *PosPanel::buildTablePanel(QWidget *parent)
{
    auto *panel  = new QWidget(parent);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    auto *titleRow = new QHBoxLayout();

    auto *title = new QLabel("Позиції чека", panel);
    title->setObjectName("sectionTitle");
    titleRow->addWidget(title, 1);

    m_btnLoad = new QPushButton("Відкрити чек", panel);
    m_btnLoad->setObjectName("btnLoadCart");
    m_btnLoad->setCursor(Qt::PointingHandCursor);
    m_btnLoad->setFixedHeight(30);
    titleRow->addWidget(m_btnLoad);

    layout->addLayout(titleRow);

    m_lblEmpty = new QLabel("Натисніть «Відкрити чек»\nщоб завантажити список товарів", panel);
    m_lblEmpty->setObjectName("emptyCartLabel");
    m_lblEmpty->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_lblEmpty, 1);

    m_table = new QTableWidget(panel);
    m_table->setObjectName("productsTable");
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({ "Найменування", "К-сть", "Ціна (₴)", "Сума (₴)" });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->setShowGrid(false);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->hide();
    layout->addWidget(m_table, 1);

    connect(m_btnLoad, &QPushButton::clicked, this, &PosPanel::onLoadClicked);

    return panel;
}

QWidget *PosPanel::buildPayPanel(QWidget *parent)
{
    auto *panel  = new QWidget(parent);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    auto *title = new QLabel("Оплата готівкою", panel);
    title->setObjectName("sectionTitle");
    layout->addWidget(title);

    layout->addWidget(buildSummaryBox(panel));

    auto *sep = new QFrame(panel);
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("separator");
    layout->addWidget(sep);

    m_payWidget = new QWidget(panel);
    auto *payL  = new QVBoxLayout(m_payWidget);
    payL->setContentsMargins(0, 0, 0, 0);
    payL->setSpacing(10);

    auto *clientLbl = new QLabel("Сума клієнта (₴):", m_payWidget);
    clientLbl->setObjectName("fieldLabel");
    payL->addWidget(clientLbl);

    m_edClientAmt = new QLineEdit(m_payWidget);
    m_edClientAmt->setObjectName("clientInput");
    m_edClientAmt->setPlaceholderText("Введіть суму...");
    m_edClientAmt->setAlignment(Qt::AlignRight);
    m_edClientAmt->setMinimumHeight(44);

    auto *validator = new QDoubleValidator(0.0, 999999.99, 2, m_edClientAmt);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setLocale(QLocale::c());
    m_edClientAmt->setValidator(validator);

    payL->addWidget(m_edClientAmt);

    m_lblWarning = new QLabel(m_payWidget);
    m_lblWarning->setObjectName("warningLabel");
    m_lblWarning->setWordWrap(true);
    m_lblWarning->hide();
    payL->addWidget(m_lblWarning);

    m_numPad = new NumPadWidget(m_payWidget);
    m_numPad->setTargetField(m_edClientAmt);
    payL->addWidget(m_numPad);

    payL->addStretch();

    auto *btnRow = new QHBoxLayout();

    m_btnCancel = new QPushButton("Відміна", m_payWidget);
    m_btnCancel->setObjectName("btnCancel");
    m_btnCancel->setMinimumHeight(48);
    m_btnCancel->setCursor(Qt::PointingHandCursor);

    m_btnPay = new QPushButton("Оплатити", m_payWidget);
    m_btnPay->setObjectName("btnPay");
    m_btnPay->setMinimumHeight(48);
    m_btnPay->setEnabled(false);
    m_btnPay->setCursor(Qt::PointingHandCursor);

    btnRow->addWidget(m_btnCancel, 1);
    btnRow->addWidget(m_btnPay,    2);
    payL->addLayout(btnRow);

    layout->addWidget(m_payWidget);

    connect(m_edClientAmt, &QLineEdit::textChanged,
            this, &PosPanel::onClientAmountChanged);
    connect(m_btnPay,    &QPushButton::clicked, this, &PosPanel::onPayClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &PosPanel::onCancelClicked);
    connect(m_numPad,    &NumPadWidget::valueConfirmed,
            this, &PosPanel::onNumPadConfirmed);

    return panel;
}

QFrame *PosPanel::buildSummaryBox(QWidget *parent)
{
    auto *box  = new QFrame(parent);
    box->setObjectName("summaryBox");
    auto *grid = new QGridLayout(box);
    grid->setContentsMargins(14, 12, 14, 12);
    grid->setVerticalSpacing(8);

    auto addRow = [&](const QString &keyText, QLabel *&valRef,
                      const QString &valId, int row)
    {
        auto *key = new QLabel(keyText, box);
        key->setObjectName("infoKey");
        valRef = new QLabel("0.00 ₴", box);
        valRef->setObjectName(valId);
        valRef->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        grid->addWidget(key,   row, 0);
        grid->addWidget(valRef, row, 1);
    };

    addRow("Сума чека:", m_lblTotal,  "totalLabel",  0);
    addRow("Готівкою:",  m_lblCash,   "cashLabel",   1);
    addRow("Решта:",     m_lblChange, "changeLabel", 2);

    return box;
}

void PosPanel::onLoadClicked()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        "Відкрити чек",
        {},
        "Файли чека (*.csv);;Всі файли (*)");

    if (path.isEmpty())
        return;

    QVector<Product> products;
    QString error;

    if (!Cart::load(path, products, error)) {
        QMessageBox::critical(this, "Помилка завантаження", error);
        return;
    }

    m_cart->setProducts(products);
    populateTable();
    resetForm();

    m_lblEmpty->hide();
    m_table->show();

    setPaymentSectionEnabled(true);

    m_lblTotal->setText(QString::number(m_cart->total(), 'f', 2) + " ₴");
}

void PosPanel::onClientAmountChanged(const QString &text)
{
    applyPaymentInfo(m_cart->validate(text));
}

void PosPanel::onPayClicked()
{
    if (m_cart->count() == 0) {
        QMessageBox::warning(this, "Чек порожній",
            "Завантажте список товарів перед оплатою.");
        return;
    }

    const Cart::PaymentInfo info = m_cart->validate(m_edClientAmt->text());
    if (!info.isPayable()) {
        QMessageBox::critical(this, "Оплата неможлива",
            info.warningText().isEmpty()
                ? "Вкажіть коректну суму клієнта."
                : info.warningText());
        return;
    }

    const int ret = QMessageBox::question(
        this, "Підтвердження",
        QString("Сума:    <b>%1 ₴</b><br>"
                "Готівка: <b>%2 ₴</b><br>"
                "Решта:   <b>%3 ₴</b><br><br>"
                "Провести оплату?")
            .arg(QString::number(m_cart->total(), 'f', 2))
            .arg(QString::number(info.cash,        'f', 2))
            .arg(QString::number(info.change,       'f', 2)),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);

    if (ret == QMessageBox::Yes)
        showReceipt(info.cash, info.change);
}

void PosPanel::onCancelClicked()
{
    if (QMessageBox::question(this, "Скасування", "Скасувати оплату?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No) == QMessageBox::Yes)
        resetForm();
}

void PosPanel::onNumPadConfirmed(const QString &value)
{
    if (!value.isEmpty())
        m_edClientAmt->setText(value);
}

void PosPanel::populateTable()
{
    const auto &products = m_cart->products();
    m_table->setRowCount(products.size());

    for (int i = 0; i < products.size(); ++i) {
        const Product &p       = products[i];
        const bool     isWhole = (p.quantity == std::floor(p.quantity));

        auto makeItem = [](const QString &text,
                           Qt::Alignment  align = Qt::AlignLeft | Qt::AlignVCenter)
        {
            auto *item = new QTableWidgetItem(text);
            item->setTextAlignment(align);
            return item;
        };

        m_table->setItem(i, 0, makeItem(p.name));
        m_table->setItem(i, 1, makeItem(
            QString::number(p.quantity, 'f', isWhole ? 0 : 3),
            Qt::AlignCenter));
        m_table->setItem(i, 2, makeItem(
            QString::number(p.price, 'f', 2),
            Qt::AlignRight | Qt::AlignVCenter));
        m_table->setItem(i, 3, makeItem(
            QString::number(p.lineTotal(), 'f', 2),
            Qt::AlignRight | Qt::AlignVCenter));

        QFont f = m_table->item(i, 3)->font();
        f.setBold(true);
        m_table->item(i, 3)->setFont(f);
    }

    m_table->resizeRowsToContents();
}

void PosPanel::applyPaymentInfo(const Cart::PaymentInfo &info)
{
    using VR = Cart::ValidationResult;

    switch (info.result) {
    case VR::Empty:
        m_lblCash  ->setText("— ₴");
        m_lblChange->setText("— ₴");
        m_lblWarning->hide();
        m_btnPay->setEnabled(false);
        break;
    case VR::BadFormat:
    case VR::Negative:
        m_lblCash  ->setText("— ₴");
        m_lblChange->setText("— ₴");
        m_lblWarning->setText(info.warningText());
        m_lblWarning->show();
        m_btnPay->setEnabled(false);
        break;
    case VR::Insufficient:
        m_lblCash  ->setText(QString::number(info.cash, 'f', 2) + " ₴");
        m_lblChange->setText("— ₴");
        m_lblWarning->setText(info.warningText());
        m_lblWarning->show();
        m_btnPay->setEnabled(false);
        break;
    case VR::Ok:
        m_lblCash  ->setText(QString::number(info.cash,   'f', 2) + " ₴");
        m_lblChange->setText(QString::number(info.change, 'f', 2) + " ₴");
        m_lblWarning->hide();
        m_btnPay->setEnabled(true);
        break;
    }
}

void PosPanel::showReceipt(double clientAmt, double change)
{
    ReceiptDialog dlg(m_cart, clientAmt, change, this);
    dlg.exec();
    resetForm();
}

void PosPanel::setPaymentSectionEnabled(bool enabled)
{
    m_payWidget->setEnabled(enabled);

    m_payWidget->setProperty("dimmed", !enabled);
    m_payWidget->style()->unpolish(m_payWidget);
    m_payWidget->style()->polish(m_payWidget);
}

void PosPanel::resetForm()
{
    m_edClientAmt->clear();
    m_numPad->setTargetField(m_edClientAmt);
    m_lblWarning->hide();
    m_lblCash  ->setText("— ₴");
    m_lblChange->setText("— ₴");
    m_btnPay->setEnabled(false);
}
