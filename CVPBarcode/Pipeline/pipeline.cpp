#include "pipeline.h"


void Pipeline::start(){
    execute((void*)&path);
}

void Pipeline::execute(void *data){std::cout << data << std::endl;} // just do something

void Pipeline::connectSteps(Step &step1, Step &step2){
    connect(&step1,SIGNAL(completed(void*)),&step2,SLOT(execute(void*)),Qt::DirectConnection);
    connect(&step1, &Step::showImage, this, &Pipeline::showImageSlot);
}

void Pipeline::setFinal(Step &step){
    connect(&step, &Step::showImage, this, &Pipeline::showImageSlot);
    connect(&step,SIGNAL(completed(void*)),this,SLOT(jobsdone(void*)),Qt::DirectConnection);
}

void Pipeline::jobsdone(void *result){
    emit completed(*static_cast<QString*>(result));
}

void Pipeline::showImageSlot(const std::string &name, const cv::Mat &img) {
    emit showImage(name, img);
}

QMap<QString, PipelineFactoryBase*> &getPipelines() {
    static QMap<QString, PipelineFactoryBase*> pipelines;
    return pipelines;
}
