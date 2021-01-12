#ifndef MYSQLDATABASE_H
#define MYSQLDATABASE_H

#include "databasefunction.h"

class MysqlDataBase : public DataBaseFunction
{
public:
    MysqlDataBase();
    virtual ~MysqlDataBase();

public:
    void SetLoginInfo(QString strIP, int iPort, QString strUser, QString strPassword);
    void SetIP(QString strIP);
    void SetPort(int iPort);
    void SetUser(QString strUser, QString strPassword);
    void SetDBName(QString strDBName);

    bool ConnectToDataBase();
    QString GetCurrentUser();

    bool creatDB(QString strDBName);
    bool DeleteDB(QString strDBName);
    QStringList GetDataBases();
    virtual QStringList GetTables();

    QStringList GetALLUser();
    QStringList GetCurrentGrants();
    QStringList GetGrantsForUser(QString strUser, QString strHost = "localhost");

    void AddUser(QString strName, QString strPassword, QString strHost = "localhost");
    void RemoveUser(QString strName, QString strHost = "localhost");
    void ResetPassword(QString strName, QString strNewPassword, QString strHost = "localhost");

    void SetPermissionToUser(QString strName, unsigned int iPermission, QString strTable = "*.*", QString strHost = "localhost");
    void RevokePermissionToUser(QString strName, unsigned int iPermission, QString strTable = "*.*", QString strHost = "localhost");

private:
    void reConnect();

private:
    QString m_strCurrentUser;
};

#endif // DATABASEFUNCTION_H
