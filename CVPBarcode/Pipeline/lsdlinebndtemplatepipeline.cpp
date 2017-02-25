#include "lsdlinebndtemplatepipeline.h"
#include "Steps/templatematchingstep.h"
#include "Steps/loaderstep.h"
#include "Steps/showstep.h"
#include "Steps/readerstep.h"
#include "Steps/lsdstep.h"
#include "Steps/lsdboundaryfinderstep.h"

void LSDLineBndTemplatePipeline::execute(void* data){
    QString path = *static_cast<QString*>(data);

    LoaderStep loader;
    LSDStep lsd;
    LSDBoundaryFinderStep var;
    //ShowStep display("After LSD");
    TemplateMatchingStep reader("../cells");


    connectSteps(loader, lsd);
    //connectSteps(lsd, display);
    //connectSteps(display, reader);
    connectSteps(lsd, var);
    connectSteps(var, reader);
    setFinal(reader);

    loader.execute((void*)&path);
}

PipelineFactory<LSDLineBndTemplatePipeline> lsdBndPipe("LSD + LSDBound + TemplateMatching");
