#include <iomanip>

#include "CompareMeshOutline.h"

CompareMeshOutline::CompareMeshOutline(unsigned int size)
{
	m_imageWidth = size;
	m_imageHeight = size;
}


CompareMeshOutline::~CompareMeshOutline()
{
}

bool CompareMeshOutline::CompareMesh(osg::Node* baseMesh, osg::Node* simMesh, const std::vector<osg::Vec3d>& viewDirects)
{
	if (baseMesh == nullptr || simMesh == nullptr)
	{
		return false;
	}

	osg::ref_ptr<osg::Switch> root = new osg::Switch();
	root->addChild(baseMesh, true);
	root->addChild(simMesh, true);
	root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	const unsigned int width = m_imageWidth;
	const unsigned int height = m_imageHeight;
	osg::BoundingSphere bs = root->getBound();
	m_gsd = (2.0 * bs.radius()) / m_imageWidth;
	m_gsd = std::pow(m_gsd, 2);
	m_diffPixCount = 0.0;

	osg::ref_ptr<osg::Camera> rttCamera = CreateRttCamera(width, height, bs);

	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->windowDecoration = true;
	traits->pbuffer = true;
	traits->doubleBuffer = true;
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	rttCamera->setGraphicsContext(gc.get());

	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(width, height, 1, GL_RGB, GL_UNSIGNED_BYTE);
	rttCamera->attach(osg::Camera::COLOR_BUFFER, image.get(), 16);

	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer();
	viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
	viewer->addSlave(rttCamera.get());
	viewer->setSceneData(root.get());

	bool isShowBase = true;
	for (std::size_t i = 0; i < viewDirects.size(); i++)
	{
		osg::Vec3d viewDirect = viewDirects[i];
		osg::Vec3d upDirect = CreateUpDirectFromViewDirect(viewDirect);
		osg::Vec3d center = bs.center();
		osg::Vec3d eyePoint = center + viewDirect * center.length() * 2.0;
		rttCamera->setViewMatrixAsLookAt(eyePoint, center, upDirect);

		std::vector<cv::Mat> viewPairImage;
		for (std::size_t j = 0; j < 2; j++)
		{
			root->setChildValue(baseMesh, isShowBase);
			root->setChildValue(simMesh, !isShowBase);
			viewer->frame();
			isShowBase = !isShowBase;

			osg::ref_ptr<osg::Image> tempImage = dynamic_cast<osg::Image*>(image->clone(osg::CopyOp::DEEP_COPY_ALL));
			cv::Mat tempMat = ConvOsgImage2CvMat(tempImage.get());
			viewPairImage.push_back(tempMat.clone());

		}

		m_imageSequence.push_back(std::make_pair(viewPairImage[0], viewPairImage[1]));


		cv::Mat grayBaseImg = ConvBGRImage2GrayImage(viewPairImage[0]);
		cv::Mat graySimImg = ConvBGRImage2GrayImage(viewPairImage[1]);

		//std::string outputPath0 = std::string("F:\\dev_data\\horse_uv") + std::string("/") +
		//	std::string("horse_sim_comp") + std::string("_") + std::to_string(i) +
		//	std::to_string(0) + std::string(".bmp");
		//cv::imwrite(outputPath0, grayBaseImg);

		//std::string outputPath1 = std::string("F:\\dev_data\\horse_uv") + std::string("/") +
		//	std::string("horse_sim_comp") + std::string("_") + std::to_string(i) +
		//	std::to_string(1) + std::string(".bmp");
		//cv::imwrite(outputPath1, graySimImg);

		cv::Mat compImage = ImageSubtract(grayBaseImg, graySimImg);

		m_imageCompared.push_back(compImage);

		std::cout << "\rWorking on " << std::setw(6) << std::setprecision(6) <<double(i + 1) / double(viewDirects.size()) * 100 << "%" << std::flush;
	}
	std::cout << std::endl;
	
	return true;

}

void CompareMeshOutline::ExportCompareMehsImage(const std::string& path, const std::string& name)
{

	for (std::size_t i = 0; i < m_imageCompared.size(); i++)
	{
		std::string outputPath = path + std::string("/") + name + std::string("_") + std::to_string(i) + std::string(".bmp");
		cv::imwrite(outputPath, m_imageCompared[i]);
	}
}

