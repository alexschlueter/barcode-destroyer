#include "muensterboundaryfinderstep.h"
#include "lsdstep.h"
#include "../utils.h"

#include <iostream>

using namespace std;
using namespace cv;

void MuensterBoundaryFinderStep::execute(void *data)
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
    float height = norm(vec);
    // direction perpendicular to the best line, pointing to the right
    Point2f lineDir;
    lineDir = {vec.y, -vec.x};

    Point2f center = 0.5*(p+q);
    float longEnough = (lsdRes->gray.rows*lsdRes->gray.rows + lsdRes->gray.cols*lsdRes->gray.cols)/height;
    Point pt1 = p - longEnough*lineDir;
    Point pt2 = p + longEnough*lineDir;
    Point pt3 = q - longEnough*lineDir;
    Point pt4 = q + longEnough*lineDir;
    clipLine(lsdRes->gray.size(), pt1, pt2);
    clipLine(lsdRes->gray.size(), pt3, pt4);
    float newLenLeft = min(norm((Point2f)pt1-p), norm((Point2f)pt3-q));
    float newLenRight = min(norm((Point2f)pt2-p), norm((Point2f)pt4-q));
    Mat crop;
    Mat trafo = getAffineTransform(vector<Point2f>{p, q, p-newLenLeft*lineDir/height}, vector<Point2f>{{newLenLeft, height}, {newLenLeft, 0}, {0, height}});
    warpAffine(lsdRes->gray, crop, trafo, {(int)(newLenLeft+newLenRight), (int)height});
    showImage("crop", crop);
    vector<uchar> colAvg;
    reduce(crop, colAvg, 0, REDUCE_AVG);

    // TODO: smooth the scanline

    // use cv::LineIterator to iterate over pixels in the scan line
    // we want the line to go through the whole image, so we need to give the constructor a point which lies
    // in the correct direction, but outside the image boundary
    // opencv will then clip the line to the image
    int numPix = colAvg.size();

    int visRows = 800;
    int visCols = 1800;
    Mat vis(visRows, visCols, CV_8UC3, {255, 255, 255});
    const uchar deltaY = 3;
    vector<pair<int, uchar>> localMinima, localMaxima;
    enum { NONE, NEXTMIN, NEXTMAX } state = NONE;
    uchar last = colAvg[0];
    // iterate over pixels on scan line
    for (int i = 1; i < (int)colAvg.size(); i++) {
        auto cur = colAvg[i];
        Point lastPlotPoint{(i-1)*(visCols/numPix), visRows-last*(visRows/255)};
        Point curPlotPoint{i*(visCols/numPix), visRows-cur*(visRows/255)};
        line(vis, lastPlotPoint, curPlotPoint, {0, 0, 0}, 1);
        switch (state) {
        case NONE:
            if (cur < last) state = NEXTMIN;
            else if (cur > last) state = NEXTMAX;
            break;
        case NEXTMIN:
            if (cur > last) {
                state = NEXTMAX;
                localMinima.emplace_back(i-1, last);
                circle(vis, lastPlotPoint, 3, {0, 0, 255}, 3);
            }
            break;
        case NEXTMAX:
            if (cur < last) {
                state = NEXTMIN;
                localMaxima.emplace_back(i-1, last);
                circle(vis, lastPlotPoint, 3, {0, 255, 0}, 3);
            }
            break;
        }
        last = cur;
    }

    auto minCopy = localMinima;
    std::nth_element(minCopy.begin(), minCopy.begin()+minCopy.size()/2, minCopy.end(), [](auto a, auto b) { return a.second < b.second; });
    auto maxCopy = localMaxima;
    std::nth_element(maxCopy.begin(), maxCopy.begin()+maxCopy.size()/2, maxCopy.end(), [](auto a, auto b) { return a.second > b.second; });
    // TODO: use local criterion for pruning
    auto minAvg = std::accumulate(minCopy.begin(), minCopy.begin()+minCopy.size()/2, 0, [](auto a, auto b) { return a + b.second; }) * 2.0 / minCopy.size();
    auto maxAvg = std::accumulate(maxCopy.begin(), maxCopy.begin()+maxCopy.size()/2, 0, [](auto a, auto b) { return a + b.second; }) * 2.0 / maxCopy.size();

    cout << "before pruning " << localMinima.size() << " " << localMaxima.size() << endl;
    cout << "minAvg " << (int)minAvg << ", maxAvg " << (int)maxAvg << endl;

    // prune unusually dark maxima / unusually light minima
    Point rect(3, 3);
    for (auto it = localMinima.begin(); it != localMinima.end();) {
        if (it->second > 1.5*minAvg) {
            Point plotPoint{(it->first+1)*(visCols/(int)colAvg.size()), visRows-it->second*(visRows/255)};
            rectangle(vis, plotPoint-rect, plotPoint+rect, {255, 0, 0}, CV_FILLED);
            it = localMinima.erase(it);
        } else ++it;
    }

    for (auto it = localMaxima.begin(); it != localMaxima.end();) {
        if (it->second < 0.75*maxAvg) {
            Point plotPoint{(it->first+1)*(visCols/(int)colAvg.size()), visRows-it->second*(visRows/255)};
            rectangle(vis, plotPoint-rect, plotPoint+rect, {255, 0, 0}, CV_FILLED);
            it = localMaxima.erase(it);
        } else ++it;
    }

    cout << "after pruning: " << localMinima.size() << " " << localMaxima.size() << endl;

    // binarize scanline
    vector<bool> binarized;
    auto minIt = localMinima.begin();
    auto maxIt = localMaxima.begin();
    int j = 0;
    uchar lastThresh = 127;
    for (int j = 0; j < (int)colAvg.size(); j++) {

        // for each point on the line, look at the 7 last inward (i.e. in the direction of the bestLine's center) maxima and minima
        if (j < newLenLeft) {
            while (minIt != localMinima.end() && j > minIt->first) ++minIt;
            while (maxIt != localMaxima.end() && j > maxIt->first) ++maxIt;
            //minCopy = vector<pair<int, uchar>>(minIt, minIt+min(7l, localMinima.end()-minIt));
            //maxCopy = vector<pair<int, uchar>>(maxIt, maxIt+min(7l, localMaxima.end()-maxIt));
            minCopy = vector<pair<int, uchar>>(minIt, safeIncr(minIt, localMinima.end(), 7));
            maxCopy = vector<pair<int, uchar>>(maxIt, safeIncr(maxIt, localMaxima.end(), 7));
        } else {
            if (j == newLenLeft) {
                --maxIt;
                --minIt;
                // draw center
                line(vis, {j*(visCols/numPix), 0}, {j*(visCols/numPix), visRows-1}, {255, 0, 0}, 1);
            }
            while (minIt != localMinima.end() && j >= (minIt+1)->first) ++minIt;
            while (maxIt != localMaxima.end() && j >= (maxIt+1)->first) ++maxIt;
            //minCopy = vector<pair<int, uchar>>(minIt-min(6l, minIt-localMinima.begin()), minIt+1);
            //maxCopy = vector<pair<int, uchar>>(maxIt-min(6l, maxIt-localMaxima.begin()), maxIt+1);
            minCopy = vector<pair<int, uchar>>(safeDecr(minIt, localMinima.begin(), 6), minIt+1);
            maxCopy = vector<pair<int, uchar>>(safeDecr(maxIt, localMaxima.begin(), 6), maxIt+1);
        }
        // get the second lowest minimum and the second highest maximum under those 7
        std::nth_element(minCopy.begin(), minCopy.begin()+1, minCopy.end(), [](auto a, auto b) { return a.second > b.second; });
        std::nth_element(maxCopy.begin(), maxCopy.begin()+1, maxCopy.end(), [](auto a, auto b) { return a.second < b.second; });

        // their average is the threshold value for the current point on the scanline
        // TODO: segfault when less than two in vector
        uchar threshold = (minCopy[1].second+maxCopy[1].second)/2;
        binarized.emplace_back(colAvg[j] >= threshold);
        Point lastPlotPoint{j*(visCols/numPix), visRows-lastThresh*(visRows/255)};
        Point curPlotPoint{(j+1)*(visCols/numPix), visRows-threshold*(visRows/255)};
        line(vis, lastPlotPoint, curPlotPoint, {255, 0, 0}, 1);

        lastThresh = threshold;
    }

    // find first black pixel to the right of the center
    auto firstBlack = find(binarized.begin()+newLenLeft, binarized.end(), false);
    auto curRight = find(firstBlack, binarized.end(), true);
    auto curLeft = find(reverse_iterator<vector<bool>::iterator>(firstBlack), binarized.rend(), true);
    int maxPairSize;
    // starting from the center black bar, iteratively add white-black pairs to both sides
    // always add the bars on the side on which the new white-black pair is smaller to prevent adding non-barcode areas
    // stop when we have 59 bars (total number of bars in an EAN-13 barcode)
    for (int pairs = 0; curLeft != binarized.rend() || curRight != binarized.end(); pairs++) {
        auto nextRight = find(curRight, binarized.end(), false);
        nextRight = find(nextRight, binarized.end(), true);

        auto nextLeft = find(curLeft, binarized.rend(), false);
        nextLeft = find(nextLeft, binarized.rend(), true);

        int curPairSize;
        int distLeft = nextLeft - curLeft;
        int distRight = nextRight - curRight;
        if (distLeft == 0 || (distRight != 0 && distRight < distLeft)) {
            curPairSize = distRight;
            if (pairs > 29 || (pairs > 0 && curPairSize > 3*maxPairSize)) {
                cout << "break: right pair size too large" << endl;
                break;
            }
            curRight = nextRight;
        } else {
            curPairSize = distLeft;
            if (pairs > 29 || (pairs > 0 && curPairSize > 3*maxPairSize)) {
                cout << "break: left pair size too large" << endl;
                break;
            }
            curLeft = nextLeft;
        }
        if (pairs == 0 || curPairSize > maxPairSize) maxPairSize = curPairSize;
    }

    Mat trafoInv;
    invertAffineTransform(trafo, trafoInv);
    vector<Point2f> transBnds;
    cout << "pre trafo bnds " << binarized.rend()-curLeft << " " << curRight-binarized.begin() << endl;
    cv::transform(vector<Point2f>{{float(binarized.rend()-curLeft), height/2}, {float(curRight-binarized.begin()), height/2}}, transBnds, trafoInv);

    j = binarized.rend() - curLeft;
    line(vis, {j*(visCols/numPix), 0}, {j*(visCols/numPix), visRows-1}, {0, 255, 0}, 1);
    j = curRight - binarized.begin();
    line(vis, {j*(visCols/numPix), 0}, {j*(visCols/numPix), visRows-1}, {0, 0, 255}, 1);
    emit showImage("Muenster Boundaries", vis);

    auto res = new LocalizationResult(lsdRes->gray, {move(transBnds[0])}, {move(transBnds[1])}, height);
    //auto res = new LocalizationResult(lsdRes->gray, {20, 100}, {100, 250}, height);
    emit completed((void*)res);
}
