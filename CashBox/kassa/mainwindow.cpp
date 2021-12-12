#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <theme.h>
#include <string>
#include <cstring>

#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsEffect>
#include <QPainter>
#include <QSpacerItem>
#include <QMenu>

#include <QDir>
#include <QTimer>

#define BUILDING_DLL


#include <hall.h>
#include <state.h>
#include <concert.h>
#include <report.h>


QGridLayout *grid_smallHall;
QVBoxLayout* pricesLayout;

enum Halls
{
    LARGE_HALL, SMALL_HALL
};

void setupPlacesGrid(MainWindow* w, QGridLayout* g);
QColor getButtonColor(QPushButton* b);
void setButtonColor(QPushButton* b, QColor c);
void setupRowsLayout(MainWindow* w, QVBoxLayout* l);
void fillColorsGrid(QGridLayout* g);
void clearPricesLayout();
QString createSqlName(int row, int column);
QString createSqlSet(QString sqlName, int id);
void setupSchemesComboBox(QComboBox* cb, const Halls hall);
void setupConcertsComboBox(QComboBox* cb, const Halls hall);
QString makeSqlString(const QString& text);


QPushButton* activeButton;
QWidget* activeMenu = nullptr;

QPushButton* overlayButton;


// табы

QTabWidget* edit;


// цвета
QColor cursorColor_largeHall = Theme::color_default;
QColor cursorColor_smallHall = Theme::color_default;
QGridLayout* colors_largeHall;
QGridLayout* colors_smallHall;

// схемы
QVBoxLayout* vl_largeHall;
QVBoxLayout* vl_largeRows_left;
QVBoxLayout* vl_largeRows_right;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // создание папки с отчетами
    createReportsFolder();

    // табы
    edit = ui->redact_menu;

    // цвета
    connect(ui->clearCursor_largeHall, SIGNAL(clicked()), this, SLOT(clearCursorColor()));
    connect(ui->clearCursor_smallHall, SIGNAL(clicked()), this, SLOT(clearCursorColor()));
    colors_largeHall = ui->colors_largeHall;
    colors_smallHall = ui->colors_smallHall;
    setupColors(colors_largeHall);
    setupColors(colors_smallHall);

    // схемы
    vl_largeHall = ui->vl_largeHall;
    setupLargeHall();
    vl_largeRows_left = ui->vl_largeRows_left;
    vl_largeRows_right = ui->vl_largeRows_right;
    setupRows(vl_largeRows_left);
    setupRows(vl_largeRows_right);

    // очистка схемы
    connect(ui->clearScheme_largeHall, SIGNAL(clicked()), this, SLOT(onClearSchemeClick()));


    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("Philarmonic_Database");
    db.setUserName("postgres");
    db.setPassword("1918");

    if(!db.open())
    {
        qDebug() << db.lastError().text();
    }
    else
    {
        qDebug() << "Success!";
    }

    model = new QSqlTableModel(this, db);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setTable("small_hall_schemes");
    model->select();

    modelCurrentConcert = new QSqlTableModel(this,db);
    modelCurrentConcert->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelCurrentConcert->setTable("concerts");
    modelCurrentConcert->select();

    grid_smallHall = ui->grid_smallHall;
    setupPlacesGrid(this, grid_smallHall);

    setupRowsLayout(this, ui->vlayout_rows_left);
    setupRowsLayout(this, ui->vlayout_rows_right);


    //ОТКЛЮЧЕНИЕ ПРИМЕРА СХЕМЫ МАЛОГО ЗАЛА В СОЗДАНИИ КОНЦЕРТОВ
    ui->concertSchemeLabel_largeHall->setVisible(false);
    ui->concertScheme_largeHall_wrapper->setVisible(false);
    ui->concertSchemeLabel_smallHall->setVisible(false);
    ui->concertScheme_smallHall_wrapper->setVisible(false);


    pricesLayout = ui->pricesLayout;
    clearPricesLayout();

    setButtonColor(ui->busyStateButton, Theme::color_overlay);
    setButtonColor(ui->busyStateButton_2, Theme::color_overlay);


    setupConcertScene(Hall::LARGE);
    setupConcertScene(Hall::SMALL);

    // касса
    setupCashbox();

    // подгрузка схем
    loadSchemes();
    // подгрузка концертов
    loadConcerts();
}

// ПОДГРУЗКА ВСЕХ СХЕМ

