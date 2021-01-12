#include "databasetest.h"
#include "ui_databasetest.h"

#include<QDebug>
#include<QFileDialog>

#include "mysqldatabase.h"
#include "dbusermanage.h"

inline QStringList convertPermissionToString(unsigned int iPermission)
{
    QStringList strPerms;

    if(iPermission&PERMISSION_ADD)
    {
        strPerms << "insert";
    }
    if(iPermission&PERMISSION_DELETE)
    {
        strPerms << "delete";
    }
    if(iPermission&PERMISSION_UPDATE)
    {
        strPerms << "update";
    }
    if(iPermission&PERMISSION_SELECT)
    {
        strPerms << "select";
    }

    return strPerms;
}

DataBaseTest::DataBaseTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataBaseTest)
{
    ui->setupUi(this);
}

DataBaseTest::~DataBaseTest()
{
    delete ui;
}

void DataBaseTest::updateUserWidget()
{
    ui->listWidget_user->clear();

    //QStringList strUsers = DBUserManage::GetInstance().GetALLUser();

    QStringList strUsers = m_sdb.GetAllUser();

    ui->listWidget_user->addItems(strUsers);
}

void DataBaseTest::ShowDBS()
{
    ui->listWidget_dbs->clear();
    //QStringList strDBs = MysqlDataBase::GetInstance().GetDataBases();

    QStringList strDBs = m_sdb.GetDataBases();

    ui->listWidget_dbs->addItems(strDBs);
}

void DataBaseTest::ShowTables()
{
    ui->listWidget_tables->clear();
    ui->listWidget_user->clear();

    QStringList strTables = m_sdb.GetTables();

    ui->listWidget_user->addItems(m_sdb.GetAllUser());
    ui->listWidget_tables->addItems(strTables);
}



void DataBaseTest::ShowGrant(QString strUser)
{
    ui->listWidget_permission->clear();

    //QStringList strGrant = convertPermissionToString( m_sdb.GetTablePermissionForUser(strUser, ));

    //ui->listWidget_permission->addItems(strGrant);
}

void DataBaseTest::on_pushButton_login_clicked()
{

    ShowDBS();

    //updateUserWidget();
}

void DataBaseTest::on_listWidget_dbs_itemClicked(QListWidgetItem *item)
{
    //qDebug() << item->text();
    //MysqlDataBase::GetInstance().SetDBName(item->text());

    //ShowTables();
    m_sdb.OpenDB(item->text(), ui->lineEdit_userName->text(), ui->lineEdit_password->text());

    ShowTables();
}

void DataBaseTest::on_pushButton_add_clicked()
{
    m_sdb.AddUser(ui->lineEdit_userName2->text(), ui->lineEdit_password2->text());

    updateUserWidget();
}

void DataBaseTest::on_listWidget_user_itemClicked(QListWidgetItem *item)
{
    ShowGrant(item->text());

    DBPermission dbp = PERMISSION_NO;
    if(ui->radioButton_all->isChecked())
    {
        dbp = m_sdb.GetTablePermissionForUser(item->text(), "all");
    }
    else
    {
        QListWidgetItem *tableItem = ui->listWidget_tables->currentItem();
        if(tableItem != nullptr)
        {
            dbp = m_sdb.GetTablePermissionForUser(item->text(), ui->listWidget_tables->currentItem()->text());
        }
    }

    //if(dbp&PERMISSION_ADD)
    {
        ui->checkBox_insert->setChecked(dbp&PERMISSION_ADD);
    }
    //if(dbp&PERMISSION_DELETE)
    {
        ui->checkBox_delete->setChecked(dbp&PERMISSION_DELETE);
    }
    //if(dbp&PERMISSION_UPDATE)
    {
        ui->checkBox_update->setChecked(dbp&PERMISSION_UPDATE);
    }
    //if(dbp&PERMISSION_SELECT)
    {
        ui->checkBox_select->setChecked(dbp&PERMISSION_SELECT);
    }
}

void DataBaseTest::on_listWidget_tables_itemClicked(QListWidgetItem *item)
{
    ui->listWidget_permission->clear();

    DBPermission dbp = m_sdb.GetPermissionOfTableForCurrentUser(item->text());

    ui->listWidget_permission->addItems(convertPermissionToString(dbp));
}

void DataBaseTest::on_pushButton_removeUser_clicked()
{
    //m_sdb.DeleteUser(ui->listWidget_user->currentItem()->text());

    updateUserWidget();
}

void DataBaseTest::on_pushButton_apply_clicked()
{
    DBPermission dbp = PERMISSION_NO;
    if(ui->checkBox_insert->isChecked())
    {
        dbp |= PERMISSION_ADD;
    }
    if(ui->checkBox_delete->isChecked())
    {
        dbp |= PERMISSION_DELETE;
    }
    if(ui->checkBox_update->isChecked())
    {
        dbp |= PERMISSION_UPDATE;
    }
    if(ui->checkBox_select->isChecked())
    {
        dbp |= PERMISSION_SELECT;
    }

    if(ui->radioButton_all->isChecked())
    {
        m_sdb.SetALLPermissionForUser(ui->listWidget_user->currentItem()->text(), dbp);
    }
    else
    {
        m_sdb.SetTablePerMissionForUser(ui->listWidget_user->currentItem()->text(), ui->listWidget_tables->currentItem()->text(), dbp);
    }
}

void DataBaseTest::on_pushButton_addDB_clicked()
{
    m_sdb.OpenDB(ui->lineEdit_dbName->text(),ui->lineEdit_userName->text(), ui->lineEdit_password->text());

    ShowDBS();
}

void DataBaseTest::on_pushButton_deleteDB_clicked()
{
    if(ui->listWidget_dbs->currentRow() > -1)
    {
        m_sdb.DeleteDB(ui->listWidget_dbs->currentItem()->text());

        ShowDBS();
    }
}

void DataBaseTest::on_pushButton_clicked()
{
    QString sqlFile = QFileDialog::getOpenFileName(this, "open", "", "*.sql");
    //m_sdb.ExcuteSqlFile(sqlFile);

    m_sdb.InsertImage(sqlFile);

    sqlFile = sqlFile.left(sqlFile.size()-4) + "1.sql";

    m_sdb.GetImage(sqlFile);

    //ShowTables();
}
