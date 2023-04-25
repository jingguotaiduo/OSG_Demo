#include <windows.h>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/GUIEventAdapter>
#include <osgViewer/ViewerEventHandlers>
#include "LonLatRad2Meter.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#include "json.hpp"
#include "tiny_gltf.h" //实测2.5版本可用，2.8版本不可用

using namespace std;
using namespace tinygltf;


//https://github.com/nothings/stb single-file public domain (or MIT licensed) libraries for C/C++ By JIAO Jingguo 2023.4.26
// dlib http://dlib.net/ By JIOA Jingguo 2023.4.26 many libs for C/C++ to invoke
//https://github.com/syoyo/tinygltf/blob/release/examples/gltfutil/main.cc By JIAO Jingguo 2023.4.26 命令行解析器
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


	// By JIAO Jingguo 2023.4.26 测试写入gltf
	// Create a model with a single mesh and save it as a gltf file
	tinygltf::Model m;
	tinygltf::Scene scene;
	tinygltf::Mesh mesh;
	tinygltf::Primitive primitive;
	tinygltf::Node node;
	tinygltf::Buffer buffer;
	tinygltf::BufferView bufferView1;
	tinygltf::BufferView bufferView2;
	tinygltf::Accessor accessor1;
	tinygltf::Accessor accessor2;
	tinygltf::Asset asset;

	// This is the raw data buffer. 
	buffer.data = {
		// 6 bytes of indices and two bytes of padding
		0x00,0x00,0x01,0x00,0x02,0x00,0x00,0x00,
		// 36 bytes of floating point numbers
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,
		0x00,0x00,0x00,0x00 };

	// "The indices of the vertices (ELEMENT_ARRAY_BUFFER) take up 6 bytes in the
	// start of the buffer.
	bufferView1.buffer = 0;
	bufferView1.byteOffset = 0;
	bufferView1.byteLength = 6;
	bufferView1.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

	// The vertices take up 36 bytes (3 vertices * 3 floating points * 4 bytes)
	// at position 8 in the buffer and are of type ARRAY_BUFFER
	bufferView2.buffer = 0;
	bufferView2.byteOffset = 8;
	bufferView2.byteLength = 36;
	bufferView2.target = TINYGLTF_TARGET_ARRAY_BUFFER;

	// Describe the layout of bufferView1, the indices of the vertices
	accessor1.bufferView = 0;
	accessor1.byteOffset = 0;
	accessor1.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
	accessor1.count = 3;
	accessor1.type = TINYGLTF_TYPE_SCALAR;
	accessor1.maxValues.push_back(2);
	accessor1.minValues.push_back(0);

	// Describe the layout of bufferView2, the vertices themself
	accessor2.bufferView = 1;
	accessor2.byteOffset = 0;
	accessor2.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accessor2.count = 3;
	accessor2.type = TINYGLTF_TYPE_VEC3;
	accessor2.maxValues = { 1.0, 1.0, 0.0 };
	accessor2.minValues = { 0.0, 0.0, 0.0 };

	// Build the mesh primitive and add it to the mesh
	primitive.indices = 0;                 // The index of the accessor for the vertex indices
	primitive.attributes["POSITION"] = 1;  // The index of the accessor for positions
	primitive.material = 0;
	primitive.mode = TINYGLTF_MODE_TRIANGLES;
	mesh.primitives.push_back(primitive);

	// Other tie ups
	node.mesh = 0;
	scene.nodes.push_back(0); // Default scene

							  // Define the asset. The version is required
	asset.version = "2.0";
	asset.generator = "tinygltf";

	// Now all that remains is to tie back all the loose objects into the
	// our single model.
	m.scenes.push_back(scene);
	m.meshes.push_back(mesh);
	m.nodes.push_back(node);
	m.buffers.push_back(buffer);
	m.bufferViews.push_back(bufferView1);
	m.bufferViews.push_back(bufferView2);
	m.accessors.push_back(accessor1);
	m.accessors.push_back(accessor2);
	m.asset = asset;

	// Create a simple material
	tinygltf::Material mat;
	mat.pbrMetallicRoughness.baseColorFactor = { 1.0f, 0.9f, 0.9f, 1.0f };
	mat.doubleSided = true;
	m.materials.push_back(mat);

	// Save it to a file
	tinygltf::TinyGLTF gltf;
	gltf.WriteGltfSceneToFile(&m, "triangle.gltf",
		true, // embedImages
		true, // embedBuffers
		true, // pretty print
		false); // write binary
	cout << "写入gltf文件成功！" << endl;

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