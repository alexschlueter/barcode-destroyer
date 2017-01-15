#include "variationboundaryfinderstep.h"
#include "lsdstep.h"
#include "utils.h"

using namespace std;
using namespace cv;

const std::array<float, 5> VariationBoundaryFinderStep::scanOffsets = {-0.3, -0.15, 0, 0.15, 0.3};

/**
 * @brief VariationBoundaryFinderStep::maxVariationDifferenceAlongLine
 * @param start start point of the scan line
 * @param dir direction from the start point to scan in
 * @return point with maximal variation difference (likely boundary of a barcode)
 */
Point VariationBoundaryFinderStep::maxVariationDifferenceAlongLine(const Mat &img, const Point2f &start, const Point2f &dir) {

    // need vector perpendicular to dir to scan multiple lines above and below the actual line
    Point2f perp = {-dir.y, dir.x};

    // use cv::LineIterator to iterate over pixels in the scan line
    // we want the line to go through the whole image, so we need to give the constructor a point which lies
    // in the correct direction, but outside the image boundary
    // opencv will then clip the line to the image
    float longEnough = (img.rows*img.rows + img.cols*img.cols)/norm(dir);
    LineIterator fullBisectIt(img, start, start+longEnough*dir);

    int maxVar = std::numeric_limits<int>::min();
    std::vector<int> vars;
    vars.reserve(fullBisectIt.count);
    Point2f maxPos;
    int visRows = 600, visCols = 1800;
    Mat vis(visRows, visCols, CV_8UC3, {255, 255, 255});
    // create Rect of the same size as the image for simple bound checking with Rect::contains
    Rect imgRect(Point(), img.size());

    // iterate over pixels on scan line
    for (int i = 0; i < fullBisectIt.count; i++, ++fullBisectIt) {

        LineIterator shortBisectIt(img, fullBisectIt.pos()-(Point)dir, fullBisectIt.pos());
        LineIterator oldShortBisectIt = shortBisectIt++;
        int variation = 0;
        for (int j = 0; j < shortBisectIt.count-1; j++, ++shortBisectIt, ++oldShortBisectIt) {
            Point2f curPos = shortBisectIt.pos();
            Point2f oldPos = oldShortBisectIt.pos();
            int tempVar = 0;
            for (const auto &scanOffset : scanOffsets) {
                Point2f offset = scanOffset*perp;
                Point curPosAdjusted = curPos+offset;
                Point oldPosAdjusted = oldPos+offset;
                if (!imgRect.contains(curPosAdjusted) || !imgRect.contains(oldPosAdjusted)) {
                    tempVar = 0;
                    break;
                }
                tempVar += std::abs(img.at<uchar>(curPosAdjusted)-img.at<uchar>(oldPosAdjusted));
            }
            variation += tempVar;
        }

        shortBisectIt = LineIterator(img, fullBisectIt.pos(), fullBisectIt.pos()+(Point)dir);
        oldShortBisectIt = shortBisectIt++;
        for (int j = 0; j < shortBisectIt.count-1; j++, ++shortBisectIt, ++oldShortBisectIt) {
            Point2f curPos = shortBisectIt.pos();
            Point2f oldPos = oldShortBisectIt.pos();
            int tempVar = 0;
            for (const auto &scanOffset : scanOffsets) {
                Point2f offset = scanOffset*perp;
                Point curPosAdjusted = curPos+offset;
                Point oldPosAdjusted = oldPos+offset;
                if (!imgRect.contains(curPosAdjusted) || !imgRect.contains(oldPosAdjusted)) {
                    tempVar = 0;
                    break;
                }
                tempVar += std::abs(img.at<uchar>(curPosAdjusted)-img.at<uchar>(oldPosAdjusted));
            }
            variation -= tempVar;
        }

        if (variation > maxVar) {
            maxVar = variation;
            maxPos = fullBisectIt.pos();
        }
        vars.push_back(variation);
    }



    // experiment to choose the local maximum closest to start instead of the global maximum
    // problem: calibration of the 0.4/0.7 constants, sometimes box too small
    int firstMax, firstMaxIdx = -1;
    //cout << "vars size " << vars.size() << " maxvar " << maxVar << endl;
    for (size_t i = 0; i < vars.size(); i++) {
        int var = vars[i];
        //cout << var << endl;
        if (var > 0.4*maxVar && (firstMaxIdx == -1 || var > firstMax)) {
            //cout << "a " << firstMax << " " << firstMaxIdx << " " << var << " " << i << endl;
            firstMax = var;
            firstMaxIdx = i;

        } else if (firstMaxIdx != -1 && var < 0.7*firstMax) {
            //cout << "b " << firstMaxIdx << " " << firstMax << " " << var << endl;
            break;
        }
    }

    // experiment to extend the boundary slightly as long as the variation difference doesn't drop too
    // low relative to the max
    // problem: calibration of 0.8, box too large in some cases
    /*for (; (uint)firstMaxIdx < vars.size(); firstMaxIdx++) {
        if (vars[firstMaxIdx] < 0.8*firstMax) {
            firstMaxIdx--;
            break;
        }
    }
    */


    fullBisectIt = LineIterator(img, start, start+longEnough*dir);
    for (; firstMaxIdx > 0; firstMaxIdx--) ++fullBisectIt;
    return fullBisectIt.pos();

    //emit showImage("VarAlongLine", vis);
    return maxPos;
}

