#include "neuralhoughstep.h"
#include "../utils.h"

#include <opencv2/core/core.hpp>

#include "neural-hough/HoughTransform.hpp"
#include "neural-hough/mlp_threshold.hpp"
#include "neural-hough/hough_histogram.hpp"
#include "neural-hough/draw_hist.hpp"

using namespace std;
using namespace cv;

NeuralHoughStep::NeuralHoughStep(const string &mlpFile)
{
    mlp.load(mlpFile);
}

void NeuralHoughStep::drawLinesAtAngle(double angle, std::vector<cv::Vec4i> lines, cv::Mat& image, int tolerance=2)
{
    cv::Scalar color = image.channels() == 1? cv::Scalar(255) : cv::Scalar(0,0,255);

    foreach(cv::Vec4i l, lines)
    {
        double delta = double(l[3] - l[1]) / double(l[2] - l[0]);
        double act_angle = atan(delta)*180/CV_PI;
        act_angle += act_angle < 0? 180 : 0;
        double diff = fabs(act_angle - angle);
        if(diff < tolerance)
        {
            int thick = int(diff) > 3? 1 : 4-int(diff);
            cv::line(image, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), color, thick, 8);
        }
    }
}

void NeuralHoughStep::smoothHistogram(cv::Mat& hist, int kernel_size, int iterations)
{
    CV_Assert(hist.cols == 1);

    int hist_type = hist.type();
    bool cvt_back = hist_type != CV_32F;
    if(cvt_back) hist.convertTo(hist, CV_32F);

    cv::Mat kernel = cv::Mat::ones(kernel_size, 1, CV_32F);
    kernel = kernel / float(kernel_size);

    for(int i=0; i < iterations; i++)
    {
        cv::filter2D(hist, hist, -1, kernel);
    }

    if(cvt_back) hist.convertTo(hist, hist_type);
}

void NeuralHoughStep::histogramsFromHoughLines(cv::Mat feature, cv::Mat& row_hist, cv::Mat& col_hist, bool smooth_and_thresh)
{
    row_hist = get_histogram(feature, artelab::HIST_ROW, CV_32F);
    col_hist = get_histogram(feature, artelab::HIST_COL, CV_32F);

    if(smooth_and_thresh)
    {
        smoothHistogram(row_hist, 5, 200);
        smoothHistogram(col_hist, 5, 200);

        double row_hist_mean = cv::sum(row_hist)[0] / double(row_hist.rows);
        double col_hist_mean = cv::sum(col_hist)[0] / double(col_hist.rows);
        cv::threshold(row_hist, row_hist, row_hist_mean, 0, cv::THRESH_TOZERO);
        cv::threshold(col_hist, col_hist, col_hist_mean, 0, cv::THRESH_TOZERO);
    }
}

cv::Mat NeuralHoughStep::projectHistograms(cv::Mat row_hist, cv::Mat col_hist)
{
    CV_Assert(row_hist.type() == CV_32F);
    CV_Assert(col_hist.type() == CV_32F);

    cv::Mat out = row_hist * col_hist.t();

    double min, max;
    cv::minMaxLoc(out, &min, &max);
    out = (out - min) / max * 255;
    out.convertTo(out, CV_8U);

    return out;
}

void NeuralHoughStep::execute(void *data)
{
    Mat gray = *static_cast<Mat*>(data);

    // Apply canny
    Mat canny;
    GaussianBlur(gray, canny, Size(17, 17), 2);
    Canny(canny, canny, 60, 100, 3);

    artelab::HoughTransform hough(canny);
    Mat neural = artelab::threshold_mlp(mlp, Size(61, 3), hough);

    double angle = artelab::max_angle_hist(artelab::get_histogram(neural));

    const int tol = 3;
    vector<Vec4i> lines;
    HoughLinesP(canny, lines, 1, CV_PI/180, 50, 20, 1);

    // draw lines of given angle on the canny image
    Mat lineImg;
    cvtColor(canny, lineImg, CV_GRAY2BGR);
    drawLinesAtAngle((int(angle+0.5) + 90) % 180, lines, lineImg, tol);
    emit showImage("Prob. Hough", lineImg);

    // obtain a feature image with only the lines. It is rectified.
    Mat featureImg = Mat::zeros(canny.size(), CV_8U);
    drawLinesAtAngle((int(angle+0.5) + 90) % 180, lines, featureImg, tol);
    featureImg = rotateImage(featureImg, angle);

    // Histograms for detection
    Mat rowHist, colHist, featureWithHistSmooth;
    histogramsFromHoughLines(featureImg, rowHist, colHist);
    featureWithHistSmooth = artelab::draw_histogram_on_image(rowHist, featureImg, Scalar(0,0,255), artelab::HIST_ROW);
    featureWithHistSmooth = artelab::draw_histogram_on_image(colHist, featureWithHistSmooth, Scalar(0,255,0), artelab::HIST_COL);
    emit showImage("Histograms Smooth", featureWithHistSmooth);

    // project histograms
    Mat histProj = projectHistograms(rowHist, colHist);
    emit showImage("Histograms Projection", histProj);

    // crop image with projected mask and threshold
    Mat bbMask;
    bbMask = rotateImage(histProj, -angle);
    double min, max;
    minMaxLoc(bbMask, &min, &max);
    threshold(bbMask, bbMask, int(0.3*max), 255, THRESH_BINARY);
    Mat imgCropped;
    gray.copyTo(imgCropped, bbMask);
    emit showImage("Cropped", imgCropped);

    /*
    // draw bounding boxes
    std::vector<Rect> rects = object_rectangles(histProj, int(0.3*max));
    Mat img_bb;
    img_orig.copyTo(img_bb);
    Mat img_detection_mask = Mat::zeros(img_orig.size(), CV_8U);
    foreach(Rect r, rects)
    {
        // BB for impression on original image
        Mat bb = Mat::zeros(img_bb.size(), CV_8U);
        rectangle(bb, r, Scalar(255), 2);
        bb = artelab::rotate_image(bb, -angle);
        img_bb.setTo(Scalar(0,0,255), bb);
    }*/
    emit completed(data);
}
