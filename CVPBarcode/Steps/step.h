#ifndef STEP_H
#define STEP_H

#include <QObject>
#include <QThread>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


class Step : public QObject
{
    Q_OBJECT
public:
    Step();
public slots:
    virtual void execute(void *data);

signals:
    void completed(void*);
};

#endif // STEP_H
