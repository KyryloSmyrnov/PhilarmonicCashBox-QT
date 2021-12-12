#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <stdio.h>
#include <stdlib.h>
#include <QMainWindow>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QGridLayout>
#include <QVBoxLayout>


#include <hall.h>
#include <state.h>
#include <concert.h>
#include "scheme.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // ПОДГРУЗКА ВСЕХ КОНЦЕРТОВ

    QList<Concert> activeConcerts;
    QList<Concert> inactiveConcerts;

    void loadConcerts();

    // ПОДГРУЗКА ВСЕХ СХЕМ

    QList<Scheme> schemes;
    void loadSchemes();

    Scheme* getScheme(const int& hall, const int& id);
    QList<Concert*> getConcerts(const int& hall);

    Concert* getCurrentConcert(const int& hall);

    void obtainConcertPlacePosition(const int& hall, const QPushButton* placeButton, int& row, int& place);

    //

    void createReportsFolder();


    QVariant getColorPrice(const int& hall, const QString& concertName, const int& colorIndex);

    void obtainConcertData();

    void setupConcertScene(const int& hall);
    QAction* createStateAction(const QString& text, QObject* parent);

    // табы
    void on_largeHall_opened();
    void on_smallHall_opened();
    void on_renameConcert_opened();

    void on_createConcert_opened();

    void on_cashbox_opened();

    void on_largeHall_schemes_opened();
    void on_smallHall_schemes_opened();


    void renameConcert(Concert& concert, const QString& newName);

    void setupCashbox();

    void sendToCashbox(const int& price);
    void extractFromCashbox(const int& price);

    void updateCashboxState();
    void updateCashboxConcerts();
    void updateCashboxRemain();

    void sendToConcert(const int& hall, const QString& name, const int& price);
    void extractFromConcert(const int& hall, const QString& name, const int& price);

    // диактивизация концерта
    void deactivateConcert(Concert& concert);

private slots:

    void on_concert_created();

    void onRowClick();

    void onPlaceClick();

    // большой зал

    void clearCursorColor();

    void setupLargeHall();
    void setupColors(const QGridLayout* colors);
    void onColorClick();

    void setupRows(const QVBoxLayout* rows);
    void onRowClick2();

    void onClearSchemeClick();

    void on_SmallHallRedactScheme_clicked();

    void on_SmallHallCreateScheme_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_hallEdit_currentIndexChanged(int index);

    void on_shemeEdit_currentIndexChanged(int index);

    void on_createConcertButton_clicked();

    void on_createScheme_largeHall_clicked();

    void on_editScheme_largeHall_clicked();

    void on_cb_schemes_largeHall_currentIndexChanged(int index);


    // ОБЩИЕ МЕТОДЫ
    QString getCurrentConcertName(const int& hall);
    int getConcertScheme(const int& hall);
    void clearCategories(const int& hall);

    QString getPlaceStateString(const int& hall, const QPushButton* place);
    QString getPlaceColorString(const int& hall, const QPushButton* place);

    void updatePlaceState(const int& hall, const QPushButton* place, const int& state);
    QString createStateString(const int& row, const int& place);
    QString createColorString(const int& row, const int& place);

    QColor getPlaceColor(const int& hall, const QPushButton* place);
    QColor getPlaceColor(const int& hall, const int& row, const int& place);

    int getPlaceState(const int& hall, const QPushButton* place);
    int getPlaceState(const int& hall, const int& row, const int& place);

    void on_currentConcertLargeHall_currentIndexChanged(int index);
    void on_largeHallConcertPlace_clicked(const QPoint&);
    void on_largeHallMenu_sell_clicked();
    void on_largeHallMenu_invite_clicked();
    void on_largeHallMenu_cashless_clicked();
    void on_largeHallMenu_free_clicked();

    void on_currentConcertSmallHall_currentIndexChanged(int index);
    void on_smallHallConcertPlace_clicked(const QPoint&);
    void on_smallHallMenu_sell_clicked();
    void on_smallHallMenu_invite_clicked();
    void on_smallHallMenu_cashless_clicked();
    void on_smallHallMenu_free_clicked();


    void on_Tab_managing_menu_currentChanged(int index);

    void on_cashbox_income_currentIndexChanged(int index);

    void on_cashbox_pass_clicked();

    void on_renameConcert_clear_clicked();

    void on_renameConcert_rename_clicked();

    void on_cashbox_reports_currentIndexChanged(int index);

    void on_cashbox_ticketsReport_clicked();

    void on_cashbox_revenueReport_clicked();

    void on_redact_menu_currentChanged(int index);

    void on_createConcert_clear_clicked();

private:

    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlTableModel* model;
    QSqlTableModel* modelCategories;
    QSqlTableModel* modelCurrentConcert;
};


#endif // MAINWINDOW_H

















































//
