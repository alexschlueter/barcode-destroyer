#ifndef MUENSTERBOUNDARYFINDERSTEP_H
#define MUENSTERBOUNDARYFINDERSTEP_H

#include "step.h"


class MuensterBoundaryFinderStep : public Step
{
    Q_OBJECT
public slots:
    void execute(void* data);
};

#endif // MUENSTERBOUNDARYFINDERSTEP_H
