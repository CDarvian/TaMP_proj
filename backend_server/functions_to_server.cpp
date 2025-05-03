#include "functions_to_server.h"

// Сюда позже можно втыкать базу или файлы — а пока просто заглушки.

QString handleLogin(const QString& payload) {
    Q_UNUSED(payload);
    // TODO: распарсить payload (username/password) и проверить
    return "LOGIN_OK: Добро пожаловать!";
}

QString handleRegister(const QString& payload) {
    Q_UNUSED(payload);
    // TODO: распарсить payload (новый юзер) и записать
    return "REGISTER_OK: Успешная регистрация!";
}

QString handleStats(const QString& payload) {
    Q_UNUSED(payload);
    // TODO: собрать реальные данные статистики
    return "STATS: Всего пользователей — 42; Активных сессий — 7;";
}

