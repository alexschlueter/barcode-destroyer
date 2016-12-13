#include "templatematchingstep.h"
#include "lsdstep.h"

#include <QDirIterator>
#include <QDebug>

#include <iostream>
#include <memory>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

double Cell::orientedDistanceToClipLine(const Point2d &point, double clipCoord, ClipDirection dir) {
    switch (dir) {
    case ClipDirection::LEFT:
        return point.x - clipCoord;
    case ClipDirection::RIGHT:
        return clipCoord - point.x;
    case ClipDirection::TOP:
        return clipCoord - point.y;
    case ClipDirection::BOTTOM:
        return point.y - clipCoord;
    }
    return 0;
}

Point2d Cell::clipLineIntersection(const Point2d &p1, const Point2d &p2, double clipCoord, ClipDirection dir)
{
    double t, newCoord;
    switch (dir) {
    case ClipDirection::LEFT: case ClipDirection::RIGHT:
        t = (clipCoord - p2.x) / (p1.x - p2.x);
        newCoord = p2.y + t*(p1.y - p2.y);
        return {clipCoord, newCoord};

    case ClipDirection::TOP: case ClipDirection::BOTTOM:
        t = (clipCoord - p2.y) / (p1.y - p2.y);
        newCoord = p2.x + t*(p1.x - p2.x);
        return {newCoord, clipCoord};
    }
    return {0, 0};
}

bool Cell::clip(double clipCoord, ClipDirection dir)
{
    const double eps = 1e-9;
    vector<Point2d> newVerts;

    for (size_t i = 0; i < vertices.size(); i++) {
        auto p1 = vertices[i];
        auto p2 = vertices[(i+1)%vertices.size()];

        double oldDist = orientedDistanceToClipLine(p1, clipCoord, dir);
        double newDist = orientedDistanceToClipLine(p2, clipCoord, dir);

        if (oldDist * newDist < 0) {
            // we crossed the clip line
            // but do we need to add a vertex?
            if ((newDist < 0 && oldDist > eps) || newDist > eps) {
                // yes we do
                newVerts.push_back(clipLineIntersection(p1, p2, clipCoord, dir));
            }
        }
        if (newDist >= 0) {
            newVerts.push_back(p2);
        }
    }

    int nv = newVerts.size();
    if (nv < 3) return false;
    vertices = move(newVerts);

    area = 0;
    centroid = {0, 0};
    for (int i = 0; i < nv; i++) {
        Point2d &p1 = vertices[i];
        Point2d &p2 = vertices[(i+1)%nv];
        area += p1.x*p2.y - p1.y*p2.x;
        centroid.x += (p1.x+p2.x) * (p1.x*p2.y - p2.x*p1.y);
        centroid.y += (p1.y+p2.y) * (p1.x*p2.y - p2.x*p1.y);
    }
    area /= 2;
    centroid /= 6*area;

    return true;
}

