#include "mainwindow.h"
#include "Pipeline/pipeline.h"
#include "Pipeline/gradientblurpipeline.h"
#include "Pipeline/lsdpipeline.h"

#define THREADCOUNT 4

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupTable();
    setupImport();
    this->showMaximized();
    for(int i = 0; i<THREADCOUNT;i++){
        threads.append(new QThread);
        threads[i]->start();
    }
}

MainWindow::~MainWindow()
{
    for(int i = 0; i<THREADCOUNT; i++){
        threads[i]->quit();
    }
    for(int i = 0; i<THREADCOUNT; i++){
        threads[i]->wait(100);
    }
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

    pb_solve_selected = new QPushButton("Solve selected");
    menulayout->addWidget(pb_solve_selected);
    pb_solve_all = new QPushButton("Solve all");
    menulayout->addWidget(pb_solve_all);
    pb_eval = new QPushButton("Evaluate");
    menulayout->addWidget(pb_eval);

    //mainTable
    mainTable = new QTableWidget();
    mainTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mainTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(mainTable,1,0,1,1);


    //settings
    preview = new QLabel();
    layout->addWidget(preview,1,1,1,1);


    //stausbar
    stausbar = new QWidget(mainWidget);
    layout->addWidget(stausbar,2,0,1,2, Qt::AlignBottom);
    QHBoxLayout * statuslayout = new QHBoxLayout(stausbar);
    //statuslayout->setAlignment(Qt::AlignLeft);
    lbl_status = new QLabel("");
    statuslayout->addWidget(lbl_status,5,Qt::AlignLeft);
    pb_status = new QProgressBar();
    pb_status->setFixedWidth(150);
    pb_status->setTextVisible(false);
    statuslayout->addWidget(pb_status,1,Qt::AlignRight);
    statuslayout->setMargin(0);


    connect(pb_clear,SIGNAL(clicked(bool)),this,SLOT(setupTable()));
    connect(pb_import,SIGNAL(clicked(bool)),this,SLOT(import()));
    connect(mainTable,SIGNAL(currentCellChanged(int,int,int,int)),this,SLOT(showPreview()));
    connect(pb_eval,SIGNAL(clicked(bool)),this,SLOT(evaluate()));
    connect(pb_solve_selected,SIGNAL(clicked(bool)),this,SLOT(detectSingle()));
    connect(pb_solve_all,SIGNAL(clicked(bool)),this,SLOT(detectAll()));
}

void MainWindow::setupTable(){
    QStringList header;
    header.append("Name");
    header.append("Code");
    header.append("Result");
    header.append("Path");
    mainTable->clear();
    mainTable->setRowCount(0);
    mainTable->setColumnCount(4);
    mainTable->setHorizontalHeaderLabels(header);
    mainTable->setColumnWidth(0,400);
    mainTable->setColumnWidth(1,200);
    mainTable->setColumnWidth(2,200);
    mainTable->setColumnWidth(3,800);
    pb_solve_all->setEnabled(false);
    pb_solve_selected->setEnabled(false);
    pb_eval->setEnabled(false);
    preview->clear();
    lbl_status->setText("");
    pb_status->setValue(0);
}

void MainWindow::import(){
    importW->show();
    importW->raise();
    importW->activateWindow();
}

void MainWindow::setupImport(){

    importW = new QWidget();
    importW->setWindowTitle("Import");
    QGridLayout * lay = new QGridLayout(importW);
    QLabel * l1 = new QLabel("Type:");
    lay->addWidget(l1,0,0);
    cb_type = new QComboBox();
    cb_type->addItem("Dir");
    cb_type->addItem("File");
    lay->addWidget(cb_type,0,1);
    QLabel * l2 = new QLabel("Path:");
    lay->addWidget(l2,1,0);
    le_path = new QLineEdit();
    lay->addWidget(le_path,1,1);
    pb_openPath = new QPushButton("►");
    pb_openPath->setFixedWidth(24);
    lay->addWidget(pb_openPath,1,2);
    QLabel * l3 = new QLabel("Code is in:");
    lay->addWidget(l3,2,0);
    cb_code = new QComboBox();
    cb_code->addItem("seperate Textfile");
    cb_code->addItem("Filname");
    lay->addWidget(cb_code,2,1);
    pb_startimport = new QPushButton("Import");
    lay->addWidget(pb_startimport,3,2);

    connect(pb_openPath,SIGNAL(clicked(bool)),this,SLOT(explorePath()));
    connect(pb_startimport,SIGNAL(clicked(bool)),this,SLOT(importFromPath()));

}

void MainWindow::explorePath(){
    QString path;
    if(cb_type->currentIndex()==0){
        path = QFileDialog::getExistingDirectory(importW,QString("Select Directory"),le_path->text());
    } else if(cb_type->currentIndex()==1) {
        path = QFileDialog::getOpenFileName(importW,"Select single File",le_path->text(),"Image Files (*.png *.jpg)");
    }
    if(!path.isEmpty())le_path->setText(path);
}

