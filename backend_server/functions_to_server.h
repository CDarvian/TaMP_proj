#ifndef FUNCTIONS_TO_SERVER_H
#define FUNCTIONS_TO_SERVER_H

#include <QString>

QString handleLogin(const QString& payload);
QString handleRegister(const QString& payload);
QString handleStats(const QString& payload);

#endif // FUNCTIONS_TO_SERVER_H
