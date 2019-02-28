#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <osg/Switch>
#include <osg/Camera>
#include <osg/Image>
#include <osg/ComputeBoundsVisitor>
#include <osgDB/Readfile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerBase>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

class CompareMeshOutline
{
public:
	CompareMeshOutline(unsigned int size = 4096);
	~CompareMeshOutline();

	bool CompareMesh(osg::Node* baseMesh, osg::Node* simMesh, const std::vector<osg::Vec3d>& viewDirects);

	void ExportCompareMehsImage(const std::string& path, const std::string& name);

	double GetDiffArea();
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
	cv::Mat ImageSubtract(const cv::Mat& baseMat, const cv::Mat& simMat);

	cv::Mat ConvBGRImage2GrayImage(const cv::Mat& bgrimage);

private:
	unsigned int                                                 m_imageWidth;
	unsigned int                                                 m_imageHeight;

	std::vector<std::pair<cv::Mat, cv::Mat>>                     m_imageSequence;
	std::vector<cv::Mat>                                         m_imageCompared;
	double                                                       m_gsd;
	double                                                       m_diffPixCount;

};

