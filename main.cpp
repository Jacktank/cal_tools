#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "opencv2/ccalib/omnidir.hpp"

//#define RESIZE_IMG

float ratio = 0.5;
cv::Mat frame;
cv::Mat M, D;
std::vector<cv::Point2f> vp;
cv::Point same_points[2];

void on_mouse(int event, int x, int y, int flags, void *ustc)
{
	cv::Point2f pt;
#ifdef RESIZE_IMG
	pt.x = (float)x / ratio;
	pt.y = (float)y / ratio;
#else
	pt.x = (float)x;
	pt.y = (float)y;
#endif

	if (event == CV_EVENT_LBUTTONDOWN)
	{
		vp.push_back(pt);
		std::cout << "\npush a point to vector..." << vp.size() << std::endl;
	}
}

int main(int argc, char* argv[])
{

	int no_cam = -1;
	int no_win = -1;
	int no_dst = -1;

	std::string str_dst;
	if (argc == 3)
	{
		no_dst = atoi(argv[1]);
		no_cam = atoi(argv[2]);
	}
	else
	{
		std::cout << "Please input five parameters, \n" \
			<< "	1th: name of exe \n" \
			<< "	2th: NO of dst \n" \
			<< "	3th: NO of camera " << std::endl;
		return -1;
	}

	std::stringstream sttr1;
	std::string str_f;
	sttr1 << "./test_img/"<<no_dst<<"/pts_" << no_cam<<".csv";
	sttr1 >> str_f;
	std::ofstream file(str_f);

	std::string str_cap;
	std::stringstream sttr;
	if (no_cam != 3)
		sttr << "rtsp://admin:12345goccia@10.0.0.10" << no_cam << ":554//Streaming/Channels/1";
	else
		sttr << "rtsp://admin:123456goccia@10.0.0.10" << no_cam << ":554//Streaming/Channels/1";

	sttr >> str_cap;
	cv::VideoCapture cap(str_cap);
	if (!cap.isOpened())
	{
		std::cout << "error:fail to load camera " << std::endl;
		return -1;
	}

	int image_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	int image_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

	std::string	str_param;
	sttr1.clear();
	sttr1<<"./params/params_"<<no_cam<<".xml";
	sttr1>>str_param;
		
	cv::FileStorage fs(str_param, cv::FileStorage::READ);

	if(!fs.isOpened())
	{	
			std::cout<<"fail to load the Calibration XML file..."<<std::endl;
			return -1;
	}
	
	float x_;
	cv::Mat M, D;
	std::vector<float> xi;
	if(no_cam <= 2)
	{
			fs["camera_matrix"]>>M;
			fs["distortion_coefficients"]>>D;
			fs["xi"]>>x_;
			xi.push_back(x_);
			std::cout<<M<<std::endl;
			std::cout<<D<<std::endl;
			std::cout<<x_<<std::endl;
	}
	else
	{
			fs["M0"]>>M;
			fs["D0"]>>D;
			std::cout<<M<<std::endl;
			std::cout<<D<<std::endl;
	}

	cv::namedWindow("img");
	cv::setMouseCallback("img", on_mouse, 0);

	cv::Mat frame_display;
	while (true)
	{
		cap >> frame;
		if (frame.empty())
		{
			std::cout << "fail to get cam image..." << std::endl;
			break;
		}

#ifdef RESIZE_IMG
		cv::Mat frame_resize;
		cv::resize(frame, frame_resize, cv::Size(), ratio, ratio, CV_INTER_LINEAR);
		frame_display = frame_resize.clone();
#else
		frame_display = frame.clone();
#endif

		if (vp.size() == 1)
		{

			std::vector<cv::Point2f> undistortPts, undistortPts_norm;

			cv::undistortPoints(vp, undistortPts_norm, M, D);
			cv::undistortPoints(vp, undistortPts, M, D, cv::Mat::eye(3, 3, CV_32FC1), M);

			std::cout << "Note: 0:distortPts, 1:undistortPts, 2:undistortPts_norm" << std::endl;
			std::cout << "0:" << vp[0].x << "," << vp[0].y << std::endl;
			std::cout << "1:" << undistortPts[0].x << "," << undistortPts[0].y << std::endl;
			std::cout << "2:" << undistortPts_norm[0].x << "," << undistortPts_norm[0].y << std::endl;

			float delt_x = vp[0].x - undistortPts[0].x;
			float delt_y = vp[0].y - undistortPts[0].y;

			std::cout << "delt_X:" << delt_x << std::endl;
			std::cout << "delt_Y:" << delt_y << std::endl;
			std::cout << "squareError:" << std::sqrt(delt_x*delt_x + delt_y*delt_y) << std::endl;

			//Copy the points to circle in frames;
#ifdef RESIZE_IMG
			same_points[0].x = (int)(ratio*vp[0].x);
			same_points[0].y = (int)(ratio*vp[0].y);
			same_points[1].x = (int)(ratio*undistortPts[0].x);
			same_points[1].y = (int)(ratio*undistortPts[0].y);
#else
			same_points[0].x = (int)(vp[0].x);
			same_points[0].y = (int)(vp[0].y);
			same_points[1].x = (int)(undistortPts[0].x);
			same_points[1].y = (int)(undistortPts[0].y);
#endif
			file <<vp[0].x<<","<< vp[0].y << "," << undistortPts[0].x << "," << undistortPts[0].y << "," 
				<<undistortPts_norm[0].x << "," << undistortPts_norm[0].y << std::endl;
			vp.clear();
		}

		cv::circle(frame_display, same_points[0], 2, cv::Scalar(255, 0, 0));
		cv::circle(frame_display, same_points[1], 2, cv::Scalar(0, 255, 0));

		cv::imshow("img", frame_display);
		char key = cv::waitKey(3);
		if (key == 'q')
			break;
		else if(key == 'u')
		{
			file << "-----------------------" << std::endl;
		}
		else if(key == 'g')
		{
				std::stringstream dst; 
				dst<<"./test_img/"<<no_dst<<"/cam_"<<no_cam<<".jpg";
				std::string dst_str;
				dst>>dst_str;
				cv::imwrite(dst_str,frame);						
				std::cout<<"grab "<<"th image..."<<std::endl;	
		}

	}

	file.close();

	return 0;
}
			

