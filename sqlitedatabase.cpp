#include "sqlitedatabase.h"

#include <QtDebug>
#include <QSqlError>
#include <QApplication>
#include <QDir>
#include <QSqlDriver>

inline bool IsSystemTable(QString strTable)
{
    if(strTable != "sys_role_permission" && strTable != "sys_user_manager")
    {
        return false;
    }

    return true;
}

SqliteDataBase::SqliteDataBase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_strCurrentDB = "";
    m_mapPermissionWithTable.clear();
}

SqliteDataBase::~SqliteDataBase()
{
    if(m_db.isOpen())
    {
        m_db.close();
    }
}

SearchDataRow SqliteDataBase::SearchForRow(const QStringList &strItems, const QString& strTable, const QString& strCondition)
{
    DBPermission dbp = GetPermissionOfTableForCurrentUser(strTable);

    if(dbp & PERMISSION_SELECT)
    {
        return DataBaseFunction::SearchForRow(strItems, strTable, strCondition);
    }
    else
    {
        qDebug() << "you have no search permission";
        return SearchDataRow();
    }
}

QList<ColData> SqliteDataBase::SearchForCol(const QStringList &strItems, const QString& strTable, const QString& strCondition)
{
    DBPermission dbp = GetPermissionOfTableForCurrentUser(strTable);

    if(dbp & PERMISSION_SELECT)
    {
        return DataBaseFunction::SearchForCol(strItems, strTable, strCondition);
    }
    else
    {
        qDebug() << "you have no search permission";
        return SearchDataRow();
    }
}

bool SqliteDataBase::InsertData(QString strTable, QList<QVariant> datas, QStringList strColNames)
{
    DBPermission dbp = GetPermissionOfTableForCurrentUser(strTable);

    if(dbp & PERMISSION_ADD)
    {
        return DataBaseFunction::InsertData(strTable, datas, strColNames);
    }
    else
    {
        qDebug() << "you have no insert permission";
        return false;
    }
}

bool SqliteDataBase::DeleteDataWithCondition(QString strTable, QString strCondition)
{
    DBPermission dbp = GetPermissionOfTableForCurrentUser(strTable);

    if(dbp & PERMISSION_DELETE)
    {
        return DataBaseFunction::DeleteDataWithCondition(strTable, strCondition);
    }
    else
    {
        qDebug() << "you have no delete permission";
        return false;
    }
}

bool SqliteDataBase::UpdateData(QString strTable, QString strChange, QString strCondition)
{
    DBPermission dbp = GetPermissionOfTableForCurrentUser(strTable);

    if(dbp & PERMISSION_UPDATE)
    {
        return DataBaseFunction::UpdateData(strTable, strChange, strCondition);
    }
    else
    {
        qDebug() << "you have no update permission";
        return false;
    }
}

bool SqliteDataBase::OpenDB(QString strDBName, QString strUser, QString strPassword)
{
    CloseUsedDB();
    m_strUser = strUser;
    m_strPassword = strPassword;
    m_strCurrentDB = strDBName;

    QString strPath = AdjuestDBPath(strDBName);
    m_db.setDatabaseName(strPath);    

    bool bSuccess = false;
    if(UserLogin())
    {
        bSuccess = true;
    }
    else
    {
        qDebug() << "login failed";
    }

    return bSuccess;
}

bool SqliteDataBase::DeleteDB(QString strDBName)
{
    if(strDBName == m_strCurrentDB && m_db.isOpen())
    {
        m_db.close();
    }

    if(QFile::remove(AdjuestDBPath(strDBName)))
    {
        return true;
    }
    else
    {
        qDebug() << "delete failed";
        return false;
    }
}

QStringList SqliteDataBase::GetTables()
{
    QStringList strTables;

    auto ite = m_mapPermissionWithTable.begin();

    for(; ite != m_mapPermissionWithTable.end(); ++ite)
    {
        if(ite.key() == "all")
        {
            SearchDataRow sdr = SearchForRow(QStringList() << "name", "sqlite_master", "type = 'table'");
            for(auto rd : sdr)
            {
                for(auto data: rd)
                {
                    QString strData = data.toString();
                    if(!IsSystemTable(strData))
                    {
                        strTables.append(data.toString());
                    }
                }
            }
        }
        else
        {
            strTables.append(ite.key());
        }
    }

    return strTables;
}

QStringList SqliteDataBase::GetDataBases()
{
    QDir dir(QApplication::applicationDirPath());
    QStringList strFiles = dir.entryList(QStringList() << "*.db", QDir::Files|QDir::Readable, QDir::Name);

    for(auto& file: strFiles)
    {
        file = file.left(file.size() - 3);
    }

    return strFiles;
}

bool SqliteDataBase::AddUser(QString strUser, QString strPassword)
{
    SearchDataRow sdr = ExecSqlAndGetRowData("select max(role_id) from sys_user_manager");
    if(sdr.size() == 1)
    {
        QString strId = std::to_string(sdr.at(0).at(0).toInt() + 1).data();
        return InsertData("sys_user_manager", QList<QVariant>() << QStringToSqlString(strUser) << QStringToSqlString(strPassword) << strId);
    }

    return false;
}

