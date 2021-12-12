#ifndef CONCERT_H
#define CONCERT_H

#include <QWidget>
#include <QComboBox>
#include <QDate>
#include <QTime>

#include "state.h"

class Concert
{
public:
    Concert();

    QList<QList<int>> placeStates;

    int hall;

    bool isActive;

    int scheme;

    QString name;
    QDate date;
    QTime time;
    int income = 0;
    int passedIncome = 0;

    int placesCount = 0;
    int invitesCount = 0;

    int cashlessCount = 0;
    int cashlessPrice = 0;

    int sellCount = 0;
    int sellPrice = 0;

    int revenue = 0;

    QDate passDate;

    QString totalTickets();
    QString totalPrice();
    QString trashCount();

    QString td(const QString& text);
    QString tableRowOf(const QString& c1, const QString& c2, const QString& c3);
    QString htmlReport();
    QString reportOfSalesHtml();

    QString reportName();
    QString reportOfSalesName();

    QString build();

    bool operator == (const Concert& other) const {
        return name == other.name;
    }
};

#endif // CONCERT_H



















































//
