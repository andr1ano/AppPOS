#include <Cart/Cart.h>

static QStringList splitCsvLine(const QString &line)
{
    QStringList fields;
    QString     current;
    bool        inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        const QChar ch = line[i];

        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                current += '"';
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            fields << current;
            current.clear();
        } else {
            current += ch;
        }
    }
    fields << current;
    return fields;
}

//  Interface of Cart

void Cart::setProducts(const QVector<Product> &products)
{
    m_products = products;
    recomputeTotal();
}

void Cart::addProduct(const Product &product)
{
    m_products.append(product);
    recomputeTotal();
}

void Cart::clear()
{
    m_products.clear();
    m_total = 0.0;
}

//  Price counter

void Cart::recomputeTotal()
{
    m_total = 0.0;
    for (const Product &p : m_products)
        m_total += p.lineTotal();
}

//  Payment validation

Cart::PaymentInfo Cart::validate(const QString &clientAmountText) const
{
    PaymentInfo info;

    if (clientAmountText.trimmed().isEmpty()) {
        info.result = ValidationResult::Empty;
        return info;
    }

    bool ok = false;
    const double amount = clientAmountText.toDouble(&ok);

    if (!ok) {
        info.result = ValidationResult::BadFormat;
        return info;
    }

    if (amount < 0.0) {
        info.result = ValidationResult::Negative;
        info.cash   = amount;
        return info;
    }

    info.cash = amount;

    constexpr double kEps = 0.001;

    if (amount < m_total - kEps) {
        info.result  = ValidationResult::Insufficient;
        info.missing = m_total - amount;
        return info;
    }

    info.result = ValidationResult::Ok;
    info.change = amount - m_total;
    return info;
}

// Info on payment problem

QString Cart::PaymentInfo::warningText() const
{
    switch (result) {
    case Cart::ValidationResult::Ok:
    case Cart::ValidationResult::Empty:
        return {};

    case Cart::ValidationResult::BadFormat:
        return QStringLiteral("Некоректне значення. Введіть числову суму.");

    case Cart::ValidationResult::Negative:
        return QStringLiteral("Сума не може бути від'ємною.");

    case Cart::ValidationResult::Insufficient:
        return QString("Недостатньо коштів. Не вистачає: %1 ₴")
                   .arg(missing, 0, 'f', 2);
    }
    return {};
}

//  Receipt body

QString Cart::receiptBody() const
{
    QString body;

    for (const Product &p : m_products) {
        const double lt   = p.lineTotal();
        const bool isWhole = (p.quantity == std::floor(p.quantity));

        body += QString("%1\n").arg(p.name, -30);

        body += QString("  %1 * %2 = %3 ₴\n")
                    .arg(p.quantity, 0, 'f', isWhole ? 0 : 3)
                    .arg(p.price,    0, 'f', 2)
                    .arg(lt,         0, 'f', 2);
    }

    body += "\n";
    body += QString("  РАЗОМ:  %1 ₴\n").arg(m_total, 8, 'f', 2);

    return body;
}

//  Load check

bool Cart::load(const QString    &path,
                  QVector<Product> &products,
                  QString          &error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = QString("Не вдалось відкрити файл:\n%1").arg(path);
        return false;
    }

    products.clear();
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);

    int lineNo = 0;
    int skipped = 0;

    while (!in.atEnd()) {
        ++lineNo;
        const QString raw = in.readLine().trimmed();

        if (raw.isEmpty() || raw.startsWith('#'))
            continue;

        const QStringList fields = splitCsvLine(raw);
        if (fields.size() < 3) {
            qWarning() << "Cart: skipping malformed line" << lineNo << ":" << raw;
            ++skipped;
            continue;
        }

        bool qtyOk = false, priceOk = false;
        const double qty   = fields[1].trimmed().toDouble(&qtyOk);
        const double price = fields[2].trimmed().toDouble(&priceOk);

        if (!qtyOk || !priceOk || qty <= 0.0 || price < 0.0) {
            qWarning() << "Cart: invalid quantity/price on line" << lineNo;
            ++skipped;
            continue;
        }

        products.append({ fields[0].trimmed(), qty, price });
    }

    if (products.isEmpty()) {
        error = skipped > 0
            ? QString("Файл не містить жодного валідного рядка товару.\n"
                      "Пропущено рядків: %1").arg(skipped)
            : QStringLiteral("Файл порожній або містить лише коментарі.");
        return false;
    }

    return true;
}
