#ifdef _WIN32
#include <Windows.h>
#endif

#include "CompareMeshOutline.h"


int main(int argc, char* argv[])
{
	osg::Vec3d  viewDirect(0.0, 0.0, 1.0);

	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile("dumptruck.osgt");

	osg::ref_ptr<osg::Image> image = CompareMeshOutline::GetImageFromView(node, viewDirect, 4096, 4096);

	osgDB::writeImageFile(*(image.get()), std::string("rtt.bmp"));

	cv::Mat mat = CompareMeshOutline::ConvOsgImage2CvMat(image.get());

	std::vector<int> compress;
	compress.push_back(cv::IMWRITE_JPEG_QUALITY);
	compress.push_back(100);
	cv::imwrite("rtt.jpg", mat, compress);

	return 0;
}

