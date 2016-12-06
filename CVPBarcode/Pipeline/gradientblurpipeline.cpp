#include "gradientblurpipeline.h"


void GradientBlurPipeline::execute(void* data){
    QString path = *static_cast<QString*>(data);

    LoaderStep loader; //use this to provide some parameters
    GradientBlurStep gb;
    ShowStep predisplay;
    ShowStep postdisplay("After Grad/Blur");
    ReaderStep reader;

    connectSteps(loader,predisplay);
    connectSteps(predisplay,gb);
    connectSteps(gb,postdisplay);
    connectSteps(postdisplay,reader);
    setFinal(reader);

    loader.execute((void*)&path);
}

