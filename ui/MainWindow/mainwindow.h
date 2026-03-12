#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QScreen>
#include <QDateTime>
#include <QTimer>
#include <QGuiApplication>
#include <QScreen>

#include <Cart/Cart.h>
#include <POSPanel/pospanel.h>
#include <MacroPanel/macropanel.h>
#include <AppStyles/appstyles.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private:
    void buildHeader(QWidget *parent, class QHBoxLayout *layout);
    void buildSplitter(QWidget *central);

    Cart *m_cart = nullptr;

    PosPanel   *m_posPanel   = nullptr;
    MacroPanel *m_macroPanel = nullptr;

    QLabel *m_macroStateBadge = nullptr;
};

#endif