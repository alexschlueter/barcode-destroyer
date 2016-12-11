#ifndef TEMPLATEMATCHINGSTEP_H
#define TEMPLATEMATCHINGSTEP_H

#include "Steps/step.h"
#include "QString"
#include <ostream>

struct Cell {
    double area;
    cv::Point2d centroid;
    int pixelsPerBar[6];

    Cell(double _area, cv::Point2d _centroid, int _pixelsPerSection[])
        : area(_area), centroid(_centroid) {
        for (int i = 0; i < 6; i++)
            pixelsPerBar[i] = _pixelsPerSection[i];
    }
};

class TemplateMatchingStep : public Step
{
    Q_OBJECT
public:
    TemplateMatchingStep(QString cellpath);
public slots:
    void execute(void* data);
private:
    static const int patterns[][4];
    std::vector<Cell> cellsPerDigit[10];
};

inline std::ostream &operator<<(std::ostream &os, const Cell &cell) {
    os << "Cell area = " << cell.area << ", centroid = " << cell.centroid << ", pixPerBar = [";
    for (int i = 0; i < 5; i++)
        os << cell.pixelsPerBar[i] << ", ";
    os << cell.pixelsPerBar[5] << "]";
    return os;
}

#endif // TEMPLATEMATCHINGSTEP_H