void VariationBoundaryFinderStep::execute(void *data){
    auto lsdRes = static_cast<LSDResult*>(data);


    // end points of best line
    Point2f p(lsdRes->bestLine[0], lsdRes->bestLine[1]);
    Point2f q(lsdRes->bestLine[2], lsdRes->bestLine[3]);
    // vector q -> p
    Point2f vec = p - q;
    float length = norm(vec);
    // direction perpendicular to the best line, pointing to the right
    Point2f lineDir;
    if (vec.y < 0) {
        lineDir = {-vec.y, vec.x};
    } else {
        lineDir = {vec.y, -vec.x};
    }
    Point2f center = 0.5*(p+q);

    // TODO: for the following, we really only need the line centers
    // maybe calculate those in advance?
    lsdRes->lines.erase(remove_if(lsdRes->lines.begin(), lsdRes->lines.end(), [&](const auto &line) { return !LSDStep::linesMaybeInSameBarcode(lsdRes->bestLine, line); }), lsdRes->lines.end());
    sort(lsdRes->lines.begin(), lsdRes->lines.end(), [&](const auto &line1, const auto &line2) {
        Point2f p1(line1[0], line1[1]);
        Point2f q1(line1[2], line1[3]);
        Point2f center1 = 0.5*(p1+q1);
        Point2f p2(line2[0], line2[1]);
        Point2f q2(line2[2], line2[3]);
        Point2f center2 = 0.5*(p2+q2);
        return (center1-center).dot(lineDir) < (center2-center).dot(lineDir);
    });

    // calculate boundaries of the barcode in both directions
    Point leftBoundary = maxVariationDifferenceAlongLine(lsdRes->gray, center, -lineDir);
    Point rightBoundary = maxVariationDifferenceAlongLine(lsdRes->gray, center, lineDir);

    // extend boundaries if there are nearby lines
    float allowedDistance = 0.08*norm(leftBoundary-rightBoundary); // TODO: tune factor
    leftBoundary = extendBoundWithLines(leftBoundary, -lineDir, allowedDistance, lsdRes->lines.rbegin(), lsdRes->lines.rend());
    rightBoundary = extendBoundWithLines(rightBoundary, lineDir, allowedDistance, lsdRes->lines.begin(), lsdRes->lines.end());

    // TODO: shrink boundaries again based on intensity?

    Mat visualization;
    cvtColor(lsdRes->gray, visualization, COLOR_GRAY2BGR);
    // draw estimated bounding box for the barcode
    RotatedRect barcodeBB(0.5*(leftBoundary+rightBoundary), {(float)norm(leftBoundary-rightBoundary), length}, std::atan2(leftBoundary.y-rightBoundary.y, leftBoundary.x-rightBoundary.x)*180/CV_PI);
    drawRotatedRect(visualization, barcodeBB);

    // draw center and boundary points
    circle(visualization, 0.5*(leftBoundary+rightBoundary), 4, {0, 0, 255});
    circle(visualization, leftBoundary, 4, {0, 255, 0});
    circle(visualization, rightBoundary, 4, {0, 0, 255});

    // show visualization
    emit showImage("VariationBoundary", visualization);

    LocalizationResult *res = new LocalizationResult(lsdRes->gray, leftBoundary, rightBoundary, length);
    delete lsdRes;
    emit completed((void*)res);
}

template <class LineIt>
Point VariationBoundaryFinderStep::extendBoundWithLines(const Point &bound, const Point2f &dir, float allowedDistance, LineIt linesBegin, LineIt linesEnd)
{
    LineIt nextLine = find_if(linesBegin, linesEnd, [&](const auto &line) {
        Point2f p(line[0], line[1]);
        Point2f q(line[2], line[3]);
        Point2f center = 0.5*(p+q);

        return (center-(Point2f)bound).dot(dir) > 0;
    });

    Point2f lastPos = bound;
    for (; nextLine < linesEnd; ++nextLine) {
        Point2f p((*nextLine)[0], (*nextLine)[1]);
        Point2f q((*nextLine)[2], (*nextLine)[3]);
        Point2f center = 0.5*(p+q);
        if (norm(center-lastPos) < allowedDistance) {
            //cout << "lsd extend " << center << " " << lastPos << " " << allowedDistance << endl;
            lastPos = center;
        } else {
            //cout << "lsd extend break " << center << " " << lastPos << " " << allowedDistance << endl;
            break;
        }
    }

    return lastPos;
}
