#ifndef FUNCTION_CLIENT_H
#define FUNCTION_CLIENT_H

#include <QWidget>

namespace Ui {
class function_client;
}

/// Отдельное окно “Tools” (SHA-512, Division, Graph)
class function_client : public QWidget
{
    Q_OBJECT

public:
    explicit function_client(QWidget *parent = nullptr);
    ~function_client() override;

private slots:
    void on_shaButton_clicked();
    void on_divButton_clicked();
    void on_graphButton_clicked();
    void on_backButton_clicked(); // Вернуться в окно регистрации/авторизации

    void onServerResponse(const QString &response);

private:
    Ui::function_client *ui;
};
#endif // FUNCTION_CLIENT_H
