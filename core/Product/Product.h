#ifndef PRODUCT_H
#define PRODUCT_H

#pragma once
#include <QString>

struct Product {
    QString name;
    double  quantity = 0.0;
    double  price    = 0.0;
    double lineTotal() const {
      return quantity * price;
    }
};

#endif
