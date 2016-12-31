#ifndef NEURALHOUGHSTEP_H
#define NEURALHOUGHSTEP_H

#include "step.h"
#include "neural-hough/MLP.hpp"

/*
 * ArteLab paper "Neural 1D Barcode Detection Using the Hough Transform"
 * code mostly copied from https://github.com/SimoneAlbertini/BarcodeDetectionHough
 * are we allowed to use this?
*/
class NeuralHoughStep : public Step
{
public:
    NeuralHoughStep(const std::string &mlpFile);

public slots:
    void execute(void* data);

private:
    artelab::MLP mlp;

    void drawLinesAtAngle(double angle, std::vector<cv::Vec4i> lines, cv::Mat &image, int tolerance);
    void smoothHistogram(cv::Mat &hist, int kernel_size, int iterations);
    void histogramsFromHoughLines(cv::Mat feature, cv::Mat &row_hist, cv::Mat &col_hist, bool smooth_and_thresh=true);
    cv::Mat projectHistograms(cv::Mat row_hist, cv::Mat col_hist);
};

#endif // NEURALHOUGHSTEP_H
