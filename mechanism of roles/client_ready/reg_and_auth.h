#ifndef REGANDAUTH_H
#define REGANDAUTH_H

#include <QWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QCryptographicHash>


// Доступные статусы пользователей
namespace UserStatus {
const QString ONLINE = "online";
const QString OFFLINE = "offline";
const QString BANNED = "banned";
const QString ADMIN = "admin";
}

class mainwindow; // Forward declaration достаточно

namespace Ui {
class RegAndAuth;
}

/// Форма регистрации/авторизации
class RegAndAuth : public QWidget
{
    Q_OBJECT

public:
    explicit RegAndAuth(QWidget *parent = nullptr);
    ~RegAndAuth() override;

private slots:
    /// Переключение на форму регистрации
    void on_showRegisterButton_clicked();

    /// Переключение на форму авторизации
    void on_showLoginButton_clicked();

    /// Обработка регистрации нового пользователя
    void on_regButton_clicked();

    /// Обработка авторизации пользователя
    void on_logButton_clicked();

    /// Запрос статистики сервера
    void on_statsButton_clicked();

    /// Обработка административного входа
    void on_adminButton_clicked();


    void onServerResponse(const QString &response);

private:
    Ui::RegAndAuth *ui;
    mainwindow *m_mainWindow = nullptr; ///< Указатель на главное окно приложения

    /**
     * @brief Показывает сообщение об ошибке
     * @param message - текст сообщения об ошибке
     */
    void showError(const QString &message);

    /**
     * @brief Показывает информационное сообщение
     * @param message - текст сообщения
     */
    void showInfo(const QString &message);

    /**
     * @brief Очищает все поля ввода и результаты
     */
    void clearForms();
};

#endif // REGANDAUTH_H
