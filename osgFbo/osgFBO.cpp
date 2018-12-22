#ifdef _WIN32
#include <Windows.h>
#endif

#include "CompareMeshOutline.h"


int main(int argc, char* argv[])
{
	osg::Vec3  viewDirect(0.0, 0.0, 1.0);

	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(argv[1]);

	osg::ref_ptr<osg::Image> image = CompareMeshOutline::GetImageFromView(node, viewDirect, 1024, 1024);

	osgDB::writeImageFile(*(image.get()), std::string(argv[2]));

	return 0;
}