void MainWindow::importFromPath(){
    if(le_path->text().isEmpty()) return;
    int count = 0;
    QString path = le_path->text();
    if(cb_type->currentIndex()==0){
        QDirIterator * it = new QDirIterator(path);
        while(it->hasNext()){
            QString filep = it->next();
            if(filep.right(4).toLower()==".png" || filep.right(4).toLower() == ".jpg" || filep.right(4).toLower() == ".jpeg"){
                QString code;
                QFileInfo * a = new QFileInfo(filep);
                if(cb_code->currentIndex()==0){
                    QFile file(filep+".txt");
                    if(!file.open(QIODevice::ReadOnly)){
                        lbl_status->setText("Error Reading: " + filep + ".txt");
                        return;
                    }
                    code = file.readLine(13);
                    file.close();
                } else if(cb_code->currentIndex()==1){
                    code = a->fileName().left(13);
                }
                includeFile(filep,a->fileName(),code);
                count++;
            }
        }
    } else if(cb_type->currentIndex()==1){
        QString code;
        QFileInfo * a = new QFileInfo(path);
        if(cb_code->currentIndex()==0){
            QFile file(path+".txt");
            if(!file.open(QIODevice::ReadOnly)){
                lbl_status->setText("Error Reading: " + path + ".txt");
                return;
            }

            code = file.readLine(13);
            file.close();
        } else if(cb_code->currentIndex()){
            code = a->fileName().left(13);
        }
        includeFile(path,a->fileName(),code);
        count++;
    }
    if(count!=0){
        pb_solve_all->setEnabled(true);
        pb_solve_selected->setEnabled(true);
        pb_eval->setEnabled(true);
    }
    lbl_status->setText("Imported " + QString::number(count) + " barcodes");
    importW->close();
}

void MainWindow::includeFile(QString filepath,QString name, QString code){
    int rowcount = mainTable->rowCount();
    mainTable->setRowCount(mainTable->rowCount()+1);
    setTableText(rowcount,0,name);
    setTableText(rowcount,3,filepath);
    setTableText(rowcount,1,code);
}

void MainWindow::setTableText(int r, int c, QString t){
    QTableWidgetItem *itab = new QTableWidgetItem();
    itab->setText(t);
    mainTable->setItem(r,c,itab);

}

QString MainWindow::getTableText(int r, int c){
    QTableWidgetItem *itab = mainTable->item(r,c);
    if(itab== NULL) return "";
    return itab->text();
}

void MainWindow::showPreview(){
    preview->clear();
    QString path = getTableText(mainTable->currentRow(),3);
    if(path.isEmpty()) return;
    QImage image = QImage(path);
    image = image.scaledToWidth(300);
    preview->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::evaluate(){
    int sum = mainTable->rowCount();
    if(sum==0) return;
    int error = 0;
    for(int i = 0; i<sum; i++){
        if(getTableText(i,1)!=getTableText(i,2).left(getTableText(i,1).length())) error++;
    }
    QString result = "Total: " + QString::number(sum) + "\tCorrect: "
            + QString::number(sum-error) + "\tErrors: " + QString::number(error)
            + "\tErrorrate:" + QString::number((error*100)/sum) +"%";
    lbl_status->setText(result);
}

void MainWindow::detectSingle(){
    int cr = mainTable->currentRow();
    if(cr>=0){
        auto pipe = new LSDPipeline(getTableText(cr,3), true);
        pipe->moveToThread(threads[0]);
        connect(pipe,&Pipeline::completed,[this,pipe,cr](QString result){
            QMetaObject::invokeMethod(this,"setTableText",Qt::QueuedConnection,Q_ARG(int,cr),Q_ARG(int,2),Q_ARG(QString,result));
            pipe->deleteLater();
        });
        QMetaObject::invokeMethod(pipe,"start",Qt::QueuedConnection);
    }
}

void MainWindow::detectAll(){
    int max = mainTable->rowCount();
    pb_status->setRange(1,max);
    for(int i = 0; i<max; i++){
        auto pipe = new LSDPipeline(getTableText(i,3));
        pipe->moveToThread(threads[i%THREADCOUNT]);
        connect(pipe,&Pipeline::completed,[this,pipe,i](QString result){
            QMetaObject::invokeMethod(this,"setTableText",Qt::QueuedConnection,Q_ARG(int,i),Q_ARG(int,2),Q_ARG(QString,result));
            QMetaObject::invokeMethod(this,"incrementStatus",Qt::QueuedConnection);
            pipe->deleteLater();
        });
        QMetaObject::invokeMethod(pipe,"start",Qt::QueuedConnection);

        //qApp->processEvents();
    }
}

void MainWindow::incrementStatus(){
    pb_status->setValue(pb_status->value()+1);
}
