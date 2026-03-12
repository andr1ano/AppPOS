#ifndef RECEIPTDIALOG_H
#define RECEIPTDIALOG_H

#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QDateTime>
#include <QFont>

#include <Cart/Cart.h>
#include <ReceiptDialog/receiptdialog.h>

class ReceiptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReceiptDialog(const Cart  *cart,
                           double       clientAmt,
                           double       change,
                           QWidget     *parent = nullptr);

private:
    static QString buildReceiptText(const Cart *cart,
                                    double      clientAmt,
                                    double      change);
};

#endif