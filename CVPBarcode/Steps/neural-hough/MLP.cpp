#include "MLP.hpp"

namespace artelab
{

    MLP::MLP() : _model(cv::ml::ANN_MLP::create()) {}

    MLP::MLP(cv::Mat layers) : _model(cv::ml::ANN_MLP::create())
    {
        _model->setLayerSizes(layers);
        _model->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM, 1, 1);

        _model->setTrainMethod(cv::ml::ANN_MLP::RPROP);
        _model->setBackpropWeightScale(0.1);
        _model->setBackpropMomentumScale(0.1);
        _model->setRpropDW0(0.1);
        _model->setRpropDWPlus(1.2);
        _model->setRpropDWMinus(0.5);
        _model->setRpropDWMin(FLT_EPSILON);
        _model->setRpropDWMax(50);
    }

    MLP::~MLP() {}

    void MLP::load(std::string file) 
    {
        _model = cv::ml::ANN_MLP::load<cv::ml::ANN_MLP>(file.c_str());
    }

    void MLP::save(std::string file) 
    {
        _model->save(file.c_str());
    }

    int MLP::train(const cv::Mat& patterns_in, const cv::Mat& targets, const int max_iter) 
    {
        CV_Assert(patterns_in.rows == targets.rows);

        _model->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, max_iter, 0.001));

        int iterations = _model->train(patterns_in, cv::ml::ROW_SAMPLE, targets);
        return iterations;
    }

    void MLP::predict(const cv::Mat& samples, cv::Mat& outPredictions) 
    {
        _model->predict(samples, outPredictions);
    }
}
