#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTableWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QWidget * mainWidget;
    QGridLayout * layout;
    QWidget * menubar;
    QWidget * stausbar;

    QPushButton * pb_clear;
    QPushButton * pb_import;
    QPushButton * pb_solve;

    QTableWidget * mainTable;

    QLabel * lbl_status;
    QProgressBar * pb_status;
    void setupUI();

private slots:
    void addRow();
    void setUp();
};

#endif // MAINWINDOW_H
