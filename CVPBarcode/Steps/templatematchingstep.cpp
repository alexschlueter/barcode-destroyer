#include "templatematchingstep.h"
#include "lsdstep.h"
#include "../utils.h"

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
                newVerts.emplace_back(clipLineIntersection(p1, p2, clipCoord, dir));
            }
        }
        if (newDist >= 0) {
            newVerts.emplace_back(p2);
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

void Cell::draw(Mat &img, int offsetX, int offsetY, RNG &rng, float scaleX=400, float scaleY=50) const
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

const int Pattern::basePatterns[][4] = {
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

const Pattern::Type Pattern::firstDigitPatterns[][6] = {
       {A, A, A, A, A, A},
       {A, A, B, A, B, B},
       {A, A, B, B, A, B},
       {A, A, B, B, B, A},
       {A, B, A, A, B, B},
       {A, B, B, A, A, B},
       {A, B, B, B, A, A},
       {A, B, A, B, A, B},
       {A, B, A, B, B, A},
       {A, B, B, A, B, A}
};

vector<Cell> TemplateMatchingStep::readCellsFromFile(QString filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString ex("TemplateMatchingStep: Could not open cell file " + filepath);
        throw BarcodeError(ex.toLatin1());

    }
    QTextStream in(&file);
    in.readLine();
    vector<Cell> cells;
    while (! in.atEnd()) {
        QStringList parts = in.readLine().split(",");
        double area = parts[1].toDouble();
        Point2d centroid(parts[2].toDouble(), parts[3].toDouble());
        int numBars = parts[4].toInt();
        vector<int> pixPerBar(numBars);
        for (int i = 0; i < numBars; i++)
            pixPerBar[i] = parts[5+i].toInt();
        vector<Point2d> vertices((parts.size()-5-numBars)/2);
        for (int i = 0; i < (parts.size()-5-numBars)/2; i++) {
            vertices[i] = Point2d(parts[5+numBars+2*i].toDouble(), parts[5+numBars+2*i+1].toDouble());
        }
        cells.emplace_back(Cell(area, centroid, move(pixPerBar), move(vertices)));
    }
    return cells;
}

TemplateMatchingStep::TemplateMatchingStep(QString cellpath)  : rng(13432123)
{
    guardCells = readCellsFromFile(cellpath + "/guard.dat");
    midCells = readCellsFromFile(cellpath + "/mid.dat");

    QDirIterator itA(cellpath + "/A"), itB(cellpath + "/B");
    QDirIterator *it;
    for (int type = 0; type < 2; type++) {
        if (type == 0) it = &itA;
        else it = &itB;

        while (it->hasNext()) {
            QString filep = it->next();
            if (filep.right(4).toLower() == ".dat") {
                int digit = filep.mid(filep.length()-5, 1).toInt();
                cellsPerDigit[type][digit] = readCellsFromFile(filep);
            }

        }
    }

    /*
    for (int type = 0; type < 2; type++) {
        cout << endl << "type " << type << endl;
        for (int i = 0; i < 10; i++) {
            for (const Cell &cell : cellsPerDigit[type][i]) {
                cout << cell << endl;
            }
        }
    }
    */
}

void TemplateMatchingStep::execute(void* data) {
    unique_ptr<LocalizationResult> locRes(static_cast<LocalizationResult*>(data));
    vector<ReadingResult> candidates;
    for (const auto &leftBnd : locRes->leftBnds) {
        for (const auto &rightBnd : locRes->rightBnds) {
            Point perp(leftBnd.y - rightBnd.y, rightBnd.x - leftBnd.x);
            perp *= locRes->height / (2*norm(perp));
            //Mat vis = locRes->img.clone();

            //array<int, 13> barcode;

            int totalSteps = numLines / 2 + 1;
            for (int line = 0; line < numLines; line++) {
                int dir = line%2 ? 1 : -1;
                float offset = ((line+1) / 2) / (float)totalSteps;
                //cout << "scan line " << line << " " << dir << " " << offset << endl;
                //cv::line(vis, locRes->leftBnd+dir*offset*perp, locRes->rightBnd+dir*offset*perp, 3, 0);
                // TODO: bounds checking
                for (int orientation = 0; orientation < 2; orientation++) {
                    Point2f leftBndOffs, rightBndOffs;
                    if (orientation) {
                        rightBndOffs = leftBnd+dir*offset*perp;
                        leftBndOffs = rightBnd+dir*offset*perp;
                    } else {
                        leftBndOffs = leftBnd+dir*offset*perp;
                        rightBndOffs = rightBnd+dir*offset*perp;
                    }
                    auto readRes = readBarcodeFromLine(locRes->img, leftBndOffs, rightBndOffs);
                    if (readRes.state == ReadingResult::SUCCESS) {
                        candidates.emplace_back(move(readRes));
                    }
                }
            }
        }
    }
    //emit showImage("scanlines", vis);
    QString *result = new QString();
    if (candidates.empty()) {
        *result = "fail";                           // draw all tries?
    } else {
        /*int maxFrequency = 0;
        array<int, 13> bestCandidate;
        map<array<int, 13>, int> freqencies;
        for (auto &&code : candidates) {
            int f = ++freqencies[code];
            if (f > maxFrequency) {
                maxFrequency = f;
                bestCandidate = code;
            }
        }
        */
        // TODO: when the same numbers are read multiple times, combine costs somehow
        const auto &bestCandidate = *min_element(candidates.begin(), candidates.end(), [](auto &a, auto &b) { return a.cost < b.cost; });

        int visRows = 400;
        int visCols = 1200;
        Mat vis(visRows, visCols, CV_8UC3, {255, 255, 255});
        bestCandidate.draw(vis);
        emit showImage("Template Matching", vis);

        for (int i = 0; i < 13; i++)
            *result += QString::number(bestCandidate.barcode[i]);
    }
    emit completed((void*)result);
}

MatchResult TemplateMatchingStep::matchTemplate(double o, double deltaO, int wmin, int wmax, const Pattern &pattern,
                                                const array<vector<Cell>*, 3> &cells, const array<vector<double>, 2> &dists) const {
    int omin = o-deltaO;
    int omax = ceil(o+deltaO);
    double oClipTop = o+deltaO-(int)(o+deltaO);
    double oClipBottom = o-deltaO-omin;
    MatchResult matchResult(pattern);
    if (wmin == wmax-1 && omin == omax-1) {
        //cout << "one section" << endl;
        for (const auto &cell : *cells[1]) {
            Cell newCell(cell);
            if (newCell.clip(oClipTop, ClipDirection::TOP) && newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                matchResult += calcIntegralsOverCell(newCell, wmin, omin, pattern, dists);
            }
        }
    } else if (wmin == wmax-1) {
        for (const auto &cell : *cells[1]) {
            Cell newCell(cell);
            if (newCell.clip(oClipTop, ClipDirection::TOP)) {
                matchResult += calcIntegralsOverCell(newCell, wmin, omax-1, pattern, dists);
            }
            newCell = cell;
            if (newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                matchResult += calcIntegralsOverCell(newCell, wmin, omin, pattern, dists);
            }
            for (int io = omin+1; io < omax-1; io++) {
                matchResult += calcIntegralsOverCell(cell, wmin, io, pattern, dists);
            }
        }
    } else if (omin == omax-1) {
        cout << "o small" << endl;
        for (const auto &cell : *cells[1]) {
            Cell newCell(cell);
            if (newCell.clip(oClipTop, ClipDirection::TOP) && newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                matchResult += calcIntegralsOverCell(newCell, wmin, omin, pattern, dists);
            }
        }
        for (const auto &cell : *cells[2]) {
            Cell newCell(cell);
            if (newCell.clip(oClipTop, ClipDirection::TOP) && newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                matchResult += calcIntegralsOverCell(newCell, wmax-1, omin, pattern, dists);
            }
        }
        for (const auto &cell : *cells[0]) {
            Cell newCell(cell);
            if (newCell.clip(oClipTop, ClipDirection::TOP) && newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                for (int iw = wmin+1; iw < wmax-1; iw++) {
                    matchResult += calcIntegralsOverCell(newCell, iw, omin, pattern, dists);
                }
            }
        }
    } else {
        //cout << "full" << endl;
        for (const auto &cell : *cells[1]) {
            Cell newCell(cell);
            if (newCell.clip(oClipTop, ClipDirection::TOP)) {
                matchResult += calcIntegralsOverCell(newCell, wmin, omax-1, pattern, dists);
            }
            newCell = cell;
            if (newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                matchResult += calcIntegralsOverCell(newCell, wmin, omin, pattern, dists);
            }
            for (int io = omin+1; io < omax-1; io++) {
                matchResult += calcIntegralsOverCell(cell, wmin, io, pattern, dists);
            }
        }
        for (const auto &cell : *cells[2]) {
            Cell newCell(cell);
            if (newCell.clip(oClipTop, ClipDirection::TOP)) {
                matchResult += calcIntegralsOverCell(newCell, wmax-1, omax-1, pattern, dists);
            }
            newCell = cell;
            if (newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                matchResult += calcIntegralsOverCell(newCell, wmax-1, omin, pattern, dists);
            }
            for (int io = omin+1; io < omax-1; io++) {
                matchResult += calcIntegralsOverCell(cell, wmax-1, io, pattern, dists);
            }
        }
        for (const auto &cell : *cells[0]) {
            Cell newCell(cell);
            if (newCell.clip(oClipTop, ClipDirection::TOP)) {
                for (int iw = wmin+1; iw < wmax-1; iw++) {
                    matchResult += calcIntegralsOverCell(newCell, iw, omax-1, pattern, dists);
                }
            }
            newCell = cell;
            if (newCell.clip(oClipBottom, ClipDirection::BOTTOM)) {
                for (int iw = wmin+1; iw < wmax-1; iw++) {
                    matchResult += calcIntegralsOverCell(newCell, iw, omin, pattern, dists);
                }
            }
            for (int iw = wmin+1; iw < wmax-1; iw++) {
                for (int io = omin+1; io < omax-1; io++) {
                    matchResult += calcIntegralsOverCell(cell, iw, io, pattern, dists);
                }
            }
        }
    } // if boundary conditions

    matchResult.woEstimate /= matchResult.likelihood;
    return matchResult;
}

void TemplateMatchingStep::prepareLeftRightClips(bool wSameSection, double wClipLeft, double wClipRight, array<vector<Cell>*, 3> &cells) {
    cells[1]->clear();
    cells[2]->clear();
    if (wSameSection) {
        //cout << "w small" << endl;
        for (const auto &cell : *cells[0]) {
            Cell newCell(cell);
            if (newCell.clip(wClipLeft, ClipDirection::LEFT) && newCell.clip(wClipRight, ClipDirection::RIGHT)) {
                cells[1]->emplace_back(move(newCell));
            }
        }
    } else {
        for (const auto &cell : *cells[0]) {
            Cell newCellLeft(cell), newCellRight(cell);
            if (newCellLeft.clip(wClipLeft, ClipDirection::LEFT)) {
                cells[1]->emplace_back(move(newCellLeft));
            }
            if (newCellRight.clip(wClipRight, ClipDirection::RIGHT)) {
                cells[2]->emplace_back(move(newCellRight));
            }
        }
    }
}

ReadingResult TemplateMatchingStep::readBarcodeFromLine(const Mat &img, Point2f leftBnd, Point2f rightBnd) {
    LineIterator scanIt(img, leftBnd, rightBnd);
    vector<uchar> scan(scanIt.count);

    for (int i = 0; i < scanIt.count; i++, ++scanIt) {
        scan[i] = **scanIt;
    }

    ReadingResult readRes(leftBnd, rightBnd, scan);

    std::nth_element(scan.begin(), scan.begin() + scanIt.count/2, scan.end());

    double lowVar = 0, highVar = 0, var = 0;
    for (int i = 0; i < scanIt.count/2; i++) {
        readRes.levels[0] += scan[i];
        lowVar += scan[i]*scan[i];
        var += scan[i]*scan[i];
    }

    for (int i = scanIt.count / 2; i < scanIt.count; i++) {
        readRes.levels[1] += scan[i];
        highVar += scan[i]*scan[i];
        var += scan[i]*scan[i];
    }

    var = var/scanIt.count-pow((readRes.levels[0]+readRes.levels[1])/scanIt.count, 2);
    readRes.levels[0] /= scanIt.count/2;
    lowVar = lowVar*2.0/scanIt.count - readRes.levels[0]*readRes.levels[0];
    int n2 = scanIt.count - scanIt.count/2;
    readRes.levels[1] /= n2;
    highVar = highVar/n2 - readRes.levels[1]*readRes.levels[1];

    double w = scanIt.count / 95.0; // TODO: scanIt vs. fullScanIt slightly different?
    //cout << "w = " << w << endl;

    float fullWidth = norm(leftBnd-rightBnd);
    float longEnough = (img.rows*img.rows+img.cols*img.cols)/fullWidth;
    LineIterator fullScanIt(img, leftBnd+longEnough*(leftBnd-rightBnd), rightBnd+longEnough*(rightBnd-leftBnd));
    array<vector<double>, 2> dists = {vector<double>(fullScanIt.count), vector<double>(fullScanIt.count)};

    readRes.oStart = -1;
    for (int i = 0; i < fullScanIt.count; i++, ++fullScanIt) {
        if (readRes.oStart == -1 && norm((Point2f)fullScanIt.pos()-rightBnd) <= fullWidth) {
            readRes.oStart = i;
        }
        dists[0][i] = pow(max(**fullScanIt-readRes.levels[0], 0.0), 2)/(2*var);
        dists[1][i] = pow(min(**fullScanIt-readRes.levels[1], 0.0), 2)/(2*var);
    }

    //cout << readRes.levels[0] << " " << lowVar << " " << readRes.levels[1] << " " << highVar << " " << var << endl << endl;

    // TODO: tune parameters
    double deltaO = 3*w;
    // TODO: bounds checking! scan line long enough?
    double deltaW = 2*deltaO/95;

    int wmin = w-deltaW;
    int wmax = ceil(w+deltaW);
    bool wSameSection = wmin == wmax-1;
    double wClipLeft = w-deltaW-wmin;
    double wClipRight = w+deltaW-(int)(w+deltaW);

    //cout << "wClipLeft " << wClipLeft << " wClipRight " << wClipRight << endl;
    //cellPlot.create(800, 1000, CV_8UC3);

    vector<Cell> leftOrBothClips, maybeRightClips;
    array<vector<Cell>*, 3> guardClips({&guardCells, &leftOrBothClips, &maybeRightClips});
    prepareLeftRightClips(wSameSection, wClipLeft, wClipRight, guardClips);
    readRes.matchResults[0] = matchTemplate(readRes.oStart, w, wmin, wmax, Pattern(Pattern::GUARD), guardClips, dists);
    readRes.matchResults[14] = matchTemplate(readRes.oStart+92*w, w, wmin, wmax, Pattern(Pattern::GUARD), guardClips, dists);

    array<vector<Cell>*, 3> midClips({&midCells, &leftOrBothClips, &maybeRightClips});
    prepareLeftRightClips(wSameSection, wClipLeft, wClipRight, midClips);
    readRes.matchResults[7] = matchTemplate(readRes.oStart+45*w, deltaO, wmin, wmax, Pattern(Pattern::MID), midClips, dists);

    array<vector<MatchResult>, 12> matchResults;
    for (int type = 0; type < 2; type++) {
        for (int digit = 0; digit < 10; digit++) {
            array<vector<Cell>*, 3> clips({&cellsPerDigit[type][digit], &leftOrBothClips, &maybeRightClips});
            prepareLeftRightClips(wSameSection, wClipLeft, wClipRight, clips);

            int totalPos = type==1 ? 6 : 12;
            for (int pos = 0; pos < totalPos; pos++) {

                double o = readRes.oStart + 3*w + pos*7*w;
                Pattern pattern;
                if (pos < 6) {
                    if (type == 0) pattern = Pattern(Pattern::A, digit);
                    else pattern = Pattern(Pattern::B, digit);
                } else {
                    o += 5*w;
                    pattern = Pattern(Pattern::C, digit);
                }
                //matchResults[pos][type][digit] = matchTemplate(o, deltaO, wmin, wmax, pattern, clips, dists);
                matchResults[pos].emplace_back(matchTemplate(o, deltaO, wmin, wmax, pattern, clips, dists));

                //imshow("cell plot", cellPlot);
                //cin.ignore();
            } // for pos
        } // for digit
    } // for type

    /*
    for (int pos = 0; pos < 12; pos++) {
        int endType = pos<6 ? 2 : 1;
        for (int type = 0; type < endType; type++) {
            for (int digit = 0; digit < 10; digit++) {
                cout << pos << " " << type << " " << digit << " " << matchResults[pos][type][digit] << endl;
            }
        }
        cout << endl;
    }
    */

    // dynamic programming
    for (int startPos : {0, 6}) {
        array<vector<double>, 6> costs;
        array<vector<double>, 7> consistencyCosts;
        array<vector<int>, 5> dpPointers;

        const auto &leftGuardMR = readRes.matchResults[startPos==0 ? 0 : 7];
        const auto &rightGuardMR = readRes.matchResults[startPos==0 ? 7 : 14];
        double leftGuardCost = -log(leftGuardMR.likelihood);
        double rightGuardCost = -log(rightGuardMR.likelihood);
        for (const auto &mr : matchResults[startPos]) {
            double consistencyCost = alpha*pow(leftGuardMR.spatialInconsistency(mr), 2);
            consistencyCosts.front().emplace_back(consistencyCost);
            costs.front().emplace_back(leftGuardCost + consistencyCost - log(mr.likelihood));
        }

        for (int pos = 1; pos < 6; pos++) {
            const auto &prevPosResults = matchResults[startPos+pos-1];
            for (auto &mr : matchResults[startPos+pos]) {
                double thisCost = -log(mr.likelihood);
                if (pos == 5) {
                    double rightConsistencyCost = alpha*pow(mr.spatialInconsistency(rightGuardMR), 2);
                    thisCost += rightConsistencyCost + rightGuardCost;
                    consistencyCosts.back().emplace_back(rightConsistencyCost);
                }
                double minCost = numeric_limits<double>::max();
                double consistencyCost;
                auto minMRIt = prevPosResults.begin();
                auto costsIt = costs[pos-1].begin();
                for (auto mr2It = prevPosResults.begin(); mr2It != prevPosResults.end(); ++mr2It, ++costsIt) {
                    double thisConsistencyCost = alpha*pow(mr2It->spatialInconsistency(mr), 2);
                    double combinedCost = *costsIt + thisCost + thisConsistencyCost;
                    if (combinedCost < minCost) {
                        minCost = combinedCost;
                        consistencyCost = thisConsistencyCost;
                        minMRIt = mr2It;
                    }
                }
                costs[pos].emplace_back(minCost);
                consistencyCosts[pos].emplace_back(consistencyCost);
                dpPointers[pos-1].emplace_back(minMRIt - prevPosResults.begin());
            }
        }

        int endPos = startPos+5;
        auto optCostIt = min_element(costs.back().begin(), costs.back().end());
        readRes.cost += *optCostIt;
        int optIdx = optCostIt-costs.back().begin();
        readRes.digitMR(endPos) = matchResults[endPos][optIdx];
        readRes.consistencyCostAfterPos(endPos) = consistencyCosts.back()[optIdx];
        readRes.consistencyCostBeforePos(endPos) = consistencyCosts[5][optIdx];

        for (int pos = 4; pos >= 0; pos--) {
            optIdx = dpPointers[pos][optIdx];
            readRes.digitMR(startPos+pos) = matchResults[startPos+pos][optIdx];
            readRes.consistencyCostBeforePos(startPos+pos) = consistencyCosts[pos][optIdx];
        }
    }

    readRes.update();
    return readRes;
}

int ReadingResult::calcCheckDigit() const
{
    int res = 0;
    for (int i = 0; i < 12; i++) {
        res += (i%2 ? 3 : 1) * barcode[i];
    }

    if (res % 10 == 0) return 0;
    else return 10 - (res % 10);
}

MatchResult TemplateMatchingStep::calcIntegralsOverCell(const Cell &cell, int iw, int io, const Pattern &pattern, const array<vector<double>, 2> &dists)
{
    //cout << "calc " << digit << " " << iw << " " << io << endl << cell << endl << endl;
    //cell.draw(cellPlot, iw, io, rng);
    double cost = 0;
    int offset = io;
    int totalPixInBar = iw + cell.pixelsPerBar.front();

    for (int ibar = 0; ibar < totalPixInBar; ibar++) {
        cost += dists[!pattern.firstWhite()][offset-ibar];
    }

    bool nextWhite = pattern.firstWhite();
    offset++;
    for (int bar = 0; bar < pattern.size(); bar++) {
        totalPixInBar = iw*pattern[bar] + cell.pixelsPerBar[1+bar];
        for (int ibar = 0; ibar < totalPixInBar; ibar++) {
            cost += dists[nextWhite][offset+ibar];
        }
        offset += totalPixInBar;
        nextWhite = !nextWhite;
    }

    totalPixInBar = iw + cell.pixelsPerBar.back();
    for (int ibar = 0; ibar < totalPixInBar; ibar++) {
        cost += dists[nextWhite][offset+ibar];
    }

    return MatchResult(exp(-cost)*cell.area, exp(-cost)*(Point2d(iw, io)+cell.centroid)*cell.area);
}

void MatchResult::draw(Mat &img, int oStart, int xScale, const array<double, 2> &levels, const Scalar &color) const {
    float scale = img.cols/float(xScale);
    line(img, Point((woEstimate.y-oStart)*scale, 0), Point((woEstimate.y-oStart)*scale, 20), color);
    line(img, Point((woEstimate.y+pattern.baseUnits()*woEstimate.x-oStart)*scale, 0), Point((woEstimate.y+pattern.baseUnits()*woEstimate.x-oStart)*scale, 20), color);
    putText(img, pattern.toString(), Point((woEstimate.y-oStart)*scale, 10), FONT_HERSHEY_PLAIN, 1, 0);
    putText(img, to_string_with_precision(-log(likelihood)), Point((woEstimate.y-oStart)*scale, 20), FONT_HERSHEY_PLAIN, 1, 0);
    int iw = 0;
    bool nextWhite = pattern.firstWhite();
    for (int i = 0; i < pattern.size(); i++) {
        line(img, Point((woEstimate.y+iw*woEstimate.x-oStart)*scale, img.rows*(1-levels[nextWhite]/255)), Point((woEstimate.y+(iw+pattern[i])*woEstimate.x-oStart)*scale, img.rows*(1-levels[nextWhite]/255)), color);
        if (i != pattern.size()-1)
            line(img, Point((woEstimate.y+(iw+pattern[i])*woEstimate.x-oStart)*scale, img.rows*(1-levels[nextWhite]/255)), Point((woEstimate.y+(iw+pattern[i])*woEstimate.x-oStart)*scale, img.rows*(1-levels[!nextWhite]/255)), color);
        iw += pattern[i];
        nextWhite = ! nextWhite;
    }
}

void ReadingResult::update()
{
    // update barcode array from match results
    for (int i = 0; i < 12; i++) {
        barcode[i+1] = digitMR(i).pattern.digit;
    }

    // look up first digit using the pattern types (A or B) of the first six matches
    for (int firstDigit = 0; firstDigit < 10; firstDigit++) {
        state = SUCCESS;
        for (int i = 0; i < 6; i++) {
            if (Pattern::firstDigitPatterns[firstDigit][i] != digitMR(i).pattern.type) {
                state = FAIL;
                break;
            }
        }
        if (state == SUCCESS) {
            barcode[0] = firstDigit;
            break;
        }
    }
    if (state == FAIL) return; // first six pattern types are invalid

    // verify check digit
    if (barcode.back() != calcCheckDigit()) state = FAIL;
}

void ReadingResult::draw(Mat &img) const
{
    //line(img, Point(0, visRows*(1-levels[1]/255.0)), Point(visCols-1, visRows*(1-levels[1]/255.0)), {0, 0, 255});
    //line(img, Point(0, visRows*(1-levels[0]/255.0)), Point(visCols-1, visRows*(1-levels[0]/255.0)), {0, 255, 0});
    for (size_t i = 0; i < scanLine.size()-1; i++) {
        line(img, Point(i*img.cols/float(scanLine.size()), img.rows*(1-scanLine[i]/255.0)), Point((i+1)*img.cols/float(scanLine.size()), img.rows*(1-scanLine[i+1]/255.0)), {0, 0, 0});
    }
    int c = 0;
    Scalar colors[] = {{255, 0, 0}, {0, 0, 255}};
    for (const auto &mr : matchResults) mr.draw(img, oStart, scanLine.size(), levels, colors[c++%2]);
    for (size_t i = 0; i < consistencyCosts.size(); i++)
        putText(img, to_string_with_precision(consistencyCosts[i]), Point((matchResults[i+1].woEstimate.y-oStart)*img.cols/float(scanLine.size()), 30), FONT_HERSHEY_PLAIN, 1, 0);

    putText(img, "total cost="+to_string_with_precision(cost), Point(0, img.rows-1), FONT_HERSHEY_PLAIN, 1, 0);
}
