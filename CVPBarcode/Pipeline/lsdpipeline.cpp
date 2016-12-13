#include "lsdpipeline.h"
#include "../Steps/templatematchingstep.h"

void LSDPipeline::execute(void* data){
    QString path = *static_cast<QString*>(data);

    LoaderStep loader; //use this to provide some parameters
    LSDStep lsd;
    //ShowStep display("After LSD");
    TemplateMatchingStep reader("../cells");


    connectSteps(loader, lsd);
    //connectSteps(lsd, display);
    //connectSteps(display, reader);
    connectSteps(lsd, reader);
    setFinal(reader);

    loader.execute((void*)&path);
}

