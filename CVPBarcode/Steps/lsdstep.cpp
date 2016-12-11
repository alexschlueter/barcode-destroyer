#include "lsdstep.h"
#include <iostream>

using namespace std;
using namespace cv;

void LSDStep::drawRotatedRect(Mat& img, RotatedRect rect) {
    Point2f points[4];
    rect.points(points);
    for (int i = 0; i < 4; i++)
        line(img, points[i], points[(i+1)%4], {0, 0, 255}, 2);
}

/**
 * @brief LSDStep::maxVariationDifferenceAlongLine
 * @param start start point of the scan line
 * @param dir direction from the start point to scan
 * @return point with maximal variation difference (likely boundary of a barcode)
 */
Point LSDStep::maxVariationDifferenceAlongLine(const Point2f &start, const Point2f &dir) {

    // need vector perpendicular to dir to scan multiple lines above and below the actual line
    Point2f perp = {-dir.y, dir.x};

    // use cv::LineIterator to iterate over pixels in the scan line
    // we want the line to go through the whole image, so we need to give the constructor a point which lies
    // in the correct direction, but outside the image boundary
    // opencv will then clip the line to the image
    float longEnough = (gray.rows*gray.rows + gray.cols*gray.cols)/norm(dir);
    LineIterator fullBisectIt(gray, start, start+longEnough*dir);

    int maxVar = std::numeric_limits<int>::min();
    /*std::vector<int> vars;
    vars.reserve(fullBisectIt.count);*/
    Point maxPos;
    // create Rect of the same size as the image for simple bound checking with Rect::contains
    Rect imgRect(Point(), gray.size());

    // iterate over pixels on scan line
    for (int i = 0; i < fullBisectIt.count; i++, ++fullBisectIt) {

        LineIterator shortBisectIt(gray, fullBisectIt.pos()-(Point)dir, fullBisectIt.pos());
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
                tempVar += std::abs(gray.at<uchar>(curPosAdjusted)-gray.at<uchar>(oldPosAdjusted));
            }
            variation += tempVar;
        }

        shortBisectIt = LineIterator(gray, fullBisectIt.pos(), fullBisectIt.pos()+(Point)dir);
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
                tempVar += std::abs(gray.at<uchar>(curPosAdjusted)-gray.at<uchar>(oldPosAdjusted));
            }
            variation -= tempVar;
        }

        if (variation > maxVar) {
            maxVar = variation;
            maxPos = fullBisectIt.pos();
        }
        //vars.push_back(variation);
    }


    /*
    // experiment to choose the local maximum closest to start instead of the global maximum
    // problem: calibration of the 0.4/0.7 constants, sometimes box too small
    int firstMax, firstMaxIdx = -1;
    cout << "vars size " << vars.size() << " maxvar " << maxVar << endl;
    for (size_t i = 0; i < vars.size(); i++) {
        int var = vars[i];
        cout << var << endl;
        if (var > 0.4*maxVar && (firstMaxIdx == -1 || var > firstMax)) {
            cout << "a " << firstMax << " " << firstMaxIdx << " " << var << " " << i << endl;
            firstMax = var;
            firstMaxIdx = i;

        } else if (firstMaxIdx != -1 && var < 0.7*firstMax) {
            cout << "b " << firstMaxIdx << " " << firstMax << " " << var << endl;
            break;
        }
    }

    // experiment to extend the boundary slightly as long as the variation difference doesn't drop too
    // low relative to the max
    // problem: calibration of 0.8, box too large in some cases
    for (; (uint)firstMaxIdx < vars.size(); firstMaxIdx++) {
        if (vars[firstMaxIdx] < 0.8*firstMax) {
            firstMaxIdx--;
            break;
        }
    }
    fullBisectIt = LineIterator(gray, start, start+longEnough*dir);
    for (; firstMaxIdx > 0; firstMaxIdx--) ++fullBisectIt;
    return fullBisectIt.pos();
    */

    return maxPos;
}

