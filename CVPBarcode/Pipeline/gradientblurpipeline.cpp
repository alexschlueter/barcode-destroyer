#include "gradientblurpipeline.h"
#include "../Steps/templatematchingstep.h"

void GradientBlurPipeline::execute(void* data){
    QString path = *static_cast<QString*>(data);

    LoaderStep loader; //use this to provide some parameters
    GradientBlurStep gb;
    TemplateMatchingStep reader(":/cells");

    connectSteps(loader,gb);
    connectSteps(gb,reader);
    setFinal(reader);

    loader.execute((void*)&path);
}

PipelineFactory<GradientBlurPipeline> gbPipe("GradientBlur + TemplateMatching");