inline QStringList convertPermissionToStringList(unsigned int iPermission)
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

DBPermission SqliteDataBase::GetTablePermissionForUser(QString strUser, QString strTable)
{
    if(strUser.isEmpty() || strTable.isEmpty())
    {
        return PERMISSION_NO;
    }

    DBPermission dbp = GetPermissionOfTableForCurrentUser("sys_role_permission")&GetPermissionOfTableForCurrentUser("sys_user_manager");

    if((dbp & PERMISSION_SELECT) == PERMISSION_NO)
    {
        return PERMISSION_NO;
    }
    QString strSql = QString("select permission from sys_role_permission a, sys_user_manager b where a.role_id = b.role_id and b.name = %1 and a.table_name = %2")
                                                                                            .arg(QStringToSqlString(strUser))
                                                                                            .arg(QStringToSqlString(strTable));

    SearchDataRow sdr = ExecSqlAndGetRowData(strSql);
    if(sdr.size() > 0)
    {
        unsigned int iPermsission = sdr.at(0).at(0).toUInt();

        return iPermsission;
    }
    else
    {
        qDebug() << "no permission data";
    }
    return 0;
}

DBPermission SqliteDataBase::GetPermissionOfTableForCurrentUser(QString strTable)
{
    if( strTable.isEmpty())
    {
        return PERMISSION_NO;
    }

    auto ite = m_mapPermissionWithTable.find(strTable);
    if(ite != m_mapPermissionWithTable.end())
    {
        return  ite.value();
    }
    else
    {
        if(!IsSystemTable(strTable))
        {
            ite = m_mapPermissionWithTable.find("all");

            if(ite != m_mapPermissionWithTable.end())
            {
                return ite.value();
            }
        }
    }
    return PERMISSION_NO;
}

void SqliteDataBase::SetALLPermissionForUser(QString strUserName, DBPermission dbp)
{
    if(strUserName.isEmpty() || strUserName == m_strUser)
    {
        return;
    }

    SearchDataRow sdr = DataBaseFunction::SearchForCol(QStringList() << "role_id", "sys_user_manager", QString("name = ") + QStringToSqlString(strUserName));
    if(sdr.size() == 0)
    {
        return;
    }

    int iId = sdr.at(0).at(0).toInt();

    QString strCondition = QString("role_id = %1 and table_name != 'sys_role_permission' and table_name != 'sys_user_manager'").arg(iId);

    DeleteDataWithCondition("sys_role_permission", strCondition);

    InsertData("sys_role_permission", QList<QVariant>() << QString::fromStdString(std::to_string(iId))
                                                    << QStringToSqlString("all")
                                                    << QString::fromStdString(std::to_string(0x000f&dbp)));
}

void SqliteDataBase::SetTablePerMissionForUser(QString strUserName, QString strTableName, DBPermission dbp)
{
    if(strUserName.isEmpty() || strTableName.isEmpty() || strUserName == m_strUser)
    {
        return;
    }

    SearchDataRow sdr = DataBaseFunction::SearchForCol(QStringList() << "role_id", "sys_role_permission", QString("name = ") + QStringToSqlString(strUserName));
    if(sdr.size() == 0)
    {
        return;
    }

    int iId = sdr.at(0).at(0).toInt();

    if(!IsSystemTable(strTableName))
    {
        DeleteDataWithCondition("sys_role_permission", QString("user_name = ") + QStringToSqlString(strUserName) + " and table_name = 'all'");
    }

    sdr = SearchForRow(QStringList()<<"user_name", "dbUser_table", QString("role_id = ") + QString(iId) + " and table_name = " + QStringToSqlString(strTableName));
    if(sdr.size() > 0)
    {
        UpdateData("sys_role_permission", QString("permission = ") + QString::fromStdString(std::to_string(0x000f&dbp)), QString("role_id = ") + QString(iId) + " and table_name = " + QStringToSqlString(strTableName));
    }
    else
    {
        InsertData("sys_role_permission", QList<QVariant>() << QString(iId) << QStringToSqlString(strTableName) << QString::fromStdString(std::to_string(0x000f&dbp)));
    }
}

bool SqliteDataBase::InsertImage(QString strFile)
{
    if(DBOperationAllow())
    {
        QFile file(strFile);
        if(!file.open(QIODevice::ReadOnly)) return false;
        QByteArray ad = file.readAll();
        QVariant v = ad;

        QSqlQuery sq;
        sq.prepare("insert into image_table(file_Data) values(?)");
        sq.addBindValue(v);

        if(sq.exec())
        {
            return true;
        }
        else
        {
            qDebug() << sq.lastError();
            return false;
        }
    }
}

bool SqliteDataBase::GetImage(QString strFile)
{
    if(DBOperationAllow())
    {
        QSqlQuery sq("select file_data from image_table;");
        if(sq.next())
        {
            QByteArray array = sq.value(0).toByteArray();

            QFile f(strFile);
            if(f.open(QIODevice::WriteOnly))
            {
                f.write(array);
                return true;
            }
        }
    }
}

