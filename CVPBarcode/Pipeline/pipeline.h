#ifndef PIPELINE_H
#define PIPELINE_H

#include "Steps/step.h"
#include <iostream>
#include <QObject>
#include <QThread>
#include <QString>
#include <QMap>
#include <QImage>

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
    void showImage(QString, QImage);
private slots:
    void showImageSlot(const std::string &, const cv::Mat &);
};


struct PipelineFactoryBase
{
    virtual ~PipelineFactoryBase() {}
    virtual Pipeline *create(QString) = 0;
};

QMap<QString, PipelineFactoryBase*> &getPipelines();

template <class PipelineT>
struct PipelineFactory : PipelineFactoryBase
{
    PipelineFactory(QString name) {
        getPipelines().insert(name, this);
    }

    virtual Pipeline *create(QString path) { return new PipelineT(path); }
};

#endif // PIPELINE_H
