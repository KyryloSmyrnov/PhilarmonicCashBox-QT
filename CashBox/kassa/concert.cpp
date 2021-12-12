#include "concert.h"

#include "hall.h"

Concert::Concert()
{

}

QString Concert::totalTickets()
{
    return QString::number(cashlessCount + sellCount);
}

QString Concert::totalPrice()
{
    return QString::number(cashlessPrice + sellPrice);
}

QString Concert::trashCount()
{
    return QString::number(placesCount - (cashlessCount + sellCount));
}



QString Concert::tableRowOf(const QString& c1, const QString& c2, const QString& c3)
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

QString Concert::reportName()
{
    return "О проданных билетах на концерт (" + name + ").html";
}

QString Concert::htmlReport()
{
    QString h;

    h += "<h1 style=\"text-align: center;\">ОТЧЕТ</h1>";
    h += "<h3 style=\"text-align: center;\"><strong>О проданных билетах на концерт: ";
    h += name;
    h += ", ";
    h += date.toString("dd.MM.yyyy");
    h += ", ";
    h += time.toString("hh:mm");
    h += "</strong></h3>";
    h += "<p><strong>Место проведения: ";

    if (hall == Hall::LARGE) {
        h += "Большой зал";
    } else {
        h += "Малый зал";
    }

    h += "</strong></p>";
    h += "<p><strong>Всего продано:</strong></p>";
    h += "<table style=\"border-collapse: collapse; width: 100%; height: 230px;\" border=\"1\">";
    h += "<tbody>";
    h += tableRowOf("", "Количество билетов", "Сумма, р.");
    h += tableRowOf("Количество мест в зале", QString::number(placesCount), "");
    h += tableRowOf("Отведено на абонементы", QString::number(invitesCount), "");
    h += tableRowOf("Выписано билетов", QString::number(placesCount), "");
    h += tableRowOf("Продано по безналичному рассчету", QString::number(cashlessCount), QString::number(cashlessPrice));
    h += tableRowOf("Продано через кассу", QString::number(sellCount), QString::number(sellPrice));
    h += tableRowOf("Итого продано билетов", totalTickets(), totalPrice());
    h += tableRowOf("Осталось билетов на сжигание", trashCount(), "");

    return h;
}

QString Concert::build()
{
    QString s;

    if (hall == Hall::LARGE) s += "Б.";
    if (hall == Hall::SMALL) s += "М.";

    s += "   |   " + name + "   |   " + date.toString("dd.MM.yyyy") + "   |   " + time.toString("hh:mm");

    if (isActive)
    {
        s += "   |   " + QString::number(income) + "   |   " + QString::number(passedIncome);
    }

    return s;
}


QString Concert::reportOfSalesName()
{
    return "Отчет о сдаче выручки (" + name + ").html";
}

QString Concert::reportOfSalesHtml()
{
    QString h;

    h+= "<h1 style=\"text-align: center;\">ОТЧЕТ</h1>";
    h+= "<h3 style=\"text-align: center;\">О сдаче выручки</h3>";
    h+= "<table style=\"border-collapse: collapse; width: 100%;\" border=\"1\">";
    h+= "<tbody>";
    h+= "<tr>";
    h+= "<td style=\"width: 18.2955%;\">Дата концерта</td>";
    h+= "<td style=\"width: 22.5567%;\">Название концерта</td>";
    h+= "<td style=\"width: 21.5626%;\">Место проведения</td>";
    h+= "<td style=\"width: 18.7216%;\">Сдано выручки</td>";
    h+= "<td style=\"width: 18.8636%;\">Дата сдачи</td>";
    h+= "</tr>";
    h+= "<tr>";
    h+= "<td style=\"width: 18.2955%;\">";
    h+= date.toString("dd.MM.yyyy");
    h+= "</td>";
    h+= "<td style=\"width: 22.5567%;\">";
    h+= name;
    h+= "</td>";
    h+= "<td style=\"width: 21.5626%;\">";
    if (hall == Hall::LARGE) {
        h += "Большой зал";
    } else {
        h += "Малый зал";
    }
    h+= "</td>";
    h+= "<td style=\"width: 18.7216%;\">";
    h+= QString::number(passedIncome);
    h+= "</td>";
    h+= "<td style=\"width: 18.8636%;\">";
    h+= passDate.toString("dd.MM.yyyy");
    h+="</td>";
    h+= "</tr>";
    h+= "<tr>";
    h+= "<td style=\"width: 18.2955%;\">&nbsp;</td>";
    h+= "<td style=\"width: 22.5567%;\">&nbsp;</td>";
    h+= "<td style=\"width: 21.5626%;\">";
    h+= "<h3><strong>ИТОГО:</strong></h3>";
    h+= "</td>";
    h+= "<td style=\"width: 18.7216%;\">";
    h+= "<h3><strong>";
    h+= QString::number(passedIncome);
    h+= "</strong></h3>";
    h+= "</td>";
    h+= "<td style=\"width: 18.8636%;\">&nbsp;</td>";
    h+= "</tr>";
    h+= "</tbody>";
    h+= "</table";

    return h;

}















































//