void LSDStep::execute(void *data){
    gray = *static_cast<Mat*>(data);

    // use the LineSegmentDetector provided by opencv
    auto lsd = createLineSegmentDetector(LSD_REFINE_ADV); // try other flags
    std::vector<Vec4f> lines;
    lsd->detect(gray, lines);

    // show found line segments
    Mat detectedLines = gray.clone();
    lsd->drawSegments(detectedLines, lines);
    imshow("lines detected by cv::LineSegmentDetector", detectedLines);

    // we're trying to find a line segment which lies approximately in the middle of the barcode
    // we assign points to each line based on how many other lines are approximately
    // parallel, have similar length and are not too far away
    std::vector<int> scores(lines.size());
    auto scoreIt = scores.begin();
    for (auto &&line : lines) {
        // calculate score for this line

        // end points of the line
        Point2f p1(line[0], line[1]);
        Point2f q1(line[2], line[3]);
        // vector q1 -> p1
        Point2f vec1 = p1 - q1;
        // use -vec1.y instead of vec1.y for the angle because opencv's y axis is inverted
        float angle1 = std::atan2(-vec1.y, vec1.x)*180/CV_PI;
        Point2f center1 = 0.5*(p1+q1);
        float length1 = norm(vec1);

        // iterate over all other lines and look for nearby lines that might belong to the same barcode as the first
        for (auto &&line2 : lines) {
            // end points of second line
            Point2f p2(line2[0], line2[1]);
            Point2f q2(line2[2], line2[3]);
            // vector q2 -> p2
            Point2f vec2 = p2 - q2;
            // as above
            float angle2 = std::atan2(-vec2.y, vec2.x)*180/CV_PI;
            Point2f center2 = 0.5*(p2+q2);
            float length2 = norm(vec2);

            // angle distance between the two lines
            float angleDist = std::abs(angle1-angle2);
            // angle distance might be smaller "the other way around", e.g. distance between
            // 1 and 359 degrees is 2 degrees, not 358
            // additionally, a line with angle 180 is the same as a line with angle 0
            angleDist = std::min(angleDist, std::abs(180-angleDist));
            // once more
            angleDist = std::min(angleDist, std::abs(180-angleDist));

            // length difference, relative to the length of the first line
            float relLengthDiff = (length1-length2)/length1;
            // center distance, relative to the length of the first line
            float relCenterDist = norm(center1-center2) / length1;

            // Two lines with same angle, length and close centers might still not be placed next
            // to each other like lines in a barcode.
            // If the lines belong to the same barcode, we expect the vector between the centers to be
            // approximately orthogonal to the lines themselves, i.e. we expect the projection of this vector
            // onto the first line to be small.
            float relProjCenterDiff = (center2-center1).dot(vec1) / (length1*length1);

            // check if the lines probably belong to the same barcode
            if (angleDist < angleTol && std::abs(relLengthDiff) < lengthTol && std::abs(relProjCenterDiff) < projCenterTol && std::abs(relCenterDist) < centerDistTol) {
                // if yes, increase the score of line 1
                // we could probably just increase the score of line 2 at this point and save half of the loop steps
                ++(*scoreIt);
            }
        }
        ++scoreIt;
    }

    // the line with the best score hopefully lies in the middle of the barcode
    auto maxScoreIt = std::max_element(scores.begin(), scores.end());
    const auto &bestLine = *(lines.begin() + std::distance(scores.begin(), maxScoreIt));
    cout << "max score " << *maxScoreIt << endl;

    // this section is just for the visualization of the line scores in the result image
    Mat onlyLines(gray.size(), CV_8U, Scalar(0));
    scoreIt = scores.begin();
    for (auto &&l : lines) {
        line(onlyLines, {(int)l[0], (int)l[1]}, {(int)l[2], (int)l[3]}, *scoreIt*255/(float)*maxScoreIt);
        ++scoreIt;
    }

    Mat color;
    Mat visualization;
    cvtColor(gray, visualization, COLOR_GRAY2BGR);
    applyColorMap(onlyLines, color, COLORMAP_JET);
    color.copyTo(visualization, onlyLines);
    // draw best line in white
    line(visualization, {(int)bestLine[0], (int)bestLine[1]}, {(int)bestLine[2], (int)bestLine[3]}, {255, 255, 255});

    // end points of best line
    Point2f p(bestLine[0], bestLine[1]);
    Point2f q(bestLine[2], bestLine[3]);
    // vector q -> p
    Point2f vec = p - q;
    float length = norm(vec);
    // direction perpendicular to the best line
    Point2f lineDir = {-vec.y, vec.x};
    Point2f center = 0.5*(p+q);

    // calculate boundaries of the barcode in both directions
    Point leftBoundary = maxVariationDifferenceAlongLine(center, -lineDir);
    Point rightBoundary = maxVariationDifferenceAlongLine(center, lineDir);

    // draw estimated bounding box for the barcode
    RotatedRect barcodeBB(0.5*(leftBoundary+rightBoundary), {(float)norm(leftBoundary-rightBoundary), length}, std::atan2(leftBoundary.y-rightBoundary.y, leftBoundary.x-rightBoundary.x)*180/CV_PI);
    drawRotatedRect(visualization, barcodeBB);

    // draw center and boundary points
    circle(visualization, 0.5*(leftBoundary+rightBoundary), 4, {0, 0, 255});
    circle(visualization, leftBoundary, 4, {0, 255, 0});
    circle(visualization, rightBoundary, 4, {0, 0, 255});

    // show visualization
    imshow("LSDStep", visualization);

    LSDResult *res = new LSDResult(gray, leftBoundary, rightBoundary, length);
    emit completed((void*)res);
}
