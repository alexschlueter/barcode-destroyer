/*
 * paper "Reading 1-D Barcodes with Mobile Phones Using Deformable Templates"
 * http://alumni.soe.ucsc.edu/~orazio/barcodes.html
*/

#ifndef TEMPLATEMATCHINGSTEP_H
#define TEMPLATEMATCHINGSTEP_H

#include "Steps/step.h"
#include "QString"
#include <ostream>

enum class ClipDirection {
    LEFT, // everything to the left of the clip line is thrown away
    RIGHT, // etc.
    TOP,
    BOTTOM
};

// Represents a cell in (w, o) space, where w is the base width (width of the smallest line in the barcode)
// and o is the offset along the scan line at which the current digit starts.
// See Fig. 6 in the paper.
// The cells are the interior of a convex polygon. For all pairs (w, o) which lie in the cell,
// the cost of matching a template with base width w and offset o to the scan line is constant,
// because no pixel jumps from one barcode bar in the template to another.
// The probability integrals in the paper can therefore be computed by summing over all cells.
struct Cell {
    double area; // needed for integrals
    cv::Point2d centroid; // needed for least squares estimator of the most likely (w, o) pair
    std::vector<int> pixelsPerBar; // For all pairs (w, o) in the cell, no pixels jump between bars in the template, so we can associate the number of pixels in each bar to the cell
    std::vector<cv::Point2d> vertices; // vertices of the boundary polygon

    Cell(double _area, cv::Point2d _centroid, std::vector<int> _pixelsPerBar, std::vector<cv::Point2d> _vertices)
        : area(_area), centroid(_centroid), pixelsPerBar(_pixelsPerBar), vertices(std::move(_vertices)) {}

    Cell(const Cell &other) : Cell(other.area, other.centroid, other.pixelsPerBar, other.vertices) {}

    // Clip the cell at a clip line. E.g. if clipCoord is 2.5 and dir=LEFT, everything to the left of w=2.5 is cut off
    // If clipCoord=3 and dir=TOP, everything above o=3 is cut off, etc.
    bool clip(double clipCoord, ClipDirection dir);
    // for debugging
    void draw(cv::Mat &img, int offsetX, int offsetY, cv::RNG &rng, float scaleX, float scaleY) const;

private:
    double orientedDistanceToClipLine(const cv::Point2d &point, double clipCoord, ClipDirection dir);
    cv::Point2d clipLineIntersection(const cv::Point2d &p1, const cv::Point2d &p2, double clipCoord, ClipDirection dir);
};

// Represents a digit pattern in the EAN-13 standard
// http://www.gs1.org/barcodes-epcrfid-id-keys/gs1-general-specifications
struct Pattern {
    enum Type { A, B, C }; // see standard paper
    // If left/right of the digit there is a guard pattern (boundary guard or mid guard), the pattern is extended by 3 bars to match the guard
    // Matching the guards on their own doesn't work as well, because the patterns have too few bars.
    // This is different from the paper! They don't match guard patterns at all.
    enum Extension { LEFT=0, MID=1, RIGHT=2 };

    // Array of the patterns of type A. The other types can be derived from these, because
    // B digits are just these patterns read right-to-left, and C digits are just black/white inverted
    static const int basePatterns[][4];
    // patterns for reading the first digit, which is calculated from the pattern types of the first six digits
    static const Type firstDigitPatterns[][6];
    Type type;
    Extension extension;
    int digit;

    Pattern() = default;
    Pattern(const Pattern &) = default;
    //Pattern(Type t) : type(t) {}
    Pattern(Type t, Extension e, int d) : type(t), extension(e), digit(d) {}

    // How many unit widths is bar i long, disregarding possible extension by a guard pattern?
    int actualBarSize(int i) const {
        switch (type) {
        case A: case C:
            return basePatterns[digit][i];
        case B:
            return basePatterns[digit][3-i];
        }
        throw "Pattern::actualBarSize";
    }
    // Unit widths of bar i, including extension
    int operator[](int i) const {
        switch (extension) {
        case LEFT:
            if (i < 3) return 1;
            else return actualBarSize(i-3);
        case MID:
            return actualBarSize(i);
        case RIGHT:
            if (i > 3) return 1;
            else return actualBarSize(i);
        }
        throw "Pattern::operator[]";
    }
    int numBars() const {
        switch (extension) {
        case LEFT: case RIGHT:
            return 7;
        case MID:
            return 4;
        }
        throw "Pattern::numBars";
    }
    int baseUnits() const {
        switch (extension) {
        case LEFT: case RIGHT:
            return 10;
        case MID:
            return 7;
        }
        throw "Pattern::baseUnits";
    }
    // Is the first bar white?
    bool firstWhite() const {
        bool fw;
        switch (type) {
        case A: case B:
            fw = true; break;
        case C:
            fw = false; break;
        }
        if (extension == LEFT) fw = !fw;
        return fw;
    }
    std::string toString() const {
        std::string s(std::to_string(digit));
        switch (type) {
        case A:
            s += "A"; break;
        case B:
            s += "B"; break;
        case C:
            s += "C"; break;
        }
        switch (extension) {
        case LEFT:
            s += "L"; break;
        case MID:
            s += "M"; break;
        case RIGHT:
            s += "R"; break;
        }
        return s;
    }
};

// Result of matching one pattern to the scan line
struct MatchResult {
    Pattern pattern;
    double likelihood;
    cv::Point2d woEstimate; // The most likely value of (w, o), see equation (15) in the paper

    MatchResult() = default;
    MatchResult(const Pattern &_pattern) : pattern(_pattern), likelihood(0), woEstimate(0, 0) {}
    MatchResult(double _likelihood, cv::Point2d _woEstimate) : likelihood(_likelihood), woEstimate(_woEstimate) {}

    // calculate the overlap between this MatchResult and right, see eq. (16)
    double spatialInconsistency(const MatchResult &right) const {
        int leftUnits = pattern.baseUnits();
        if (pattern.extension == Pattern::RIGHT) leftUnits--; // there's one unit overlap in the middle of the barcode, because both sides match 3 bars of the mid guard
        return woEstimate.y + leftUnits*woEstimate.x - right.woEstimate.y;
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

// Result of the template matching the whole scan line
struct ReadingResult {
    cv::Point2f leftBnd, rightBnd;
    std::vector<uchar> scanLine;
    std::array<std::vector<double>, 2> dists;
    std::array<double, 2> levels;
    int oStart;

    enum { SUCCESS, FAIL } state;
    std::array<MatchResult, 12> matchResults;
    std::array<double, 11> consistencyCosts;
    std::array<int, 13> barcode;
    double cost;

    ReadingResult(cv::Point2f _leftBnd, cv::Point2f _rightBnd, std::vector<uchar> _scanLine)
        : leftBnd(std::move(_leftBnd)), rightBnd(std::move(_rightBnd)), scanLine(std::move(_scanLine)), levels({}), cost(0) {}

    MatchResult &digitMR(int pos) { return matchResults[pos]; }
    double &consistencyCostBeforePos(int pos) { return consistencyCosts[pos-1]; }
    double &consistencyCostAfterPos(int pos) { return consistencyCosts[pos]; }

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

    std::vector<Cell> cellsPerDigit[2][3][10]; // read from cell folder
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
