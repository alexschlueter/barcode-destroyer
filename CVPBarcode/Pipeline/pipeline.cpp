#include "pipeline.h"


void Pipeline::start(){
    execute((void*)&path);
}

void Pipeline::execute(void *data){std::cout << data << std::endl;} // just do something

void Pipeline::connectSteps(Step &step1, Step &step2){
    connect(&step1,SIGNAL(completed(void*)),&step2,SLOT(execute(void*)));
}

void Pipeline::setFinal(Step &step){
    connect(&step,SIGNAL(completed(void*)),this,SLOT(jobsdone(void*)));
}

void Pipeline::jobsdone(void *result){
    emit completed(*static_cast<QString*>(result));
}
