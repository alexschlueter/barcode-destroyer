#include "lsdpipeline.h"
#include "Steps/templatematchingstep.h"
#include "Steps/loaderstep.h"
#include "Steps/showstep.h"
#include "Steps/readerstep.h"
#include "Steps/lsdstep.h"

void LSDTemplatePipeline::execute(void* data){
    QString path = *static_cast<QString*>(data);

    LoaderStep loader;
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

PipelineFactory<LSDTemplatePipeline> lsdPipe("LSD + TemplateMatching");
