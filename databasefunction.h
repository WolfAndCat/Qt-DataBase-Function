#ifndef DATABASEFUNCTION_H
#define DATABASEFUNCTION_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include <QVariant>

#define PERMISSION_NO 0x0000
#define PERMISSION_ADD 0x0001
#define PERMISSION_DELETE 0x0002
#define PERMISSION_UPDATE 0x0004
#define PERMISSION_SELECT 0x0008

using ColData = QList<QVariant>;
using RowData = QList<QVariant>;

using SearchDataRow = QList<RowData>;
using SearchDataCol = QList<ColData>;

using DBPermission = unsigned int;

class DataBaseFunction
{
public:
    DataBaseFunction();
    virtual ~DataBaseFunction();

    virtual QList<RowData> SearchForRow(const QStringList& strItems, const QString &strTable, const QString &strCondition = "true");
    virtual QList<ColData> SearchForCol(const QStringList& strItems, const QString &strTable, const QString &strCondition = "true");

    virtual bool InsertData(QString strTable, QList<QVariant> datas, QStringList strColNames = QStringList());
    virtual bool DeleteDataWithCondition(QString strTable, QString strCondition);
    virtual bool UpdateData(QString strTable, QString strChange, QString strCondition);

    QString QStringToSqlString(QString strSrc);

    bool ExcuteSqlFile(QString strFile);

    virtual bool OpenDB(QString strDBName, QString strUser, QString strPassword) = 0;
    virtual QStringList GetTables()=0;

protected:
    bool DBOperationAllow();

    bool ExecSqlString(QString strSql);
    SearchDataRow ExecSqlAndGetRowData(const QString& strSql, int iCol = 1);
    SearchDataCol ExecSqlAndGetColumnData(const QString& strSql, int iCol = 1);
    bool ExecMitulSqlString(QStringList strSqls);

protected:
    QSqlDatabase m_db;
};

#endif // DATABASEFUNCTION_H
