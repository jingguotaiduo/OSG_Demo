#include <windows.h>
#include <direct.h>
#include <io.h>
#include <list>
#include <vector>
#include <iostream>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/GUIEventAdapter>
#include <osgViewer/ViewerEventHandlers>
#include "LonLatRad2Meter.h"
#include "ogrsf_frmts.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#include "json.hpp"
#include "tiny_gltf.h" //实测2.5版本可用，2.8版本不可用
#include "tinyxml2.h"

#include "earcut.hpp"



using namespace std;
using namespace tinygltf;
using namespace tinyxml2;

/*
//https://github.com/mapbox/earcut.hpp
//https://github.com/nothings/stb single-file public domain (or MIT licensed) libraries for C/C++ By JIAO Jingguo 2023.4.26
// dlib http://dlib.net/ By JIAO Jingguo 2023.4.26 many libs for C/C++ to invoke
//https://github.com/syoyo/tinygltf/blob/release/examples/gltfutil/main.cc By JIAO Jingguo 2023.4.26 命令行解析器
*/

double stringToDouble(string num) //字符串转小数的函数 2023.5.21
{
	bool minus = false;      //标记是否是负数  
	string real = num;       //real表示num的绝对值
	if (num.at(0) == '-')
	{
		minus = true;
		real = num.substr(1, num.size() - 1);
	}

	char c;
	unsigned int i = 0;
	double result = 0.0, dec = 10.0;
	bool isDec = false;       //标记是否有小数
	unsigned long size = real.size();
	while (i < size)
	{
		c = real.at(i);
		if (c == '.')
		{//包含小数
			isDec = true;
			i++;
			continue;
		}
		if (!isDec)
		{
			result = result * 10 + c - '0';
		}
		else
		{//识别小数点之后都进入这个分支
			result = result + (c - '0') / dec;
			dec *= 10;
		}
		i++;
	}

	if (minus == true) {
		result = -result;
	}

	return result;
}

bool isDirExist(string filename)
{
	//判断文件夹是否存在，不存在则创建 _access也可用来判断文件是否存在
	const char* dir = filename.c_str();

	if (_access(dir, 0) == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

long singleFolderfileSize = 0;
void getAllFiles(string path, vector<string>& files, string fileType)
{
	
	long long hFile = 0;//文件句柄
	struct _finddata_t fileinfo;//文件信息
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do{
			if ((fileinfo.attrib & _A_SUBDIR))
			{  //比较文件类型是否是文件夹
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					//if (singleFolderfileSize)
					//	cout << p.assign(path).append("\\").append(fileinfo.name)<<"目录下包含 "<<singleFolderfileSize << "个" + fileType + "文件！" << endl;
					//singleFolderfileSize = 0;
					//递归搜索
					getAllFiles(p.assign(path).append("\\").append(fileinfo.name), files, fileType);
				}
			}
			else {
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				/*singleFolderfileSize++;*/
			}
		} while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
		_findclose(hFile);
	}
}


string GetPathDir(string filePath)//得到文件路径的目录
{
	string dirPath = filePath;
	size_t p = filePath.find_last_of('\\');
	if (p != -1)
	{
		dirPath.erase(p);
	}
	return dirPath;
}


void CreateMultiLevel(string dir)//创建多级目录
{
	if (_access(dir.c_str(), 00) == 0)
	{
		return;
	}

	list <string> dirList;
	dirList.push_front(dir);

	string curDir = GetPathDir(dir);
	while (curDir != dir)
	{
		if (_access(curDir.c_str(), 00) == 0)
		{
			break;
		}

		dirList.push_front(curDir);

		dir = curDir;
		curDir = GetPathDir(dir);
	}

	for (auto it : dirList)
	{
		_mkdir(it.c_str());
	}
}

string& replace_all_distinct(string& str, const string& old_value, const string& new_value)
{
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else break;
	}
	return str;
}

void singleOSGB2singleB3DM(double center_x, double center_y, string inputOSGB, string outputB3DM) // By JIAO Jingguo 2023.5.22
{
	double rad_x = degree2rad(center_x);
	double rad_y = degree2rad(center_y);
	int max_lvl = 20;//100

}


