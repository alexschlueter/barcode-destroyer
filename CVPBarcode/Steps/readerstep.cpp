#include "readerstep.h"

ReaderStep::ReaderStep()
{

}
void ReaderStep::execute(QString data){(void)data;}
void ReaderStep::execute(cv::Mat data){
    this->image = data;
    //TODO read the Barcode
    emit completed(QString("1234567890123"));
}
