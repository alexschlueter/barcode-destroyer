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
    Pipeline(QString path){this->path = path;}
    virtual void execute(void* data);
    void connectSteps(Step &step1, Step &step2);
    void setFinal(Step & step);
    QString path;
public slots:
    void start();
    void jobsdone(void* result);
signals:
    void completed(QString);
    void showImage(const std::string &, const cv::Mat &);
private slots:
    void showImageSlot(const std::string &, const cv::Mat &);
};

#endif // PIPELINE_H
