#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include "databasefunction.h"

class SqliteDataBase : public DataBaseFunction
{
public:
    SqliteDataBase();
    virtual ~SqliteDataBase();

    virtual SearchDataRow SearchForRow(const QStringList& strItems, const QString& strTable, const QString& strCondition = "true");
    virtual QList<ColData> SearchForCol(const QStringList& strItems, const QString& strTable, const QString& strCondition = "true");

    virtual bool InsertData(QString strTable, QList<QVariant> datas, QStringList strColNames = QStringList());
    virtual bool DeleteDataWithCondition(QString strTable, QString strCondition);
    virtual bool UpdateData(QString strTable, QString strChange, QString strCondition);

    bool OpenDB(QString strDBName, QString strUser, QString strPassword);
    bool DeleteDB(QString strDBName);

    QStringList GetTables();
    QStringList GetDataBases();

    bool AddUser(QString strUser, QString strPassword);
    QStringList GetAllUser();
    bool DeleteUser(QString strUser);

    DBPermission GetTablePermissionForUser(QString strUser, QString strTable);
    DBPermission GetPermissionOfTableForCurrentUser(QString strTable);
    void SetALLPermissionForUser(QString strUserName, DBPermission dbp);
    void SetTablePerMissionForUser(QString strUserName, QString strTableName, DBPermission dbp);

    bool InsertImage(QString strFile);
    bool GetImage(QString strFile);

protected:
    bool GetManagerInfo();
    bool UserLogin();

private:
    bool ManagerTableExisted();
    bool CreateDBUserTable();
    void CloseUsedDB();

    QString AdjuestDBPath(QString strDBName);

protected:
    QString m_strUser;
    QString m_strPassword;
    QString m_strCurrentDB;

    QMap<QString, DBPermission> m_mapPermissionWithTable;
};

#endif // SQLITEDATABASE_H
