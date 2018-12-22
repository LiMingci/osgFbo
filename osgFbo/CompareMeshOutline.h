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

class CompareMeshOutline
{
public:
	CompareMeshOutline();
	~CompareMeshOutline();

public:
	static osg::Camera* 
		CreateRttCamera(unsigned int texWidth, unsigned int texHeight, const osg::BoundingSphere& bs, bool useortho = true);

	static std::vector<osg::Vec3> CreateViewDirect(unsigned int numDirect);

	static osg::Vec3 CreateUpDirectFromViewDirect(const osg::Vec3& viewDirect);

	static std::vector<osg::ref_ptr<osg::Image>>
		GetImageFromView(osg::Node* node, std::vector<osg::Vec3> viewDirects, unsigned int width, unsigned int height);

	static osg::Image* GetImageFromView(osg::Node* node, osg::Vec3 viewDirect, unsigned int width, unsigned int height);

private:

};

