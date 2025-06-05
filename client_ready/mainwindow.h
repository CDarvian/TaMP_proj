#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

namespace Ui {
class mainwindow;
}

/// Отдельное окно “Tools” (SHA-512, Division, Graph)
class mainwindow : public QWidget
{
    Q_OBJECT

public:
    explicit mainwindow(QWidget *parent = nullptr);
    ~mainwindow() override;

private slots:
    void on_shaButton_clicked();
    void on_divButton_clicked();
    void on_graphButton_clicked();
    void on_backButton_clicked(); // Вернуться в окно регистрации/авторизации

    void onServerResponse(const QString &response);

private:
    Ui::mainwindow *ui;
};

#endif // MAINWINDOW_H
