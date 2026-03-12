#ifndef APPSTYLES_H
#define APPSTYLES_H

#pragma once

#include <QString>
#include <QFile>
#include <QDebug>

//  Single source for the application stylesheet.
struct AppStyles
{
    static QString global();
};

#endif