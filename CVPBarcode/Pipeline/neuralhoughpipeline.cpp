#include "neuralhoughpipeline.h"

#include "../Steps/loaderstep.h"
#include "../Steps/readerstep.h"
#include "../Steps/lsdstep.h"
#include "../Steps/neuralhoughstep.h"

void NeuralHoughPipeline::execute(void* data){
    QString path = *static_cast<QString*>(data);

    LoaderStep loader;
    NeuralHoughStep localizer("../net61x3.net");
    ReaderStep reader;


    connectSteps(loader, localizer);
    connectSteps(localizer, reader);
    setFinal(reader);

    loader.execute((void*)&path);
}

PipelineFactory<NeuralHoughPipeline> nhPipe("NeuralHough (no reader)");
