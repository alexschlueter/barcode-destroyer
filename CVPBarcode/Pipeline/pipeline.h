#ifndef PIPELINE_H
#define PIPELINE_H

#include "Steps/step.h"
#include <iostream>
#include <QObject>
#include <QThread>
#include <QString>

class Pipeline : public QObject
{
    Q_OBJECT
public:
    Pipeline(QString path){this->path = path;};
    virtual void execute(void* data);
    void connectSteps(Step &step1, Step &step2);
    void setFinal(Step & step);
    void start();
    QString path;
public slots:
    void jobsdone(void* result);
signals:
    void completed(QString);
};

#endif // PIPELINE_H
