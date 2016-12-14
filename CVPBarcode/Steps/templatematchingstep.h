#ifndef TEMPLATEMATCHINGSTEP_H
#define TEMPLATEMATCHINGSTEP_H

#include "Steps/step.h"
#include "QString"
#include <ostream>

enum class ClipDirection {
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

struct Cell {
    double area;
    cv::Point2d centroid;
    int pixelsPerBar[6];
    std::vector<cv::Point2d> vertices;

    Cell(double _area, cv::Point2d _centroid, const int _pixelsPerBar[], std::vector<cv::Point2d> _vertices)
        : area(_area), centroid(_centroid), vertices(std::move(_vertices)) {
        for (int i = 0; i < 6; i++)
            pixelsPerBar[i] = _pixelsPerBar[i];
    }

    Cell(const Cell &other) : Cell(other.area, other.centroid, other.pixelsPerBar, other.vertices) {}

    bool clip(double clipCoord, ClipDirection dir);
    void draw(cv::Mat &img, int offsetX, int offsetY, cv::RNG &rng, float scaleX, float scaleY) const;

private:
    double orientedDistanceToClipLine(const cv::Point2d &point, double clipCoord, ClipDirection dir);
    cv::Point2d clipLineIntersection(const cv::Point2d &p1, const cv::Point2d &p2, double clipCoord, ClipDirection dir);
};

struct MatchResult {
    double likelihood;
    cv::Point2d woEstimate;

    MatchResult() : likelihood(0), woEstimate(0, 0) {}
    MatchResult(double _likelihood, cv::Point2d _woEstimate) : likelihood(_likelihood), woEstimate(_woEstimate) {}

    MatchResult &operator+=(const MatchResult &other) {
        likelihood += other.likelihood;
        woEstimate += other.woEstimate;
        return *this;
    }
};

inline std::ostream &operator<<(std::ostream &os, const MatchResult &mr) {
    os << "MatchResult likelihood = " << mr.likelihood << ", woEstimate = " << mr.woEstimate;
    return os;
}

class TemplateMatchingStep : public Step
{
    Q_OBJECT
public:
    TemplateMatchingStep(QString cellpath);
    bool readBarcodeFromLine(const cv::Mat &img, cv::Point2f leftBnd, cv::Point2f rightBnd, int barcode[]);

    static int calcCheckDigit(int barcode[12]);
public slots:
    void execute(void* data);

private:
    const double alpha = 0.1;
    const int numLines = 7;

    static const int patterns[][4];
    static const int firstDigitPatterns[][6];
    std::vector<Cell> cellsPerDigit[2][10];
    cv::Mat cellPlot;
    cv::RNG rng;

    MatchResult calcIntegralsOverCell(const Cell &cell, int iw, int io, int type, int digit, const std::vector<double> &lowDists, const std::vector<double> &highDists);
};

inline std::ostream &operator<<(std::ostream &os, const Cell &cell) {
    os << "Cell area = " << cell.area << ", centroid = " << cell.centroid << ", pixPerBar = [";
    for (int i = 0; i < 5; i++)
        os << cell.pixelsPerBar[i] << ", ";
    os << cell.pixelsPerBar[5] << "], vertices = [ ";
    for (size_t i = 0; i < cell.vertices.size()-1; i++)
        os << cell.vertices[i] << ", ";
    os << cell.vertices[cell.vertices.size()-1] << " ]";
    return os;
}

#endif // TEMPLATEMATCHINGSTEP_H
