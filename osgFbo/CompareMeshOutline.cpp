#include "CompareMeshOutline.h"



CompareMeshOutline::CompareMeshOutline()
{
}


CompareMeshOutline::~CompareMeshOutline()
{
}

osg::Camera*
CompareMeshOutline::CreateRttCamera(unsigned int texWidth, unsigned int texHeight, const osg::BoundingSphere& bs, bool useortho)
{
	osg::ref_ptr<osg::Camera> rttCamera = new osg::Camera();
	rttCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	rttCamera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	rttCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	rttCamera->setViewport(0, 0, texWidth, texHeight);
	rttCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	rttCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);


	double viewDistance = bs.radius();
	double znear = viewDistance;
	double zfar = viewDistance * 2.0;
	float top = bs.radius();
	float right = bs.radius();
	if (useortho)
	{
		rttCamera->setProjectionMatrixAsOrtho(-right, right, -top, top, znear, zfar);
	}
	else
	{
		rttCamera->setProjectionMatrixAsFrustum(-right, right, -top, top, znear, zfar);
	}

	//osg::Vec3d upDirection(0.0, 1.0, 0.0);
	//osg::Vec3d viewDirection(0.0, 1.0, 1.0);
	//osg::Vec3d center = bs.center();
	//osg::Vec3d eyePoint = center + viewDirection * viewDistance;
	//rttCamera->setViewMatrixAsLookAt(eyePoint, center, upDirection);

	return rttCamera.release();

}

std::vector<osg::Vec3> CompareMeshOutline::CreateViewDirect(unsigned int numDirect)
{
	std::vector<osg::Vec3> result;
	double eps = 1e-6;

	double detAngle = 2.0 * osg::PI / numDirect;
	for (unsigned int i = 0; i < numDirect; i++)
	{
		double coordZ = std::sin(detAngle * i);
		double coordXY = std::cos(detAngle * i);
		if (abs(coordZ) < eps) coordZ = 0.0;
		for (unsigned int j = 0; j < numDirect; j++)
		{
			double coordX = coordXY * std::cos(detAngle * j);
			double coordY = coordXY * std::sin(detAngle * j);
			if (abs(coordX) < eps) coordX = 0.0;
			if (abs(coordY) < eps) coordY = 0.0;

			osg::Vec3 tempDirection(coordX, coordY, coordZ);
			bool isInsert = true;
			for (unsigned int k = 0; k < result.size(); k++)
			{
				if (fabs(result[k][0] - tempDirection[0]) < eps &&
					fabs(result[k][1] - tempDirection[1]) < eps &&
					fabs(result[k][2] - tempDirection[2]) < eps)
				{
					isInsert = false;
					break;
				}
			}
			if (isInsert)
			{
				result.push_back(tempDirection);
			}
		}
	}

	return result;
}


osg::Vec3 CompareMeshOutline::CreateUpDirectFromViewDirect(const osg::Vec3& viewDirect)
{
	osg::Vec3 nz(0.0, 0.0, 1.0);

	osg::Vec3 nx = nz ^ viewDirect;

	if (nx.length2() < 1e-6)
	{
		nx = osg::Vec3(1.0, 0.0, 0.0);
	}

	osg::Vec3 upDirect = viewDirect ^ nx;

	return upDirect;
}

std::vector<osg::ref_ptr<osg::Image>>
CompareMeshOutline::GetImageFromView(osg::Node* node, std::vector<osg::Vec3> viewDirects, unsigned int width, unsigned int height)
{
	std::vector<osg::ref_ptr<osg::Image>> result;

	if (node == nullptr) return result;
	node->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	osg::BoundingSphere bs = node->getBound();
	osg::ref_ptr<osg::Camera> rttCamera = CreateRttCamera(width, height, bs);

	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->windowDecoration = true;
	traits->pbuffer = true;
	traits->doubleBuffer = true;
	//traits->samples = 16;
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	rttCamera->setGraphicsContext(gc.get());

	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(width, height, 1, GL_RGB, GL_UNSIGNED_BYTE);
	rttCamera->attach(osg::Camera::COLOR_BUFFER, image.get(), 16);

	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer();
	viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
	//osg::ref_ptr< osg::DisplaySettings > displaySettings = new osg::DisplaySettings;
	//displaySettings->setNumMultiSamples(8);
	//viewer->setDisplaySettings(displaySettings.get());
	viewer->addSlave(rttCamera.get());
	viewer->setSceneData(node);

	for (std::size_t i = 0; i < viewDirects.size(); i++)
	{
		osg::Vec3 viewDirect = viewDirects[i];
		osg::Vec3 upDirect = CreateUpDirectFromViewDirect(viewDirect);

		osg::Vec3 center = bs.center();
		osg::Vec3 eyePoint = center + viewDirect * center.length() * 2.0;
		rttCamera->setViewMatrixAsLookAt(eyePoint, center, upDirect);

		viewer->frame();

		osg::ref_ptr<osg::Image> tempImage = dynamic_cast<osg::Image*>(image->clone(osg::CopyOp::DEEP_COPY_ALL));
		result.push_back(tempImage);
	}

	return result;

}

osg::Image* CompareMeshOutline::GetImageFromView(osg::Node* node, osg::Vec3 viewDirect, unsigned int width, unsigned int height)
{
	std::vector<osg::Vec3> viewDirects;
	viewDirects.push_back(viewDirect);

	std::vector<osg::ref_ptr<osg::Image>> result = GetImageFromView(node, viewDirects, width, height);

	return result[0].release();
}

cv::Mat CompareMeshOutline::ConvOsgImage2CvMat(const osg::Image* osgimage)
{
	if (osgimage == nullptr)
	{
		return cv::Mat(0, 0, CV_8UC3);
	}

	cv::Mat result(osgimage->t(), osgimage->s(), CV_8UC3);
	result.data = (uchar *)osgimage->data();
	cv::flip(result, result, 0); 
	cv::cvtColor(result, result, cv::COLOR_RGB2BGR);

	return result;
}

//osg::Image* CompareMeshOutline::ConvCvMat2OsgImage(const cv::Mat& cvmat)
//{
//	return nullptr;
//}