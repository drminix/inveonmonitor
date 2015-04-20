#ifndef UTILITY_H
#define UTILITY_H

#include<QString>
#include<QProcess>


class Utility
{
public:
    Utility();
    QString& runExternalCommand(QString& command);

};

#endif // UTILITY_H
