#include <QApplication>

#include "databasetest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DataBaseTest dbt;
    dbt.show();

    return a.exec();
}
