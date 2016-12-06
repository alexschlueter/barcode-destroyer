#include "step.h"
#include <iostream>
Step::Step()
{

}

void Step::execute(void* data){std::cout << data << std::endl;}
