#include "gradientblurpipeline.h"


void GradientBlurPipeline::execute(void* data){
    QString path = *static_cast<QString*>(data);

    LoaderStep loader; //use this to provide some parameters
    GradientBlurStep gb;
    ShowStep display("After Grad/Blur");
    ReaderStep reader;


    connectSteps(loader,gb);
    connectSteps(gb,display);
    connectSteps(display,reader);
    setFinal(reader);

    loader.execute((void*)&path);
}