// By JIAO Jingguo 2023.5.21 parse metadata.xml and get all files and create all same directories as origin
int main()
{
	std::cout << "This is JIAO Jingguo's OSG Demo Program(------2023.3.13)!" << std::endl;
	//std::vector<unsigned char> jpeg_buf;
	//jpeg_buf.reserve(512 * 512 * 3);
	//for(int i=0;i<jpeg_buf.size();i++)
	//	std::cout << jpeg_buf[i];
	//std::cout << std::endl;

	//double lon = 117.0, lat = 39.0, tile_w = 250, tile_h = 250, height_min = 0, height_max = 100, geometricError = 50;
	//double rad_lon = degree2rad(lon), rad_lat = degree2rad(lat);
	//string full_path = "D:\\Program Files (x86)\\Microsoft Visual Studio 2015\\myprojects\\OSG_Demo\\OSG_Demo\\tileset-First.json";
	//const char* filename = "./tile+01_05/01_05.b3dm";
	//write_tileset(rad_lon, rad_lat, tile_w, tile_h, height_min, height_max, geometricError, filename,full_path.data());


	//// By JIAO Jingguo 2023.4.26 测试写入gltf
	//// Create a model with a single mesh and save it as a gltf file
	//tinygltf::Model m;
	//tinygltf::Scene scene;
	//tinygltf::Mesh mesh;
	//tinygltf::Primitive primitive;
	//tinygltf::Node node;
	//tinygltf::Buffer buffer;
	//tinygltf::BufferView bufferView1;
	//tinygltf::BufferView bufferView2;
	//tinygltf::Accessor accessor1;
	//tinygltf::Accessor accessor2;
	//tinygltf::Asset asset;

	//// This is the raw data buffer. 
	//buffer.data = {
	//	// 6 bytes of indices and two bytes of padding
	//	0x00,0x00,0x01,0x00,0x02,0x00,0x00,0x00,
	//	// 36 bytes of floating point numbers
	//	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//	0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,
	//	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//	0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,
	//	0x00,0x00,0x00,0x00 };

	//// "The indices of the vertices (ELEMENT_ARRAY_BUFFER) take up 6 bytes in the
	//// start of the buffer.
	//bufferView1.buffer = 0;
	//bufferView1.byteOffset = 0;
	//bufferView1.byteLength = 6;
	//bufferView1.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

	//// The vertices take up 36 bytes (3 vertices * 3 floating points * 4 bytes)
	//// at position 8 in the buffer and are of type ARRAY_BUFFER
	//bufferView2.buffer = 0;
	//bufferView2.byteOffset = 8;
	//bufferView2.byteLength = 36;
	//bufferView2.target = TINYGLTF_TARGET_ARRAY_BUFFER;

	//// Describe the layout of bufferView1, the indices of the vertices
	//accessor1.bufferView = 0;
	//accessor1.byteOffset = 0;
	//accessor1.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
	//accessor1.count = 3;
	//accessor1.type = TINYGLTF_TYPE_SCALAR;
	//accessor1.maxValues.push_back(2);
	//accessor1.minValues.push_back(0);

	//// Describe the layout of bufferView2, the vertices themself
	//accessor2.bufferView = 1;
	//accessor2.byteOffset = 0;
	//accessor2.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	//accessor2.count = 3;
	//accessor2.type = TINYGLTF_TYPE_VEC3;
	//accessor2.maxValues = { 1.0, 1.0, 0.0 };
	//accessor2.minValues = { 0.0, 0.0, 0.0 };

	//// Build the mesh primitive and add it to the mesh
	//primitive.indices = 0;                 // The index of the accessor for the vertex indices
	//primitive.attributes["POSITION"] = 1;  // The index of the accessor for positions
	//primitive.material = 0;
	//primitive.mode = TINYGLTF_MODE_TRIANGLES;
	//mesh.primitives.push_back(primitive);

	//// Other tie ups
	//node.mesh = 0;
	//scene.nodes.push_back(0); // Default scene

	//						  // Define the asset. The version is required
	//asset.version = "2.0";
	//asset.generator = "tinygltf";

	//// Now all that remains is to tie back all the loose objects into the
	//// our single model.
	//m.scenes.push_back(scene);
	//m.meshes.push_back(mesh);
	//m.nodes.push_back(node);
	//m.buffers.push_back(buffer);
	//m.bufferViews.push_back(bufferView1);
	//m.bufferViews.push_back(bufferView2);
	//m.accessors.push_back(accessor1);
	//m.accessors.push_back(accessor2);
	//m.asset = asset;

	//// Create a simple material
	//tinygltf::Material mat;
	//mat.pbrMetallicRoughness.baseColorFactor = { 1.0f, 0.9f, 0.9f, 1.0f };
	//mat.doubleSided = true;
	//m.materials.push_back(mat);

	//// Save it to a file
	//tinygltf::TinyGLTF gltf;
	//gltf.WriteGltfSceneToFile(&m, "triangle.gltf",
	//	true, // embedImages
	//	true, // embedBuffers
	//	true, // pretty print
	//	false); // write binary
	//gltf.WriteGltfSceneToFile(&m, "triangle-bin.gltf",
	//	true, // embedImages
	//	true, // embedBuffers
	//	true, // pretty print
	//	true); // write binary
	//cout << "写入gltf文件成功！" << endl;

	cout << "开始解析metadata.xml文件！" << endl;
	tinyxml2::XMLDocument xml;

	string mXMLFileName = "E:\\KY_work\\Production_3\\metadata.xml";//"metadata2.xml"
	xml.LoadFile(mXMLFileName.c_str());

	XMLElement *modelMetadata = xml.RootElement();
	XMLElement *SRS = modelMetadata->FirstChildElement("SRS");
	XMLElement *SRSOrigin = modelMetadata->FirstChildElement("SRSOrigin");
	const char* SRS_value = SRS->GetText();
	const char* SRSOrigin_value = SRSOrigin->GetText();

	/* 需要读取metadata.xml文件获取模型基点的经纬度坐标，如果带EPSG则需要根据SRS里的空间参考和SRSOrigin里的三维坐标来进行坐标转换来拿到经纬度*/
	ModelMetadata mmd;
	mmd.SRS = SRS_value;
	mmd.SRSOrigin = SRSOrigin_value;
	cout << "空间参考："<<SRS_value << endl;
	cout <<"空间参考原点："<< SRSOrigin_value << endl;

	string srs = mmd.SRS, ENU = "ENU";//可能存在 wkt 或 EPSG
	
	if (srs.length() >=3 && srs.substr(0, 3) == ENU)
	{
		cout << "The metadata.xml 文件的空间参考是北偏东坐标系！" << endl;
		string lonlatStr = srs.substr(4, srs.length());
		double lon = 0;
		double lat = 0;
		int index = -1;
		for (int i = 0; i < lonlatStr.length(); i++)
		{
			if (lonlatStr[i] == ',')
			{
				index = i; break;
			}
		}
		if (index != -1)
		{
			string lonStr = lonlatStr.substr(0, index), latStr = lonlatStr.substr(index + 1, lonlatStr.length());
			/*cout << lonStr << endl;
			cout << latStr << endl;
			lon = stringToDouble(lonStr);
			lat = stringToDouble(latStr);

			cout << lon << endl;
			cout << lat << endl;*/
			lat = stof(lonlatStr.substr(0, index).c_str());
			lon = stof(lonlatStr.substr(index + 1, lonlatStr.length()).c_str());
		}
		cout << "经度为：" << lon << endl;
		cout << "纬度为：" << lat << endl;
	}


	string::size_type iPos = (mXMLFileName.find_last_of('\\') + 1) == 0 ? mXMLFileName.find_last_of('/') + 1 : mXMLFileName.find_last_of('\\') + 1;
	string fileName = mXMLFileName.substr(iPos, mXMLFileName.length() - iPos);//获取带后缀的文件名
	string filePath = mXMLFileName.substr(0, iPos);//获取文件路径
	string fileName_noSuffix = fileName.substr(0, fileName.rfind("."));//获取不带后缀的文件名
	string fileName_withSuffix = fileName.substr(fileName.rfind("."), fileName.length());//获取后缀名
	//cout << "获取带后缀的文件名为：" << fileName << endl \
	//	 << "获取文件路径为：" << filePath << endl \
	//	 << "不带后缀的文件名为：" << fileName_noSuffix << endl \
	//	 << "后缀名为：" << fileName_withSuffix << endl;

	string dataDirectory = filePath + "Data", fileType = ".osgb", newFileType = ".b3dm";
	string newDir = "E:\\KY_work\\Production_3-jjgConvert3dtiles";
	cout << "OSG数据目录为：" << dataDirectory << endl;
	vector<string> osgbfiles;
	if (isDirExist(dataDirectory))
	{
		cout << "OSG数据目录" << dataDirectory <<"存在！"<< endl;
		vector<string> temp;
		getAllFiles(dataDirectory, temp, fileType);

		for (int i = 0; i < temp.size();++i	)
		{
			string tempStr = temp[i];
			string replaceNewDataFileName = replace_all_distinct(temp[i], dataDirectory, newDir + "\\Data");
			//cout << replaceNewDataFileName << endl;

			string::size_type dotPos = replaceNewDataFileName.find_last_of('.') + 1;
			if (dotPos && replaceNewDataFileName.substr(dotPos - 1, replaceNewDataFileName.length() - fileType.length()) == fileType) //如果后缀名为.osgb
			{
				osgbfiles.push_back(tempStr);
				string newFilename = replace_all_distinct(replaceNewDataFileName, fileType, newFileType);
				cout << tempStr << " ----> " <<newFilename << endl;
			}
			else //如果是文件夹，则创建新目录
			{
				CreateMultiLevel(replaceNewDataFileName);
			}
		}
	}
	else
	{
		cout << "OSG数据目录" << dataDirectory << "不存在！" << endl;
	}
	
	

	//system("pause");
	//return 0;

	//cout << "Start to Read Shapefile!" << std::endl;
	//GDALAllRegister();
	//GDALDataset   *poDS;
	//CPLSetConfigOption("SHAPE_ENCODING", "");  //解决中文乱码问题
	//										   //读取shp文件
	//										   //注：GDAL不能读取中文路径，所以记得要换成英文路径
	//poDS = (GDALDataset*)GDALOpenEx("E:/jing_zhong/OSG_Demo/x64/Release/world.shp", GDAL_OF_VECTOR, NULL, NULL, NULL);

	//if (poDS == NULL)
	//{
	//	printf("Open failed.\n%s");
	//	return 0;
	//}

	//OGRLayer  *poLayer;
	//poLayer = poDS->GetLayer(0); //读取层
	//OGRFeature *poFeature;

	//poLayer->ResetReading();
	//int i = 0;
	//while ((poFeature = poLayer->GetNextFeature()) != NULL)
	//{
	//	i = i++;
	//	cout << i << "  ";
	//	OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
	//	int iField;
	//	int n = poFDefn->GetFieldCount(); //获得字段的数目，不包括前两个字段（FID,Shape);
	//	for (iField = 0; iField <n; iField++)
	//	{
	//		//输出每个字段的值
	//		cout << poFeature->GetFieldAsString(iField) << "    ";
	//	}
	//	cout << endl;
	//	OGRFeature::DestroyFeature(poFeature);
	//}
	//GDALClose(poDS);

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
	std::string filenameStr = "E:\\KY_work\\Production_3\\Data\\Tile_+000_+000\\Tile_+000_+000.osgb";//"E:\\科研_work\\tile_32_25\\Data\\Model\\tile_0_0_0_tex_children.osgb"
	filenameStr = "./cow.osg";
	view.setSceneData(osgDB::readNodeFile(filenameStr));

	//vector<string> tempData;
	//string tempDataDirectory = "E:\\科研_work\\tile_32_25\\Data\\Model";// "E:\\KY_work\\Production_3\\Data\\Tile_+000_+000";
	//getAllFiles(tempDataDirectory, tempData, fileType);
	//
	//if (osgbfiles.size() > 0)
	//{
	//	cout << osgbfiles.size() << endl;
	//	osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(osgbfiles);
	//	view.setSceneData(root);
	//}
	
	return view.run();
}