void Cell::draw(Mat &img, int offsetX, int offsetY, RNG &rng, float scaleX=400, float scaleY=100) const
{
    Scalar color(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
    auto shiftedVerts = std::make_unique<Point[]>(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++) {
        shiftedVerts[i].x = scaleX*(offsetX + vertices[i].x);
        shiftedVerts[i].y = img.rows - scaleY*(offsetY + vertices[i].y);
        //cout << img.rows << " " << shiftedVerts[i] << endl;
    }
    fillConvexPoly(img, shiftedVerts.get(), vertices.size(), color);
}

const int TemplateMatchingStep::patterns[][4] = {
       {3, 2, 1, 1},
       {2, 2, 2, 1},
       {2, 1, 2, 2},
       {1, 4, 1, 1},
       {1, 1, 3, 2},
       {1, 2, 3, 1},
       {1, 1, 1, 4},
       {1, 3, 1, 2},
       {1, 2, 1, 3},
       {3, 1, 1, 2}
};

const int TemplateMatchingStep::firstDigitPatterns[][6] = {
       {0, 0, 0, 0, 0, 0},
       {0, 0, 1, 0, 1, 1},
       {0, 0, 1, 1, 0, 1},
       {0, 0, 1, 1, 1, 0},
       {0, 1, 0, 0, 1, 1},
       {0, 1, 1, 0, 0, 1},
       {0, 1, 1, 1, 0, 0},
       {0, 1, 0, 1, 0, 1},
       {0, 1, 0, 1, 1, 0},
       {0, 1, 1, 0, 1, 0}
};

TemplateMatchingStep::TemplateMatchingStep(QString cellpath)  : rng(13432123)
{
    QDirIterator itA(cellpath + "/A"), itB(cellpath + "/B");
    QDirIterator *it;
    for (int type = 0; type < 2; type++) {
        if (type == 0) it = &itA;
        else it = &itB;

        while (it->hasNext()) {
            QString filep = it->next();
            if (filep.right(4).toLower() == ".dat") {
                QFile file(filep);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    throw "TemplateMatchingStep: Could not open cell file";
                }
                int digit = filep.mid(filep.length()-5, 1).toInt();
                QTextStream in(&file);
                in.readLine();
                while (! in.atEnd()) {
                    QStringList parts = in.readLine().split(",");
                    double area = parts[1].toDouble();
                    Point2d centroid(parts[2].toDouble(), parts[3].toDouble());
                    int pixelsPerSection[6];
                    for (int i = 0; i < 6; i++)
                        pixelsPerSection[i] = parts[4+i].toInt();
                    vector<Point2d> vertices((parts.size()-10)/2);
                    for (int i = 0; i < (parts.size()-10)/2; i++) {
                        vertices[i] = Point2d(parts[10+2*i].toDouble(), parts[10+2*i+1].toDouble());
                    }
                    cellsPerDigit[type][digit].push_back(Cell(area, centroid, pixelsPerSection, move(vertices)));
                }
            }

        }
    }
    for (int type = 0; type < 2; type++) {
        cout << endl << "type " << type << endl;
        for (int i = 0; i < 10; i++) {
            for (const Cell &cell : cellsPerDigit[type][i]) {
                cout << cell << endl;
            }
        }
    }
}

