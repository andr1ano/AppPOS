#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <MainWindow/mainwindow.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    app.setApplicationName("MART POS");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MART");

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "payment_form_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();

    return app.exec();
}
