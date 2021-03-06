/* INCLUDES FOR THIS PROJECT */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "matching2D.hpp"
#include "ring_buffer.h"

using namespace std;

/* MAIN PROGRAM */
int main(int argc, const char *argv[])
{

    /* INIT VARIABLES AND DATA STRUCTURES */

    // data location
    string dataPath = "../";

    // camera
    string imgBasePath = dataPath + "images/";
    string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
    string imgFileType = ".png";
    int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
    int imgEndIndex = 9;   // last file index to load
    int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)

    // misc
    int dataBufferSize = 2; // no. of images which are held in memory (ring buffer) at the same time
    ring_buffer<DataFrame> img_buffer(dataBufferSize);
    bool bVis = false; // visualize results

    double avg_time = 0;
    double avg_number_of_keypoints_matches = 0;

    /* MAIN LOOP OVER ALL IMAGES */
    for (size_t imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
    {
        /* 
            LOAD IMAGE 
        */

        // assemble filenames for current index
        ostringstream imgNumber;
        imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
        string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

        // load image from file and convert to grayscale
        cv::Mat img, imgGray;
        img = cv::imread(imgFullFilename);
        cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

        DataFrame frame;
        frame.cameraImg = imgGray;

        cout << "#1 : LOAD IMAGE done" << endl;
        /* 
            DETECT IMAGE KEYPOINTS

            - extract 2D keypoints from current image
            - create empty feature list for current image
            - invoke desired detector   
            - only keep keypoints on the preceding vehicle
         */

        if (imgIndex == 0)
        {
            avg_time = 0;
        }
        double t = (double)cv::getTickCount();
        vector<cv::KeyPoint> keypoints;
        string detectorType = "ORB"; // SHITOMASI, HARRIS, FAST, BRISK, ORB, AKAZE and SIFT

        try
        {
            detect_keypoints(keypoints, imgGray, detectorType, false);
        }
        catch (const char *msg)
        {
            cerr << msg << endl;
            return -1;
        }

        bool bFocusOnVehicle = true;
        cv::Rect vehicleRect(535, 180, 180, 150);
        if (bFocusOnVehicle)
        {
            vector<cv::KeyPoint> relevant_keypoints;
            for (auto it = keypoints.begin(); it != keypoints.end(); it++)
            {
                if (vehicleRect.contains(it->pt))
                {
                    relevant_keypoints.push_back(*it);
                }
            }
            keypoints = relevant_keypoints;
        }

        // optional : limit number of keypoints (helpful for debugging and learning)
        bool bLimitKpts = false;
        if (bLimitKpts)
        {
            int maxKeypoints = 50;

            if (detectorType.compare("SHITOMASI") == 0)
            { // there is no response info, so keep the first 50 as they are sorted in descending quality order
                keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
            }
            cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
            cout << " NOTE: Keypoints have been limited!" << endl;
        }

        // push keypoints and descriptor for current frame to end of data buffer
        frame.keypoints = keypoints;
        cout << "#2 : DETECT KEYPOINTS done" << endl;

        /* 
            EXTRACT KEYPOINT DESCRIPTORS 
        
        */

        cv::Mat descriptors;
        string descriptorType = "BRISK"; // BRISK, BRIEF, ORB, FREAK, AKAZE and SIFT
        try
        {
            keypoints_descriptor(frame.keypoints, frame.cameraImg, descriptors, descriptorType);
        }
        catch (const char *msg)
        {
            cerr << msg << endl;
            return -1;
        }

        // push descriptors for current frame to end of data buffer
        frame.descriptors = descriptors;
        img_buffer.insert(frame);

        cout << "#3 : EXTRACT DESCRIPTORS done" << endl;

        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        avg_time += 1000 * t / 1.0;

        int number_of_keypoints_matches = 0;
        if (img_buffer.size() > 1) // wait until at least two images have been processed
        {
            auto current_frame = img_buffer.get();
            img_buffer.pop();
            auto next_frame = img_buffer.get();

            if (current_frame == nullptr || next_frame == nullptr)
            {
                cout << "Ring buffer returned a null pointer!";
                return -1;
            }

            /* MATCH KEYPOINT DESCRIPTORS */

            vector<cv::DMatch> matches;
            string matcherType = "MAT_BF";      // MAT_BF, MAT_FLANN
            string descriptor_type = "DES_BINARY"; // DES_BINARY, DES_HOG
            string selectorType = "SEL_KNN";       // SEL_NN, SEL_KNN

            //// STUDENT ASSIGNMENT
            //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
            //// TASK MP.6 -> add KNN match selection and perform descriptor distance ratio filtering with t=0.8 in file matching2D.cpp

            matchDescriptors(next_frame->keypoints, current_frame->keypoints,
                             next_frame->descriptors, current_frame->descriptors,
                             matches, descriptor_type, matcherType, selectorType);

            //// EOF STUDENT ASSIGNMENT
            number_of_keypoints_matches = matches.size();
            avg_number_of_keypoints_matches += number_of_keypoints_matches;

            // store matches in current data frame
            current_frame->kptMatches = matches;

            cout << "#4 : MATCH KEYPOINT DESCRIPTORS done" << endl;

            // visualize matches between current and previous image
            bVis = true;
            if (bVis)
            {
                cv::Mat matchImg = (current_frame->cameraImg).clone();
                cv::drawMatches(next_frame->cameraImg, next_frame->keypoints,
                                current_frame->cameraImg, current_frame->keypoints,
                                matches, matchImg,
                                cv::Scalar::all(-1), cv::Scalar::all(-1),
                                vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

                string windowName = "Matching keypoints between two camera images";
                cv::namedWindow(windowName, 7);
                cv::imshow(windowName, matchImg);
                bool save_to_file = false;
                if (save_to_file)
                {
                    string image_output = "../images/readme_images/" + detectorType + "_AND_" + descriptorType + ".png";
                    cv::imwrite(image_output, matchImg);
                }
                cout << "Press key to continue to next image" << endl;
                cv::waitKey(0); // wait for key to be pressed
            }
            bVis = false;
        }

        bool statistics = false;
        if (statistics)
        {
            ofstream statistics_file;
            string path_to_statistics_file = "../statistics/statistics_" + detectorType + "_AND_" + descriptorType + ".txt";
            statistics_file.open(path_to_statistics_file, std::ios_base::app);

            statistics_file << "\nSTATISTICS: DETECTOR " << detectorType << " and DESCRIPTOR " << descriptorType << endl;

            statistics_file << detectorType << " detector on image : " << imgStartIndex + imgIndex << "; No. of keypoints: " << keypoints.size() << endl;

            statistics_file << detectorType << " detector and " << descriptorType << " descriptor on image : " << imgStartIndex + imgIndex << "; Processing time: " << 1000 * t / 1.0 << " ms" << endl;

            statistics_file << detectorType << " detector and " << descriptorType << " descriptor on image : " << imgStartIndex + imgIndex << "; No. of keypoint mathces: " << number_of_keypoints_matches << endl;

            if (imgIndex == imgEndIndex - imgStartIndex)
            {
                statistics_file << "\nAVERAGE TIME : " << avg_time / 10 << "  ms " << endl;
                statistics_file << "AVERAGE No. OF KEYPOINT MATHES : " << avg_number_of_keypoints_matches / 9 << endl;
            }

            statistics_file << endl;
        }

    } // eof loop over all images

    return 0;
}
