#include "databasefunction.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlDriver>
#include <QDir>

inline QString ConvertStrListToString(const QStringList &strItems)
{
    QString strTmp = "";
    if(strItems.size())
    {
        strTmp = strItems.at(0);
        for(int i = 1; i < strItems.size(); ++i)
        {
            strTmp.append(',');
            strTmp.append(strItems.at(i));
        }
    }

    return strTmp;
}

DataBaseFunction::DataBaseFunction()
{

}

DataBaseFunction::~DataBaseFunction()
{
    if(m_db.isOpen())
    {
        m_db.close();
    }
}

bool DataBaseFunction::ExecSqlString(QString strSql)
{
    if(DBOperationAllow())
    {
        QSqlQuery query;
        if(!query.exec(strSql))
        {
            qDebug() << strSql;
            qDebug() << query.lastError().text();
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}

SearchDataRow DataBaseFunction::ExecSqlAndGetRowData(const QString& strSql, int iCol)
{
    SearchDataRow sdr;
    if(DBOperationAllow())
    {
        QSqlQuery query;
        if(!query.exec(strSql))
        {
            qDebug() << strSql;
            qDebug() << query.lastError().text();

            return sdr;
        }

        while (query.next())
        {
            RowData rd;

            for(int i = 0; i < iCol; ++i)
            {
                rd.append(query.value(i));
            }
            sdr.append(rd);
        }
    }

    return sdr;
}

SearchDataCol DataBaseFunction::ExecSqlAndGetColumnData(const QString &strSql, int iCol)
{
    SearchDataCol sdc;
    for(int i = 0; i < iCol; ++i)
    {
        sdc.push_back(QList<QVariant>());
    }
    QSqlQuery query;
    if(!query.exec(strSql))
    {
        qDebug() << strSql;
        qDebug() << query.lastError().text();

        return sdc;
    }

    while (query.next())
    {
        for(int i = 0; i < iCol; ++i)
        {
            sdc[i].append(query.value(i));
        }
    }

    return sdc;
}

bool DataBaseFunction::ExecMitulSqlString(QStringList strSqls)
{
    if(DBOperationAllow())
    {
        if(m_db.driver()->hasFeature(QSqlDriver::Transactions))
        {
            if(m_db.transaction())
            {
                QSqlQuery query;
                bool bSuccess = true;
                for(auto strSql : strSqls)
                {
                    bool bResult = query.exec(strSql);
                    if(!bResult)
                    {
                        qDebug() << strSql;
                        qDebug() << query.lastError();
                    }
                    bSuccess = bSuccess && bResult;
                }

                if(bSuccess)
                {
                    if(m_db.commit())
                    {
                        return true;
                    }
                }
                else
                {
                    if(m_db.rollback())
                    {
                        qDebug() << "rollback";
                        return false;
                    }
                }
            }
            qDebug() << m_db.lastError();
        }
    }

    return false;
}

QList<RowData> DataBaseFunction::SearchForRow(const QStringList &strItems, const QString& strTable, const QString& strCondition)
{
    QString strSql = QString("select %1 from %2 where %3;")
            .arg(ConvertStrListToString(strItems))
            .arg(strTable)
            .arg(strCondition);

    return  ExecSqlAndGetRowData(strSql, strItems.size());
}

QList<ColData> DataBaseFunction::SearchForCol(const QStringList &strItems, const QString& strTable, const QString& strCondition)
{
    QString strSql = QString("select %1 from %2 where %3;")
            .arg(ConvertStrListToString(strItems))
            .arg(strTable)
            .arg(strCondition);

    return ExecSqlAndGetColumnData(strSql, strItems.size());
}

bool DataBaseFunction::DBOperationAllow()
{
    if(m_db.isOpen() || m_db.open())
    {
        return true;
    }
    else
    {
        qDebug() << m_db.lastError();
        return false;
    }
}

QString DataBaseFunction::QStringToSqlString(QString strSrc)
{
    return QString("'") + strSrc + "'";
}

bool DataBaseFunction::ExcuteSqlFile(QString strFile)
{
    QFile file(strFile);
    if(!file.exists())
    {
        return false;
    }

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream istream(&file);
    istream.setCodec("UTF-8");
    QString strData = istream.readAll();

    QStringList strLines = strData.split(";");
    QStringList strSqls;
    for(auto strSql: strLines)
    {
        strSql.remove('\n');
        if(!strSql.isEmpty())
        {
            strSqls.append(strSql);
        }
    }

    return ExecMitulSqlString(strSqls);
}

bool DataBaseFunction::InsertData(QString strTable, QList<QVariant> datas, QStringList strColNames)
{
    if(DBOperationAllow())
    {
        QString strSql;
        if(strColNames.size() == 0)
        {
            strSql = QString("insert into %1 values(").arg(strTable);
            for(int i = 0; i < datas.size(); ++i)
            {
                if(i==0)
                {
                    strSql = strSql + "?";
                }
                else
                {
                    strSql += ", ?";
                }
            }
            strSql += ");";
        }
        else
        {
            strSql = QString("insert into %1(").arg(strTable);

            for(int i = 0; i < strColNames.size(); ++i)
            {
                if(0 == i)
                {
                    strSql += strColNames.at(i);
                }
                else
                {
                    strSql = strSql + ", " + strColNames.at(i);
                }
            }

            strSql += ") values(";
            for(int i = 0; i < datas.size(); ++i)
            {
                if(i==0)
                {
                    strSql = strSql + "?";
                }
                else
                {
                    strSql += ", ?";
                }
            }
            strSql += ");";
        }

        QSqlQuery sq;
        sq.prepare(strSql);
        for(auto val : datas)
        {
            sq.addBindValue(val);
        }

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
    else
    {
        return false;
    }
}

bool DataBaseFunction::DeleteDataWithCondition(QString strTable, QString strCondition)
{
    QString strSql = QString("delete from %1 where %2;")
            .arg(strTable)
            .arg(strCondition);

    return ExecSqlString(strSql);
}

bool DataBaseFunction::UpdateData(QString strTable, QString strChange, QString strCondition)
{
    QString strSql = QString("update %1 set %2 where %3;")
            .arg(strTable)
            .arg(strChange)
            .arg(strCondition);

    return ExecSqlString(strSql);
}

