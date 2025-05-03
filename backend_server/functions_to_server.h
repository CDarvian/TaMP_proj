#ifndef FUNCTIONS_TO_SERVER_H
#define FUNCTIONS_TO_SERVER_H

#include <QString>

// Обработка логина: вернёт ответ-сообщение
QString handleLogin(const QString &payload);

// Обработка регистрации: вернёт ответ-сообщение
QString handleRegister(const QString &payload);

// Обработка запроса статистики: вернёт ответ-сообщение
QString handleStats(const QString &payload);

#endif // FUNCTIONS_TO_SERVER_H
