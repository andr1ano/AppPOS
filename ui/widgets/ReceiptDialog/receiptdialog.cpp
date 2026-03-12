#include <ReceiptDialog/receiptdialog.h>

ReceiptDialog::ReceiptDialog(const Cart *cart,
                             double      clientAmt,
                             double      change,
                             QWidget    *parent)
    : QDialog(parent)
{
    setWindowTitle("Чек оплачено");
    setMinimumWidth(380);
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(16, 16, 16, 16);

    auto *banner = new QLabel("Оплата проведена успішно!", this);
    banner->setObjectName("receiptBanner");
    layout->addWidget(banner);

    auto *te = new QTextEdit(this);
    te->setObjectName("receiptBody");
    te->setReadOnly(true);
    te->setText(buildReceiptText(cart, clientAmt, change));
    layout->addWidget(te);

    auto *closeBtn = new QPushButton("Закрити та відкрити новий чек", this);
    closeBtn->setObjectName("btnReceiptClose");
    closeBtn->setMinimumHeight(44);
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeBtn);
}

QString ReceiptDialog::buildReceiptText(const Cart *cart,
                                        double      clientAmt,
                                        double      change)
{
    constexpr char kRule[] = "════════════════════════════════\n";

    QString text;

    text += kRule;
    text += "        MART POS  ...  Каса №1\n";
    text += QDateTime::currentDateTime().toString("  dd.MM.yyyy  hh:mm:ss\n");
    text += kRule;
    text += "\n";

    text += cart->receiptBody();

    text += kRule;
    text += QString("  ГОТІВКА:  %1 ₴\n").arg(clientAmt, 8, 'f', 2);
    text += QString("  РЕШТА:    %1 ₴\n").arg(change,    8, 'f', 2);
    text += kRule;
    text += "\n";
    text += "        Дякуємо за покупку!\n";

    return text;
}
