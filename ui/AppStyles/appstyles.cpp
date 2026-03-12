#include <AppStyles/appstyles.h>

QString AppStyles::global()
{
    QFile f(":/styles.qss");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "AppStyles: could not open :/styles.qss — falling back to no style";
        return {};
    }
    return QString::fromUtf8(f.readAll());
}
