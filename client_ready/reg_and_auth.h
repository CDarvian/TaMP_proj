#ifndef REGANDAUTH_H
#define REGANDAUTH_H

#include <QWidget>
class mainwindow;

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

    /// Привязка окна задач, чтобы открывать его по кнопке
    void setmainwindow(mainwindow *w);

private slots:
    void on_showRegisterButton_clicked();
    void on_showLoginButton_clicked();
    void on_regButton_clicked();
    void on_logButton_clicked();
    void on_statsButton_clicked();
    void onServerResponse(const QString &response);

private:
    Ui::RegAndAuth *ui;
    mainwindow   *m_mainWindow; // указатель на окно задач
};

#endif // REGANDAUTH_H
