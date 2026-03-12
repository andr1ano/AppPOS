#ifndef POSPANEL_H
#define POSPANEL_H

#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDoubleValidator>

#include <cmath>

#include <Cart/Cart.h>
#include <NumpadWidget/numpadwidget.h>
#include <ReceiptDialog/receiptdialog.h>

class PosPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PosPanel(Cart *cart, QWidget *parent = nullptr);

private slots:
    void onLoadClicked();
    void onClientAmountChanged(const QString &text);
    void onPayClicked();
    void onCancelClicked();
    void onNumPadConfirmed(const QString &value);

private:
    void buildLayout();
    QWidget *buildTablePanel(QWidget *parent);
    QWidget *buildPayPanel  (QWidget *parent);
    QFrame  *buildSummaryBox(QWidget *parent);

    void populateTable();
    void applyPaymentInfo(const Cart::PaymentInfo &info);
    void showReceipt(double clientAmt, double change);
    void setPaymentSectionEnabled(bool enabled);
    void resetForm();

    Cart *m_cart = nullptr;

    QTableWidget *m_table       = nullptr;

    QWidget      *m_payWidget   = nullptr;

    QLabel *m_lblTotal  = nullptr;
    QLabel *m_lblCash   = nullptr;
    QLabel *m_lblChange = nullptr;
    QLabel       *m_lblEmpty    = nullptr;

    QLineEdit    *m_edClientAmt = nullptr;
    QLabel       *m_lblWarning  = nullptr;
    NumPadWidget *m_numPad      = nullptr;
    QPushButton  *m_btnPay      = nullptr;
    QPushButton  *m_btnCancel   = nullptr;
    QPushButton  *m_btnLoad     = nullptr;
};

#endif