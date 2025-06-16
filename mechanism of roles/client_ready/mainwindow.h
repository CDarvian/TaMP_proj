#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>

namespace Ui {
class mainwindow;
}

class mainwindow : public QWidget
{
    Q_OBJECT

public:
    explicit mainwindow(QWidget *parent = nullptr);
    ~mainwindow();
    void updateAdminUI(bool isAdmin);
private slots:
    void on_shaButton_clicked();
    void on_divButton_clicked();
    void on_graphButton_clicked();
    void on_backButton_clicked();
    void on_adminButton_clicked();  // Добавлено
    void onServerResponse(const QString &response);


private:
    Ui::mainwindow *ui;
};

#endif // MAINWINDOW_H
