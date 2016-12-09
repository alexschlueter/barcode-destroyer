#include "readerstep.h"

ReaderStep::ReaderStep()
{

}

void ReaderStep::execute(void* data){
    this->image = *static_cast<cv::Mat*>(data);
    //TODO read the Barcode
    QString result = "1234567890123";
    emit completed((void*)&result);
}