double CompareMeshOutline::GetDiffArea()
{
	return m_diffPixCount;
}

cv::Mat CompareMeshOutline::ImageSubtract(const cv::Mat& baseMat, const cv::Mat& simMat)
{
	int baseWidth = baseMat.cols;
	int baseHeight = baseMat.rows;
	int baseChannel = baseMat.channels();
	if (baseWidth != simMat.cols || baseHeight != simMat.rows ||
		baseChannel != 1 || baseChannel != simMat.channels())
	{
		return cv::Mat(0, 0, CV_8UC3);
	}

	cv::Mat diffMat(baseHeight, baseWidth, CV_8UC3);

	for (int i = 0; i < baseHeight; i++)
	{
		for (int j = 0; j < baseWidth; j++)
		{
			float basePixValue = float(baseMat.at<uchar>(i, j)) / 255.0f;
			float simPixValue = float(simMat.at<uchar>(i, j)) / 255.0f;
			if (basePixValue - simPixValue > 0)
			{
				diffMat.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 255);
				m_diffPixCount += m_gsd;
			}
			else if (basePixValue - simPixValue < 0)
			{
				diffMat.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 255, 0);
				m_diffPixCount += m_gsd;
			}
			else
			{
				diffMat.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0);
			}
			
		}
	}

	return diffMat;


}

cv::Mat CompareMeshOutline::ConvBGRImage2GrayImage(const cv::Mat& bgrimage)
{
	int imageWidth = bgrimage.cols;
	int imageHeight = bgrimage.rows;

	int imageChannel = bgrimage.channels();

	cv::Mat grayImage(imageHeight, imageWidth, CV_8U);

	for (int i = 0; i < imageHeight; i++)
	{
		for (int j = 0; j < imageWidth; j++)
		{
			uchar b = bgrimage.at<cv::Vec3b>(i, j)[0];
			uchar g = bgrimage.at<cv::Vec3b>(i, j)[1];
			uchar r = bgrimage.at<cv::Vec3b>(i, j)[2];
			//std::cout << bgrimage.at<cv::Vec3b>(i, j) << std::endl;
			if (b == 0 && g == 0 && r == 0)
			{
				grayImage.at<uchar>(i, j) = 0;
			}
			else
			{
				grayImage.at<uchar>(i, j) = 255;
			}
		}
	}

	return grayImage;
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
	double top = bs.radius();
	double right = bs.radius();
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

std::vector<osg::Vec3d> CompareMeshOutline::CreateViewDirect(unsigned int numDirect)
{
	std::vector<osg::Vec3d> result;
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

			osg::Vec3d tempDirection(coordX, coordY, coordZ);
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


osg::Vec3d CompareMeshOutline::CreateUpDirectFromViewDirect(const osg::Vec3d& viewDirect)
{
	osg::Vec3d nz(0.0, 0.0, 1.0);

	osg::Vec3d nx = nz ^ viewDirect;

	if (nx.length2() < 1e-6)
	{
		nx = osg::Vec3d(1.0, 0.0, 0.0);
	}

	osg::Vec3d upDirect = viewDirect ^ nx;

	return upDirect;
}

std::vector<osg::ref_ptr<osg::Image>>
CompareMeshOutline::GetImageFromView(osg::Node* node, std::vector<osg::Vec3d> viewDirects, unsigned int width, unsigned int height)
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
		osg::Vec3d viewDirect = viewDirects[i];
		osg::Vec3d upDirect = CreateUpDirectFromViewDirect(viewDirect);

		osg::Vec3d center = bs.center();
		osg::Vec3d eyePoint = center + viewDirect * center.length() * 2.0;
		rttCamera->setViewMatrixAsLookAt(eyePoint, center, upDirect);

		viewer->frame();

		osg::ref_ptr<osg::Image> tempImage = dynamic_cast<osg::Image*>(image->clone(osg::CopyOp::DEEP_COPY_ALL));
		result.push_back(tempImage);
	}

	return result;

}

osg::Image* CompareMeshOutline::GetImageFromView(osg::Node* node, osg::Vec3d viewDirect, unsigned int width, unsigned int height)
{
	std::vector<osg::Vec3d> viewDirects;
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