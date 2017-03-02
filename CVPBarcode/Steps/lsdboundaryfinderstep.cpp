#include "lsdboundaryfinderstep.h"
#include "lsdstep.h"
#include "../utils.h"

#include <iostream>

using namespace std;
using namespace cv;

void LSDBoundaryFinderStep::execute(void *data)
{
    auto lsdRes = static_cast<LSDResult*>(data);

    // the line ends of the best line
    Point2f p, q;
    // Point p is at the top
    if (lsdRes->bestLine[1] > lsdRes->bestLine[3]) {
        p = {lsdRes->bestLine[0], lsdRes->bestLine[1]};
        q = {lsdRes->bestLine[2], lsdRes->bestLine[3]};
    } else {
        q = {lsdRes->bestLine[0], lsdRes->bestLine[1]};
        p = {lsdRes->bestLine[2], lsdRes->bestLine[3]};
    }
    // vector q -> p
    Point2f vec = p - q;
    float length = norm(vec);
    // direction perpendicular to the best line, pointing to the right
    Point2f lineDir;
    lineDir = {vec.y, -vec.x};
    lineDir /= length;

    Point2f center = 0.5*(p+q);

    // TODO: filter very short lines
    lsdRes->lines.erase(remove_if(lsdRes->lines.begin(), lsdRes->lines.end(), [&](const auto &line) { return !LSDStep::linesMaybeInSameBarcode(lsdRes->bestLine, line); }), lsdRes->lines.end());
    Mat lineVis;
    //Mat onlyLines(lsdRes->gray.size(), CV_8U, Scalar(0));
    cvtColor(lsdRes->gray, lineVis, COLOR_GRAY2BGR);
    for (auto &&l : lsdRes->lines) {
        line(lineVis, {(int)l[0], (int)l[1]}, {(int)l[2], (int)l[3]}, {0, 0, 255});
        //line(onlyLines, {(int)l[0], (int)l[1]}, {(int)l[2], (int)l[3]}, 255, 3);
    }
    /*
    float longEnough = (lsdRes->gray.rows*lsdRes->gray.rows + lsdRes->gray.cols*lsdRes->gray.cols);
    Point pt1 = p - longEnough*lineDir;
    Point pt2 = p + longEnough*lineDir;
    Point pt3 = q - longEnough*lineDir;
    Point pt4 = q + longEnough*lineDir;
    clipLine(onlyLines.size(), pt1, pt2);
    clipLine(onlyLines.size(), pt3, pt4);
    float newLenLeft = min(norm((Point2f)pt1-p), norm((Point2f)pt3-q));
    float newLenRight = min(norm((Point2f)pt2-p), norm((Point2f)pt4-q));
    Mat crop;
    Mat trafo = getAffineTransform(vector<Point2f>{p, q, p-newLenLeft*lineDir}, vector<Point2f>{{newLenLeft, length}, {newLenLeft, 0}, {0, length}});
    warpAffine(onlyLines, crop, trafo, {(int)(newLenLeft+newLenRight), (int)length});
    showImage("crop", crop);
    vector<uchar> colAvg;
    reduce(crop, colAvg, 0, REDUCE_AVG);*/

    // use intersections instead of projected centers??
    vector<pair<double, double>> lineInfo(lsdRes->lines.size());
    transform(lsdRes->lines.begin(), lsdRes->lines.end(), lineInfo.begin(), [&](const auto &line) {
        Point2f p(line[0], line[1]);
        Point2f q(line[2], line[3]);
        Point2f center2 = 0.5*(p+q);
        return make_pair((center2-center).dot(lineDir), norm(p-q));
    });
    sort(lineInfo.begin(), lineInfo.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    auto bestIt = find_if(lineInfo.begin(), lineInfo.end(), [](const auto &a) { return a.first >= 0; });
    auto bestPos = bestIt - lineInfo.begin();
    vector<pair<int, double>> scores(lineInfo.size());
    auto scoreIt = scores.begin();

    for (auto it = lineInfo.begin(); it != lineInfo.end(); ++it, ++scoreIt) {
        int parity = it->first < 0 ? 1 : -1;
        scoreIt->first = scoreIt - scores.begin();



        for (auto it2 = it+1; it2 != lineInfo.end(); ++it2) {
            if (abs(it2->first - it->first) > length) break;
            else scoreIt->second += parity*(length-(it2->first - it->first))*max(0.0, length-abs(length-it2->second));
            if (it-lineInfo.begin() == bestPos) circle(lineVis, center+it2->first*lineDir, 1, {0, 255, 0});
        }
        for (auto it2 = vector<pair<double, double>>::reverse_iterator(it); it2 != lineInfo.rend(); ++it2) {
            if (abs(it2->first - it->first) > length) break;
            else scoreIt->second -= parity*(length-(it->first - it2->first))*max(0.0, length-abs(length-it2->second));
            if (it-lineInfo.begin() == bestPos) circle(lineVis, center+it2->first*lineDir, 1, {255, 0, 0});
        }
    }

    // TODO: merge very close centers
    partial_sort(scores.begin(), safeIncr(scores.begin(), scores.begin()+bestPos, 3), scores.begin() + bestPos, [](const auto &s1, const auto &s2) { return s1.second > s2.second; });
    partial_sort(scores.begin()+bestPos, safeIncr(scores.begin(), scores.end(), bestPos+3), scores.end(), [](const auto &s1, const auto &s2) { return s1.second > s2.second; });
    vector<Point> leftBoundaries(3), rightBoundaries(3);
    transform(scores.begin(), safeIncr(scores.begin(), scores.begin()+bestPos, 3), leftBoundaries.begin(), [&](const auto &s) { return center+lineInfo[s.first].first*lineDir; });
    transform(scores.begin()+bestPos, safeIncr(scores.begin(), scores.end(), bestPos+3), rightBoundaries.begin(), [&](const auto &s) { return center+lineInfo[s.first].first*lineDir; });

    emit showImage("Lines after erase", lineVis);

    Mat visualization;
    cvtColor(lsdRes->gray, visualization, COLOR_GRAY2BGR);
    for (const auto &bnd : leftBoundaries)
        circle(visualization, bnd, 4, {0, 0, 255});
    for (const auto &bnd : rightBoundaries)
        circle(visualization, bnd, 4, {0, 0, 255});

    // show visualization
    emit showImage("LineBoundaries", visualization);
    LocalizationResult *res = new LocalizationResult(lsdRes->gray, move(leftBoundaries), move(rightBoundaries), length);
    delete lsdRes;
    emit completed((void*)res);
}

double cross(Point2f v1, Point2f v2){
    return v1.x*v2.y - v1.y*v2.x;
}

Point2f getIntersectionPoint(Point2f a1, Point2f a2, Point2f b1, Point2f b2)
{
    Point2f r(a2-a1);
    Point2f s(b2-b1);

    if (cross(r, s) == 0) throw "getIntersectionPoint";

    double t = cross(b1-a1, s)/cross(r, s);

    return a1 + t*r;
}