void TemplateMatchingStep::execute(void* data){
    LSDResult *lsdres = static_cast<LSDResult*>(data);

    LineIterator scanIt(lsdres->img, lsdres->leftBnd, lsdres->rightBnd, 8, true);
    vector<uchar> scan(scanIt.count);

    for (int i = 0; i < scanIt.count; i++, ++scanIt) {
        scan[i] = **scanIt;
    }
    std::sort(scan.begin(), scan.end());
    double lowMean = 0, highMean = 0, lowVar = 0, highVar = 0, var = 0;
    for (int i = 0; i < scanIt.count/2; i++) {
        cout << (int)scan[i] << " ";
        lowMean += scan[i];
        lowVar += scan[i]*scan[i];
        var += scan[i]*scan[i];
    }
    cout << endl << endl;


    for (int i = scanIt.count / 2; i < scanIt.count; i++) {
        cout << (int)scan[i] << " ";
        highMean += scan[i];
        highVar += scan[i]*scan[i];
        var += scan[i]*scan[i];
    }
    cout << endl << endl;
    var = var/scanIt.count-pow((lowMean+highMean)/scanIt.count, 2);
    lowMean /= scanIt.count/2;
    lowVar = lowVar*2/scanIt.count - lowMean*lowMean;
    int n2 = scanIt.count - scanIt.count/2;
    highMean /= n2;
    highVar = highVar/n2 - highMean*highMean;

    double w = scanIt.count / 95.0;
    cout << "w = " << w << endl;
    vector<double> lowDists(scanIt.count), highDists(scanIt.count);
    scanIt = LineIterator(lsdres->img, lsdres->leftBnd, lsdres->rightBnd, 8, true);
    for (int i = 0; i < scanIt.count; i++, ++scanIt) {
        lowDists[i] = pow(max(**scanIt-lowMean, 0.0), 2)/(2*var);
        highDists[i] = pow(min(**scanIt-highMean, 0.0), 2)/(2*var);
    }

    cout << lowMean << " " << lowVar << " " << highMean << " " << highVar << " " << var << endl << endl;

    Mat plot(600, 800, CV_8UC3);
    for (unsigned int i = 1; i < lowDists.size(); i++) {
        cout << lowDists[i] << " ";
        line(plot, {2*i, (int)(200-lowDists[i-1]*2000)}, {2*(i+1), (int)(200-lowDists[i]*2000)}, {0, 255, 0});
    }
    cout << endl << endl;
    for (unsigned int i = 1; i < highDists.size(); i++) {
        cout << highDists[i] << " ";
        line(plot, {2*i, (int)(500-highDists[i-1]*2000)}, {2*(i+1), (int)(500-highDists[i]*2000)}, {0, 0, 255});
    }
    cout << endl << endl << endl;
    imshow("plot", plot);

    double deltaO = 2*w;
    double deltaW = 2*deltaO/95;

    int wmin = w-deltaW;
    int wmax = ceil(w+deltaW);
    double wClipLeft = w-deltaW-wmin;
    double wClipRight = w+deltaW-(int)(w+deltaW);
    double punif = 1 / (4*deltaO*deltaW);

    cout << "left " << wClipLeft << " right " << wClipRight << endl;
    cellPlot.create(800, 1000, CV_8UC3);

    MatchResult matchResults[12][2][10];
    for (int type = 0; type < 2; type++) {
        for (int digit = 0; digit < 10; digit++) {

            vector<Cell> leftRightClips[2];
            if (wmin == wmax-1) {
                for (const auto &cell : cellsPerDigit[type][digit]) {
                    Cell newCell(cell);
                    if (newCell.clip(wClipLeft, ClipDirection::LEFT) && newCell.clip(wClipRight, ClipDirection::RIGHT)) {
                        leftRightClips[0].push_back(move(newCell));
                    }
                }
            } else {
                for (const auto &cell : cellsPerDigit[type][digit]) {
                    Cell newCellLeft(cell), newCellRight(cell);
                    if (newCellLeft.clip(wClipLeft, ClipDirection::LEFT)) {
                        leftRightClips[0].push_back(move(newCellLeft));
                    }
                    if (newCellRight.clip(wClipRight, ClipDirection::RIGHT)) {
                        leftRightClips[1].push_back(move(newCellRight));
                    }
                }
            }

            int totalPos = type==1 ? 6 : 12;
            for (int pos = 0; pos < totalPos; pos++) {


                MatchResult &matchResult = matchResults[pos][type][digit];

                double o = 3*w + pos*7*w;
                int codeType;
                if (pos < 6) {
                    codeType = type;
                } else {
                    o += 5*w;
                    codeType = 2;
                }
                int omin = o-deltaO;
                int omax = ceil(o+deltaO);
                double oClipTop = o+deltaO-(int)(o+deltaO);
                double oClipBottom = o-deltaO-omin;

                if (wmin == wmax-1 && omin == omax-1) {
                    cout << "one section" << endl;
                    for (const auto &cell : leftRightClips[0]) {
                        Cell newCell(cell);
                        if (newCell.clip(oClipTop, ClipDirection::TOP) && newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                            matchResult += calcIntegralsOverCell(newCell, wmin, omin, codeType, digit, lowDists, highDists);
                        }
                    }
                } else if (wmin == wmax-1) {
                    cout << "w small" << endl;
                    for (const auto &cell : leftRightClips[0]) {
                        Cell newCell(cell);
                        if (newCell.clip(oClipTop, ClipDirection::TOP)) {
                            matchResult += calcIntegralsOverCell(newCell, wmin, omax-1, codeType, digit, lowDists, highDists);
                        }
                        newCell = cell;
                        if (newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                            matchResult += calcIntegralsOverCell(newCell, wmin, omin, codeType, digit, lowDists, highDists);
                        }
                        for (int io = omin+1; io < omax-1; io++) {
                            matchResult += calcIntegralsOverCell(cell, wmin, io, codeType, digit, lowDists, highDists);
                        }
                    }
                } else if (omin == omax-1) {
                    cout << "o small" << endl;
                    for (const auto &cell : leftRightClips[0]) {
                        Cell newCell(cell);
                        if (newCell.clip(oClipTop, ClipDirection::TOP) && newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                            matchResult += calcIntegralsOverCell(newCell, wmin, omin, codeType, digit, lowDists, highDists);
                        }
                    }
                    for (const auto &cell : leftRightClips[1]) {
                        Cell newCell(cell);
                        if (newCell.clip(oClipTop, ClipDirection::TOP) && newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                            matchResult += calcIntegralsOverCell(newCell, wmax-1, omin, codeType, digit, lowDists, highDists);
                        }
                    }
                    for (const auto &cell : cellsPerDigit[type][digit]) {
                        Cell newCell(cell);
                        if (newCell.clip(oClipTop, ClipDirection::TOP) && newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                            for (int iw = wmin+1; iw < wmax-1; iw++) {
                                matchResult += calcIntegralsOverCell(newCell, iw, omin, codeType, digit, lowDists, highDists);
                            }
                        }
                    }
                } else {
                    cout << "full" << endl;
                    for (const auto &cell : leftRightClips[0]) {
                        Cell newCell(cell);
                        if (newCell.clip(oClipTop, ClipDirection::TOP)) {
                            matchResult += calcIntegralsOverCell(newCell, wmin, omax-1, codeType, digit, lowDists, highDists);
                        }
                        newCell = cell;
                        if (newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                            matchResult += calcIntegralsOverCell(newCell, wmin, omin, codeType, digit, lowDists, highDists);
                        }
                        for (int io = omin+1; io < omax-1; io++) {
                            matchResult += calcIntegralsOverCell(cell, wmin, io, codeType, digit, lowDists, highDists);
                        }
                    }
                    for (const auto &cell : leftRightClips[1]) {
                        Cell newCell(cell);
                        if (newCell.clip(oClipTop, ClipDirection::TOP)) {
                            matchResult += calcIntegralsOverCell(newCell, wmax-1, omax-1, codeType, digit, lowDists, highDists);
                        }
                        newCell = cell;
                        if (newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                            matchResult += calcIntegralsOverCell(newCell, wmax-1, omin, codeType, digit, lowDists, highDists);
                        }
                        for (int io = omin+1; io < omax-1; io++) {
                            matchResult += calcIntegralsOverCell(cell, wmax-1, io, codeType, digit, lowDists, highDists);
                        }
                    }
                    for (const auto &cell : cellsPerDigit[type][digit]) {
                        Cell newCell(cell);
                        if (newCell.clip(oClipTop, ClipDirection::TOP)) {
                            for (int iw = wmin+1; iw < wmax-1; iw++) {
                                matchResult += calcIntegralsOverCell(newCell, iw, omax-1, codeType, digit, lowDists, highDists);
                            }
                        }
                        newCell = cell;
                        if (newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                            for (int iw = wmin+1; iw < wmax-1; iw++) {
                                matchResult += calcIntegralsOverCell(newCell, iw, omin, codeType, digit, lowDists, highDists);
                            }
                        }
                        for (int iw = wmin+1; iw < wmax-1; iw++) {
                            for (int io = omin+1; io < omax-1; io++) {
                                matchResult += calcIntegralsOverCell(cell, iw, io, codeType, digit, lowDists, highDists);
                            }
                        }
                    }
                } // if boundary conditions

                matchResult.woEstimate /= matchResult.likelihood;

                //imshow("cell plot", cellPlot);
                //cin.ignore();
            } // for pos
        } // for digit
    } // for type

    for (int pos = 0; pos < 12; pos++) {
        int endType = pos<6 ? 2 : 1;
        for (int type = 0; type < endType; type++) {
            for (int digit = 0; digit < 10; digit++) {
                cout << pos << " " << type << " " << digit << " " << matchResults[pos][type][digit] << endl;
            }
        }
        cout << endl;
    }

    double cost[12][2][10];
    int minTypes[11][2][10];
    int minDigits[11][2][10];
    for (int type = 0; type < 2; type++) {
        for (int digit = 0; digit < 10; digit++) {
            cost[0][type][digit] = -log(matchResults[0][type][digit].likelihood);
        }
    }
    for (int pos = 1; pos < 12; pos++) {
        int endType = pos<6 ? 2 : 1;
        for (int type = 0; type < endType; type++) {
            for (int digit = 0; digit < 10; digit++) {

                const MatchResult &mr = matchResults[pos][type][digit];
                double minCost = numeric_limits<double>::max();
                int minType, minDigit;
                int endType2 = pos<7 ? 2 : 1;
                for (int type2 = 0; type2 < endType2; type2++) {
                    for (int digit2 = 0; digit2 < 10; digit2++) {
                        const MatchResult &mr2 = matchResults[pos-1][type2][digit2];
                        double thisCost = cost[pos-1][type2][digit2] - log(mr.likelihood);
                        if (pos == 6) {
                            thisCost += alpha*pow(mr2.woEstimate.y + 12*mr2.woEstimate.x - mr.woEstimate.y, 2);
                        } else {
                            thisCost += alpha*pow(mr2.woEstimate.y + 7*mr2.woEstimate.x - mr.woEstimate.y, 2);
                        }
                        if (thisCost < minCost) {
                            minCost = thisCost;
                            minType = type2;
                            minDigit = digit2;
                        }
                    }
                }

                cost[pos][type][digit] = minCost;
                minTypes[pos-1][type][digit] = minType;
                minDigits[pos-1][type][digit] = minDigit;
            }
        }
    }

    double minCost = numeric_limits<double>::max();
    int minDigit;
    for (int digit = 0; digit < 10; digit++) {
        if (cost[11][0][digit] < minCost) {
            minCost = cost[11][0][digit];
            minDigit = digit;
        }
    }

    int barcode[13];
    int types[12];
    barcode[12] = minDigit;
    types[11] = 0;
    for (int pos = 10; pos >= 0; pos--) {
        barcode[pos+1] = minDigits[pos][types[pos+1]][barcode[pos+2]];
        types[pos] = minTypes[pos][types[pos+1]][barcode[pos+2]];
    }

    barcode[0] = -1;
    for (int firstDigit = 0; firstDigit < 10; firstDigit++) {
        bool found = true;
        for (int i = 0; i < 6; i++) {
            if (firstDigitPatterns[firstDigit][i] != types[i]) {
                found = false;
                break;
            }
        }
        if (found) {
            barcode[0] = firstDigit;
            break;
        }
    }
    QString *result = new QString();
    if (barcode[0] == -1) {
        *result = "firstDigitPattern invalid";
    } else {
        for (int i = 0; i < 13; i++) {
            *result += QString::number(barcode[i]);
        }
    }
    emit completed((void*)result);
}

