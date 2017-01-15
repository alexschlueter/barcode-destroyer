#include "mainwindow.h"
#include "Pipeline/pipeline.h"
#include "Pipeline/gradientblurpipeline.h"
#include "Pipeline/lsdtemplatepipeline.h"
#include "aspectratiopixmaplabel.h"
#include <QScrollBar>
#include "utils.h"

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
    cb_pipelines = new QComboBox;
    for (const auto &pname : getPipelines().keys())
        cb_pipelines->addItem(pname);
    if (!settings.value("pipeline").isNull())
        cb_pipelines->setCurrentText(settings.value("pipeline").toString());
    connect(cb_pipelines, &QComboBox::currentTextChanged, [this](const QString &text) {
        settings.setValue("pipeline", text);
    });
    menulayout->addWidget(cb_pipelines);

    splitter = new QSplitter;
    splitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout->addWidget(splitter, 1, 0);

    //mainTable
    mainTable = new QTableWidget;
    mainTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mainTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    splitter->addWidget(mainTable);

    //image preview area
    scrollArea = new QScrollArea;
    scrollWidget = new QWidget;
    scrollArea->setWidget(scrollWidget);
    scrollLayout = new QVBoxLayout(scrollWidget);
    scrollArea->setWidgetResizable(true);
    splitter->addWidget(scrollArea);
    // what's wrong with this?
    //splitter->setStretchFactor(0, 3);
    //splitter->setStretchFactor(1, 1);

    splitter->setSizes({500, 100});

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


    connect(pb_clear,SIGNAL(clicked(bool)),this,SLOT(clear()));
    connect(pb_import,SIGNAL(clicked(bool)),this,SLOT(import()));
    connect(mainTable,SIGNAL(currentCellChanged(int,int,int,int)),this,SLOT(showPreview()));
    connect(this, &MainWindow::previewChanged, this, &MainWindow::showPreview);
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
}

