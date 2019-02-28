#ifdef _WIN32
#include <Windows.h>
#endif

#include <osg/Switch>

#include "CompareMeshOutline.h"


int main(int argc, char* argv[])
{
#if 0
	osg::Vec3d  viewDirect(0.0, 0.0, 1.0);

	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile("D:\\OpenSceneGraph\\Data\\dumptruck.osgt");

	osg::ref_ptr<osg::Image> image = CompareMeshOutline::GetImageFromView(node, viewDirect, 4096, 4096);

	osgDB::writeImageFile(*(image.get()), std::string("rtt.bmp"));

	cv::Mat mat = CompareMeshOutline::ConvOsgImage2CvMat(image.get());

	std::vector<int> compress;
	compress.push_back(cv::IMWRITE_JPEG_QUALITY);
	compress.push_back(100);
	cv::imwrite("rtt.jpg", mat, compress);
#endif

	std::vector<osg::Vec3d> viewDirects = CompareMeshOutline::CreateViewDirect(4);

	CompareMeshOutline compMesh;
	
	osg::ref_ptr<osg::Node> baseNode = osgDB::readNodeFile("F:\\dev_data\\horse_uv\\horse.osgb");
	osg::ref_ptr<osg::Node> simNode = osgDB::readNodeFile("F:\\dev_data\\horse_uv\\horse_sim01.osgb");

	compMesh.CompareMesh(baseNode, simNode, viewDirects);

	compMesh.ExportCompareMehsImage("F:\\dev_data\\horse_uv\\comp", "horse_sim_comp");

	double diffArea = compMesh.GetDiffArea();

	std::cout << "Different Area : " << diffArea << std::endl;


	return 0;
}

