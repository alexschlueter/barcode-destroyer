#include "detector.h"
#include <iostream>
using namespace std;

Detector::Detector(int index, QString path, int function)
{
    this->index = index;
    this->path = path;
    this->code = "";
    this->successful = false;
    this->function = function;

}

void Detector::detect(){
    if(loadImage()){
        cv::imshow("test",gray);
        switch (function) {
        case 1:
            lineSegmentDetector();
            break;
        default:
            defaultDetector();
            break;
        }
        //TODO transform barcode

        //TODO readBarcode

        //successful = true;
        //code = "*somecode*";
    } else {
        successful = false;
    }
}

bool Detector::isSuccessful(){
    return successful;
}

QString Detector::result(){
    return this->code;
}

bool Detector::loadImage(){
    cv::Size scaledSize;
    color = cv::imread(path.toStdString());
    //Turn image to Grayscale
    cv::cvtColor(color,gray,CV_BGR2GRAY);
    /*scaledSize = getNewSize( gray, 500 );
    cv::resize( gray, gray, scaledSize );*/
    return true;
}

cv::Size Detector::getNewSize( cv::Mat src, uint maxSize ) {
    uint newHeight, newWidth;
    float scaleX, scaleY;

    scaleX = (float) maxSize / (float) src.cols;
    scaleY = (float) maxSize / (float) src.rows;

    if ( scaleX < scaleY ) {
        newWidth = maxSize;
        newHeight = src.rows * scaleX;
    } else {
        newWidth = src.cols * scaleY;
        newHeight = maxSize;
    }

    return cv::Size(newWidth, newHeight);
}

void Detector::defaultDetector() {
    cv::Mat gradX, gradY, grad, gradAbs, blur, threshold, kernel, closed, eroded, dilated;
    std::vector< std::vector< cv::Point > > contours;
    cv::RotatedRect boundingBox;

    cv::Sobel( gray, gradX, CV_32F, 1, 0, -1 );
    cv::Sobel( gray, gradY, CV_32F, 0, 1, -1 );

    cv::subtract(gradX, gradY, grad);
    cv::convertScaleAbs(grad, gradAbs );

    cv::blur( gradAbs, blur, cv::Point(9, 9) );
    cv::threshold( blur, threshold, 225, 255, cv::THRESH_BINARY );

    kernel = cv::getStructuringElement( cv::MORPH_RECT, cv::Point(21, 7) );
    cv::morphologyEx( threshold, closed, cv::MORPH_CLOSE, kernel );

    cv::erode( closed, eroded, cv::Mat(), cv::Point(-1, -1), 4 );
    cv::dilate( eroded, dilated, cv::Mat(), cv::Point(-1, -1), 4 );

    cv::findContours( eroded.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );
    std::sort( contours.begin(), contours.end(), compareContourAreas );
    drawContourOnOriginalImage( contours, 1 );


    cv::imshow("Detected Code", gray);
}


void Detector::drawRotatedRect(cv::Mat& img, cv::RotatedRect rect) {
    cv::Point2f points[4];
    rect.points(points);
    for (int i = 0; i < 4; i++)
        cv::line(img, points[i], points[(i+1)%4], {0, 0, 255});
}