MatchResult TemplateMatchingStep::calcIntegralsOverCell(const Cell &cell, int iw, int io, int type, int digit, const std::vector<double> &lowDists,
                                                 const std::vector<double> &highDists)
{
    //cout << "calc " << digit << " " << iw << " " << io << endl << cell << endl << endl;
    //cell.draw(cellPlot, iw, io, rng);
    double cost = 0;
    int offset = io;
    int totalPixInBar = iw + cell.pixelsPerBar[0];
    int parity = type==2 ? -1 : 1;
    for (int ibar = 0; ibar < totalPixInBar; ibar++) {
        cost += parity==1 ? lowDists[offset-ibar] : highDists[offset-ibar];
    }

    offset++;

    for (int bar = 0; bar < 4; bar++) {
        if (type == 1) {
            totalPixInBar = iw*patterns[digit][3-bar] + cell.pixelsPerBar[1+bar];
        } else {
            totalPixInBar = iw*patterns[digit][bar] + cell.pixelsPerBar[1+bar];
        }
        for (int ibar = 0; ibar < totalPixInBar; ibar++) {
            cost += parity==1 ? highDists[offset+ibar] : lowDists[offset+ibar];
        }
        offset += totalPixInBar;
        parity *= -1;
    }

    totalPixInBar = iw + cell.pixelsPerBar[5];
    for (int ibar = 0; ibar < totalPixInBar; ibar++) {
        cost += parity==1 ? highDists[offset+ibar] : lowDists[offset+ibar];
    }

    return MatchResult(exp(-cost)*cell.area, exp(-cost)*(Point2d(iw, io)+cell.centroid)*cell.area);
}
