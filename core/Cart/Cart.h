#ifndef CART_H
#define CART_H

#pragma once

#include <QVector>
#include <QString>
#include <QTextStream>
#include <QDebug>
#include <QFile>

#include <cmath>

#include <Product/Product.h>

class Cart
{
public:
    void setProducts(const QVector<Product> &products);

    void addProduct(const Product &product);

    void clear();

    const QVector<Product> &products() const { return m_products; }
    int                     count()    const { return m_products.size(); }

    double total() const { return m_total; }

    enum class ValidationResult
    {
        Ok,
        Empty,
        BadFormat,
        Negative,
        Insufficient,
    };

    struct PaymentInfo
    {
        ValidationResult result  = ValidationResult::Empty;
        double           cash    = 0.0;
        double           change  = 0.0;
        double           missing = 0.0;

        bool isPayable() const { return result == ValidationResult::Ok; }

        QString warningText() const;
    };

    PaymentInfo validate(const QString &clientAmountText) const;

    QString receiptBody() const;

    static bool load(const QString        &path,
                     QVector<Product>     &products,
                     QString              &error);

private:
    void recomputeTotal();

    QVector<Product> m_products;
    double           m_total = 0.0;
};

#endif
