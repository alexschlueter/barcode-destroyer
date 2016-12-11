#include "templatematchingstep.h"
#include "lsdstep.h"

#include <QDirIterator>
#include <QDebug>

#include <iostream>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

const int TemplateMatchingStep::patterns[][4] = {
       {3, 2, 1, 1},
       {2, 2, 2, 1},
       {2, 1, 2, 2},
       {1, 4, 1, 1},
       {1, 1, 3, 2},
       {1, 2, 3, 1},
       {1, 1, 1, 4},
       {1, 3, 1, 2},
       {1, 2, 1, 3},
       {3, 1, 1, 2}
};

TemplateMatchingStep::TemplateMatchingStep(QString cellpath)
{
    QDirIterator it(cellpath);
    while (it.hasNext()) {
        QString filep = it.next();
        if (filep.right(4).toLower() == ".dat") {
            QFile file(filep);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                throw "TemplateMatchingStep: Could not open cell files";
            }
            int digit = filep.mid(filep.length()-5, 1).toInt();
            QTextStream in(&file);
            in.readLine();
            while (! in.atEnd()) {
                QStringList parts = in.readLine().split(",");
                double area = parts[1].toDouble();
                Point2d centroid(parts[2].toDouble(), parts[3].toDouble());
                int pixelsPerSection[6];
                for (int i = 0; i < 6; i++)
                    pixelsPerSection[i] = parts[4+i].toInt();
                cellsPerDigit[digit].push_back(Cell(area, centroid, pixelsPerSection));
            }
        }

    }
    for (int i = 0; i < 10; i++) {
        for (const Cell &cell : cellsPerDigit[i]) {
            cout << cell << endl;
        }
    }
}

void TemplateMatchingStep::execute(void* data){
    LSDResult *lsdres = static_cast<LSDResult*>(data);

    LineIterator scanIt(lsdres->img, lsdres->leftBnd, lsdres->rightBnd, 8, true);
    vector<uchar> scan(scanIt.count);

    for (int i = 0; i < scanIt.count; i++, ++scanIt) {
        scan[i] = **scanIt;
    }
    std::sort(scan.begin(), scan.end());
    double lowMean = 0, highMean = 0, lowVar = 0, highVar = 0, var = 0;
    for (int i = 0; i < scanIt.count/2; i++) {
        cout << (int)scan[i] << " ";
        lowMean += scan[i];
        lowVar += scan[i]*scan[i];
        var += scan[i]*scan[i];
    }
    cout << endl << endl;


    for (int i = scanIt.count / 2; i < scanIt.count; i++) {
        cout << (int)scan[i] << " ";
        highMean += scan[i];
        highVar += scan[i]*scan[i];
        var += scan[i]*scan[i];
    }
    cout << endl << endl;
    var = var/scanIt.count-pow((lowMean+highMean)/scanIt.count, 2);
    lowMean /= scanIt.count/2;
    lowVar = lowVar*2/scanIt.count - lowMean*lowMean;
    int n2 = scanIt.count - scanIt.count/2;
    highMean /= n2;
    highVar = highVar/n2 - highMean*highMean;

    double w = scanIt.count / 95.0;
    cout << "w = " << w << endl;
    vector<double> lowDists(scanIt.count), highDists(scanIt.count);
    scanIt = LineIterator(lsdres->img, lsdres->leftBnd, lsdres->rightBnd, 8, true);
    for (int i = 0; i < scanIt.count; i++, ++scanIt) {
        lowDists[i] = pow(max(**scanIt-lowMean, 0.0), 2)/(2*var);
        highDists[i] = pow(min(**scanIt-highMean, 0.0), 2)/(2*var);
    }

    cout << lowMean << " " << lowVar << " " << highMean << " " << highVar << " " << var << endl << endl;

    Mat plot(600, 800, CV_8UC3);
    for (int i = 1; i < lowDists.size(); i++) {
        cout << lowDists[i] << " ";
        line(plot, {2*i, (int)(200-lowDists[i-1]*2000)}, {2*(i+1), (int)(200-lowDists[i]*2000)}, {0, 255, 0});
    }
    cout << endl << endl;
    for (int i = 1; i < highDists.size(); i++) {
        cout << highDists[i] << " ";
        line(plot, {2*i, (int)(500-highDists[i-1]*2000)}, {2*(i+1), (int)(500-highDists[i]*2000)}, {0, 0, 255});
    }
    cout << endl << endl << endl;
    imshow("plot", plot);

    double probabilities[6][10];
    for (int pos = 0; pos < 6; pos++) {
        for (int digit = 0; digit < 10; digit++) {
            probabilities[pos][digit] = 0;

            float o = 3*w + pos*7*w;
            float deltaO = 2*w;
            float deltaW = 2*deltaO/95;
            int wmin = w-deltaW;
            int omin = o-deltaO;
            int wmax = ceil(w+deltaW);
            int omax = ceil(o+deltaO);

            /*double distSum = 0;
            int parity = 1;
            int offset = ceil(o);
            for (int bar = 0; bar < 4; bar++) {
                int totalPixInBar = floor(w*patterns[digit][bar]);
                //cout << "bar " << bar << " pix " << totalPixInBar << endl;
                for (int ibar = 0; ibar < totalPixInBar; ibar++) {
                    //cout << distSum << " " << parity << " " << offset << " " << bar << " " << totalPixInBar << " " << ibar << endl;
                    distSum += parity==1 ? highDists[offset+ibar] : lowDists[offset+ibar];
                }
                offset += totalPixInBar;
                parity *= -1;
            }
            probabilities[pos][digit] = exp(-distSum);*/

            for (int iw = wmin; iw < wmax; iw++) {
                for (int io = omin; io < omax; io++) {
                    //if (w-deltaW <= iw && o-deltaO <= io && w+deltaW >= iw+1 && o+deltaO >= io+1) {
                    if (true) {
                        for (const Cell &cell : cellsPerDigit[digit]) {
                            double distSum = 0;
                            int offset = io;
                            int totalPixInBar = iw + cell.pixelsPerBar[0];
                            for (int ibar = 0; ibar < totalPixInBar; ibar++) {
                                distSum += lowDists[offset-ibar];
                            }

                            offset++;
                            int parity = 1;
                            for (int bar = 0; bar < 4; bar++) {
                                totalPixInBar = iw*patterns[digit][bar] + cell.pixelsPerBar[1+bar];
                                for (int ibar = 0; ibar < totalPixInBar; ibar++) {
                                    distSum += parity==1 ? highDists[offset+ibar] : lowDists[offset+ibar];
                                }
                                offset += totalPixInBar;
                                parity *= -1;
                            }

                            totalPixInBar = iw + cell.pixelsPerBar[5];
                            for (int ibar = 0; ibar < totalPixInBar; ibar++) {
                                distSum += parity==1 ? highDists[offset+ibar] : lowDists[offset+ibar];
                            }
                            probabilities[pos][digit] += exp(-distSum)*cell.area;
                        }
                    }
                }
            }
            cout << pos << " " << digit << " " << probabilities[pos][digit] << endl;
        }
        cout << endl;
    }

    //TODO read the Barcode
    QString result = "1234567890123";
    emit completed((void*)&result);
}
