#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>
#include <QDirIterator>
#include <QMessageBox>
#include <QApplication>
#include <QThread>
#include <QVector>

#include "Pipeline/pipeline.h"
#include "Pipeline/gradientblurpipeline.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QVector<QThread*> threads;
    QWidget * mainWidget;
    QGridLayout * layout;
    QWidget * menubar;
    QWidget * stausbar;
    QLabel * preview;

    QWidget * importW;

    QPushButton * pb_clear;
    QPushButton * pb_import;
    QPushButton * pb_solve_selected;
    QPushButton * pb_solve_all;
    QPushButton * pb_eval;

    QComboBox * cb_type;
    QComboBox * cb_code;
    QLineEdit * le_path;
    QPushButton * pb_openPath;
    QPushButton * pb_startimport;

    QTableWidget * mainTable;

    QLabel * lbl_status;
    QProgressBar * pb_status;
    void setupUI();
    void includeFile(QString filepath,QString name,QString code);
    QString getTableText(int r, int c);

private slots:
    void setTableText(int r, int c, QString t);
    void incrementStatus();
    void import();
    void setupTable();
    void setupImport();
    void explorePath();
    void importFromPath();
    void showPreview();
    void evaluate();
    void detectSingle();
    void detectAll();
};

#endif // MAINWINDOW_H
