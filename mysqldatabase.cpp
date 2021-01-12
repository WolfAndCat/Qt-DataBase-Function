#include "mysqldatabase.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

MysqlDataBase::MysqlDataBase()
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
    //SetLoginInfo("192.168.29.3", 3306, "testUser","123456");
    //SetLoginInfo("127.0.0.1", 3306, "root","123456");
    //SetTargetDBName("test_schema");
}

MysqlDataBase::~MysqlDataBase()
{

}

void MysqlDataBase::SetLoginInfo(QString strIP, int iPort, QString strUser, QString strPassword)
{
    m_strCurrentUser = strUser;

    m_db.setHostName(strIP);
    m_db.setPort(iPort);
    m_db.setUserName(strUser);
    m_db.setPassword(strPassword);

    reConnect();
}

void MysqlDataBase::SetIP(QString strIP)
{
    m_db.setHostName(strIP);
}

void MysqlDataBase::SetPort(int iPort)
{
    m_db.setPort(iPort);
}

void MysqlDataBase::SetUser(QString strUser, QString strPassword)
{
    m_strCurrentUser = strUser;
    m_db.setUserName(strUser);
    m_db.setPassword(strPassword);
    reConnect();
}

void MysqlDataBase::SetDBName(QString strDBName)
{
    m_db.setDatabaseName(strDBName);
    reConnect();
}

bool MysqlDataBase::ConnectToDataBase()
{
    bool bSuccess = m_db.open();
    if(bSuccess)
    {
        qDebug() << "success";
    }
    else
    {
        qDebug() << m_db.lastError().text();
    }

    return bSuccess;
}

QString MysqlDataBase::GetCurrentUser()
{
    return m_strCurrentUser;
}

bool MysqlDataBase::creatDB(QString strDBName)
{
    if(DBOperationAllow())
    {
        return ExecSqlString(QString("create database ") + strDBName + ";");
    }
    else
    {
        return false;
    }
}

bool MysqlDataBase::DeleteDB(QString strDBName)
{
    if(DBOperationAllow())
    {
        return ExecSqlString(QString("drop database ") + strDBName + ";");
    }
    else
    {
        return false;
    }
}

QStringList MysqlDataBase::GetDataBases()
{
    QStringList strDataBases;
    QStringList mySqlDBs;
    mySqlDBs << "information_schema" << "performance_schema" << "mysql" << "sakila" << "sys" << "world";
    QSqlQuery query;
    if(!query.exec("show databases"))
    {
        qDebug() << query.lastError().text();
    }
    else
    {
        while (query.next())
        {
            QString strDB = query.value(0).toString();
            if(!mySqlDBs.contains(strDB))
            {
                strDataBases << strDB;
            }
        }
    }

    return strDataBases;
}

QStringList MysqlDataBase::GetTables()
{
    QStringList strTables;
    QStringList mySqlDBs;
    mySqlDBs << "information_schema" << "performance_schema" << "mysql" << "sakila" << "sys" << "world";
    QSqlQuery query;
    if(!query.exec("show tables"))
    {
        qDebug() << query.lastError().text();
    }
    else
    {
        while (query.next())
        {
            QString strDB = query.value(0).toString();
            if(!mySqlDBs.contains(strDB))
            {
                strTables << strDB;
            }
        }
    }

    return strTables;
}

void MysqlDataBase::reConnect()
{
    if(m_db.isOpen())
    {
        m_db.close();
    }

    bool bSuccess = m_db.open();
    if(bSuccess)
    {
        qDebug() << "success";
    }
    else
    {
        qDebug() << m_db.lastError().text();
    }
}

QStringList MysqlDataBase::GetALLUser()
{
    QStringList strUsers;

    SearchDataRow sdr = SearchForRow(QStringList() << "user" << "host", "mysql.user");

    for(auto sr : sdr)
    {
        QString strUser = sr.at(0).toString();

        if(strUser.left(6) != "mysql.")
        {
            strUsers << strUser;
        }
    }

    for(auto strUser: strUsers)
    {
        qDebug() << strUser;
    }

    return strUsers;
}