QStringList SqliteDataBase::GetAllUser()
{
    auto ite = m_mapPermissionWithTable.find("sys_user_manager");
    if(ite == m_mapPermissionWithTable.end())
    {
        return QStringList();
    }

    if(ite.value() & 0x0008)
    {
        QStringList strUsers;
        SearchDataRow sdr = SearchForRow(QStringList() << "name" << "role_id", "sys_user_manager");
        for(auto rd : sdr)
        {
            if(rd.size()>0)
            {
                QString strName = rd.at(0).toString();
                int iId = rd.at(1).toInt();
                if(strName != "root" || iId > 1)
                {
                    strUsers.append(rd.at(0).toString());
                }
            }
        }

        return strUsers;
    }

    return QStringList();
}

bool SqliteDataBase::DeleteUser(QString strUser)
{
    return DeleteDataWithCondition("sys_user_manager", QString("name = %1").arg(strUser));
}

bool SqliteDataBase::UserLogin()
{
    if(ManagerTableExisted())
    {
        return GetManagerInfo();
    }
    else
    {
        if(CreateDBUserTable())
        {
            return GetManagerInfo();
        }
    }
    return false;
}

bool SqliteDataBase::ManagerTableExisted()
{
    SearchDataRow sdr = DataBaseFunction::SearchForRow(QStringList() << "name", "sqlite_master", "type = 'table' and (name = 'sys_role_permission' or name = 'sys_user_manager')");

    if(sdr.size() == 2)
    {
        return true;
    }

    return false;
}

bool SqliteDataBase::CreateDBUserTable()
{
    QStringList strSqls;
    strSqls.append("CREATE TABLE sys_role_permission (role_id int NOT NULL, table_name varchar (20), permission int NOT NULL, PRIMARY KEY (role_id, table_name));");
    strSqls.append("INSERT INTO sys_role_permission (role_id, table_name, permission) VALUES (0, 'all', 15);");
    strSqls.append("INSERT INTO sys_role_permission (role_id, table_name, permission) VALUES (0, 'sys_role_permission', 15);");
    strSqls.append("INSERT INTO sys_role_permission (role_id, table_name, permission) VALUES (0, 'sys_user_manager', 15);");
    strSqls.append("INSERT INTO sys_role_permission (role_id, table_name, permission) VALUES (1, 'sys_role_permission', 15);");
    strSqls.append("INSERT INTO sys_role_permission (role_id, table_name, permission) VALUES (1, 'sys_user_manager', 15);");
    strSqls.append("INSERT INTO sys_role_permission (role_id, table_name, permission) VALUES (2, 'all', 15);");
    strSqls.append("CREATE TABLE sys_user_manager(name varchar(20) not null PRIMARY KEY, password varchar(50), role_id int not null);");
    strSqls.append("CREATE TRIGGER deletetrg BEFORE DELETE ON sys_user_manager BEGIN delete from sys_role_permission where old.role_id > 100 and role_id = old.role_id;  END;");
    strSqls.append("INSERT INTO sys_user_manager (name, password, role_id) VALUES ('root', '123456', 0);");
    strSqls.append("INSERT INTO sys_user_manager (name, password, role_id) VALUES ('manager', '123456', 1);");
    strSqls.append("INSERT INTO sys_user_manager (name, password, role_id) VALUES ('user', '123456', 2);");

    return ExecMitulSqlString(strSqls);
}

void SqliteDataBase::CloseUsedDB()
{
    if(m_db.isOpen())
    {
        m_db.close();
    }

    m_strCurrentDB = "";
    m_strUser = "";
    m_strPassword = "";

    m_mapPermissionWithTable.clear();
}

bool SqliteDataBase::GetManagerInfo()
{
    QString strCondition = QString("name = %1 and password = %2")
                                        .arg(QStringToSqlString(m_strUser))
                                        .arg(QStringToSqlString(m_strPassword));

    SearchDataRow sdr = DataBaseFunction::SearchForRow(QStringList() << "role_id", "sys_user_manager", strCondition);
    if(sdr.size() > 0)
    {
        int iRole = sdr.at(0).at(0).toInt();
        SearchDataRow sdr = DataBaseFunction::SearchForRow(QStringList() << "table_name" << "permission", "sys_role_permission", QString("role_id = %1").arg(iRole));
        for(auto rd : sdr)
        {
            m_mapPermissionWithTable.insert(rd.at(0).toString(), rd.at(1).toUInt());
        }
        return true;
    }

    return false;
}

QString SqliteDataBase::AdjuestDBPath(QString strDBName)
{
    QString strPath = "";
    if(!strDBName.contains(':'))
    {
        strPath = QApplication::applicationDirPath() + "/" + strDBName;
    }

    if(strDBName.right(3) != ".db")
    {
        strPath.append(".db");
    }

    return strPath;
}
