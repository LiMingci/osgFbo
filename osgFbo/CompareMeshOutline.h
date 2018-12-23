#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <osg/Camera>
#include <osg/Image>
#include <osg/ComputeBoundsVisitor>
#include <osgDB/Readfile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerBase>

#include <opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>

class CompareMeshOutline
{
public:
	CompareMeshOutline();
	~CompareMeshOutline();

public:
	static osg::Camera* 
		CreateRttCamera(unsigned int texWidth, unsigned int texHeight, const osg::BoundingSphere& bs, bool useortho = true);

	static std::vector<osg::Vec3d> CreateViewDirect(unsigned int numDirect);

	static osg::Vec3d CreateUpDirectFromViewDirect(const osg::Vec3d& viewDirect);

	static std::vector<osg::ref_ptr<osg::Image>>
		GetImageFromView(osg::Node* node, std::vector<osg::Vec3d> viewDirects, unsigned int width, unsigned int height);

	static osg::Image* GetImageFromView(osg::Node* node, osg::Vec3d viewDirect, unsigned int width, unsigned int height);

	static cv::Mat ConvOsgImage2CvMat(const osg::Image* osgimage);

	//static osg::Image* ConvCvMat2OsgImage(const cv::Mat& cvmat);

private:

};