QStringList MysqlDataBase::GetCurrentGrants()
{
    QStringList listGrants;
    SearchDataCol sdc = ExecSqlAndGetColumnData("show grants");
    if(sdc.size() == 0)
    {
        return listGrants;
    }

    QList<QVariant> dataList = sdc[0];

    if(dataList.size() > 0)
    {
        QString strGant = dataList.at(0).toString().toLower();

        if(strGant.contains("select"))
        {
            listGrants.append("select");
        }

        if(strGant.contains("insert"))
        {
            listGrants.append("insert");
        }

        if(strGant.contains("delete"))
        {
            listGrants.append("delete");
        }

        if(strGant.contains("update"))
        {
            listGrants.append("update");
        }
    }

    return listGrants;
}

QStringList MysqlDataBase::GetGrantsForUser(QString strUser, QString strHost)
{
    QStringList listGrants;
    QString strSql = "show grants for ";
    strSql.append(strUser);
    strSql.append("@");
    strSql.append(strHost);
    strSql.append(";");

    SearchDataCol sdc = ExecSqlAndGetColumnData(strSql);
    if(sdc.size() == 0)
    {
        return listGrants;
    }

    QList<QVariant> dataList = sdc[0];

    for(auto data: dataList)
    {
        QString strGant = data.toString().toLower();

        if(strGant.contains("select"))
        {
            listGrants.append("select");
        }

        if(strGant.contains("insert"))
        {
            listGrants.append("insert");
        }

        if(strGant.contains("delete"))
        {
            listGrants.append("delete");
        }

        if(strGant.contains("update"))
        {
            listGrants.append("update");
        }
    }


    return listGrants;
}

inline QString convertPermissionFromIntToString(unsigned int iPermission)
{
    QString strPerm;
    bool bFirst = true;

    if(iPermission&PERMISSION_ADD)
    {
        strPerm = "insert";
    }
    if(iPermission&PERMISSION_DELETE)
    {
        if(bFirst)
        {
            strPerm = "delete";
        }
        else
        {
            strPerm.append(", delete");
        }
    }
    if(iPermission&PERMISSION_UPDATE)
    {
        if(bFirst)
        {
            strPerm = "update";
        }
        else
        {
            strPerm.append(", update");
        }
    }
    if(iPermission&PERMISSION_SELECT)
    {
        if(bFirst)
        {
            strPerm = "select";
        }
        else
        {
            strPerm.append(", select");
        }
    }

    return strPerm;
}

void MysqlDataBase::AddUser(QString strName, QString strPassword, QString strHost)
{
    QString strSql;
    strSql = "create user ";
    strSql.append(strName);
    strSql.append("@");
    strSql.append(strHost);
    strSql.append(" identified by '");
    strSql.append(strPassword);
    strSql.append("';");

    ExecSqlString(strSql);
}

void MysqlDataBase::RemoveUser(QString strName, QString strHost)
{
    QString strSql;
    strSql = "drop user ";
    strSql.append(strName);
    strSql.append("@");
    strSql.append(strHost);
    strSql.append(";");

    ExecSqlString(strSql);
}

void MysqlDataBase::ResetPassword(QString strName, QString strNewPassword, QString strHost)
{
    QString strSql;

    if(strName == m_strCurrentUser)
    {
        strSql = "set password = password('";
        strSql.append(strNewPassword);
        strSql.append("');");
    }
    else
    {
        strSql = "set password for ";
        strSql.append(strName);
        strSql.append("@");
        strSql.append(strHost);
        strSql.append(" = '");
        strSql.append(strNewPassword);
        strSql.append("';");
    }

    ExecSqlString(strSql);
}

void MysqlDataBase::SetPermissionToUser(QString strName, unsigned int iPermission, QString strTable, QString strHost)
{
    QString strSql;

    strSql = "grant ";
    strSql.append(convertPermissionFromIntToString(iPermission));
    strSql.append(" on ");
    strSql.append(strTable);
    strSql.append(" to ");
    strSql.append(strName);
    strSql.append("@");
    strSql.append(strHost);
    strSql.append(";");

    ExecSqlString(strSql);
}

void MysqlDataBase::RevokePermissionToUser(QString strName, unsigned int iPermission, QString strTable, QString strHost)
{
    QString strSql;

    strSql = "revoke ";
    strSql.append(convertPermissionFromIntToString(iPermission));
    strSql.append(" on ");
    strSql.append(strTable);
    strSql.append(" from ");
    strSql.append(strName);
    strSql.append("@");
    strSql.append(strHost);
    strSql.append(";");

    ExecSqlString(strSql);
}
