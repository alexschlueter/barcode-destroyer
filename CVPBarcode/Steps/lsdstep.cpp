#include "lsdstep.h"
#include "../utils.h"
#include <iostream>

using namespace std;
using namespace cv;

bool LSDStep::linesMaybeInSameBarcode(const Vec4f &line1, const Vec4f &line2)
{
    // end points of the line
    Point2f p1(line1[0], line1[1]);
    Point2f q1(line1[2], line1[3]);
    // vector q1 -> p1
    Point2f vec1 = p1 - q1;
    // use -vec1.y instead of vec1.y for the angle because opencv's y axis is inverted
    float angle1 = std::atan2(-vec1.y, vec1.x)*180/CV_PI;
    Point2f center1 = 0.5*(p1+q1);
    float length1 = norm(vec1);
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

    // Two lines with same angle, length and close centers might still not be placed next
    // to each other like lines in a barcode.
    // If the lines belong to the same barcode, we expect the vector between the centers to be
    // approximately orthogonal to the lines themselves, i.e. we expect the projection of this vector
    // onto the first line to be small.
    float relProjCenterDiff = (center2-center1).dot(vec1) / (length1*length1);

    // check if the lines probably belong to the same barcode
    return angleDist < angleTol && std::abs(relLengthDiff) < lengthTol && std::abs(relProjCenterDiff) < projCenterTol;
}

void LSDStep::execute(void *data){
    gray = *static_cast<Mat*>(data);
    delete static_cast<Mat*>(data);

    // use the LineSegmentDetector provided by opencv
    auto lsd = createLineSegmentDetector(LSD_REFINE_ADV); // try other flags
    std::vector<Vec4f> lines;
    lsd->detect(gray, lines);

    // show found line segments
    Mat detectedLines = gray.clone();
    lsd->drawSegments(detectedLines, lines);
    //imshow("lines detected by cv::LineSegmentDetector", detectedLines);

    // we're trying to find a line segment which lies approximately in the middle of the barcode
    // we assign points to each line based on how many other lines are approximately
    // parallel, have similar length and are not too far away
    std::vector<int> scores(lines.size());
    auto scoreIt = scores.begin();
    for (auto &&line : lines) {
        // calculate score for this line

        // end points end center of the line
        Point2f p1(line[0], line[1]);
        Point2f q1(line[2], line[3]);
        Point2f center1 = 0.5*(p1+q1);
        float length1 = norm(p1-q1);

        // iterate over all other lines and look for nearby lines that might belong to the same barcode as the first
        for (auto &&line2 : lines) {

            // end points and center of second line
            Point2f p2(line2[0], line2[1]);
            Point2f q2(line2[2], line2[3]);
            Point2f center2 = 0.5*(p2+q2);
            // center distance, relative to the length of the first line
            float relCenterDist = norm(center1-center2) / length1;

            if (std::abs(relCenterDist) < centerDistTol && linesMaybeInSameBarcode(line, line2)) {
                // increase the score of line 1
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
    line(visualization, {(int)bestLine[0], (int)bestLine[1]}, {(int)bestLine[2], (int)bestLine[3]}, {255, 255, 255}, 2);
    emit showImage("LineSegmentDetector", visualization);



    auto res = new LSDResult(move(gray), move(bestLine), move(lines));
    emit completed((void*)res);
}
