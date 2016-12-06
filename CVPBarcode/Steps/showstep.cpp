#include "showstep.h"

ShowStep::ShowStep(QString name)
{
    this->name = name;
}

void ShowStep::execute(void* data){
    this->image = *static_cast<cv::Mat*>(data);
    cv::imshow(name.toStdString(), image);
    emit completed((void*)&image);
}
