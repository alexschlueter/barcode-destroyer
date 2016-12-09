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

Point LSDStep::maxVariationDifferenceAlongLine(const Point2f &start, const Point2f &dir) {
    const std::array<float, 5> scanOffsets = {-0.3, -0.15, 0, 0.15, 0.3};
    Point2f perp = {-dir.y, dir.x};

    float longEnough = (gray.rows*gray.rows + gray.cols*gray.cols)/norm(dir);
    LineIterator fullBisectIt(gray, start, start+longEnough*dir);

    int maxVar = std::numeric_limits<int>::min();
    /*std::vector<int> vars;
    vars.reserve(fullBisectIt.count);*/
    Point maxPos;
    Rect imgRect(Point(), gray.size());

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
    /*int firstMax, firstMaxIdx = -1;
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
    for (; (uint)firstMaxIdx < vars.size(); firstMaxIdx++) {
        if (vars[firstMaxIdx] < 0.8*firstMax) {
            firstMaxIdx--;
            break;
        }
    }
    fullBisectIt = LineIterator(gray, start, start+longEnough*dir);
    for (; firstMaxIdx > 0; firstMaxIdx--) ++fullBisectIt;
    return fullBisectIt.pos();*/
    return maxPos;
}

void LSDStep::execute(void *data){
    gray = *static_cast<Mat*>(data);

    const float angleTol = 0.1*180/CV_PI;
    const float lengthTol = 0.3;
    const float centerTol = 0.1;

    auto lsd = createLineSegmentDetector(LSD_REFINE_ADV); // try other flags
    std::vector<Vec4f> lines;
    std::vector<double> widths;
    lsd->detect(gray, lines, widths);

    // Show found lines
    Mat detectedLines = gray.clone();
    lsd->drawSegments(detectedLines, lines);
    imshow("lines detected by cv::LineSegmentDetector", detectedLines);

    std::vector<int> scores(lines.size());
    auto scoreIt = scores.begin();
    for (auto &&line : lines) {
        Point2f p1(line[0], line[1]);
        Point2f q1(line[2], line[3]);
        Point2f vec1 = p1 - q1;
        float angle1 = std::atan2(-vec1.y, vec1.x)*180/CV_PI;
        Point2f center1 = 0.5*(p1+q1);
        float length1 = norm(vec1);

        auto widthIt = widths.cbegin();
        for (auto &&line2 : lines) {
            Point2f p2(line2[0], line2[1]);
            Point2f q2(line2[2], line2[3]);
            Point2f vec2 = p2 - q2;
            float angle2 = std::atan2(-vec2.y, vec2.x)*180/CV_PI;
            Point2f center2 = 0.5*(p2+q2);
            float length2 = norm(vec2);
            float angleDist = std::abs(angle1-angle2);
            angleDist = std::min(angleDist, std::abs(180-angleDist));
            angleDist = std::min(angleDist, std::abs(180-angleDist));
            float relProjCenterDiff = (center2-center1).dot(vec1) / (length1*length1);
            float relLengthDiff = (length1-length2)/length1;
            if (angleDist < angleTol && std::abs(relLengthDiff) < lengthTol && std::abs(relProjCenterDiff) < centerTol && norm(center1-center2)<length1) {
                ++(*scoreIt);
            }
            ++widthIt;
        }
        ++scoreIt;
    }
    auto maxScoreIt = std::max_element(scores.begin(), scores.end());
    const auto &bestLine = *(lines.begin() + std::distance(scores.begin(), maxScoreIt));
    cout << "max score " << *maxScoreIt << endl;

    Mat onlyLines(gray.size(), CV_8U, Scalar(0));
    scoreIt = scores.begin();
    for (auto &&l : lines) {
        line(onlyLines, {(int)l[0], (int)l[1]}, {(int)l[2], (int)l[3]}, *scoreIt*255/(float)*maxScoreIt);
        ++scoreIt;
    }

    Mat color;
    Mat *result = new Mat;
    cvtColor(gray, *result, COLOR_GRAY2BGR);
    applyColorMap(onlyLines, color, COLORMAP_JET);
    color.copyTo(*result, onlyLines);
    line(*result, {(int)bestLine[0], (int)bestLine[1]}, {(int)bestLine[2], (int)bestLine[3]}, {255, 255, 255});

    Point2f p(bestLine[0], bestLine[1]);
    Point2f q(bestLine[2], bestLine[3]);
    Point2f vec = p - q;
    float length = norm(vec);
    Point2f lineDir = {-vec.y, vec.x};
    Point2f center = 0.5*(p+q);
    Point leftBoundary = maxVariationDifferenceAlongLine(center, -lineDir);
    Point rightBoundary = maxVariationDifferenceAlongLine(center, lineDir);

    RotatedRect barcodeBB(0.5*(leftBoundary+rightBoundary), {(float)norm(leftBoundary-rightBoundary), length}, std::atan2(leftBoundary.y-rightBoundary.y, leftBoundary.x-rightBoundary.x)*180/CV_PI);
    drawRotatedRect(*result, barcodeBB);
    circle(*result, 0.5*(leftBoundary+rightBoundary), 4, {0, 0, 255});
    circle(*result, leftBoundary, 4, {0, 255, 0});
    circle(*result, rightBoundary, 4, {0, 0, 255});

    emit completed((void*)result);
}