void MainWindow::loadSchemes()
{
    // с большого зала
    QSqlQuery query;
    QString queryPrepare = "select * from large_hall_schemes";
    query.prepare(queryPrepare);
    query.exec();

    while (query.next())
    {
        Scheme s;
        s.hall = Hall::LARGE;
        s.id = query.value("id").toInt();

        for (int i = 0; i < ui->vl_largeHall_concert->count(); ++i)
        {
            QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(ui->vl_largeHall_concert->itemAt(i)->layout());

            int spacersCount = 0;

            QList<QColor> colors;

            for (int j = 0; j < row->count(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
                if (place == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                int r = i + 1;
                int p = j - spacersCount + 1;

                QString str = createColorString(r, p);

                int colorIndex = query.value(str).toInt() - 1;

                QColor color = Theme::colors[colorIndex];

                colors.push_back(color);
            }

            s.placeColors.push_back(colors);
        }

        schemes.push_back(s);
    }

    queryPrepare = "select * from small_hall_schemes";
    query.prepare(queryPrepare);
    query.exec();

    while (query.next())
    {
        Scheme s;
        s.hall = Hall::SMALL;
        s.id = query.value("id").toInt();

        for (int i = 0; i < ui->grid_smallHall_concert->rowCount(); ++i)
        {
            int spacersCount = 0;

            QList<QColor> colors;

            for (int j = 0; j < ui->grid_smallHall_concert->columnCount(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(ui->grid_smallHall_concert->itemAtPosition(i, j)->widget());
                if (place == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                int r = i + 1;
                int p = j - spacersCount + 1;

                QString str = createColorString(r, p);

                int colorIndex = query.value(str).toInt() - 1;

                QColor color = Theme::colors[colorIndex];

                colors.push_back(color);

            }

            s.placeColors.push_back(colors);
        }

        schemes.push_back(s);
    }

    std::sort(schemes.begin(), schemes.end(), [](const Scheme& s1, const Scheme& s2){
        return s1.id < s2.id;
    });
}

// ПОДГРУЗКА ВСЕХ КОНЦЕРТОВ

void MainWindow::loadConcerts()
{
    QSqlQuery query;
    QString queryPrepare = "select * from large_hall_concerts";
    query.prepare(queryPrepare);
    query.exec();

    // с большого зала
    while (query.next())
    {
        Concert concert;

        concert.hall = Hall::LARGE;
        concert.scheme = query.value("scheme").toInt();
        concert.isActive = query.value("is_active").toBool();
        concert.name = query.value("name").toString();
        concert.date = query.value("date").toDate();
        concert.time = query.value("time").toTime();
        concert.income = query.value("income").toInt();
        concert.passedIncome = query.value("passed_income").toInt();

        concert.placesCount = 493;

        QVariant passDate = query.value("pass_date");
        if ( ! passDate.isNull())
        {
            concert.passDate = passDate.toDate();
        }

        for (int i = 0; i < ui->vl_largeHall_concert->count(); ++i)
        {
            QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(ui->vl_largeHall_concert->itemAt(i)->layout());

            int spacersCount = 0;

            QList<int> states;

            for (int j = 0; j < row->count(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
                if (place == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                int r = i + 1;
                int p = j - spacersCount + 1;

                QString str = createStateString(r, p);

                int state = query.value(str).toInt();

                if (state == State::INVITE)
                {
                    concert.invitesCount++;
                }
                else if (state == State::CASHLESS)
                {
                    concert.cashlessCount++;

                    Scheme* s = getScheme(Hall::LARGE, concert.scheme);

                    QColor color = s->placeColors[r-1][p-1];

                    int colorIndex;
                    for (int i = 0; i < Theme::colors.size(); ++i)
                    {
                        if (color == Theme::colors[i])
                        {
                            colorIndex = i;
                        }
                    }

                    QVariant price = getColorPrice(Hall::LARGE, concert.name, colorIndex);
                    if ( ! price.isNull())
                    {
                        concert.cashlessPrice += price.toInt();
                    }
                }
                else if (state == State::SELL)
                {
                    concert.sellCount++;

                    Scheme* s = getScheme(Hall::LARGE, concert.scheme);

                    QColor color = s->placeColors[r-1][p-1];

                    int colorIndex;
                    for (int i = 0; i < Theme::colors.size(); ++i)
                    {
                        if (color == Theme::colors[i])
                        {
                            colorIndex = i;
                        }
                    }

                    QVariant price = getColorPrice(Hall::LARGE, concert.name, colorIndex);
                    if ( ! price.isNull())
                    {
                        concert.sellPrice += price.toInt();
                    }
                }

                states.push_back(state);
            }

            concert.placeStates.push_back(states);
        }

        if (concert.isActive)
        {
            activeConcerts.push_back(concert);
        }
        else
        {
            inactiveConcerts.push_back(concert);
        }
    }

    queryPrepare = "select * from small_hall_concerts";
    query.prepare(queryPrepare);
    query.exec();

    // с малого зала
    while (query.next())
    {
        Concert concert;

        concert.hall = Hall::SMALL;
        concert.scheme = query.value("scheme").toInt();
        concert.isActive = query.value("is_active").toBool();
        concert.name = query.value("name").toString();
        concert.date = query.value("date").toDate();
        concert.time = query.value("time").toTime();
        concert.income = query.value("income").toInt();
        concert.passedIncome = query.value("passed_income").toInt();

        concert.placesCount = 80;

        QVariant passDate = query.value("pass_date");
        if ( ! passDate.isNull())
        {
            concert.passDate = passDate.toDate();
        }

        for (int i = 0; i < ui->grid_smallHall_concert->rowCount(); ++i)
        {
            int spacersCount = 0;

            QList<int> states;

            for (int j = 0; j < ui->grid_smallHall_concert->columnCount(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(ui->grid_smallHall_concert->itemAtPosition(i, j)->widget());
                if (place == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                int r = i + 1;
                int p = j - spacersCount + 1;

                QString str = createStateString(r, p);

                int state = query.value(str).toInt();

                states.push_back(state);

                if (state == State::INVITE)
                {
                    concert.invitesCount++;
                }
                else if (state == State::CASHLESS)
                {
                    concert.cashlessCount++;

                    Scheme* s = getScheme(Hall::SMALL, concert.scheme);

                    QColor color = s->placeColors[r-1][p-1];

                    int colorIndex;
                    for (int i = 0; i < Theme::colors.size(); ++i)
                    {
                        if (color == Theme::colors[i])
                        {
                            colorIndex = i;
                        }
                    }

                    QVariant price = getColorPrice(Hall::SMALL, concert.name, colorIndex);
                    if ( ! price.isNull())
                    {
                        concert.cashlessPrice += price.toInt();
                    }
                }
                else if (state == State::SELL)
                {
                    concert.sellCount++;

                    Scheme* s = getScheme(Hall::SMALL, concert.scheme);

                    QColor color = s->placeColors[r-1][p-1];

                    int colorIndex;
                    for (int i = 0; i < Theme::colors.size(); ++i)
                    {
                        if (color == Theme::colors[i])
                        {
                            colorIndex = i;
                        }
                    }

                    QVariant price = getColorPrice(Hall::SMALL, concert.name, colorIndex);
                    if ( ! price.isNull())
                    {
                        concert.sellPrice += price.toInt();
                    }
                }
            }

            concert.placeStates.push_back(states);
        }

        if (concert.isActive)
        {
            activeConcerts.push_back(concert);
        }
        else
        {
            inactiveConcerts.push_back(concert);
        }
    }
}



Scheme* MainWindow::getScheme(const int& hall, const int& id)
{
    for (int i = 0; i < schemes.size(); ++i)
    {
        Scheme* s = &schemes[i];
        if (s->hall != hall) continue;
        if (s->id == id) return s;
    }
}

QList<Concert*> MainWindow::getConcerts(const int& hall)
{
    QList<Concert*> c;
    for (int i = 0; i < activeConcerts.size(); ++i)
    {
        Concert* curr = &activeConcerts[i];
        if (curr->hall == hall)
        {
            c.push_back(curr);
        }
    }
    return c;
}


//


QString reportsFolderPath;
void MainWindow::createReportsFolder()
{
    QDir dir;
    dir.cdUp();

    reportsFolderPath = dir.absolutePath() + "/Отчеты/";

    if ( ! dir.exists(reportsFolderPath))
    {
        dir.mkpath(reportsFolderPath);
    }
}



void MainWindow::setupConcertScene(const int& hall)
{
    if (hall == Hall::LARGE)
    {
        for (int i = 0; i < ui->vl_largeHall_concert->count(); ++i)
        {
            QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(ui->vl_largeHall_concert->itemAt(i)->layout());

            for (int j = 0; j < row->count(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
                if (place == nullptr)
                {
                    continue;
                }

                place->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(place, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(on_largeHallConcertPlace_clicked(const QPoint&)));
            }
        }
    }
    else if (hall == Hall::SMALL)
    {
        for (int i = 0; i < ui->grid_smallHall_concert->rowCount(); ++i)
        {
            for (int j = 0; j < ui->grid_smallHall_concert->columnCount(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(ui->grid_smallHall_concert->itemAtPosition(i, j)->widget());
                if (place == nullptr)
                {
                    continue;
                }

                place->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(place, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(on_smallHallConcertPlace_clicked(const QPoint&)));
            }
        }
    }
}



void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if (index == -1) return;

    for (int i = 0; i < ui->grid_smallHall->rowCount(); ++i)
    {
        int spacersCount = 0;

        for (int j = 0; j < ui->grid_smallHall->columnCount(); ++j)
        {
            auto place = dynamic_cast<QPushButton*>(ui->grid_smallHall->itemAtPosition(i, j)->widget());
            if (place == nullptr)
            {
                spacersCount++;
                continue;
            }

            int r = i;
            int p = j - spacersCount;

            Scheme* s = getScheme(Hall::SMALL, index + 1);

            QColor color = s->placeColors[r][p];

            setButtonColor(place, color);
        }
    }
}

void MainWindow::on_SmallHallCreateScheme_clicked()
{
    QSqlQuery query;

    QString places;
    QString values;

    Scheme newScheme;
    newScheme.hall = Hall::SMALL;
    newScheme.id = ui->comboBox->count() + 1;

    for (int i = 0; i < ui->grid_smallHall->rowCount(); ++i)
    {
        int spacersCount = 0;
        QList<QColor> colors;

        for (int j = 0; j < ui->grid_smallHall->columnCount(); ++j)
        {
            auto place = qobject_cast<QPushButton*>(ui->grid_smallHall->itemAtPosition(i, j)->widget());
            if (place == nullptr) {
                spacersCount++;
                continue;
            }

            QColor placeColor = getButtonColor(place);
            if (placeColor == Theme::color_default)
            {
                ui->comboBox->currentIndexChanged(ui->comboBox->currentIndex());
                return;
            }

            colors.push_back(placeColor);

            int r = i + 1;
            int p = j - spacersCount + 1;

            QString sqlName = createColorString(r, p);

            int colorId = 0;
            for (size_t i = 0; i < Theme::colors.size(); ++i)
            {
                if (placeColor == Theme::colors[i])
                {
                    colorId = i + 1;
                    break;
                }
            }

            places += sqlName + ", ";
            values += QString::number(colorId) + ", ";
        }

        newScheme.placeColors.push_back(colors);
    }

    schemes.push_back(newScheme);

    places.chop(2);
    values.chop(2);
    QString prepareStr = "INSERT INTO small_hall_schemes (" + places + ") VALUES (" + values + ")";
    query.prepare(prepareStr);
    query.exec();

    ui->comboBox->addItem(QString::number(newScheme.id));
    ui->comboBox->setCurrentIndex(ui->comboBox->count() - 1);
}
void MainWindow::on_SmallHallRedactScheme_clicked()
{
    QString id = ui->comboBox->currentText();

    for (int i = 0; i < activeConcerts.size(); ++i)
    {
        Concert* c = &activeConcerts[i];
        if (c->hall == Hall::SMALL) continue;
        if (id.toInt() == activeConcerts[i].scheme) return;
    }

    QSqlQuery query;
    QString queryPrepare;

    QString sets;

    Scheme* currScheme = getScheme(Hall::SMALL, id.toInt());

    for (int i = 0; i < ui->grid_smallHall->rowCount(); ++i)
    {

        int spacersCount = 0;

        for (int j = 0; j < ui->grid_smallHall->columnCount(); ++j)
        {
            auto place = qobject_cast<QPushButton*>(ui->grid_smallHall->itemAtPosition(i, j)->widget());
            if (place == nullptr) {
                spacersCount++;
                continue;
            }

            QColor placeColor = getButtonColor(place);
            if (placeColor == Theme::color_default)
            {
                ui->comboBox->currentIndexChanged(ui->comboBox->currentIndex());
                return;
            }

            int r = i + 1;
            int p = j - spacersCount + 1;

            currScheme->placeColors[r-1][p-1] = placeColor;

            QString sqlName = createColorString(r, p);

            int colorId = 0;
            for (size_t i = 0; i < Theme::colors.size(); ++i)
            {
                if (placeColor == Theme::colors[i])
                {
                    colorId = i + 1;
                    break;
                }
            }

            sets += createSqlSet(sqlName, colorId);
        }
    }

    sets.chop(2);
    queryPrepare = "UPDATE small_hall_schemes SET " + sets + " WHERE id = " + id;
    qDebug() << queryPrepare;
    query.prepare(queryPrepare);
    query.exec();
}

void MainWindow::on_cb_schemes_largeHall_currentIndexChanged(int index)
{
    if (index == -1) return;

    for (int i = 0; i < vl_largeHall->count(); ++i)
    {
        QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(vl_largeHall->itemAt(i)->layout());

        int spacersCount = 0;

        for (int j = 0; j < row->count(); ++j)
        {
            auto place = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
            if (place == nullptr)
            {
                spacersCount++;
                continue;
            }

            int r = i;
            int p = j - spacersCount;

            Scheme* s = getScheme(Hall::LARGE, index + 1);

            QColor color = s->placeColors[r][p];

            setButtonColor(place, color);
        }
    }
}

void MainWindow::on_createScheme_largeHall_clicked()
{
    QSqlQuery query;

    QString places;
    QString values;

    Scheme newScheme;
    newScheme.hall = Hall::LARGE;
    newScheme.id = ui->cb_schemes_largeHall->count() + 1;

    for (int i = 0; i < vl_largeHall->count(); ++i)
    {
        QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(vl_largeHall->itemAt(i)->layout());

        int spacersCount = 0;

        QList<QColor> colors;

        for (int j = 0; j < row->count(); ++j)
        {
            auto place = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
            if (place == nullptr)
            {
                spacersCount++;
                continue;
            }

            QColor placeColor = getButtonColor(place);
            if (placeColor == Theme::color_default)
            {
                ui->cb_schemes_largeHall->currentIndexChanged(ui->cb_schemes_largeHall->currentIndex());
                return;
            }

            int row = i + 1;
            int column = j - spacersCount + 1;

            colors.push_back(placeColor);

            QString placeName = createColorString(row, column);

            int colorId = 0;
            for (size_t i = 0; i < Theme::colors.size(); ++i)
            {
                if (placeColor == Theme::colors[i])
                {
                    colorId = i + 1;
                    break;
                }
            }

            places += placeName + ", ";
            values += QString::number(colorId) + ", ";

        }

        newScheme.placeColors.push_back(colors);
    }

    schemes.push_back(newScheme);

    places.chop(2);
    values.chop(2);
    QString prepareStr = "INSERT INTO large_hall_schemes (" + places + ") VALUES (" + values + ")";
    query.prepare(prepareStr);
    query.exec();

    ui->cb_schemes_largeHall->addItem(QString::number(newScheme.id));
    ui->cb_schemes_largeHall->setCurrentIndex(ui->cb_schemes_largeHall->count() - 1);
}
void MainWindow::on_editScheme_largeHall_clicked()
{
    QString id = ui->cb_schemes_largeHall->currentText();

    for (int i = 0; i < activeConcerts.size(); ++i)
    {
        Concert* c = &activeConcerts[i];
        if (c->hall == Hall::LARGE) continue;
        if (id.toInt() == activeConcerts[i].scheme) return;
    }

    QSqlQuery query;
    QString queryPrepare;

    QString sets;

    Scheme* currScheme = getScheme(Hall::LARGE, id.toInt());

    for (int i = 0; i < vl_largeHall->count(); ++i)
    {
        QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(vl_largeHall->itemAt(i)->layout());

        int spacersCount = 0;

        for (int j = 0; j < row->count(); ++j)
        {
            auto place = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
            if (place == nullptr)
            {
                spacersCount++;
                continue;
            }

            QColor placeColor = getButtonColor(place);
            if (placeColor == Theme::color_default)
            {
                ui->cb_schemes_largeHall->currentIndexChanged(ui->cb_schemes_largeHall->currentIndex());
                return;
            }

            int r = i + 1;
            int p = j - spacersCount + 1;

            currScheme->placeColors[r-1][p-1] = placeColor;

            QString placeName = createColorString(r, p);

            int colorId = 0;
            for (size_t i = 0; i < Theme::colors.size(); ++i)
            {
                if (placeColor == Theme::colors[i])
                {
                    colorId = i + 1;
                    break;
                }
            }

            sets += createSqlSet(placeName, colorId);

        }
    }
    sets.chop(2);

    queryPrepare = "UPDATE large_hall_schemes SET " + sets + " WHERE id = " + id;
    query.prepare(queryPrepare);
    query.exec();
}



// ОБЩИЕ МЕТОДЫ

Concert* MainWindow::getCurrentConcert(const int& hall)
{
    if (hall == Hall::LARGE)
    {
        return getConcerts(Hall::LARGE)[ui->currentConcertLargeHall->currentIndex()];
    }
    if (hall == Hall::SMALL)
    {
        return getConcerts(Hall::SMALL)[ui->currentConcertSmallHall->currentIndex()];
    }
}

QString MainWindow::getCurrentConcertName(const int& hall)
{
    if (hall == Hall::LARGE)
    {
        return ui->currentConcertLargeHall->currentText();
    }

    return ui->currentConcertSmallHall->currentText();
}

int MainWindow::getConcertScheme(const int& hall)
{
    // получение номера схемы
    QSqlQuery query;
    QString queryPrepare;

    if (hall == Hall::LARGE)
    {
        queryPrepare = "SELECT scheme FROM large_hall_concerts WHERE name = " + makeSqlString(getCurrentConcertName(hall));
    }
    else if (hall == Hall::SMALL)
    {
        queryPrepare = "SELECT scheme FROM small_hall_concerts WHERE name = " + makeSqlString(getCurrentConcertName(hall));
    }

    query.prepare(queryPrepare);
    query.exec();
    query.next();

    return query.value(0).toInt();
}

QVariant MainWindow::getColorPrice(const int& hall, const QString& concertName, const int& colorIndex)
{
    QSqlQuery query;
    QString queryPrepare;
    if (hall == Hall::LARGE)
    {
        queryPrepare = "select price_ofcolor_" + QString::number(colorIndex) + " from large_hall_concerts where name = " + makeSqlString(concertName);
    }
    else if (hall == Hall::SMALL)
    {
        queryPrepare = "select price_ofcolor_" + QString::number(colorIndex) + " from small_hall_concerts where name = " + makeSqlString(concertName);
    }
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    return query.value(0);
}

void MainWindow::clearCategories(const int& hall)
{
    QGridLayout* categories;
    if (hall == Hall::LARGE)
    {
        categories = ui->grid_largeHall_categories;
    }
    else if (hall == Hall::SMALL)
    {
        categories = ui->grid_smallHall_categories;
    }

    for (int i = 0; i < categories->count(); ++i)
    {
        QPushButton* b = dynamic_cast<QPushButton*>(categories->itemAt(i)->widget());

        b->setHidden(true);
    }
}

QString MainWindow::getPlaceStateString(const int& hall, const QPushButton* place)
{
    if (hall == Hall::LARGE)
    {
        for (int i = 0; i < ui->vl_largeHall_concert->count(); ++i)
        {
            QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(ui->vl_largeHall_concert->itemAt(i)->layout());

            int spacersCount = 0;

            for (int j = 0; j < row->count(); ++j)
            {
                auto p = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
                if (p == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                if (p == place)
                {
                    int Row = i + 1;
                    int Place = (j - spacersCount + 1);
                    return createStateString(Row, Place);
                }
            }
        }
    }
    else if (hall == Hall::SMALL)
    {
        for (int i = 0; i < ui->grid_smallHall_concert->rowCount(); ++i)
        {
            int spacer = 0;

            for (int j = 0; j < ui->grid_smallHall_concert->columnCount(); ++j)
            {
                auto p = qobject_cast<QPushButton*>(ui->grid_smallHall_concert->itemAtPosition(i, j)->widget());
                if (p == nullptr)
                {
                    spacer = 1;
                    continue;
                }

                if (p == place)
                {
                    int Row = i + 1;
                    int Place = j - spacer + 1;
                    return createStateString(Row, Place);
                }
            }
        }
    }
}

QString MainWindow::getPlaceColorString(const int& hall, const QPushButton* place)
{
    if (hall == Hall::LARGE)
    {
        for (int i = 0; i < ui->vl_largeHall_concert->count(); ++i)
        {
            QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(ui->vl_largeHall_concert->itemAt(i)->layout());

            int spacersCount = 0;

            for (int j = 0; j < row->count(); ++j)
            {
                auto p = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
                if (p == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                if (p == place)
                {
                    int Row = i + 1;
                    int Place = (j - spacersCount + 1);
                    return createColorString(Row, Place);
                }
            }
        }
    }
    else if (hall == Hall::SMALL)
    {
        for (int i = 0; i < ui->grid_smallHall_concert->rowCount(); ++i)
        {
            int spacer = 0;

            for (int j = 0; j < ui->grid_smallHall_concert->columnCount(); ++j)
            {
                auto p = qobject_cast<QPushButton*>(ui->grid_smallHall_concert->itemAtPosition(i, j)->widget());
                if (p == nullptr)
                {
                    spacer = 1;
                    continue;
                }

                if (p == place)
                {
                    int Row = i + 1;
                    int Place = j - spacer + 1;
                    return createColorString(Row, Place);
                }
            }
        }
    }
}

QString MainWindow::createStateString(const int& row, const int& place)
{
    return "state_ofplace_" + QString::number(row) + "_" + QString::number(place);
}

QString MainWindow::createColorString(const int& row, const int& place)
{
    return "place_" + QString::number(row) + "_" + QString::number(place);
}

void MainWindow::updatePlaceState(const int& hall, const QPushButton* place, const int& state)
{
    QSqlQuery query;
    QString queryPrepare;
    if (hall == Hall::LARGE)
    {
        queryPrepare = "update large_hall_concerts set " + getPlaceStateString(hall, place) + " = " + QString::number(state) + " where name = " + makeSqlString(getCurrentConcertName(hall));
    }
    else if (hall == Hall::SMALL)
    {
        queryPrepare = "update small_hall_concerts set " + getPlaceStateString(hall, place) + " = " + QString::number(state) + " where name = " + makeSqlString(getCurrentConcertName(hall));
    }
    query.prepare(queryPrepare);
    query.exec();
}



QColor MainWindow::getPlaceColor(const int& hall, const QPushButton* place)
{
    QSqlQuery query;
    QString queryPrepare;
    if (hall == Hall::LARGE)
    {
        queryPrepare = "select " + getPlaceColorString(hall, place) + " from large_hall_schemes where id = " + QString::number(getConcertScheme(hall));
    }
    if (hall == Hall::SMALL)
    {
        queryPrepare = "select " + getPlaceColorString(hall, place) + " from small_hall_schemes where id = " + QString::number(getConcertScheme(hall));
    }
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    return Theme::colors[query.value(0).toInt() - 1];
}

QColor MainWindow::getPlaceColor(const int& hall, const int& row, const int& place)
{
    QSqlQuery query;
    QString queryPrepare;
    if (hall == Hall::LARGE)
    {
        queryPrepare = "select " + createColorString(row, place) + " from large_hall_schemes where id = " + QString::number(getConcertScheme(hall));
    }
    if (hall == Hall::SMALL)
    {
        queryPrepare = "select " + createColorString(row, place) + " from small_hall_schemes where id = " + QString::number(getConcertScheme(hall));
    }
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    int colorIndex = query.value(0).toInt() - 1;

    return Theme::colors[colorIndex];
}



int MainWindow::getPlaceState(const int& hall, const QPushButton* place)
{
    QSqlQuery query;
    QString queryPrepare;
    if (hall == Hall::LARGE)
    {
        queryPrepare = "select " + getPlaceStateString(hall, place) + " from large_hall_concerts where name = " + makeSqlString(getCurrentConcertName(hall));
    }
    else if (hall == Hall::SMALL)
    {
        queryPrepare = "select " + getPlaceStateString(hall, place) + " from small_hall_concerts where name = " + makeSqlString(getCurrentConcertName(hall));
    }
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    return query.value(0).toInt();
}

int MainWindow::getPlaceState(const int& hall, const int& row, const int& place)
{
    QSqlQuery query;
    QString queryPrepare;
    if (hall == Hall::LARGE)
    {
        queryPrepare = "select " + createStateString(row, place) + " from large_hall_concerts where name = " + makeSqlString(getCurrentConcertName(hall));
    }
    else if (hall == Hall::SMALL)
    {
        queryPrepare = "select " + createStateString(row, place) + " from small_hall_concerts where name = " + makeSqlString(getCurrentConcertName(hall));
    }
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    return query.value(0).toInt();
}

//


Halls activeTab()
{
    if (edit->currentIndex() == Halls::LARGE_HALL) return Halls::LARGE_HALL;
    if (edit->currentIndex() == Halls::SMALL_HALL) return Halls::SMALL_HALL;
}


void MainWindow::clearCursorColor()
{
    if (activeTab() == Halls::LARGE_HALL)
    {
        cursorColor_largeHall = Theme::color_default;
    }
    else if (activeTab() == Halls::SMALL_HALL)
    {
        cursorColor_smallHall = Theme::color_default;
    }
}

void MainWindow::setupColors(const QGridLayout* colors)
{
    for (int i = 0; i < colors->rowCount(); ++i)
    {
        for (int j = 0; j < colors->columnCount(); ++j)
        {
            QPushButton* b = dynamic_cast<QPushButton*>(colors->itemAtPosition(i, j)->widget());

            setButtonColor(b, Theme::colors[i * colors->columnCount() + j]);
            connect(b, SIGNAL(clicked()), this, SLOT(onColorClick()));
        }
    }
}

void MainWindow::onColorClick()
{
    QPushButton* b = qobject_cast<QPushButton*>(sender());

    QColor color = getButtonColor(b);

    if (activeTab() == Halls::LARGE_HALL)
    {
        cursorColor_largeHall = color;
    }
    else if (activeTab() == Halls::SMALL_HALL)
    {
        cursorColor_smallHall = color;
    }
}

void MainWindow::onPlaceClick()
{
    QPushButton* b = qobject_cast<QPushButton*>(sender());

    if (activeTab() == Halls::LARGE_HALL)
    {
        setButtonColor(b, cursorColor_largeHall);
    }
    else if (activeTab() == Halls::SMALL_HALL)
    {
        setButtonColor(b, cursorColor_smallHall);
    }

}


// большой зал

void MainWindow::setupLargeHall()
{
    for (int i = 0; i < ui->vl_largeHall->count(); ++i)
    {
        QHBoxLayout* hl_row = dynamic_cast<QHBoxLayout*>(ui->vl_largeHall->itemAt(i)->layout());
        for (int j = 0; j < hl_row->count(); ++j)
        {
            auto place = qobject_cast<QPushButton*>(hl_row->itemAt(j)->widget());
            if (place == nullptr) continue;

            connect(place, SIGNAL(clicked()), this, SLOT(onPlaceClick()));
        }
    }
}

// малый зал

void MainWindow::setupRows(const QVBoxLayout* rows)
{
    for (int i = 0; i < rows->count(); ++i)
    {
        QPushButton* row = dynamic_cast<QPushButton*>(rows->itemAt(i)->widget());

        connect(row, SIGNAL(clicked()), this, SLOT(onRowClick2()));
    }
}
void MainWindow::onRowClick2()
{
    QPushButton *rowBtn = qobject_cast<QPushButton*>(sender());

    QString indexStr;

    for (int i = 0; i < rowBtn->text().size(); ++i)
    {
        if (rowBtn->text()[i].isDigit())
        {
            indexStr += rowBtn->text()[i];
        }
    }

    int selectedRowIndex = indexStr.toInt() - 1;

    QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(vl_largeHall->itemAt(selectedRowIndex)->layout());

    for (int i = 0; i < row->count(); ++i)
    {
        auto place = qobject_cast<QPushButton*>(row->itemAt(i)->widget());
        if (place == nullptr) continue;

        place->click();
    }
}

void MainWindow::onClearSchemeClick()
{
    for (int i = 0; i < ui->vl_largeHall->count(); ++i)
    {
        QHBoxLayout* hl_row = dynamic_cast<QHBoxLayout*>(ui->vl_largeHall->itemAt(i)->layout());
        for (int j = 0; j < hl_row->count(); ++j)
        {
            auto place = qobject_cast<QPushButton*>(hl_row->itemAt(j)->widget());
            if (place == nullptr) continue;

            setButtonColor(place, Theme::color_default);
        }
    }
}



void setupPlacesGrid(MainWindow* w, QGridLayout* g)
{
    for (int i = 0; i < g->rowCount(); ++i)
    {
        for (int j = 0; j < g->columnCount(); ++j)
        {

            if (j != 4)
            {
                QPushButton *b = dynamic_cast<QPushButton*>(g->itemAtPosition(i, j)->widget());

                w->connect(b, SIGNAL(clicked()), w, SLOT(onPlaceClick()));
            }
        }
    }
}



void clearPricesLayout()
{
    for (int i = 0; i < pricesLayout->count(); ++i)
    {
        QHBoxLayout* l = dynamic_cast<QHBoxLayout*>(pricesLayout->itemAt(i)->layout());

        QPushButton* b = dynamic_cast<QPushButton*>(l->itemAt(0)->widget());
        setButtonColor(b, Theme::color_default);
        b->setEnabled(false);

        QTextEdit* te = dynamic_cast<QTextEdit*>(l->itemAt(1)->widget());
        te->clear();
        te->setReadOnly(true);
    }
}



void setupRowsLayout(MainWindow* w, QVBoxLayout* l)
{
    for (int i = 0; i < l->count(); ++i)
    {
        QPushButton *b = dynamic_cast<QPushButton*>(l->itemAt(i)->widget());

        w->connect(b, SIGNAL(clicked()), w, SLOT(onRowClick()));
    }
}

void MainWindow::onRowClick()
{
    QPushButton *rowBtn = qobject_cast<QPushButton*>(sender());

    QString indexStr;

    for (int i = 0; i < rowBtn->text().size(); ++i)
    {
        if (rowBtn->text()[i].isDigit())
        {
            indexStr += rowBtn->text()[i];
        }
    }

    int selectedRowIndex = indexStr.toInt() - 1;

    for (int j = 0; j < grid_smallHall->columnCount(); ++j)
    {
        if (j == 4) continue;

        QPushButton *b = dynamic_cast<QPushButton*>(grid_smallHall->itemAtPosition(selectedRowIndex, j)->widget());
        b->click();
    }
}

QString createSqlSet(QString sqlName, int id)
{
    return sqlName + " = " + QString::number(id) + ", ";
}

void setButtonColor(QPushButton* b, QColor c)
{
    QPalette p = b->palette();

    p.setColor(QPalette::Button, c);

    b->setAutoFillBackground(true);
    b->setPalette(p);
    b->show();
}

QColor getButtonColor(QPushButton* b)
{
    QPalette p = b->palette();
    return p.color(QPalette::Button);
}





MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::on_hallEdit_currentIndexChanged(int index)
{
    if (index == -1) return;

    if (index == 0)
    {
        ui->shemeEdit->clear();
        clearPricesLayout();
        ui->concertSchemeLabel_smallHall->setVisible(false);
        ui->concertSchemeLabel_largeHall->setVisible(false);
        ui->concertScheme_smallHall_wrapper->setVisible(false);
        ui->concertScheme_largeHall_wrapper->setVisible(false);
        return;
    }
    if(index == 1)
    {
        ui->concertSchemeLabel_smallHall->setVisible(false);
        ui->concertSchemeLabel_largeHall->setVisible(true);
        ui->concertScheme_smallHall_wrapper->setVisible(false);
        ui->concertScheme_largeHall_wrapper->setVisible(true);
    }
    else if (index == 2)
    {
        ui->concertSchemeLabel_largeHall->setVisible(false);
        ui->concertSchemeLabel_smallHall->setVisible(true);
        ui->concertScheme_largeHall_wrapper->setVisible(false);
        ui->concertScheme_smallHall_wrapper->setVisible(true);
    }

    ui->shemeEdit->clear();

    int hall = index - 1;

    for (int i = 0; i < schemes.size(); ++i)
    {
        if (schemes[i].hall == hall)
        {
            ui->shemeEdit->addItem(QString::number(schemes[i].id));
        }
    }
}

void MainWindow::on_shemeEdit_currentIndexChanged(int index)
{
    if (index == -1) return;

    // очистка ценовых категорий
    clearPricesLayout();

    QSqlQuery query;
    QString currentID = QString::number(index+1);
    QString queryPrepare;

    int hall;
    if (ui->hallEdit->currentText() == "Большой зал")
    {
        hall = Hall::LARGE;
    }
    else if (ui->hallEdit->currentText() == "Малый зал")
    {
        hall = Hall::SMALL;
    }

    QList<QColor> colors;

    if (hall == Hall::LARGE)
    {
        for (int i = 0; i < ui->concertScheme_largeHall->count(); ++i)
        {
            QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(ui->concertScheme_largeHall->itemAt(i)->layout());

            int spacersCount = 0;

            for (int j = 0; j < row->count(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
                if (place == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                int r = i;
                int p = j - spacersCount;

                Scheme* s = getScheme(Hall::LARGE, index + 1);

                QColor color = s->placeColors[r][p];

                bool isIn = (std::find(colors.begin(), colors.end(), color) != colors.end());
                if ( ! isIn)
                {
                    colors.push_back(color);
                }

                setButtonColor(place, color);
            }
        }
    }
    else if (hall == Hall::SMALL)
    {
        for (int i = 0; i < ui->concertScheme_smallHall->rowCount(); ++i)
        {
            int spacersCount = 0;

            for (int j = 0; j < ui->concertScheme_smallHall->columnCount(); ++j)
            {
                auto place = dynamic_cast<QPushButton*>(ui->concertScheme_smallHall->itemAtPosition(i, j)->widget());
                if (place == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                int r = i;
                int p = j - spacersCount;

                Scheme* s = getScheme(Hall::SMALL, index + 1);

                QColor color = s->placeColors[r][p];

                bool isIn = (std::find(colors.begin(), colors.end(), color) != colors.end());
                if ( ! isIn)
                {
                    colors.push_back(color);
                }

                setButtonColor(place, color);
            }
        }
    }

    // заполнение ценовых категорий
    for (int i = 0; i < colors.size(); ++i)
    {
        QHBoxLayout* l = dynamic_cast<QHBoxLayout*>(pricesLayout->itemAt(i)->layout());

        QPushButton* b = dynamic_cast<QPushButton*>(l->itemAt(0)->widget());
        setButtonColor(b, colors[i]);

        QTextEdit* te = dynamic_cast<QTextEdit*>(l->itemAt(1)->widget());
        te->clear();
        te->setReadOnly(false);
    }

}


QString makeSqlString(const QString& text)
{
    return "'" + text + "'";
}


QString makeSqlColorString(const int& colorIndex)
{
    return "price_ofcolor_" + QString::number(colorIndex);
}



void MainWindow::on_createConcertButton_clicked()
{
    QString sqlFormDate = "";

    // если поля пустые выходим нахуй пока не произошел пиздец
    if(ui->nameEdit->toPlainText() == "" && ui->hallEdit->currentText() == "" && ui->shemeEdit->currentIndex() + 1 == 0) return;

    QString hall = ui->hallEdit->currentText();

    QString queryPrepare;

    Concert concert;

    concert.scheme = ui->shemeEdit->currentIndex() + 1;
    concert.isActive = true;
    concert.name = ui->nameEdit->toPlainText();
    concert.date = ui->dateEdit->date();
    concert.time = ui->timeEdit->time();
    concert.income = 0;
    concert.passedIncome = 0;

    QString name = makeSqlString(concert.name);
    QString date = makeSqlString(concert.date.toString("yyyy-MM-dd"));
    QString time = makeSqlString(concert.time.toString("hh:mm"));
    QString scheme = makeSqlString(QString::number(concert.scheme));

    QString queryValues = " VALUES (" + name + ", " + date + ", " + time + ", " + scheme + ", 'true', ";

    if (hall == "Большой зал")
    {
        concert.hall = Hall::LARGE;
        concert.placesCount = 493;
        queryPrepare = "INSERT INTO large_hall_concerts (name, date, time, scheme, is_active, ";

        for (int i = 0; i < ui->vl_largeHall_concert->count(); ++i)
        {
            QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(ui->vl_largeHall_concert->itemAt(i)->layout());

            QList<int> states;

            int spacersCount = 0;

            for (int j = 0; j < row->count(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
                if (place == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                int r = i + 1;
                int p = j - spacersCount + 1;

                queryPrepare += createStateString(r, p);
                queryValues += QString::number(0);

                queryPrepare += ", ";
                queryValues += ", ";

                states.push_back(0);
            }

            concert.placeStates.push_back(states);
        }
    }
    else if (hall == "Малый зал")
    {
        concert.hall = Hall::SMALL;
        concert.placesCount = 80;
        queryPrepare = "INSERT INTO small_hall_concerts (name, date, time, scheme, is_active, ";

        for (int i = 0; i < ui->grid_smallHall_concert->rowCount(); ++i)
        {

            int spacersCount = 0;

            QList<int> states;

            for (int j = 0; j < ui->grid_smallHall_concert->columnCount(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(ui->grid_smallHall_concert->itemAtPosition(i, j)->widget());
                if (place == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                int r = i + 1;
                int p = j - spacersCount + 1;

                queryPrepare += createStateString(r, p);
                queryValues += QString::number(0);

                queryPrepare += ", ";
                queryValues += ", ";

                states.push_back(0);
            }

            concert.placeStates.push_back(states);
        }
    }

    activeConcerts.push_back(concert);

    QList<int> colorIndexes;
    QList<QString> prices;

    for (int i = 0; i < pricesLayout->count(); ++i)
    {
        QHBoxLayout* l = dynamic_cast<QHBoxLayout*>(pricesLayout->itemAt(i)->layout());

        QTextEdit* te = dynamic_cast<QTextEdit*>(l->itemAt(1)->widget());

        // если поле неактивно то ценовые категории по цветам закончились
        // и мы можемвыйти из цикла
        if (te->isReadOnly()) break;

        // записываем индекс цвета в список для дальнейшей записи в базу данных
        QPushButton* b = dynamic_cast<QPushButton*>(l->itemAt(0)->widget());
        QColor buttonColor = getButtonColor(b);
        auto it = std::find(Theme::colors.begin(), Theme::colors.end(), buttonColor);
        int colorIndex = it - Theme::colors.begin();
        colorIndexes.push_back(colorIndex);

        // записываем цену в список для дальнейшей записи в базу данных
        QString price = te->toPlainText();
        prices.push_back(price);
    }

    // перебор списков с данными и подготовка их к записи в базу данных
    for (int i = 0; i < colorIndexes.size(); ++i)
    {
        // добавляем в ячейку с цветом цену
        queryPrepare += makeSqlColorString(colorIndexes[i]);
        queryValues += prices[i];

        // если не конец списка, добавляем запятую, если конец, запятая не нужна
        if (i != colorIndexes.size() - 1)
        {
            queryPrepare += ", ";
            queryValues += ", ";
        }
    }

    queryPrepare += ")";
    queryValues += ")";

    QSqlQuery query;
    QString queryFinal = queryPrepare + queryValues;
    qDebug() << queryFinal;
    query.prepare(queryFinal);
    query.exec();

    ui->createConcert_message->setText("Концерт успешно создан");

    QTimer* timer = new QTimer();
    timer->start(2000);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_concert_created()));

    on_createConcert_clear_clicked();
}

void MainWindow::on_concert_created()
{
    ui->createConcert_message->setText("");
}



void addCategoryButton(Ui::MainWindow* ui, QPushButton* button)
{
    int row;
    int column;

    if (ui->grid_smallHall_categories->count() == 0)
    {
        row = 0;
        column = 0;
    }
    else
    {
        row = ui->grid_smallHall_categories->count() / 3;
        column = ui->grid_smallHall_categories->count() % 3;
    }

    ui->grid_smallHall_categories->addWidget(button, row, column, Qt::AlignJustify);
}



QWidget* wrap(QLayout* l)
{
    auto widget = new QWidget();
    widget->setLayout( l );
    return widget;
}

QAction* MainWindow::createStateAction(const QString& text, QObject* parent)
{
    QAction* action = new QAction(text, parent);
    QFont* font = new QFont();
    font->setPointSize(12);
    font->setBold(true);

    action->setFont(*font);

    return action;
}


void MainWindow::obtainConcertPlacePosition(const int& hall, const QPushButton* placeButton, int& r, int& p)
{
    if (hall == Hall::LARGE)
    {
        for (int i = 0; i < ui->vl_largeHall_concert->count(); ++i)
        {
            QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(ui->vl_largeHall_concert->itemAt(i)->layout());

            int spacersCount = 0;

            for (int j = 0; j < row->count(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
                if (place == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                if (placeButton == place)
                {
                    r = i + 1;
                    p = j - spacersCount + 1;
                }
            }
        }
    }
    else if (hall == Hall::SMALL)
    {
        for (int i = 0; i < ui->grid_smallHall_concert->rowCount(); ++i)
        {
            int spacersCount = 0;

            for (int j = 0; j < ui->grid_smallHall_concert->columnCount(); ++j)
            {
                auto place = qobject_cast<QPushButton*>(ui->grid_smallHall_concert->itemAtPosition(i, j)->widget());
                if (place == nullptr)
                {
                    spacersCount++;
                    continue;
                }

                if (placeButton == place)
                {
                    r = i + 1;
                    p = j - spacersCount + 1;
                }
            }
        }
    }
}


void MainWindow::on_currentConcertLargeHall_currentIndexChanged(int index)
{
    if (index == -1) return;

    // очистка категорий
    clearCategories(Hall::LARGE);

    // получение имени концерта
    QString concertName = getCurrentConcertName(Hall::LARGE);

    // запись цен и цветов
    QList<QColor> colors;
    QList<int> prices;

    QSqlQuery query;
    QString queryPrepare = "select * from large_hall_concerts where name = " + makeSqlString(concertName);
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    QString base;

    for (size_t i = 0; i < Theme::colors.size(); ++i)
    {
        base = "price_ofcolor_" + QString::number(i);

        QVariant price = query.value(base);

        if ( ! price.isNull())
        {
            prices.push_back(price.toInt());
            colors.push_back(Theme::colors[i]);
        }
    }

    // получение схемы

    Concert* currentConcert = getConcerts(Hall::LARGE)[index];

    for (int i = 0; i < ui->vl_largeHall_concert->count(); ++i)
    {
        QHBoxLayout* row = dynamic_cast<QHBoxLayout*>(ui->vl_largeHall_concert->itemAt(i)->layout());

        int spacersCount = 0;

        for (int j = 0; j < row->count(); ++j)
        {
            auto place = qobject_cast<QPushButton*>(row->itemAt(j)->widget());
            if (place == nullptr)
            {
                spacersCount++;
                continue;
            }

            int r = i;
            int p = j - spacersCount;

            int state = currentConcert->placeStates[r][p];

            if (state == State::FREE)
            {
                //получение цвета

                int schemeId = currentConcert->scheme;

                Scheme* s = getScheme(Hall::LARGE, schemeId);

                QColor placeColor = s->placeColors[r][p];

                setButtonColor(place, placeColor);
            }
            else
            {
                setButtonColor(place, Theme::color_overlay);
            }
        }
    }

    // запись категорий

    int iterator = 0;

    for (int i = 0; i < ui->grid_largeHall_categories->rowCount(); ++i)
    {
        for (int j = 0; j < ui->grid_largeHall_categories->columnCount(); ++j)
        {
            if (iterator == prices.size()) break;

            QPushButton* b = dynamic_cast<QPushButton*>(ui->grid_largeHall_categories->itemAtPosition(i, j)->widget());
            b->setHidden(false);
            setButtonColor(b, colors[iterator]);

            b->setText(QString::number(prices[iterator]));

            iterator++;
        }
    }
}
void MainWindow::on_largeHallConcertPlace_clicked(const QPoint& pos)
{
    QPushButton* place = qobject_cast<QPushButton*>(sender());

    QMenu* statesMenu = new QMenu(place);

    QAction* sell = createStateAction("Продано", statesMenu);
    QAction* invite = createStateAction("Пригласительный", statesMenu);
    QAction* cashless = createStateAction("Безналично", statesMenu);
    QAction* free = createStateAction("Свободно", statesMenu);

    connect(sell, SIGNAL(triggered()), this, SLOT(on_largeHallMenu_sell_clicked()));
    connect(invite, SIGNAL(triggered()), this, SLOT(on_largeHallMenu_invite_clicked()));
    connect(cashless, SIGNAL(triggered()), this, SLOT(on_largeHallMenu_cashless_clicked()));
    connect(free, SIGNAL(triggered()), this, SLOT(on_largeHallMenu_free_clicked()));

    statesMenu->addAction(sell);
    statesMenu->addAction(invite);
    statesMenu->addAction(cashless);
    statesMenu->addAction(free);

    statesMenu->popup(place->mapToGlobal(pos));
}
void MainWindow::on_largeHallMenu_sell_clicked()
{
    QAction* menuButton = qobject_cast<QAction*>(sender());
    QMenu* menu = qobject_cast<QMenu*>(menuButton->parent());
    QPushButton* button = qobject_cast<QPushButton*>(menu->parent());

    int r;
    int p;
    obtainConcertPlacePosition(Hall::LARGE, button, r, p);

    Concert* curr = getCurrentConcert(Hall::LARGE);

    int state = curr->placeStates[r-1][p-1];
    if (state == State::SELL) return;

    updatePlaceState(Hall::LARGE, button, State::SELL);
    curr->placeStates[r-1][p-1] = State::SELL;

    if (state == State::INVITE) curr->invitesCount--;
    if (state == State::CASHLESS) curr->cashlessCount--;
    curr->sellCount++;

    Scheme* s = getScheme(Hall::LARGE, curr->scheme);

    QColor buttonColor = s->placeColors[r-1][p-1];

    int colorIndex;
    for (size_t i = 0; i < Theme::colors.size(); ++i)
    {
        if (buttonColor == Theme::colors[i])
        {
            colorIndex = i;
        }
    }

    int price = getColorPrice(Hall::LARGE, getCurrentConcertName(Hall::LARGE), colorIndex).toInt();

    sendToConcert(Hall::LARGE, getCurrentConcertName(Hall::LARGE), price);
    sendToCashbox(price);

    setButtonColor(button, Theme::color_overlay);
}
void MainWindow::on_largeHallMenu_invite_clicked()
{
    QAction* menuButton = qobject_cast<QAction*>(sender());
    QMenu* menu = qobject_cast<QMenu*>(menuButton->parent());
    QPushButton* place = qobject_cast<QPushButton*>(menu->parent());

    int r;
    int p;
    obtainConcertPlacePosition(Hall::LARGE, place, r, p);

    Concert* curr = getCurrentConcert(Hall::LARGE);
    Scheme* s = getScheme(Hall::LARGE, curr->scheme);

    int state = curr->placeStates[r-1][p-1];
    if (state == State::INVITE) return;

    updatePlaceState(Hall::LARGE, place, State::INVITE);
    curr->placeStates[r-1][p-1] = State::INVITE;

    if (state == State::SELL) curr->sellCount--;
    if (state == State::CASHLESS) curr->cashlessCount--;
    curr->invitesCount++;

    if (state == State::SELL)
    {
        QColor placeColor = s->placeColors[r-1][p-1];

        int colorIndex;
        for (size_t i = 0; i < Theme::colors.size(); ++i)
        {
            if (placeColor == Theme::colors[i])
            {
                colorIndex = i;
            }
        }

        int price = getColorPrice(Hall::LARGE, getCurrentConcertName(Hall::LARGE), colorIndex).toInt();

        extractFromConcert(Hall::LARGE, getCurrentConcertName(Hall::LARGE), price);
        extractFromCashbox(price);
    }

    setButtonColor(place, Theme::color_overlay);
}
void MainWindow::on_largeHallMenu_cashless_clicked()
{
    QAction* menuButton = qobject_cast<QAction*>(sender());
    QMenu* menu = qobject_cast<QMenu*>(menuButton->parent());
    QPushButton* place = qobject_cast<QPushButton*>(menu->parent());

    int r;
    int p;
    obtainConcertPlacePosition(Hall::LARGE, place, r, p);

    Concert* curr = getCurrentConcert(Hall::LARGE);
    Scheme* s = getScheme(Hall::LARGE, curr->scheme);

    int state = curr->placeStates[r-1][p-1];
    if (state == State::CASHLESS) return;

    updatePlaceState(Hall::LARGE, place, State::CASHLESS);
    curr->placeStates[r-1][p-1] = State::CASHLESS;

    if (state == State::SELL) curr->sellCount--;
    if (state == State::INVITE) curr->invitesCount--;
    curr->cashlessCount++;

    if (state == State::SELL)
    {
        QColor placeColor = s->placeColors[r-1][p-1];

        int colorIndex;
        for (size_t i = 0; i < Theme::colors.size(); ++i)
        {
            if (placeColor == Theme::colors[i])
            {
                colorIndex = i;
            }
        }

        int price = getColorPrice(Hall::LARGE, getCurrentConcertName(Hall::LARGE), colorIndex).toInt();

        extractFromConcert(Hall::LARGE, getCurrentConcertName(Hall::LARGE), price);
        extractFromCashbox(price);
    }

    setButtonColor(place, Theme::color_overlay);
}
void MainWindow::on_largeHallMenu_free_clicked()
{
    // получение кнопки
    QAction* menuButton = qobject_cast<QAction*>(sender());
    QMenu* menu = qobject_cast<QMenu*>(menuButton->parent());
    QPushButton* button = qobject_cast<QPushButton*>(menu->parent());

    int r;
    int p;
    obtainConcertPlacePosition(Hall::LARGE, button, r, p);

    Concert* curr = getCurrentConcert(Hall::LARGE);
    Scheme* s = getScheme(Hall::LARGE, curr->scheme);

    int state = curr->placeStates[r-1][p-1];
    if (state == State::FREE) return;

    updatePlaceState(Hall::LARGE, button, State::FREE);
    curr->placeStates[r-1][p-1] = State::FREE;

    if (state == State::SELL) curr->sellCount--;
    if (state == State::INVITE) curr->invitesCount--;
    if (state == State::CASHLESS) curr->cashlessCount--;

    // получение цвета
    QColor placeColor = s->placeColors[r-1][p-1];
    setButtonColor(button, placeColor);

    if (state != State::SELL) return;

    QColor buttonColor = getButtonColor(button);

    int colorIndex;
    for (size_t i = 0; i < Theme::colors.size(); ++i)
    {
        if (buttonColor == Theme::colors[i])
        {
            colorIndex = i;
        }
    }

    int price = getColorPrice(Hall::LARGE, getCurrentConcertName(Hall::LARGE), colorIndex).toInt();

    extractFromConcert(Hall::LARGE, getCurrentConcertName(Hall::LARGE), price);
    extractFromCashbox(price);
}


void MainWindow::on_currentConcertSmallHall_currentIndexChanged(int index)
{
    if (index == -1) return;

    // очистка категорий
    clearCategories(Hall::SMALL);

    // получение имени концерта
    QString concertName = getCurrentConcertName(Hall::SMALL);

    // запись цен и цветов
    QList<QColor> colors;
    QList<int> prices;

    QSqlQuery query;
    QString queryPrepare = "select * from small_hall_concerts where name = " + makeSqlString(concertName);
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    QString base;

    for (size_t i = 0; i < Theme::colors.size(); ++i)
    {
        base = "price_ofcolor_" + QString::number(i);

        QVariant price = query.value(base);

        if ( ! price.isNull())
        {
            prices.push_back(price.toInt());
            colors.push_back(Theme::colors[i]);
        }
    }

    // получение схемы

    Concert* currentConcert = getConcerts(Hall::SMALL)[index];

    for (int i = 0; i < ui->grid_smallHall_concert->rowCount(); ++i)
    {
        int spacersCount = 0;

        for (int j = 0; j < ui->grid_smallHall_concert->columnCount(); ++j)
        {
            auto place = qobject_cast<QPushButton*>(ui->grid_smallHall_concert->itemAtPosition(i, j)->widget());
            if (place == nullptr)
            {
                spacersCount = 1;
                continue;
            }

            int r = i;
            int p = j - spacersCount;

            int state = currentConcert->placeStates[r][p];

            if (state == State::FREE)
            {
                //получение цвета

                int schemeId = currentConcert->scheme;

                Scheme* s = getScheme(Hall::SMALL, schemeId);

                QColor placeColor = s->placeColors[r][p];

                setButtonColor(place, placeColor);
            }
            else
            {
                setButtonColor(place, Theme::color_overlay);
            }
        }
    }

    int iterator = 0;

    for (int i = 0; i < ui->grid_smallHall_categories->rowCount(); ++i)
    {
        for (int j = 0; j < ui->grid_smallHall_categories->columnCount(); ++j)
        {
            if (iterator >= prices.size()) continue;

            QPushButton* b = dynamic_cast<QPushButton*>(ui->grid_smallHall_categories->itemAtPosition(i, j)->widget());
            b->setHidden(false);
            setButtonColor(b, colors[iterator]);

            b->setText(QString::number(prices[iterator]));

            iterator++;
        }
    }

}
void MainWindow::on_smallHallConcertPlace_clicked(const QPoint& pos)
{
    QPushButton* place = qobject_cast<QPushButton*>(sender());

    QMenu* statesMenu = new QMenu(place);

    QAction* sell = createStateAction("Продано", statesMenu);
    QAction* invite = createStateAction("Пригласительный", statesMenu);
    QAction* cashless = createStateAction("Безналично", statesMenu);
    QAction* free = createStateAction("Свободно", statesMenu);

    connect(sell, SIGNAL(triggered()), this, SLOT(on_smallHallMenu_sell_clicked()));
    connect(invite, SIGNAL(triggered()), this, SLOT(on_smallHallMenu_invite_clicked()));
    connect(cashless, SIGNAL(triggered()), this, SLOT(on_smallHallMenu_cashless_clicked()));
    connect(free, SIGNAL(triggered()), this, SLOT(on_smallHallMenu_free_clicked()));

    statesMenu->addAction(sell);
    statesMenu->addAction(invite);
    statesMenu->addAction(cashless);
    statesMenu->addAction(free);

    statesMenu->popup(place->mapToGlobal(pos));
}
void MainWindow::on_smallHallMenu_sell_clicked()
{
    QAction* menuButton = qobject_cast<QAction*>(sender());
    QMenu* menu = qobject_cast<QMenu*>(menuButton->parent());
    QPushButton* place = qobject_cast<QPushButton*>(menu->parent());

    int r;
    int p;
    obtainConcertPlacePosition(Hall::SMALL, place, r, p);

    Concert* curr = getCurrentConcert(Hall::SMALL);

    int state = curr->placeStates[r-1][p-1];
    if (state == State::SELL) return;

    updatePlaceState(Hall::SMALL, place, State::SELL);
    curr->placeStates[r-1][p-1] = State::SELL;

    if (state == State::INVITE) curr->invitesCount--;
    if (state == State::CASHLESS) curr->cashlessCount--;
    curr->sellCount++;

    Scheme* s = getScheme(Hall::SMALL, curr->scheme);

    QColor placeColor = s->placeColors[r-1][p-1];

    int colorIndex;
    for (size_t i = 0; i < Theme::colors.size(); ++i)
    {
        if (placeColor == Theme::colors[i])
        {
            colorIndex = i;
            break;
        }
    }

    int price = getColorPrice(Hall::SMALL, getCurrentConcertName(Hall::SMALL), colorIndex).toInt();

    sendToConcert(Hall::SMALL, getCurrentConcertName(Hall::SMALL), price);
    sendToCashbox(price);

    setButtonColor(place, Theme::color_overlay);
}
void MainWindow::on_smallHallMenu_invite_clicked()
{
    QAction* menuButton = qobject_cast<QAction*>(sender());
    QMenu* menu = qobject_cast<QMenu*>(menuButton->parent());
    QPushButton* place = qobject_cast<QPushButton*>(menu->parent());

    int r;
    int p;
    obtainConcertPlacePosition(Hall::SMALL, place, r, p);

    Concert* curr = getCurrentConcert(Hall::SMALL);
    Scheme* s = getScheme(Hall::LARGE, curr->scheme);

    int state = curr->placeStates[r-1][p-1];
    if (state == State::INVITE) return;

    updatePlaceState(Hall::SMALL, place, State::INVITE);
    curr->placeStates[r-1][p-1] = State::INVITE;

    if (state == State::SELL) curr->sellCount--;
    if (state == State::CASHLESS) curr->cashlessCount--;
    curr->invitesCount++;

    if (state == State::SELL)
    {
        QColor placeColor = s->placeColors[r-1][p-1];

        int colorIndex;
        for (size_t i = 0; i < Theme::colors.size(); ++i)
        {
            if (placeColor == Theme::colors[i])
            {
                colorIndex = i;
            }
        }

        int price = getColorPrice(Hall::SMALL, getCurrentConcertName(Hall::SMALL), colorIndex).toInt();

        extractFromConcert(Hall::SMALL, getCurrentConcertName(Hall::SMALL), price);
        extractFromCashbox(price);
    }

    setButtonColor(place, Theme::color_overlay);
}
void MainWindow::on_smallHallMenu_cashless_clicked()
{
    QAction* menuButton = qobject_cast<QAction*>(sender());
    QMenu* menu = qobject_cast<QMenu*>(menuButton->parent());
    QPushButton* place = qobject_cast<QPushButton*>(menu->parent());

    int r;
    int p;
    obtainConcertPlacePosition(Hall::SMALL, place, r, p);

    Concert* curr = getCurrentConcert(Hall::SMALL);
    Scheme* s = getScheme(Hall::SMALL, curr->scheme);

    int state = curr->placeStates[r-1][p-1];
    if (state == State::CASHLESS) return;

    updatePlaceState(Hall::SMALL, place, State::CASHLESS);
    curr->placeStates[r-1][p-1] = State::CASHLESS;

    if (state == State::SELL) curr->sellCount--;
    if (state == State::INVITE) curr->invitesCount--;
    curr->cashlessCount++;

    if (state == State::SELL)
    {
        QColor placeColor = getPlaceColor(Hall::SMALL, place);

        int colorIndex;
        for (size_t i = 0; i < Theme::colors.size(); ++i)
        {
            if (placeColor == Theme::colors[i])
            {
                colorIndex = i;
            }
        }

        int price = getColorPrice(Hall::SMALL, getCurrentConcertName(Hall::SMALL), colorIndex).toInt();

        extractFromConcert(Hall::SMALL, getCurrentConcertName(Hall::SMALL), price);
        extractFromCashbox(price);
    }

    setButtonColor(place, Theme::color_overlay);
}
void MainWindow::on_smallHallMenu_free_clicked()
{
    // получение кнопки
    QAction* menuButton = qobject_cast<QAction*>(sender());
    QMenu* menu = qobject_cast<QMenu*>(menuButton->parent());
    QPushButton* place = qobject_cast<QPushButton*>(menu->parent());

    int r;
    int p;
    obtainConcertPlacePosition(Hall::SMALL, place, r, p);

    Concert* curr = getCurrentConcert(Hall::SMALL);
    Scheme* s = getScheme(Hall::SMALL, curr->scheme);

    int state = curr->placeStates[r-1][p-1];
    if (state == State::FREE) return;

    updatePlaceState(Hall::SMALL, place, State::FREE);
    curr->placeStates[r-1][p-1] = State::FREE;

    if (state == State::SELL) curr->sellCount--;
    if (state == State::INVITE) curr->invitesCount--;
    if (state == State::CASHLESS) curr->cashlessCount--;

    // получение цвета
    QColor placeColor = getPlaceColor(Hall::SMALL, place);
    setButtonColor(place, placeColor);

    if (state != State::SELL) return;

    int colorIndex;
    for (size_t i = 0; i < Theme::colors.size(); ++i)
    {
        if (placeColor == Theme::colors[i])
        {
            colorIndex = i;
        }
    }

    int price = getColorPrice(Hall::SMALL, getCurrentConcertName(Hall::SMALL), colorIndex).toInt();

    extractFromConcert(Hall::SMALL, getCurrentConcertName(Hall::SMALL), price);
    extractFromCashbox(price);
}



void MainWindow::sendToConcert(const int &hall, const QString &name, const int &price)
{
    QSqlQuery query;
    QString queryPrepare;

    if (hall == Hall::LARGE)
    {
        queryPrepare = "select income from large_hall_concerts where name = " + makeSqlString(name);
    }
    else if (hall == Hall::SMALL)
    {
        queryPrepare = "select income from small_hall_concerts where name = " + makeSqlString(name);
    }
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    int currentIncome = query.value(0).toInt();

    int newIncome = currentIncome + price;

    if (hall == Hall::LARGE)
    {
        queryPrepare = "update large_hall_concerts set income = " + QString::number(newIncome) + " where name = " + makeSqlString(name);
    }
    else if (hall == Hall::SMALL)
    {
        queryPrepare = "update small_hall_concerts set income = " + QString::number(newIncome) + " where name = " + makeSqlString(name);
    }
    query.prepare(queryPrepare);
    query.exec();

    for (int i = 0; i < activeConcerts.size(); ++i)
    {
        Concert* c = &activeConcerts[i];
        if (c->name == name)
        {
            c->income = newIncome;
        }
    }
}
void MainWindow::extractFromConcert(const int &hall, const QString &name, const int &price)
{
    QSqlQuery query;
    QString queryPrepare;

    if (hall == Hall::LARGE)
    {
        queryPrepare = "select income from large_hall_concerts where name = " + makeSqlString(name);
    }
    else if (hall == Hall::SMALL)
    {
        queryPrepare = "select income from small_hall_concerts where name = " + makeSqlString(name);
    }
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    int currentIncome = query.value(0).toInt();

    int newIncome = currentIncome - price;

    if (hall == Hall::LARGE)
    {
        queryPrepare = "update large_hall_concerts set income = " + QString::number(newIncome) + " where name = " + makeSqlString(name);
    }
    else if (hall == Hall::SMALL)
    {
        queryPrepare = "update small_hall_concerts set income = " + QString::number(newIncome) + " where name = " + makeSqlString(name);
    }
    query.prepare(queryPrepare);
    query.exec();

    for (int i = 0; i < activeConcerts.size(); ++i)
    {
        Concert* c = &activeConcerts[i];
        if (c->name == name)
        {
            c->income = newIncome;
        }
    }
}


void MainWindow::on_Tab_managing_menu_currentChanged(int index)
{
    if (index == 0)
    {
        on_largeHall_opened();
    }
    else if (index == 1)
    {
        on_smallHall_opened();
    }
    else if (index == 2)
    {
        on_createConcert_opened();
    }
    else if (index == 3)
    {
        on_renameConcert_opened();
    }
    else if (index == 4)
    {
        on_cashbox_opened();
    }
}

void MainWindow::on_redact_menu_currentChanged(int index)
{
    if (index == 0)
    {
        on_largeHall_schemes_opened();
    }
    else if (index == 1)
    {
        on_smallHall_schemes_opened();
    }
}

// ПЕРЕИМЕНОВАТЬ КОНЦЕРТ
void MainWindow::on_renameConcert_clear_clicked()
{
    ui->renameConcert_renameField->clear();
}
void MainWindow::on_renameConcert_rename_clicked()
{
    int currentConcertIndex = ui->renameConcert_concerts->currentIndex();

    Concert* currentConcert = &activeConcerts[currentConcertIndex];

    renameConcert(*currentConcert, ui->renameConcert_renameField->toPlainText());

    ui->renameConcert_concerts->setItemText(currentConcertIndex, ui->renameConcert_renameField->toPlainText());

    ui->renameConcert_concerts->setCurrentIndex(currentConcertIndex);

    on_renameConcert_clear_clicked();
}
void MainWindow::renameConcert(Concert& concert, const QString& newName)
{
    QSqlQuery query;
    QString queryPrepare;
    if (concert.hall == Hall::LARGE)
    {
        queryPrepare = "UPDATE large_hall_concerts SET name = " + makeSqlString(newName) + " where name = " + makeSqlString(concert.name);
    }
    else if (concert.hall == Hall::SMALL)
    {
        queryPrepare = "UPDATE small_hall_concerts SET name = " + makeSqlString(newName) + " where name = " + makeSqlString(concert.name);
    }
    query.prepare(queryPrepare);
    query.exec();

    concert.name = newName;
}


// ВКЛАДКИ

void MainWindow::on_largeHall_opened()
{
    ui->currentConcertLargeHall->clear();

    QList<Concert*> concerts = getConcerts(Hall::LARGE);

    for (int i = 0; i < concerts.size(); ++i)
    {
        ui->currentConcertLargeHall->addItem(concerts[i]->name);
    }
}

void MainWindow::on_smallHall_opened()
{
    ui->currentConcertSmallHall->clear();

    QList<Concert*> concerts = getConcerts(Hall::SMALL);

    for (int i = 0; i < concerts.size(); ++i)
    {
        ui->currentConcertSmallHall->addItem(concerts[i]->name);
    }
}



void MainWindow::on_renameConcert_opened()
{
    ui->renameConcert_concerts->clear();
    ui->renameConcert_renameField->clear();

    for (int i = 0; i < activeConcerts.size(); ++i)
    {
        ui->renameConcert_concerts->addItem(activeConcerts[i].name);
    }
}

void MainWindow::on_cashbox_opened()
{
    // обновление состояния кассы
    updateCashboxState();

    // обновление списка концертов
    updateCashboxConcerts();

}

void MainWindow::on_createConcert_opened()
{
    on_createConcert_clear_clicked();
}

void MainWindow::on_largeHall_schemes_opened()
{
    ui->cb_schemes_largeHall->clear();

    for (int i = 0; i < schemes.size(); ++i)
    {
        if (schemes[i].hall == Hall::LARGE)
        {
            ui->cb_schemes_largeHall->addItem(QString::number(schemes[i].id));
        }
    }
}

void MainWindow::on_smallHall_schemes_opened()
{
    ui->comboBox->clear();

    for (int i = 0; i < schemes.size(); ++i)
    {
        if (schemes[i].hall == Hall::SMALL)
        {
            ui->comboBox->addItem(QString::number(schemes[i].id));
        }
    }
}

// создать концерт

void MainWindow::on_createConcert_clear_clicked()
{
    ui->dateEdit->setDate(QDate::currentDate());
    ui->timeEdit->setTime(QTime::currentTime());
    ui->nameEdit->clear();
    ui->hallEdit->setCurrentIndex(0);
}


// касса

void MainWindow::setupCashbox()
{
    ui->cashbox_limit->setAlignment(Qt::AlignRight);
    ui->cashbox_invites->setAlignment(Qt::AlignRight);


    on_cashbox_opened();
}

void MainWindow::sendToCashbox(const int& price)
{
    QSqlQuery query;
    QString queryPrepare = "select state from cashbox";
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    int actualState = query.value(0).toInt();

    qDebug() << actualState;

    queryPrepare = "update cashbox set state = " + QString::number(actualState + price);
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    updateCashboxState();
}
void MainWindow::extractFromCashbox(const int& price)
{
    QSqlQuery query;
    QString queryPrepare = "select state from cashbox";
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    int actualState = query.value(0).toInt();

    queryPrepare = "update cashbox set state = " + QString::number(actualState - price);
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    updateCashboxState();
}

void MainWindow::updateCashboxState()
{
    QSqlQuery query;
    QString queryPrepare = "select state from cashbox";
    query.prepare(queryPrepare);
    query.exec();
    query.next();

    int actualState = query.value(0).toInt();

    ui->cashbox_state->setText(QString::number(actualState));
    ui->cashbox_state->setAlignment(Qt::AlignRight);
}
void MainWindow::updateCashboxConcerts()
{
    // очистка концертов в "Сдача выручки"
    ui->cashbox_income->clear();
    // очистка концертов в "Отчеты"
    ui->cashbox_reports->clear();

    // добавление концертов в combo box
    for (int i = 0; i < activeConcerts.size(); ++i)
    {
        ui->cashbox_income->addItem(activeConcerts[i].build());
    }
    for (int i = 0; i < inactiveConcerts.size(); ++i)
    {
        ui->cashbox_reports->addItem(inactiveConcerts[i].build());
    }
}
void MainWindow::updateCashboxRemain()
{
    int concertIndex = ui->cashbox_income->currentIndex();
    if (concertIndex != -1)
    {
        Concert* currentConcert = &activeConcerts[concertIndex];

        int remain = currentConcert->income - currentConcert->passedIncome;

        ui->cashbox_remain->setText(QString::number(remain));
    }
    else
    {
        ui->cashbox_remain->setText("");
    }

    ui->cashbox_remain->setAlignment(Qt::AlignRight);
}

void MainWindow::on_cashbox_income_currentIndexChanged(int index)
{
    if (index == -1) return;

    Concert* currentConcert = &activeConcerts[index];

    ui->cashbox_income->setItemText(index, currentConcert->build());

    updateCashboxRemain();
    ui->cashbox_passValue->clear();
}
void MainWindow::on_cashbox_reports_currentIndexChanged(int index)
{
    if (index == -1)
    {
        if (ui->cashbox_reports->count() > 0) {
            index = 0;
        } else {
            ui->cashbox_sellCount->setText("");
            ui->cashbox_cashlessCount->setText("");
            ui->cashbox_invitesCount->setText("");
            return;
        }
    }

    qDebug() << "qqq";

    Concert* currConcert = &inactiveConcerts[index];

    ui->cashbox_sellCount->setText(QString::number(currConcert->sellCount));
    ui->cashbox_cashlessCount->setText(QString::number(currConcert->cashlessCount));
    ui->cashbox_invitesCount->setText(QString::number(currConcert->invitesCount));
}

void MainWindow::on_cashbox_pass_clicked()
{
    // добавить проверку на пустое поле

    int remain = ui->cashbox_remain->toPlainText().toInt();
    int wannaPass = ui->cashbox_passValue->toPlainText().toInt();

    if (wannaPass <= remain)
    {
        int concertIndex = ui->cashbox_income->currentIndex();
        Concert* currentConcert = &activeConcerts[concertIndex];

        int newPass = currentConcert->passedIncome + wannaPass;

        QSqlQuery query;
        QString queryPrepare = "update ";
        if (currentConcert->hall == Hall::LARGE)
        {
            queryPrepare += "large_hall_concerts ";
        }
        else if (currentConcert->hall == Hall::SMALL)
        {
            queryPrepare += "small_hall_concerts ";
        }
        queryPrepare += "set passed_income = " + QString::number(newPass) + " where name = " + makeSqlString(currentConcert->name);

        query.prepare(queryPrepare);
        query.exec();

        currentConcert->passedIncome = newPass;

        on_cashbox_income_currentIndexChanged(concertIndex);

        // если хотим сдать столько же, сколько и осталось
        if (wannaPass == remain)
        {
            // проверка, закончился ли концерт
            QDate currDate = QDate::currentDate();
            QTime currTime = QTime::currentTime();

            QDate concertDate = currentConcert->date;
            QTime concertTime = currentConcert->time;

            qDebug() << concertDate;
            qDebug() << concertTime;

            qDebug() << currDate;
            qDebug() << currTime;

            if (currDate > concertDate)
            {
                // деактивируем в базе данных
                currentConcert->passDate = currDate;
                deactivateConcert(*currentConcert);

                updateCashboxConcerts();

                updateCashboxRemain();
                ui->cashbox_passValue->clear();
            }
            else if (currDate == concertDate)
            {
                if (currTime > concertTime)
                {
                    // деактивируем в базе данных
                    currentConcert->passDate = currDate;
                    deactivateConcert(*currentConcert);

                    updateCashboxConcerts();

                    updateCashboxRemain();
                    ui->cashbox_passValue->clear();
                }
            }
        }

        // забираем деньги из кассы
        extractFromCashbox(wannaPass);
    }
    else
    {
        ui->cashbox_passValue->clear();
    }

}
void MainWindow::on_cashbox_ticketsReport_clicked()
{
    int index = ui->cashbox_reports->currentIndex();
    if (index == -1) return;

    Concert* concert = &inactiveConcerts[index];

    QString reportFileName = concert->reportName();

    QFile file(reportsFolderPath + reportFileName);

    if (file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream ts(&file);
        ts << concert->htmlReport();
    }

    file.close();
}
void MainWindow::on_cashbox_revenueReport_clicked()
{
    int index = ui->cashbox_reports->currentIndex();
    if (index == -1) return;

    Concert* concert = &inactiveConcerts[index];

    QString reportFileName = concert->reportOfSalesName();

    QFile file(reportsFolderPath + reportFileName);

    if (file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream ts(&file);
        ts << concert->reportOfSalesHtml();
    }

    file.close();
}

void MainWindow::deactivateConcert(Concert& concert)
{
    QSqlQuery query;
    QString queryPrepare = "update ";
    if (concert.hall == Hall::LARGE)
    {
        queryPrepare += "large_hall_concerts ";
    }
    else if (concert.hall == Hall::SMALL)
    {
        queryPrepare += "small_hall_concerts ";
    }
    queryPrepare += "set is_active = 'false', ";
    queryPrepare += "pass_date = " + makeSqlString(concert.date.toString("yyyy-MM-dd"));
    queryPrepare += " where name = " + makeSqlString(concert.name);
    query.prepare(queryPrepare);
    query.exec();

    concert.isActive = false;

    activeConcerts.removeOne(concert);
    inactiveConcerts.push_back(concert);
}




// отчеты




QMap<QPushButton*, QColor> buttonColors;


















































//

