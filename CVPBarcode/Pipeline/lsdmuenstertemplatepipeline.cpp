#include "lsdmuenstertemplatepipeline.h"

#include "Steps/templatematchingstep.h"
#include "Steps/loaderstep.h"
#include "Steps/readerstep.h"
#include "Steps/lsdstep.h"
#include "Steps/muensterboundaryfinderstep.h"

void LSDMuensterTemplatePipeline::execute(void* data){
    QString path = *static_cast<QString*>(data);

    LoaderStep loader;
    LSDStep lsd;
    MuensterBoundaryFinderStep bound;
    //ShowStep display("After LSD");
    TemplateMatchingStep reader("../cells");
    //ReaderStep reader;


    connectSteps(loader, lsd);
    //connectSteps(lsd, display);
    //connectSteps(display, reader);
    connectSteps(lsd, bound);
    connectSteps(bound, reader);
    setFinal(reader);

    loader.execute((void*)&path);
}

PipelineFactory<LSDMuensterTemplatePipeline> lsdMuensterPipe("LSD + Muenster + TemplateMatching");
