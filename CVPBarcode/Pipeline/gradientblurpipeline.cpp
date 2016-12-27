#include "gradientblurpipeline.h"

void GradientBlurPipeline::execute(void* data){
    QString path = *static_cast<QString*>(data);

    LoaderStep loader; //use this to provide some parameters
    GradientBlurStep gb;
    ReaderStep reader;

    connectSteps(loader,gb);
    connectSteps(gb,reader);
    setFinal(reader);

    loader.execute((void*)&path);
}

PipelineFactory<GradientBlurPipeline> gbPipe("GradientBlur (no reader)");