void MainWindow::import(){
    pb_openPath->setEnabled(true);
    pb_startimport->setEnabled(true);
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
    pb_openPath->setEnabled(false);
    pb_startimport->setEnabled(false);
    int count = 0;
    QString path = le_path->text();
    if(cb_type->currentIndex()==0){
        QDirIterator it(path);
        while(it.hasNext()){
            QString filep = it.next();
            if(filep.right(4).toLower()==".png" || filep.right(4).toLower() == ".jpg" || filep.right(4).toLower() == ".jpeg"){
                QString code;
                QFileInfo a(filep);
                if(cb_code->currentIndex()==0){
                    QFile file(filep+".txt");
                    if(!file.open(QIODevice::ReadOnly)){
                        lbl_status->setText("Error Reading: " + filep + ".txt");
                        pb_openPath->setEnabled(true);
                        pb_startimport->setEnabled(true);
                        return;
                    }
                    code = file.readLine(14);
                    file.close();
                } else if(cb_code->currentIndex()==1){
                    code = a.fileName().left(13);
                }
                includeFile(filep,a.fileName(),code);
                count++;
            }
        }
    } else if(cb_type->currentIndex()==1){
        QString code;
        QFileInfo a(path);
        if(cb_code->currentIndex()==0){
            QFile file(path+".txt");
            if(!file.open(QIODevice::ReadOnly)){
                lbl_status->setText("Error Reading: " + path + ".txt");
                pb_openPath->setEnabled(true);
                pb_startimport->setEnabled(true);
                return;
            }

            code = file.readLine(13);
            file.close();
        } else if(cb_code->currentIndex()){
            code = a.fileName().left(13);
        }
        includeFile(path,a.fileName(),code);
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
    images.push_back({{"Original", QImage(filepath)}});
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

bool MainWindow::resultIsCorrect(int row, const QString &result)
{
    return getTableText(row,1) == result.left(getTableText(row,1).length());
}

void MainWindow::updateRowWithResult(int row, const QString &result)
{
    setTableText(row, 2, result);
    auto color = resultIsCorrect(row, result) ? QColor(0, 255, 0, 20) : QColor(255, 0, 0, 20);
    for (int col = 0; col < mainTable->columnCount(); col++) {
        mainTable->item(row, col)->setBackground(color);
    }
}

void MainWindow::showPreview(){
    int cr = mainTable->currentRow();
    if (cr < 0) return;

    QColor color;
    QPalette tablePalette = mainTable->palette();
    if (getTableText(cr, 2).isEmpty()) {
        tablePalette.setBrush(QPalette::HighlightedText, Qt::white);
        color = QColor(48, 140, 198);
    } else {
        tablePalette.setBrush(QPalette::HighlightedText, Qt::black);
        if (resultIsCorrect(cr, getTableText(cr, 2))) {
            //color = QColor(0, 255, 0, 50);
            color = Qt::darkGreen;
            color.setAlpha(50);
        } else {
            color = QColor(255, 0, 0, 50);
        }
    }
    tablePalette.setBrush(QPalette::Highlight, color);
    mainTable->setPalette(tablePalette);
    // TODO: if the barcode result for an image arrives while the image is selected
    // the highlight color doesn't change
    // repaint doesn't work either
    //mainTable->repaint();

    clearLayout(scrollLayout);
    for (const auto &pair : images[cr]) {

        QLabel *name = new QLabel(pair.first);
        name->setAlignment(Qt::AlignHCenter);

        scrollLayout->addWidget(name);
        auto img = new AspectRatioPixmapLabel(QPixmap::fromImage(pair.second));
        scrollLayout->addWidget(img, 0, Qt::AlignHCenter);
    }
    scrollLayout->addStretch();
}

void MainWindow::evaluate(){
    int sum = mainTable->rowCount();
    if(sum==0) return;
    int error = 0;
    for(int i = 0; i<sum; i++){
        if(!resultIsCorrect(i, getTableText(i, 2))) error++;
    }
    QString result = "Total: " + QString::number(sum) + "\tCorrect: "
            + QString::number(sum-error) + "\tErrors: " + QString::number(error)
            + "\tErrorrate:" + QString::number((error*100)/sum) +"%";
    lbl_status->setText(result);
}

void MainWindow::detectSingle(){
    int cr = mainTable->currentRow();
    if(cr>=0){
        images[cr].resize(1);
        auto pipe = getPipelines()[cb_pipelines->currentText()]->create(getTableText(cr,3));
        pipe->moveToThread(threads[0]);
        connect(pipe, &Pipeline::showImage, this, [this, cr](QString name, QImage img) {
            if (images.size() <= (uint)cr) return;
            images[cr].push_back({name, img});
            emit previewChanged();
        });
        connect(pipe, &Pipeline::completed, this, [this,pipe,cr](QString result){
            updateRowWithResult(cr, result);
            pipe->deleteLater();
        });
        QMetaObject::invokeMethod(pipe,"start",Qt::QueuedConnection);
    }
}

void MainWindow::detectAll(){
    uint max = mainTable->rowCount();
    pb_status->setRange(1,max);
    for(uint i = 0; i<max; i++){
        images[i].resize(1);
        auto pipe = getPipelines()[cb_pipelines->currentText()]->create(getTableText(i,3));
        pipe->moveToThread(threads[i%THREADCOUNT]);
        connect(pipe, &Pipeline::showImage, this, [this, i](QString name, QImage img) {
            if (images.size() <= i) return;
            images[i].push_back({name, img});
            if (mainTable->currentRow() == (int)i) emit previewChanged();
        });
        connect(pipe, &Pipeline::completed, this, [this,pipe,i](QString result){
            updateRowWithResult(i, result);
            incrementStatus();
            pipe->deleteLater();
        });
        QMetaObject::invokeMethod(pipe,"start",Qt::QueuedConnection);

        //qApp->processEvents();
    }
}

void MainWindow::clear()
{
    clearLayout(scrollLayout);
    lbl_status->setText("");
    pb_status->reset();
    images.clear();
    setupTable();
}

void MainWindow::incrementStatus(){
    pb_status->setValue(pb_status->value()+1);
}
