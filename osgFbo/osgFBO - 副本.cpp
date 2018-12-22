#ifdef _WIN32
#include <Windows.h>
#endif

#include "CompareMeshOutline.h"


#if 0
osg::Camera* createRttCamera(int texWidth, int texHeight, const osg::BoundingBox& bs)
{
	osg::ref_ptr<osg::Camera> rttCamera = new osg::Camera();
	rttCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	rttCamera->setClearColor(osg::Vec4(0.4, 0.7, 0.9, 0.5));
	rttCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	rttCamera->setViewport(0, 0, texWidth, texHeight);
	rttCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	rttCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);


	double viewDistance = bs.zMax() - bs.zMin();
	double znear = viewDistance / 2.0;
	double zfar = viewDistance ;
	float top = (bs.yMax() - bs.yMin()) / 2.0;
	float right = (bs.xMax() - bs.xMin()) / 2.0;
	//rttCamera->setProjectionMatrixAsFrustum(-right, right, 
	//	                                    -top, top, 
	//										znear, zfar);
	rttCamera->setProjectionMatrixAsOrtho(-right, right, 
		                                    -top, top, 
											znear, zfar);

	osg::Vec3d upDirection(0.0, 1.0, 0.0);
	osg::Vec3d viewDirection(0.0, 1.0, 1.0);
	osg::Vec3d center = bs.center();
	osg::Vec3d eyePoint = center + viewDirection * viewDistance;
	rttCamera->setViewMatrixAsLookAt(eyePoint, center, upDirection);
	return rttCamera.release();

}


int main(int argc, char **argv)
{
	osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("cessna.osg");

	model->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	int texWidth = 512;
	int texHeight = 512;

	osg::ComputeBoundsVisitor cbv;
	model->accept(cbv);
	osg::BoundingBox bbox = cbv.getBoundingBox();
	osg::Camera* rttCamera = createRttCamera(texWidth, texHeight, bbox);

	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 0;
	traits->y = 0;
	traits->width = 512;
	traits->height = 512;
	traits->windowDecoration = true;
	traits->pbuffer = true;
	traits->doubleBuffer = true;
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

	rttCamera->setGraphicsContext(gc.get());

	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(texWidth, texHeight, 1, GL_RGB, GL_UNSIGNED_BYTE);
	//rttCamera->addChild(model.get());
	rttCamera->attach(osg::Camera::COLOR_BUFFER, image.get());

	osgViewer::Viewer viewer;

	//osg::ref_ptr<osg::Camera> camera = new osg::Camera();
	//camera->setGraphicsContext(gc.get());
	//camera->setViewport(0, 0, 512, 512);
	
	
	viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
	//viewer.setCamera(camera.get());

	viewer.addSlave(rttCamera);

	viewer.setSceneData(model.get());

	//osg::ref_ptr<osg::Group> root = new osg::Group();
	//root->addChild(rttCamera);
	//root->addChild(osgDB::readNodeFile("lz.osg"));

	//viewer.setSceneData();

	viewer.frame();
	osgDB::writeImageFile(*(image.get()),"rtt.bmp");

	//viewer.run();

	return 0;
}

#endif

int main(int argc, char* argv[])
{
	osg::Vec3  viewDirect(0.0, 0.0, 1.0);

	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(argv[1]);

	osg::ref_ptr<osg::Image> image = CompareMeshOutline::GetImageFromView(node, viewDirect, 1024, 1024);

	osgDB::writeImageFile(*(image.get()), std::string(argv[2]));

	return 0;
}

