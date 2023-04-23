#include <windows.h>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/GUIEventAdapter>
#include <osgViewer/ViewerEventHandlers>
#include "LonLatRad2Meter.h"
using namespace std;

int main()
{
	std::cout << "This is JIAO Jingguo's OSG Demo Program(------2023.3.13)!" << std::endl;
	//std::vector<unsigned char> jpeg_buf;
	//jpeg_buf.reserve(512 * 512 * 3);
	//for(int i=0;i<jpeg_buf.size();i++)
	//	std::cout << jpeg_buf[i];
	//std::cout << std::endl;
	double lon = 117.0, lat = 39.0, tile_w = 250, tile_h = 250, height_min = 0, height_max = 100, geometricError = 50;
	double rad_lon = degree2rad(lon), rad_lat = degree2rad(lat);
	string full_path = "D:\\Program Files (x86)\\Microsoft Visual Studio 2015\\myprojects\\OSG_Demo\\OSG_Demo\\tileset-First.json";
	const char* filename = "./tile+01_05/01_05.b3dm";
	write_tileset(rad_lon, rad_lat, tile_w, tile_h, height_min, height_max, geometricError, filename,full_path.data());

	osgViewer::Viewer view;
	view.addEventHandler(new osgViewer::ScreenCaptureHandler);//截图  快捷键 c
	view.addEventHandler(new osgViewer::WindowSizeHandler);//全屏  快捷键f
	//方法一
	view.addEventHandler(new osgViewer::StatsHandler);//查看帧数 s
	//方法二
	//osgViewer::StatsHandler* pStatsHandler = new osgViewer::StatsHandler;
	//pStatsHandler->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F11);
	//view.addEventHandler(pStatsHandler);
	//view.setSceneData(osgDB::readNodeFile("./cow.osg"));
	std::string filenameStr = "E:\\科研_work\\Production_3\\Data\\Tile_+000_+000\\Tile_+000_+000.osgb";//"E:\\科研_work\\tile_32_25\\Data\\Model\\tile_0_0_0_tex_children.osgb"
	view.setSceneData(osgDB::readNodeFile(filenameStr));
	return view.run();
}