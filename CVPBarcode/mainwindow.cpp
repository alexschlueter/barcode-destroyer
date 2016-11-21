#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    this->showMaximized();
}

MainWindow::~MainWindow()
{

}

void MainWindow::setupUI(){
    mainWidget = new QWidget(this);
    layout = new QGridLayout(mainWidget);
    this->setCentralWidget(mainWidget);

    //menubar
    menubar = new QWidget(mainWidget);
    layout->addWidget(menubar,0,0,1,2,Qt::AlignTop);
    QHBoxLayout * menulayout = new QHBoxLayout(menubar);
    menulayout->setMargin(0);
    menulayout->setAlignment(Qt::AlignLeft);

    pb_clear = new QPushButton("Clear");
    menulayout->addWidget(pb_clear);

    pb_import = new QPushButton("Import");
    menulayout->addWidget(pb_import);

    pb_solve = new QPushButton("Solve");
    menulayout->addWidget(pb_solve);

    //mainTable
    mainTable = new QTableWidget();
    mainTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(mainTable,1,0,1,2);


    //stausbar
    stausbar = new QWidget(mainWidget);
    layout->addWidget(stausbar,2,0,1,2, Qt::AlignBottom);
    QHBoxLayout * statuslayout = new QHBoxLayout(stausbar);
    //statuslayout->setAlignment(Qt::AlignLeft);
    lbl_status = new QLabel("Test");
    statuslayout->addWidget(lbl_status,5,Qt::AlignLeft);
    pb_status = new QProgressBar();
    pb_status->setFixedWidth(150);
    pb_status->setTextVisible(false);
    statuslayout->addWidget(pb_status,1,Qt::AlignRight);
    statuslayout->setMargin(0);

    connect(pb_clear,SIGNAL(clicked(bool)),this,SLOT(setUp()));
    connect(pb_import,SIGNAL(clicked(bool)),this,SLOT(addRow()));
}

void MainWindow::setUp(){
    QStringList header;
    header.append("Path");
    header.append("Label");
    mainTable->clear();
    mainTable->setRowCount(0);
    mainTable->setColumnCount(2);
    mainTable->setHorizontalHeaderLabels(header);
}

void MainWindow::addRow(){
    mainTable->setRowCount(mainTable->rowCount()+1);
}
