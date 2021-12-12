#include "report.h"
#include "hall.h"

Report::Report()
{

}

void Report::setConcertDate(const QDate& date)
{
    concertDate = date.toString("dd.MM.yyyy");
}

void Report::setConcertTime(const QTime& time)
{
    concertTime = time.toString("hh:mm");
}

void Report::setConcertHall(const int& hall)
{
    if (hall == Hall::LARGE) {
        concertHall = "Большой зал";
    } else if (hall == Hall::SMALL) {
        concertHall = "Малый зал";
    }
}

QString Report::getPlacesCount()
{
    return QString::number(placesCount);
}

QString Report::getInvitesCount()
{
    return QString::number(invitesCount);
}

QString Report::getCashlessCount()
{
    return QString::number(cashlessCount);
}

QString Report::getCashlessPrice()
{
    return QString::number(cashlessPrice);
}

QString Report::getSellCount()
{
    return QString::number(sellCount);
}

QString Report::getSellPrice()
{
    return QString::number(sellPrice);
}

QString Report::totalTickets()
{
    return QString::number(cashlessCount + sellCount);
}

QString Report::totalPrice()
{
    return QString::number(cashlessPrice + sellPrice);
}

QString Report::trashCount()
{
    return QString::number(placesCount - (cashlessCount + sellCount));
}





QString Report::tableRowOf(const QString& c1, const QString& c2, const QString& c3)
{
    QString r;

    r += "<tr style=\"height: 30px;\">";
    r += "<td style=\"width: 44.6971%; height: 18px;\">";
    r += c1;
    r += "</td>";
    r += "<td style=\"width: 28.0774%; height: 18px;\">";
    r += c2;
    r += "</td>";
    r += "<td style=\"width: 27.2254%; height: 18px;\">";
    r += c3;
    r += "</td>";
    r += "</tr>";

    return r;
}

QString Report::toHtml()
{
    QString h;

    h += "<h1 style=\"text-align: center;\">ОТЧЕТ</h1>";
    h += "<h3 style=\"text-align: center;\"><strong>О проданных билетах на концерт: ";
    h += concertName;
    h += ", ";
    h += concertDate;
    h += ", ";
    h += concertTime;
    h += "</strong></h3>";
    h += "<p><strong>Место проведения: ";
    h += concertHall;
    h += "</strong></p>";
    h += "<p><strong>Всего продано:</strong></p>";
    h += "<table style=\"border-collapse: collapse; width: 100%; height: 230px;\" border=\"1\">";
    h += "<tbody>";
    h += tableRowOf("", "Количество билетов", "Сумма, р.");
    h += tableRowOf("Количество мест в зале", placesCount, "");
    h += tableRowOf("Отведено на абонементы", invitesCount, "");
    h += tableRowOf("Выписано билетов", placesCount, "");
    h += tableRowOf("Продано по безналичному рассчету", cashlessCount, cashlessPrice);
    h += tableRowOf("Продано через кассу", sellCount, sellPrice);
    h += tableRowOf("Итого продано билетов", totalTickets(), totalPrice());
    h += tableRowOf("Осталось билетов на сжигание", trashCount(), "");

    return h;
}


















































//
