/*
 * paper "Reading 1-D Barcodes with Mobile Phones Using Deformable Templates"
 * http://alumni.soe.ucsc.edu/~orazio/barcodes.html
*/

#ifndef TEMPLATEMATCHINGSTEP_H
#define TEMPLATEMATCHINGSTEP_H

#include "../Steps/step.h"
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
    std::vector<int> pixelsPerBar;
    std::vector<cv::Point2d> vertices;

    Cell(double _area, cv::Point2d _centroid, std::vector<int> _pixelsPerBar, std::vector<cv::Point2d> _vertices)
        : area(_area), centroid(_centroid), pixelsPerBar(_pixelsPerBar), vertices(std::move(_vertices)) {}

    Cell(const Cell &other) : Cell(other.area, other.centroid, other.pixelsPerBar, other.vertices) {}

    bool clip(double clipCoord, ClipDirection dir);
    void draw(cv::Mat &img, int offsetX, int offsetY, cv::RNG &rng, float scaleX, float scaleY) const;

private:
    double orientedDistanceToClipLine(const cv::Point2d &point, double clipCoord, ClipDirection dir);
    cv::Point2d clipLineIntersection(const cv::Point2d &p1, const cv::Point2d &p2, double clipCoord, ClipDirection dir);
};

struct Pattern {
    enum Type { A, B, C, GUARD, MID };

    static const int basePatterns[][4];
    static const Type firstDigitPatterns[][6];
    Type type;
    int digit;

    Pattern() = default;
    Pattern(const Pattern &) = default;
    Pattern(Type t) : type(t) {}
    Pattern(Type t, int d) : type(t), digit(d) {}

    int operator[](int i) const {
        switch (type) {
        case A: case C:
            return basePatterns[digit][i];
        case B:
            return basePatterns[digit][3-i];
        case GUARD: case MID:
            return 1;
        }
        throw "Pattern::operator[]";
    }
    int size() const {
        switch (type) {
        case A: case B: case C:
            return 4;
        case GUARD:
            return 3;
        case MID:
            return 5;
        }
        throw "Pattern::size";
    }
    int baseUnits() const {
        switch (type) {
        case A: case B: case C:
            return 7;
        case GUARD:
            return 3;
        case MID:
            return 5;
        }
        throw "Pattern::baseUnits";
    }
    bool firstWhite() const {
        switch (type) {
        case A: case B: case MID:
            return true;
        case C: case GUARD:
            return false;
        }
        throw "Pattern::firstWhite";
    }
    std::string toString() const {
        switch (type) {
        case A:
            return std::to_string(digit)+"A";
        case B:
            return std::to_string(digit)+"B";
        case C:
            return std::to_string(digit)+"C";
        case GUARD:
            return "G";
        case MID:
            return "MID";
        }
        throw "Pattern::toString";
    }
};

struct MatchResult {
    Pattern pattern;
    double likelihood;
    cv::Point2d woEstimate;

    MatchResult() = default;
    MatchResult(const Pattern &_pattern) : pattern(_pattern), likelihood(0), woEstimate(0, 0) {}
    MatchResult(double _likelihood, cv::Point2d _woEstimate) : likelihood(_likelihood), woEstimate(_woEstimate) {}

    double spatialInconsistency(const MatchResult &right) const {
        return woEstimate.y + pattern.baseUnits()*woEstimate.x - right.woEstimate.y;
    }
    MatchResult &operator+=(const MatchResult &other) {
        likelihood += other.likelihood;
        woEstimate += other.woEstimate;
        return *this;
    }

    void draw(cv::Mat &img, int oStart, int xScale, const std::array<double, 2> &levels, const cv::Scalar &color) const;
};

inline std::ostream &operator<<(std::ostream &os, const MatchResult &mr) {
    os << "MatchResult likelihood = " << mr.likelihood << ", woEstimate = " << mr.woEstimate;
    return os;
}

struct ReadingResult {
    cv::Point2f leftBnd, rightBnd;
    std::vector<uchar> scanLine;
    std::array<double, 2> levels;
    int oStart;

    enum { SUCCESS, FAIL } state;
    std::array<MatchResult, 15> matchResults;
    std::array<double, 14> consistencyCosts;
    std::array<int, 13> barcode;
    double cost;

    ReadingResult(cv::Point2f _leftBnd, cv::Point2f _rightBnd, std::vector<uchar> _scanLine)
        : leftBnd(std::move(_leftBnd)), rightBnd(std::move(_rightBnd)), scanLine(std::move(_scanLine)), levels({}), cost(0) {}

    MatchResult &digitMR(int pos) { return matchResults[pos < 6 ? pos+1 : pos+2]; }
    double &consistencyCostBeforePos(int pos) { return consistencyCosts[pos < 6 ? pos : pos+1]; }
    double &consistencyCostAfterPos(int pos) { return consistencyCosts[pos < 6 ? pos+1 : pos+2]; }

    void update();
    void draw(cv::Mat &img) const;
    int calcCheckDigit() const;
};

class TemplateMatchingStep : public Step
{
    Q_OBJECT
public:
    TemplateMatchingStep(QString cellpath);
    ReadingResult readBarcodeFromLine(const cv::Mat &img, cv::Point2f leftBnd, cv::Point2f rightBnd);

public slots:
    void execute(void* data);

private:
    const double alpha = 0.1;
    //const double alpha = 10; // too high
    const int numLines = 7;

    std::vector<Cell> cellsPerDigit[2][10];
    std::vector<Cell> guardCells;
    std::vector<Cell> midCells;
    cv::Mat cellPlot;
    cv::Mat spatialConsistency;
    cv::RNG rng;

    static std::vector<Cell> readCellsFromFile(QString filepath);
    static void prepareLeftRightClips(bool wSameSection, double wClipLeft, double wClipRight, std::array<std::vector<Cell>*, 3> &cells);
    static MatchResult calcIntegralsOverCell(const Cell &cell, int iw, int io, const Pattern &pattern, const std::array<std::vector<double>, 2> &dists);
    MatchResult matchTemplate(double o, double deltaO, int wmin, int wmax, const Pattern &pattern,
                              const std::array<std::vector<Cell>*, 3> &cells, const std::array<std::vector<double>, 2> &dists) const;
};

inline std::ostream &operator<<(std::ostream &os, const Cell &cell) {
    os << "Cell area = " << cell.area << ", centroid = " << cell.centroid << ", pixPerBar = [";
    for (size_t i = 0; i < cell.pixelsPerBar.size()-1; i++)
        os << cell.pixelsPerBar[i] << ", ";
    os << cell.pixelsPerBar.back() << "], vertices = [ ";
    for (size_t i = 0; i < cell.vertices.size()-1; i++)
        os << cell.vertices[i] << ", ";
    os << cell.vertices.back() << " ]";
    return os;
}

#endif // TEMPLATEMATCHINGSTEP_H
