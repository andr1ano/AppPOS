#ifndef NUMPADWIDGET_H
#define NUMPADWIDGET_H

#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>


class NumPadWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NumPadWidget(QWidget *parent = nullptr);

    void setTargetField(QLineEdit *field);

    signals:
        void valueConfirmed(const QString &value);

    private slots:
        void onDigitClicked();
    void onDot();
    void onBackspace();
    void onClear();
    void onConfirm();

private:
    QPushButton *makeButton(const QString &text,
                            const QString &styleClass = "digit");
    void appendChar(const QString &ch);
    void syncDisplay();

    QLineEdit *m_targetField = nullptr;
    QLabel    *m_display     = nullptr;
    QString    m_buffer;

    static constexpr int kMaxLen      = 10;
    static constexpr int kMaxDecimals = 2;
};

#endif