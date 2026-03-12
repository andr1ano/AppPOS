QT       += core gui widgets
CONFIG   += c++17
TARGET    = payment_form
TEMPLATE  = app

INCLUDEPATH += $$PWD \
               $$PWD/core \
               $$PWD/core/macro \
               $$PWD/ui \
               $$PWD/ui/panels \
               $$PWD/ui/widgets

SOURCES += \
    main.cpp \
    core/Cart/Cart.cpp \
    core/macro/Player/player.cpp \
    core/macro/Recorder/recorder.cpp \
    core/macro/Scriptio/scriptio.cpp \
    ui/AppStyles/appstyles.cpp \
    ui/MainWindow/mainwindow.cpp \
    ui/panels/MacroPanel/macropanel.cpp \
    ui/panels/POSPanel/pospanel.cpp \
    ui/widgets/NumpadWidget/numpadwidget.cpp \
    ui/widgets/ReceiptDialog/receiptdialog.cpp

HEADERS += \
    core/Product/Product.h \
    core/Cart/Cart.h \
    core/macro/Player/player.h \
    core/macro/Recorder/recorder.h \
    core/macro/Scriptio/scriptio.h \
    ui/AppStyles/appstyles.h \
    ui/MainWindow/mainwindow.h \
    ui/panels/MacroPanel/macropanel.h \
    ui/panels/POSPanel/pospanel.h \
    ui/widgets/NumpadWidget/numpadwidget.h \
    ui/widgets/ReceiptDialog/receiptdialog.h

LIBS += -lX11 -lXtst -lXext

RESOURCES += resources.qrc