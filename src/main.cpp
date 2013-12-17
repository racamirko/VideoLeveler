/*
 *     VideoLeveler
 *
 *
 *  Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013, Chili lab, EPFL.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Chili, EPFL nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  File:    main.cpp
 *  Author:  Mirko Raca <name.lastname@epfl.ch>
 *  Created: December 16, 2013.
 */

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <string>
#include <iostream>

#include <glog/logging.h>

using namespace std;
using namespace cv;

void skinDetectionGM( Mat& _frameMat, Mat& _resMask );

int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);

    /*      tmp params      */
    string videoFilename = string(argv[1]);

    /*      inits       */
    VideoCapture vSrc = VideoCapture(videoFilename);
    if( !vSrc.isOpened() ){
        LOG(FATAL) << "Video " << videoFilename << " not found";
        cout << "Video not found" << endl;
        exit(1);
    }

    /*      processing      */
    double curFrameNo = vSrc.get(CV_CAP_PROP_POS_FRAMES);
    double maxFrames = vSrc.get(CV_CAP_PROP_FRAME_COUNT);
    int skipCount = vSrc.get(CV_CAP_PROP_FPS) * 20,
        curSkipNo = 0;
    Mat curFrame, curMask;
    while(curFrameNo < maxFrames){
        vSrc >> curFrame;
        if( curSkipNo++ < skipCount )
            continue;
        curSkipNo = 0;
        skinDetectionGM(curFrame, curMask);
//        imshow("curMaskWnd",curMask);
        curFrame.setTo(Scalar(0.0f, 255.0f, 0.0f), curMask);
        imshow("testwnd",curFrame);
        waitKey(10);
    }

    return 0;
}

/*
 *  Outputs the mask of skin-colored regions of the image based on the paper
 *          Automatic Feature Construction and a Simple Rule Induction Algorithm
 *          for Skin Detection
 *      G. Gomez, E. F. Morales (2002)
 *
 *
 */
void skinDetectionGM( Mat& _frameMat, Mat& _resMask ){
    _resMask = _frameMat.clone();
    _resMask.setTo(Scalar(0)); // TEMPORARY !!
    // normalizing the image
    Mat curFrameFloat; _frameMat.convertTo(curFrameFloat, CV_32FC3);
    vector<Mat> channels;
    split(curFrameFloat, channels);
    Mat sumValues = Mat::zeros(_frameMat.rows, _frameMat.cols, CV_32FC1);
    for( Mat& ch : channels )
        sumValues = sumValues + ch;
    for( Mat& ch : channels )
        ch = ch / sumValues;
    merge(channels, curFrameFloat);
    Mat sumAllChannels = channels[0] + channels[1] + channels[2];
    // implementing the rules
    // rule 1: b/g < 1.249
    Mat rule1Binary;
    {
        Mat rule1 = channels[0] / channels[1];
        inRange(rule1, Scalar(0.0f), Scalar(1.249f), rule1Binary );
    }
    // rule 2: (r+g+b)/(3*r) > 0.696
    Mat rule2Binary;
    {
        Mat rule2 = Mat::zeros(_frameMat.rows, _frameMat.cols, CV_32FC1);
        rule2 = ( sumAllChannels )/ ( 3.0f*channels[2] );
        inRange(rule2, Scalar(0.696f), Scalar(1000.0f), rule2Binary );
    }
    // rule 3: 1/3.0 - b/(r+g+b) > 0.014
    Mat rule3Binary;
    {
        Mat constMat = Mat::zeros(_frameMat.rows, _frameMat.cols, CV_32FC1);
        constMat.setTo(Scalar(1.0f/3.0f));
        Mat rule3 = Mat::zeros(_frameMat.rows, _frameMat.cols, CV_32FC1);
        rule3 = constMat - channels[0]/( sumAllChannels );
        inRange(rule3, Scalar(0.014f), Scalar(1000.0f), rule3Binary);
    }
    // rule 4: g/(3*(r+g+b)) < 0.108
    Mat rule4Binary;
    {
        Mat rule4 = Mat::zeros(_frameMat.rows, _frameMat.cols, CV_32FC1);
        rule4 = channels[1] / ( 3.0f * sumAllChannels );
        inRange(rule4, Scalar(0.0f), Scalar(0.108f), rule4Binary);
    }
    // show
    // final mask
    _resMask = rule1Binary & rule2Binary & rule3Binary & rule4Binary;
//    imshow("normChanels", curFrameFloat);
//    waitKey(10);
}
