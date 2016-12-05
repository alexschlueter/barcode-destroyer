#include "gradientblurpipeline.h"

GradientBlurPipeline::GradientBlurPipeline()
{

}

void GradientBlurPipeline::execute(){
    LoaderStep loader;
    ShowStep display;
    connect(&loader,SIGNAL(completed(cv::Mat)),&display,SLOT(execute(cv::Mat))); // das ist noch nicht schöhn

    //connectSteps(loader,display); // TODO make this work


    loader.execute("C:\\Daten\\CV_DB\\name1\\0282925037198-01_N95-2592x1944_scaledTo800x600bilinear.jpg");
}

void GradientBlurPipeline::connectSteps(Step &step1, Step &step2){
    connect(&step1,SIGNAL(completed(auto)),&step2,SLOT(execute(auto)));
}

