#ifndef REPORT_H
#define REPORT_H

#include <QString>
#include <QDate>
#include <QTime>

class Report
{
public:
    Report();


    QString concertName;

    QString concertDate;
    void setConcertDate(const QDate& date);

    QString concertTime;
    void setConcertTime(const QTime& time);

    QString concertHall;
    void setConcertHall(const int& hall);

    int placesCount;
    QString getPlacesCount();

    int invitesCount;
    QString getInvitesCount();

    int cashlessCount;
    QString getCashlessCount();

    int cashlessPrice;
    QString getCashlessPrice();

    int sellCount;
    QString getSellCount();

    int sellPrice;
    QString getSellPrice();

    QString totalTickets();
    QString totalPrice();
    QString trashCount();

    QString td(const QString& text);
    QString tableRowOf(const QString& c1, const QString& c2, const QString& c3);
    QString toHtml();

};

#endif // REPORT_H



















































//