void Detector::lineSegmentDetector() {
    const float angleTol = 0.1*180/CV_PI;
    const float lengthTol = 0.3;
    const float centerTol = 0.1;
    const float scanOffsets[] = {-0.3, -0.15, 0, 0.15, 0.3};

    auto lsd = cv::createLineSegmentDetector(cv::LSD_REFINE_ADV);
    std::vector<cv::Vec4f> lines;
    std::vector<double> widths;
    lsd->detect(gray, lines, widths);

    using namespace std;
    // Show found lines
    cv::Mat res = gray.clone();
    lsd->drawSegments(res, lines);
    imshow("lsd", res);

    cv::Mat drawnLines = gray.clone();
    std::vector<int> scores(lines.size());
    auto scoreIt = scores.begin();
    for (auto &&line : lines) {
        cv::Point2f p1(line[0], line[1]);
        cv::Point2f q1(line[2], line[3]);
        cv::Point2f vec1 = p1 - q1;
        float angle1 = std::atan2(-vec1.y, vec1.x)*180/CV_PI;
        cv::Point2f center1 = 0.5*(p1+q1);
        float length1 = cv::norm(vec1);

        cv::RotatedRect boundingBox(center1, {2*length1, length1}, 90-angle1);
        drawRotatedRect(drawnLines, boundingBox);

        auto widthIt = widths.cbegin();
        for (auto &&line2 : lines) {
            cv::Point2f p2(line2[0], line2[1]);
            cv::Point2f q2(line2[2], line2[3]);
            cv::Point2f vec2 = p2 - q2;
            float angle2 = std::atan2(-vec2.y, vec2.x)*180/CV_PI;
            cv::Point2f center2 = 0.5*(p2+q2);
            float length2 = cv::norm(vec2);
            float angleDist = std::abs(angle1-angle2);
            angleDist = std::min(angleDist, std::abs(180-angleDist));
            angleDist = std::min(angleDist, std::abs(180-angleDist));
            float relProjCenterDiff = (center2-center1).dot(vec1) / (length1*length1);
            float relLengthDiff = (length1-length2)/length1;
            if (angleDist < angleTol && std::abs(relLengthDiff) < lengthTol && std::abs(relProjCenterDiff) < centerTol) {
                cv::RotatedRect line2Box(center2, {(float)*widthIt, length2}, 90-angle2);
                drawRotatedRect(drawnLines, line2Box);
                std::vector<cv::Point2f> dummy;
                int intersects = cv::rotatedRectangleIntersection(boundingBox, line2Box, dummy);
                if (intersects)
                    ++(*scoreIt);
            } else {
                /*cout << angle1 << " " << angle2 << " " << angleDist << " " << std::abs(relLengthDiff) << " " << std::abs(relProjCenterDiff) << endl;
                cout << center1 << " " << center2 << " " << vec1 << " " << vec2 << endl;*/
            }
            ++widthIt;
        }
        ++scoreIt;
    }
    auto maxScoreIt = std::max_element(scores.begin(), scores.end());
    const auto &bestLine = *(lines.begin() + std::distance(scores.begin(), maxScoreIt));

    cv::Mat onlyLines(gray.size(), CV_8U, cv::Scalar(0));
    scoreIt = scores.begin();
    for (auto &&line : lines) {
        //cout << *scoreIt << endl;
        cv::line(onlyLines, {(int)line[0], (int)line[1]}, {(int)line[2], (int)line[3]}, *scoreIt*255/(float)*maxScoreIt);
        ++scoreIt;
    }
    cout << "max score " << *maxScoreIt << endl;
    cv::applyColorMap(onlyLines, color, cv::COLORMAP_JET);
    //cv::line(color, {(int)bestLine[0], (int)bestLine[1]}, {(int)bestLine[2], (int)bestLine[3]}, {255, 255, 255});
    imshow("boxes", drawnLines);


    cv::Point2f p(bestLine[0], bestLine[1]);
    cv::Point2f q(bestLine[2], bestLine[3]);
    cv::Point2f vec = p - q;
    float length = cv::norm(vec);
    cv::Point2f perp = {-vec.y, vec.x};
    cv::Point2f center = 0.5*(p+q);

    float longEnough = (gray.rows*gray.rows + gray.cols*gray.cols)/length;
    cv::LineIterator fullBisectIt(gray, center-longEnough*perp, center+longEnough*perp);

    int maxVar = 0, minVar = 0;
    cv::Point minPos, maxPos;
    cv::Rect imgRect(cv::Point(), gray.size());
    cv::Mat plot(600, 1800, CV_8UC3, cv::Scalar(0));
    int lastvar=0, lastposvar=0, cnt=0;

    for (int i = 0; i < fullBisectIt.count; i++, ++fullBisectIt) {

        cv::LineIterator shortBisectIt(gray, fullBisectIt.pos()-(cv::Point)perp, fullBisectIt.pos());
        cv::LineIterator oldShortBisectIt = shortBisectIt++;
        int variation = 0;
        for (int j = 0; j < shortBisectIt.count-1; j++, ++shortBisectIt, ++oldShortBisectIt) {
            //cout << fullBisectIt.pos() << " " << shortBisectIt.pos() << " " << oldShortBisectIt.pos() <<  " " << (int)**shortBisectIt << " " << (int)**oldShortBisectIt << " " << std::abs(**shortBisectIt-**oldShortBisectIt) << endl;
            if (i==500)
            cv::circle(color, shortBisectIt.pos(), 4, {0, 0, (double)j});
            cv::Point2f curPos = shortBisectIt.pos();
            cv::Point2f oldPos = oldShortBisectIt.pos();
            int tempVar = 0;
            for (int k = 0; k < 5; k++) {
                cv::Point2f offset = scanOffsets[k]*vec;
                cv::Point curPosAdjusted = curPos+offset;
                cv::Point oldPosAdjusted = oldPos+offset;
                if (!imgRect.contains(curPosAdjusted) || !imgRect.contains(oldPosAdjusted)) {
                    tempVar = 0;
                    break;
                }
                //cv::circle(color, curPosAdjusted, 1, {0, 255, 0});
                tempVar += std::abs(gray.at<uchar>(curPosAdjusted)-gray.at<uchar>(oldPosAdjusted));
            }
            variation += tempVar;
        }
        int posvar = variation;
        shortBisectIt = cv::LineIterator(gray, fullBisectIt.pos(), fullBisectIt.pos()+(cv::Point)perp);
        oldShortBisectIt = shortBisectIt++;
        for (int j = 0; j < shortBisectIt.count-1; j++, ++shortBisectIt, ++oldShortBisectIt) {
            if (i == 500)
            cv::circle(color, shortBisectIt.pos(), 4, {(double)j, 0, 0});
            cv::Point2f curPos = shortBisectIt.pos();
            cv::Point2f oldPos = oldShortBisectIt.pos();
            int tempVar = 0;
            for (int k = 0; k < 5; k++) {
                cv::Point2f offset = scanOffsets[k]*vec;
                cv::Point curPosAdjusted = curPos+offset;
                cv::Point oldPosAdjusted = oldPos+offset;
                if (!imgRect.contains(curPosAdjusted) || !imgRect.contains(oldPosAdjusted)) {
                    tempVar = 0;
                    break;
                }
                tempVar += std::abs(gray.at<uchar>(curPosAdjusted)-gray.at<uchar>(oldPosAdjusted));
            }
            variation -= tempVar;
        }
        if (variation <= minVar) {
            minVar = variation;
            minPos = fullBisectIt.pos();
        }
        if (variation >= maxVar) {
            maxVar = variation;
            maxPos = fullBisectIt.pos();
        }
        cnt++;
        int mii = 38000;
        int sc = 145;
        cv::line(plot, {(int)cnt-1, (lastposvar+mii)/sc}, {(int)cnt, (posvar+mii)/sc}, {0, 255, 0});
        cv::line(plot, {(int)cnt-1, (lastposvar-lastvar+mii)/sc}, {(int)cnt, (posvar-variation+mii)/sc}, {255, 0, 0});
        cv::line(plot, {(int)cnt-1, (lastvar+mii)/sc}, {(int)cnt, (variation+mii)/sc}, {0, 0, 255});
        lastvar=variation;
        lastposvar=posvar;
    }


    cout << minPos << " " << minVar << " " << maxPos << " " << maxVar << endl;
    cv::circle(color, 0.5*(minPos+maxPos), 4, {0, 0, 255});
    cv::circle(color, minPos, 4, {0, 255, 0});
    cv::circle(color, maxPos, 4, {0, 0, 255});
    cv::RotatedRect barcodeBB(0.5*(minPos+maxPos), {(float)cv::norm(minPos-maxPos), length}, std::atan2(minPos.y-maxPos.y, minPos.x-maxPos.x)*180/CV_PI);
    drawRotatedRect(color, barcodeBB);
    imshow("best line", color);
    imshow("plot", plot);
}

bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 ) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i > j );
}

void Detector::drawContourOnOriginalImage( std::vector< std::vector< cv::Point > > contours, uint contoursToDrawCount ) {
    int count = ( contoursToDrawCount < contours.size() ) ? contoursToDrawCount : contours.size();
    for ( int i = 0; i < count; ++i ) {
        cv::drawContours( gray, contours, i, cv::Scalar(0, 255, 0), 2, 8 );
    }
}
