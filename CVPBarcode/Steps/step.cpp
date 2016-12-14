#include "step.h"
#include <iostream>
Step::Step(bool _visualize) : visualize(_visualize)
{

}

void Step::execute(void* data){std::cout << data << std::endl;}
