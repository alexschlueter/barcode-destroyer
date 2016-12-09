#include "lsdpipeline.h"

void LSDPipeline::execute(void* data){
    QString path = *static_cast<QString*>(data);

    LoaderStep loader; //use this to provide some parameters
    LSDStep lsd;
    ShowStep display("After LSD");
    ReaderStep reader;


    connectSteps(loader, lsd);
    connectSteps(lsd, display);
    connectSteps(display, reader);
    setFinal(reader);

    loader.execute((void*)&path);
}

