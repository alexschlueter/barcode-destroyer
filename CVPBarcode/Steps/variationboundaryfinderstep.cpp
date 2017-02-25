#include "variationboundaryfinderstep.h"
#include "lsdstep.h"
#include "utils.h"

using namespace std;
using namespace cv;

void VariationBoundaryFinderStep::execute(void *data){
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

    float longEnough = lsdRes->gray.rows*lsdRes->gray.rows + lsdRes->gray.cols*lsdRes->gray.cols;
    Point pt1 = p - longEnough*lineDir;
    Point pt2 = p + longEnough*lineDir;
    Point pt3 = q - longEnough*lineDir;
    Point pt4 = q + longEnough*lineDir;
    clipLine(lsdRes->gray.size(), pt1, pt2);
    clipLine(lsdRes->gray.size(), pt3, pt4);
    float newLenLeft = min(norm((Point2f)pt1-p), norm((Point2f)pt3-q));
    float newLenRight = min(norm((Point2f)pt2-p), norm((Point2f)pt4-q));
    Mat crop;
    Mat trafo = getAffineTransform(vector<Point2f>{p, q, p-newLenLeft*lineDir}, vector<Point2f>{{newLenLeft, length}, {newLenLeft, 0}, {0, length}});
    warpAffine(lsdRes->gray, crop, trafo, {(int)(newLenLeft+newLenRight), (int)length});
    showImage("crop", crop);
    vector<uchar> colAvg;
    reduce(crop, colAvg, 0, REDUCE_AVG);
    int visRows = 600, visCols = 900;
    Mat avgPlot(visRows, visCols, CV_8UC3, {255, 255, 255});
    for (size_t i = 0; i < colAvg.size()-1; i++)
        line(avgPlot, Point(i*visCols/float(colAvg.size()), visRows-colAvg[i]*visRows/255.0f), Point((i+1)*visCols/float(colAvg.size()), visRows-colAvg[i+1]*visRows/255.0f), {0, 0, 0}, 1);
    emit showImage("Col Averages", avgPlot);
    vector<pair<int, double>> scores(colAvg.size());
    auto scoreIt = scores.begin();
    for (auto it = colAvg.begin(); it != colAvg.end(); ++it, ++scoreIt) {
        int parity = it-colAvg.begin() <= newLenLeft  ? 1 : -1;
        scoreIt->first = scoreIt - scores.begin();

        for (auto it2 = it+1; it2 != colAvg.end(); ++it2) {
            if (it2-it > length/2) break;
            else scoreIt->second += parity*abs(*it2-*(it2-1))*(length/2-(it2-it));
            //if (it-colAvg.begin() == bestPos) circle(lineVis, center+*it2*lineDir, 1, {0, 255, 0});
        }
        for (auto it2 = vector<uchar>::reverse_iterator(it); it2 != colAvg.rend(); ++it2) {
            if (it-it2.base()+1 > length/2) break;
            else scoreIt->second -= parity*abs(*it2-*(it2-1))*(length/2-(it-it2.base()+1));
            //if (it-colAvg.begin() == bestPos) circle(lineVis, center+*it2*lineDir, 1, {255, 0, 0});
        }
    }
    Mat varPlot(visRows, visCols, CV_8UC3, {255, 255, 255});
    auto minmaxIts = minmax_element(scores.begin(), scores.end(), [](const auto &a, const auto &b) { return a.second < b.second; });
    float range = minmaxIts.second->second - minmaxIts.first->second;
    for (size_t i = 0; i < scores.size()-1; i++)
        line(varPlot, Point(i*visCols/float(scores.size()), visRows/2*(1-scores[i].second/range)), Point((i+1)*visCols/float(scores.size()), visRows/2*(1-scores[i+1].second/range)), {0, 0, 0}, 1);
    emit showImage("Variation", varPlot);

    // TODO: range checks
    partial_sort(scores.begin()+floor(newLenLeft)+1, scores.begin()+floor(newLenLeft)+4, scores.end(), [](const auto &s1, const auto &s2) { return s1.second > s2.second; });
    partial_sort(scores.begin(), scores.begin()+3, scores.begin() + floor(newLenLeft), [](const auto &s1, const auto &s2) { return s1.second > s2.second; });
    //vector<Point> leftBoundaries(3), rightBoundaries(3);
    vector<Point> bnds(6);
    transform(scores.begin(), scores.begin()+3, bnds.begin(), [&](const auto &s) -> Point { return {s.first, int(length/2)}; });
    transform(scores.begin()+floor(newLenLeft)+1, scores.begin()+floor(newLenLeft)+4, bnds.begin()+3, [&](const auto &s) -> Point { return {s.first, int(length/2)}; });

    Mat trafoInv;
    invertAffineTransform(trafo, trafoInv);
    vector<Point> transBnds;
    cv::transform(bnds, transBnds, trafoInv);

    Mat visualization;
    cvtColor(lsdRes->gray, visualization, COLOR_GRAY2BGR);
    for (const auto &bnd : transBnds)
        circle(visualization, bnd, 4, {0, 0, 255});
    // show visualization
    emit showImage("Variation Bounds", visualization);
    LocalizationResult *res = new LocalizationResult(lsdRes->gray, vector<Point>(transBnds.begin(), transBnds.begin()+3), vector<Point>(transBnds.begin()+3, transBnds.end()), length);

    delete lsdRes;
    emit completed((void*)res);
}
