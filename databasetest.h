#ifndef DATABASETEST_H
#define DATABASETEST_H

#include <QWidget>
#include <QListWidgetItem>

#include "sqlitedatabase.h"

#include "mysqldatabase.h"

namespace Ui {
class DataBaseTest;
}

class DataBaseTest : public QWidget
{
    Q_OBJECT

public:
    explicit DataBaseTest(QWidget *parent = nullptr);
    ~DataBaseTest();

private slots:
    void on_pushButton_login_clicked();

    void on_listWidget_dbs_itemClicked(QListWidgetItem *item);

    void on_pushButton_add_clicked();

    void on_listWidget_user_itemClicked(QListWidgetItem *item);

    void on_listWidget_tables_itemClicked(QListWidgetItem *item);

    void on_pushButton_removeUser_clicked();

    void on_pushButton_apply_clicked();

    void on_pushButton_addDB_clicked();

    void on_pushButton_deleteDB_clicked();

    void on_pushButton_clicked();

private:
    void updateUserWidget();
    void ShowDBS();
    void ShowTables();
    void ShowGrant(QString strUser);

private:
    Ui::DataBaseTest *ui;

    SqliteDataBase m_sdb;

    //MysqlDataBase m_mdb;
};

#endif // DATABASETEST_H
