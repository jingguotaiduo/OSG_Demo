﻿#include <iostream>
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <string>
#include <list>
#include <vector>
#include <time.h>   // or  #include <ctime>
#include "jjg.h"
#include "dxt_img.h"
using namespace std;

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Material>
#include <osg/PagedLOD>
#include <osgDB/ConvertUTF>
#include <osgUtil/DelaunayTriangulator>
#include <osgUtil/Tessellator>
#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/GUIEventAdapter>
#include <osgViewer/ViewerEventHandlers>
#include "ogrsf_frmts.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#include "json.hpp"
#include "tiny_gltf.h" //实测2.5版本可用，2.8版本不可用(已修改内部代码，如Material添加属性成员）
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

struct TileResult {
	json selfjson;
	string path;
	std::vector<double> box_v;
};

struct OsgbInfo {
	string in_dir;
	string out_dir;
	TileResult sender;
};

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
		do {
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

string &replace_all_distinct(string& str, const string& old_value, const string& new_value)
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

struct osg_tree {
	TileBox bbox;
	double geometricError;
	std::string file_name;
	std::vector<osg_tree> sub_nodes;
};

class InfoVisitor : public osg::NodeVisitor
{
	std::string path;
public:
	InfoVisitor(std::string _path)
		:osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
		, path(_path)
	{}

	~InfoVisitor() {
	}

	void apply(osg::Geometry& geometry) {
		geometry_array.push_back(&geometry);
		if (auto ss = geometry.getStateSet()) {
			osg::Texture* tex = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
			if (tex) {
				texture_array.insert(tex);
				texture_map[&geometry] = tex;
			}
		}
	}

	void apply(osg::PagedLOD& node) {
		//std::string path = node.getDatabasePath();
		int n = node.getNumFileNames();
		for (size_t i = 1; i < n; i++)
		{
			std::string file_name = path + "/" + node.getFileName(i);
			sub_node_names.push_back(file_name);
		}
		traverse(node);
	}

public:
	std::vector<osg::Geometry*> geometry_array;
	std::set<osg::Texture*> texture_array;
	std::map<osg::Geometry*, osg::Texture*> texture_map;
	std::vector<std::string> sub_node_names;
};

std::string get_file_name(std::string path) {
	auto p0 = path.find_last_of("/\\");
	return path.substr(p0 + 1);
}

std::string replace(std::string str, std::string s0, std::string s1) {
	auto p0 = str.find(s0);
	return str.replace(p0, p0 + s0.length() - 1, s1);
}

std::string get_parent(std::string str) {
	auto p0 = str.find_last_of("/\\");
	if (p0 != std::string::npos)
		return str.substr(0, p0);
	else
		return "";
}

osg_tree get_all_tree(std::string& file_name) {
	osg_tree root_tile;
	vector<string> fileNames = { file_name };

	InfoVisitor infoVisitor(get_parent(file_name));
	{   // add block to release Node
		osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(fileNames);
		if (!root) {
			std::string name = (file_name.c_str());
			LOG_E("read node files [%s] fail!", name.c_str());
			return root_tile;
		}
		root_tile.file_name = file_name;
		root->accept(infoVisitor);
	}

	for (auto& i : infoVisitor.sub_node_names) {
		osg_tree tree = get_all_tree(i);
		if (!tree.file_name.empty()) {
			root_tile.sub_nodes.push_back(tree);
		}
	}
	return root_tile;
}

double get_geometric_error(TileBox& bbox) {
	if (bbox.max.empty() || bbox.min.empty())
	{
		LOG_E("bbox is empty!");
		return 0;
	}

	double max_err = (bbox.max[0] - bbox.min[0]) > (bbox.max[1] - bbox.min[1]) ? (bbox.max[0] - bbox.min[0]): (bbox.max[1] - bbox.min[1]);
	max_err = max_err > (bbox.max[2] - bbox.min[2]) ? max_err: (bbox.max[2] - bbox.min[2]);
	return max_err / 20.0;
	//     const double pi = std::acos(-1);
	//     double round = 2 * pi * 6378137.0 / 128.0;
	//     return round / std::pow(2.0, lvl );
}

void calc_geometric_error(osg_tree& tree) {
	const double EPS = 1e-12;
	// depth first
	for (auto& i : tree.sub_nodes) {
		calc_geometric_error(i);
	}
	if (tree.sub_nodes.empty()) {
		tree.geometricError = 0.0;
	}
	else {
		bool has = false;
		osg_tree leaf;
		for (auto& i : tree.sub_nodes) {
			if (abs(i.geometricError) > EPS)
			{
				has = true;
				leaf = i;
			}
		}

		if (has == false)
			tree.geometricError = get_geometric_error(tree.bbox);
		else
			tree.geometricError = leaf.geometricError * 2.0;
	}
}

std::vector<double> convert_bbox(TileBox tile) {
	double center_mx = (tile.max[0] + tile.min[0]) / 2;
	double center_my = (tile.max[1] + tile.min[1]) / 2;
	double center_mz = (tile.max[2] + tile.min[2]) / 2;
	double x_meter = (tile.max[0] - tile.min[0]) * 1;
	double y_meter = (tile.max[1] - tile.min[1]) * 1;
	double z_meter = (tile.max[2] - tile.min[2]) * 1;
	if (x_meter < 0.01) { x_meter = 0.01; }
	if (y_meter < 0.01) { y_meter = 0.01; }
	if (z_meter < 0.01) { z_meter = 0.01; }
	std::vector<double> v = {
		center_mx,center_my,center_mz,
		x_meter / 2, 0, 0,
		0, y_meter / 2, 0,
		0, 0, z_meter / 2
	};
	return v;
}

int get_lvl_num(std::string file_name) {
	std::string stem = get_file_name(file_name);
	auto p0 = stem.find("_L");
	auto p1 = stem.find("_", p0 + 2);
	if (p0 != std::string::npos && p1 != std::string::npos) {
		std::string substr = stem.substr(p0 + 2, p1 - p0 - 2);
		try { return std::stol(substr); }
		catch (...) {
			return -1;
		}
	}
	else if (p0 != std::string::npos) {
		int end = p0 + 2;
		while (true) {
			if (isdigit(stem[end]))
				end++;
			else
				break;
		}
		std::string substr = stem.substr(p0 + 2, end - p0 - 2);
		try { return std::stol(substr); }
		catch (...) {
			return -1;
		}
	}
	return -1;
}

template<class T>
void put_val(std::vector<unsigned char>& buf, T val) {
	buf.insert(buf.end(), (unsigned char*)&val, (unsigned char*)&val + sizeof(T));
}

template<class T>
void put_val(std::string& buf, T val) {
	buf.append((unsigned char*)&val, (unsigned char*)&val + sizeof(T));
}

void write_buf(void* context, void* data, int len) {
	std::vector<char> *buf = (std::vector<char>*)context;
	buf->insert(buf->end(), (char*)data, (char*)data + len);
}

struct MeshInfo
{
	string name;
	std::vector<double> min;
	std::vector<double> max;
};

template<class T>
void alignment_buffer(std::vector<T>& buf) {
	while (buf.size() % 4 != 0) {
		buf.push_back(0x00);
	}
}

struct OsgBuildState
{
	tinygltf::Buffer* buffer;
	tinygltf::Model* model;
	osg::Vec3f point_max;
	osg::Vec3f point_min;
	int draw_array_first;
	int draw_array_count;
};

void expand_bbox3d(osg::Vec3f& point_max, osg::Vec3f& point_min, osg::Vec3f point)
{
	point_max.x() = point.x() > point_max.x() ? point.x() : point_max.x();
	point_min.x() = point.x() < point_min.x()? point.x() : point_min.x();
	point_max.y() = point.y() > point_max.y()? point.y() : point_max.y();
	point_min.y() = point.y() < point_min.y()? point.y() : point_min.y();
	point_max.z() = point.z() > point_max.z()? point.z() : point_max.z();
	point_min.z() = point.z() < point_min.z()? point.z() : point_min.z();
}

void expand_bbox2d(osg::Vec2f& point_max, osg::Vec2f& point_min, osg::Vec2f point)
{
	point_max.x() = point.x() > point_max.x() ? point.x() : point_max.x();
	point_min.x() = point.x() < point_min.x() ? point.x() : point_min.x();
	point_max.y() = point.y() >  point_max.y() ? point.y() :  point_max.y();
	point_min.y() = point.y() < point_min.y() ? point.y() : point_min.y();
}

template<class T> void
write_osg_indecis(T* drawElements, OsgBuildState* osgState, int componentType)
{
	unsigned max_index = 0;
	unsigned min_index = 1 << 30;
	unsigned buffer_start = osgState->buffer->data.size();

	unsigned IndNum = drawElements->getNumIndices();
	for (unsigned m = 0; m < IndNum; m++)
	{
		auto idx = drawElements->at(m);
		put_val(osgState->buffer->data, idx);
		if (idx > max_index) max_index = idx;
		if (idx < min_index) min_index = idx;
	}
	alignment_buffer(osgState->buffer->data);

	tinygltf::Accessor acc;
	acc.bufferView = osgState->model->bufferViews.size();
	acc.type = TINYGLTF_TYPE_SCALAR;
	acc.componentType = componentType;
	acc.count = IndNum;
	acc.maxValues = { (double)max_index };
	acc.minValues = { (double)min_index };
	osgState->model->accessors.push_back(acc);

	tinygltf::BufferView bfv;
	bfv.buffer = 0;
	bfv.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
	bfv.byteOffset = buffer_start;
	bfv.byteLength = osgState->buffer->data.size() - buffer_start;
	osgState->model->bufferViews.push_back(bfv);
}

void
write_vec3_array(osg::Vec3Array* v3f, OsgBuildState* osgState, osg::Vec3f& point_max, osg::Vec3f& point_min)
{
	int vec_start = 0;
	int vec_end = v3f->size();
	if (osgState->draw_array_first >= 0)
	{
		vec_start = osgState->draw_array_first;
		vec_end = osgState->draw_array_count + vec_start;
	}
	unsigned buffer_start = osgState->buffer->data.size();
	for (int vidx = vec_start; vidx < vec_end; vidx++)
	{
		osg::Vec3f point = v3f->at(vidx);
		put_val(osgState->buffer->data, point.x());
		put_val(osgState->buffer->data, point.y());
		put_val(osgState->buffer->data, point.z());
		expand_bbox3d(point_max, point_min, point);
	}
	alignment_buffer(osgState->buffer->data);

	tinygltf::Accessor acc;
	acc.bufferView = osgState->model->bufferViews.size();
	acc.count = vec_end - vec_start;
	acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	acc.type = TINYGLTF_TYPE_VEC3;
	acc.maxValues = { point_max.x(), point_max.y(), point_max.z() };
	acc.minValues = { point_min.x(), point_min.y(), point_min.z() };
	osgState->model->accessors.push_back(acc);

	tinygltf::BufferView bfv;
	bfv.buffer = 0;
	bfv.target = TINYGLTF_TARGET_ARRAY_BUFFER;
	bfv.byteOffset = buffer_start;
	bfv.byteLength = osgState->buffer->data.size() - buffer_start;
	osgState->model->bufferViews.push_back(bfv);
}

void
write_vec2_array(osg::Vec2Array* v2f, OsgBuildState* osgState)
{
	int vec_start = 0;
	int vec_end = v2f->size();
	if (osgState->draw_array_first >= 0)
	{
		vec_start = osgState->draw_array_first;
		vec_end = osgState->draw_array_count + vec_start;
	}
	osg::Vec2f point_max(-1e38, -1e38);
	osg::Vec2f point_min(1e38, 1e38);
	unsigned buffer_start = osgState->buffer->data.size();
	for (int vidx = vec_start; vidx < vec_end; vidx++)
	{
		osg::Vec2f point = v2f->at(vidx);
		put_val(osgState->buffer->data, point.x());
		put_val(osgState->buffer->data, point.y());
		expand_bbox2d(point_max, point_min, point);
	}
	alignment_buffer(osgState->buffer->data);

	tinygltf::Accessor acc;
	acc.bufferView = osgState->model->bufferViews.size();
	acc.count = vec_end - vec_start;
	acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	acc.type = TINYGLTF_TYPE_VEC2;
	acc.maxValues = { point_max.x(), point_max.y() };
	acc.minValues = { point_min.x(), point_min.y() };
	osgState->model->accessors.push_back(acc);

	tinygltf::BufferView bfv;
	bfv.buffer = 0;
	bfv.target = TINYGLTF_TARGET_ARRAY_BUFFER;
	bfv.byteOffset = buffer_start;
	bfv.byteLength = osgState->buffer->data.size() - buffer_start;
	osgState->model->bufferViews.push_back(bfv);
}


struct PrimitiveState
{
	int vertexAccessor;
	int normalAccessor;
	int textcdAccessor;
};

tinygltf::Material make_color_material_osgb(double r, double g, double b) {
	tinygltf::Material material;
	material.name = "default";
	tinygltf::Parameter baseColorFactor;
	baseColorFactor.number_array = { r, g, b, 1.0 };
	material.values["baseColorFactor"] = baseColorFactor;

	tinygltf::Parameter metallicFactor;
	metallicFactor.number_value = 0;
	material.values["metallicFactor"] = metallicFactor;
	tinygltf::Parameter roughnessFactor;
	roughnessFactor.number_value = 1;
	material.values["roughnessFactor"] = roughnessFactor;
	//
	return material;
}

void
write_element_array_primitive(osg::Geometry* g, osg::PrimitiveSet* ps, OsgBuildState* osgState, PrimitiveState* pmtState)
{
	tinygltf::Primitive primits;
	// indecis
	primits.indices = osgState->model->accessors.size();
	// reset draw_array state
	osgState->draw_array_first = -1;
	osg::PrimitiveSet::Type t = ps->getType();
	switch (t)
	{
	case(osg::PrimitiveSet::DrawElementsUBytePrimitiveType):
	{
		const osg::DrawElementsUByte* drawElements = static_cast<const osg::DrawElementsUByte*>(ps);
		write_osg_indecis(drawElements, osgState, TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE);
		break;
	}
	case(osg::PrimitiveSet::DrawElementsUShortPrimitiveType):
	{
		const osg::DrawElementsUShort* drawElements = static_cast<const osg::DrawElementsUShort*>(ps);
		write_osg_indecis(drawElements, osgState, TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
		break;
	}
	case(osg::PrimitiveSet::DrawElementsUIntPrimitiveType):
	{
		const osg::DrawElementsUInt* drawElements = static_cast<const osg::DrawElementsUInt*>(ps);
		write_osg_indecis(drawElements, osgState, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT);
		break;
	}
	case osg::PrimitiveSet::DrawArraysPrimitiveType:
	{
		primits.indices = -1;
		osg::DrawArrays* da = dynamic_cast<osg::DrawArrays*>(ps);
		osgState->draw_array_first = da->getFirst();
		osgState->draw_array_count = da->getCount();
		break;
	}
	default:
	{
		LOG_E("unsupport osg::PrimitiveSet::Type [%d]", t);
		exit(1);
		break;
	}
	}
	// vertex: full vertex and part indecis
	if (pmtState->vertexAccessor > -1 && osgState->draw_array_first == -1)
	{
		primits.attributes["POSITION"] = pmtState->vertexAccessor;
	}
	else
	{
		osg::Vec3f point_max(-1e38, -1e38, -1e38);
		osg::Vec3f point_min(1e38, 1e38, 1e38);
		osg::Vec3Array* vertexArr = (osg::Vec3Array*)g->getVertexArray();
		primits.attributes["POSITION"] = osgState->model->accessors.size();
		// reuse vertex accessor if multi indecis
		if (pmtState->vertexAccessor == -1 && osgState->draw_array_first == -1)
		{
			pmtState->vertexAccessor = osgState->model->accessors.size();
		}
		write_vec3_array(vertexArr, osgState, point_max, point_min);
		// merge mesh bbox
		expand_bbox3d(osgState->point_max, osgState->point_min, point_max);
		expand_bbox3d(osgState->point_max, osgState->point_min, point_min);
	}
	// normal
	osg::Vec3Array* normalArr = (osg::Vec3Array*)g->getNormalArray();
	if (normalArr)
	{
		if (pmtState->normalAccessor > -1 && osgState->draw_array_first == -1)
		{
			primits.attributes["NORMAL"] = pmtState->normalAccessor;
		}
		else
		{
			osg::Vec3f point_max(-1e38, -1e38, -1e38);
			osg::Vec3f point_min(1e38, 1e38, 1e38);
			primits.attributes["NORMAL"] = osgState->model->accessors.size();
			// reuse vertex accessor if multi indecis
			if (pmtState->normalAccessor == -1 && osgState->draw_array_first == -1)
			{
				pmtState->normalAccessor = osgState->model->accessors.size();
			}
			write_vec3_array(normalArr, osgState, point_max, point_min);
		}
	}
	// textcoord
	osg::Vec2Array* texArr = (osg::Vec2Array*)g->getTexCoordArray(0);
	if (texArr)
	{
		if (pmtState->textcdAccessor > -1 && osgState->draw_array_first == -1)
		{
			primits.attributes["TEXCOORD_0"] = pmtState->textcdAccessor;
		}
		else
		{
			primits.attributes["TEXCOORD_0"] = osgState->model->accessors.size();
			// reuse textcoord accessor if multi indecis
			if (pmtState->textcdAccessor == -1 && osgState->draw_array_first == -1)
			{
				pmtState->textcdAccessor = osgState->model->accessors.size();
			}
			write_vec2_array(texArr, osgState);
		}
	}
	// material
	primits.material = -1;

	switch (ps->getMode())
	{
	case GL_TRIANGLES:
		primits.mode = TINYGLTF_MODE_TRIANGLES;
		break;
	case GL_TRIANGLE_STRIP:
		primits.mode = TINYGLTF_MODE_TRIANGLE_STRIP;
		break;
	case GL_TRIANGLE_FAN:
		primits.mode = TINYGLTF_MODE_TRIANGLE_FAN;
		break;
	default:
		LOG_E("Unsupport Primitive Mode: %d", (int)ps->getMode());
		exit(1);
		break;
	}
	osgState->model->meshes.back().primitives.push_back(primits);
}

void write_osgGeometry(osg::Geometry* g, OsgBuildState* osgState)
{
	osg::PrimitiveSet::Type t = g->getPrimitiveSet(0)->getType();
	PrimitiveState pmtState = { -1, -1, -1 };
	for (unsigned int k = 0; k < g->getNumPrimitiveSets(); k++)
	{
		osg::PrimitiveSet* ps = g->getPrimitiveSet(k);
		if (t != ps->getType())
		{
			LOG_E("PrimitiveSets type are NOT same in osgb");
			exit(1);
		}
		write_element_array_primitive(g, ps, osgState, &pmtState);
	}
}

//bool osgb2glb_buf(std::string path, std::string& glb_buff, MeshInfo& mesh_info) {
//	return true;
//}

bool osgb2glb_buf(std::string path, std::string& glb_buff, MeshInfo& mesh_info) {
	vector<string> fileNames = { path };
	std::cout << path << endl;
	std::string parent_path = get_parent(path);
	std::cout << parent_path << endl;
	//osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(fileNames);

	osg::ref_ptr<osg::Node> root = osgDB::readNodeFile(path);
	if (!root.valid()) {
		return false;
	}
	InfoVisitor infoVisitor(parent_path);
	root->accept(infoVisitor);
	if (infoVisitor.geometry_array.empty())
		return false;

	osgUtil::SmoothingVisitor sv;
	root->accept(sv);

	tinygltf::TinyGLTF gltf;
	tinygltf::Model model;
	tinygltf::Buffer buffer;

	osg::Vec3f point_max, point_min;
	OsgBuildState osgState = {
		&buffer, &model, osg::Vec3f(-1e38,-1e38,-1e38), osg::Vec3f(1e38,1e38,1e38), -1, -1
	};
	// mesh
	model.meshes.resize(1);
	int primitive_idx = 0;
	for (auto g : infoVisitor.geometry_array)
	{
		if (!g->getVertexArray() || g->getVertexArray()->getDataSize() == 0)
			continue;

		write_osgGeometry(g, &osgState);
		// update primitive material index
		if (infoVisitor.texture_array.size())
		{
			for (unsigned int k = 0; k < g->getNumPrimitiveSets(); k++)
			{
				auto tex = infoVisitor.texture_map[g];
				// if hava texture
				if (tex)
				{
					for (auto texture : infoVisitor.texture_array)
					{
						model.meshes[0].primitives[primitive_idx].material++;
						if (tex == texture)
							break;
					}
				}
				primitive_idx++;
			}
		}
	}
	// empty geometry or empty vertex-array
	if (model.meshes[0].primitives.empty())
		return false;

	mesh_info.min = {
		osgState.point_min.x(),
		osgState.point_min.y(),
		osgState.point_min.z()
	};
	mesh_info.max = {
		osgState.point_max.x(),
		osgState.point_max.y(),
		osgState.point_max.z()
	};
	// image
	{
		for (auto tex : infoVisitor.texture_array)
		{
			unsigned buffer_start = buffer.data.size();
			std::vector<unsigned char> jpeg_buf;
			jpeg_buf.reserve(512 * 512 * 3);
			int width, height, comp;
			if (tex) {
				if (tex->getNumImages() > 0) {
					osg::Image* img = tex->getImage(0);
					if (img) {
						width = img->s();
						height = img->t();
						comp = img->getPixelSizeInBits();
						if (comp == 8) comp = 1;
						if (comp == 24) comp = 3;
						if (comp == 4) {
							comp = 3;
							fill_4BitImage(jpeg_buf, img, width, height);
						}
						else
						{
							unsigned row_step = img->getRowStepInBytes();
							unsigned row_size = img->getRowSizeInBytes();
							for (size_t i = 0; i < height; i++)
							{
								jpeg_buf.insert(jpeg_buf.end(),
									img->data() + row_step * i,
									img->data() + row_step * i + row_size);
							}
						}
					}
				}
			}
			if (!jpeg_buf.empty()) {
				int buf_size = buffer.data.size();
				buffer.data.reserve(buffer.data.size() + width * height * comp);
				stbi_write_jpg_to_func(write_buf, &buffer.data, width, height, comp, jpeg_buf.data(), 80);
			}
			else {
				std::vector<char> v_data;
				width = height = 256;
				v_data.resize(width * height * 3);
				stbi_write_jpg_to_func(write_buf, &buffer.data, width, height, 3, v_data.data(), 80);
			}
			tinygltf::Image image;
			image.mimeType = "image/jpeg";
			image.bufferView = model.bufferViews.size();
			model.images.push_back(image);
			tinygltf::BufferView bfv;
			bfv.buffer = 0;
			bfv.byteOffset = buffer_start;
			alignment_buffer(buffer.data);
			bfv.byteLength = buffer.data.size() - buffer_start;
			model.bufferViews.push_back(bfv);
		}
	}
	// node
	{
		tinygltf::Node node;
		node.mesh = 0;
		model.nodes.push_back(node);
	}
	// scene
	{
		tinygltf::Scene sence;
		sence.nodes.push_back(0);
		model.scenes = { sence };
		model.defaultScene = 0;
	}
	// sample
	{
		tinygltf::Sampler sample;
		sample.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
		sample.minFilter = TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
		sample.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
		sample.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
		model.samplers = { sample };
	}
	// use KHR_materials_unlit
	model.extensionsRequired = { "KHR_materials_unlit" };
	model.extensionsUsed = { "KHR_materials_unlit" };
	for (int i = 0; i < infoVisitor.texture_array.size(); i++)
	{
		tinygltf::Material mat = make_color_material_osgb(1.0, 1.0, 1.0);
		mat.b_unlit = true; // use KHR_materials_unlit
		tinygltf::Parameter baseColorTexture;
		baseColorTexture.json_int_value = { std::pair<string,int>("index",i) };
		mat.values["baseColorTexture"] = baseColorTexture;
		model.materials.push_back(mat);
	}

	// finish buffer
	model.buffers.push_back(std::move(buffer));
	// texture
	{
		int texture_index = 0;
		for (auto tex : infoVisitor.texture_array)
		{
			tinygltf::Texture texture;
			texture.source = texture_index++;
			texture.sampler = 0;
			model.textures.push_back(texture);
		}
	}
	model.asset.version = "2.0";
	model.asset.generator = "fanvanzh";

	glb_buff = gltf.Serialize(&model);
	return true;
}

bool osgb2glb(const char* in, const char* out)
{
	bool b_pbr_texture = true;
	MeshInfo minfo;
	std::string glb_buf;
	std::string path = (in);
	bool ret = osgb2glb_buf(path, glb_buf, minfo);
	if (!ret)
	{
		LOG_E("convert to glb failed");
		return false;
	}

	ret = write_file(out, glb_buf.data());
	if (!ret)
	{
		LOG_E("write glb file failed");
		return false;
	}
	return true;
}

bool osgb2b3dm_buf(std::string path, std::string& b3dm_buf, TileBox& tile_box)
{
	using nlohmann::json;

	std::string glb_buf;
	MeshInfo minfo;
	bool ret = osgb2glb_buf(path, glb_buf, minfo);
	if (!ret)
		return false;

	tile_box.max = minfo.max;
	tile_box.min = minfo.min;

	int mesh_count = 1;
	std::string feature_json_string;
	feature_json_string += "{\"BATCH_LENGTH\":";
	feature_json_string += std::to_string(mesh_count);
	feature_json_string += "}";
	while ((feature_json_string.size() + 28) % 8 != 0) {
		feature_json_string.push_back(' ');
	}
	json batch_json;
	std::vector<int> ids;
	for (int i = 0; i < mesh_count; ++i) {
		ids.push_back(i);
	}
	std::vector<std::string> names;
	for (int i = 0; i < mesh_count; ++i) {
		std::string mesh_name = "mesh_";
		mesh_name += std::to_string(i);
		names.push_back(mesh_name);
	}
	batch_json["batchId"] = ids;
	batch_json["name"] = names;
	std::string batch_json_string = batch_json.dump();
	while (batch_json_string.size() % 8 != 0) {
		batch_json_string.push_back(' ');
	}


	// how length total ?
	//test
	//feature_json_string.clear();
	//batch_json_string.clear();
	//end-test

	int feature_json_len = feature_json_string.size();
	int feature_bin_len = 0;
	int batch_json_len = batch_json_string.size();
	int batch_bin_len = 0;
	int total_len = 28 /*header size*/ + feature_json_len + batch_json_len + glb_buf.size();

	b3dm_buf += "b3dm";
	int version = 1;
	put_val(b3dm_buf, version);
	put_val(b3dm_buf, total_len);
	put_val(b3dm_buf, feature_json_len);
	put_val(b3dm_buf, feature_bin_len);
	put_val(b3dm_buf, batch_json_len);
	put_val(b3dm_buf, batch_bin_len);
	//put_val(b3dm_buf, total_len);
	b3dm_buf.append(feature_json_string.begin(), feature_json_string.end());
	b3dm_buf.append(batch_json_string.begin(), batch_json_string.end());
	b3dm_buf.append(glb_buf);
	return true;
}


void do_tile_job(osg_tree& tree, std::string out_path, int max_lvl) {
	std::string json_str;
	if (tree.file_name.empty()) return;
	int lvl = get_lvl_num(tree.file_name);
	if (lvl > max_lvl) return;
	std::string b3dm_buf;
	osgb2b3dm_buf(tree.file_name, b3dm_buf, tree.bbox);
	std::string out_file = out_path;
	out_file += "/";
	out_file += replace(get_file_name(tree.file_name), ".osgb", ".b3dm");
	if (!b3dm_buf.empty()) {
		write_file(out_file.c_str(), b3dm_buf.data(), b3dm_buf.size());
	}
	// test
	// std::string glb_buf;
	// std::vector<mesh_info> v_info;
	// osgb2glb_buf(tree.file_name, glb_buf, v_info);
	// out_file = replace(out_file, ".b3dm", ".glb");
	// write_file(out_file.c_str(), glb_buf.data(), glb_buf.size());
	// end test
	for (auto& i : tree.sub_nodes) {
		do_tile_job(i, out_path, max_lvl);
	}
}

void expend_box(TileBox& box, TileBox& box_new) {
	if (box_new.max.empty() || box_new.min.empty()) {
		return;
	}
	if (box.max.empty()) {
		box.max = box_new.max;
	}
	if (box.min.empty()) {
		box.min = box_new.min;
	}
	for (int i = 0; i < 3; i++) {
		if (box.min[i] > box_new.min[i])
			box.min[i] = box_new.min[i];
		if (box.max[i] < box_new.max[i])
			box.max[i] = box_new.max[i];
	}
}

TileBox extend_tile_box(osg_tree& tree) {
	TileBox box = tree.bbox;
	for (auto& i : tree.sub_nodes) {
		TileBox sub_tile = extend_tile_box(i);
		expend_box(box, sub_tile);
	}
	tree.bbox = box;
	return box;
}

std::string get_boundingBox(TileBox bbox) {
	std::string box_str = "\"boundingVolume\":{";
	box_str += "\"box\":[";
	std::vector<double> v_box = convert_bbox(bbox);
	for (auto v : v_box) {
		box_str += std::to_string(v);
		box_str += ",";
	}
	box_str.pop_back();
	box_str += "]}";
	return box_str;
}

std::string encode_tile_json(osg_tree& tree, double x, double y)
{
	if (tree.bbox.max.empty() || tree.bbox.min.empty())
		return "";

	std::string file_name = get_file_name(tree.file_name);
	std::string parent_str = get_parent(tree.file_name);
	std::string file_path = get_file_name(parent_str);

	char buf[512];
	sprintf(buf, "{ \"geometricError\":%.2f,", tree.geometricError);
	std::string tile = buf;
	TileBox cBox = tree.bbox;
	//cBox.extend(0.1);
	std::string content_box = get_boundingBox(cBox);
	TileBox bbox = tree.bbox;
	//bbox.extend(0.1);
	std::string tile_box = get_boundingBox(bbox);

	tile += tile_box;
	tile += ",";
	tile += "\"content\":{ \"uri\":";
	// Data/Tile_0/Tile_0.b3dm
	std::string uri_path = "./";
	uri_path += file_name;
	std::string uri = replace(uri_path, ".osgb", ".b3dm");
	tile += "\"";
	tile += uri;
	tile += "\",";
	tile += content_box;
	tile += "},\"children\":[";
	for (int i = 0; i < tree.sub_nodes.size(); i++)
	{
		std::string node_json = encode_tile_json(tree.sub_nodes[i], x, y);
		if (!node_json.empty()) {
			tile += node_json;
			tile += ",";
		}
	}
	if (tile.back() == ',')
		tile.pop_back();
	tile += "]}";
	return tile;
}


void* osgb23dtile_path(const char* in_path, const char* out_path,
	double *box, int* len, double x, double y,
	int max_lvl, bool pbr_texture)
{
	std::string path =in_path;
	osg_tree root = get_all_tree(path);
	if (root.file_name.empty())
	{
		LOG_E("open file [%s] fail!", in_path);
		return NULL;
	}
	bool b_pbr_texture = pbr_texture;
	do_tile_job(root, out_path, max_lvl);
	extend_tile_box(root);
	if (root.bbox.max.empty() || root.bbox.min.empty())
	{
		LOG_E("[%s] bbox is empty!", in_path);
		return NULL;
	}
	// prevent for root node disappear
	calc_geometric_error(root);
	root.geometricError = 1000.0;
	std::string json = encode_tile_json(root, x, y);
	root.bbox.extend(0.2);
	memcpy(box, root.bbox.max.data(), 3 * sizeof(double));
	memcpy(box + 3, root.bbox.min.data(), 3 * sizeof(double));
	void* str = malloc(json.length());
	memcpy(str, json.c_str(), json.length());
	*len = json.length();
	return str;
}

//std::vector<unsigned char> jpeg_buf;
//jpeg_buf.reserve(512 * 512 * 3);
//for(int i=0;i<jpeg_buf.size();i++)
//	std::cout << jpeg_buf[i];
//std::cout << std::endl;
// By JIAO Jingguo 2023.5.21 parse metadata.xml and get all files and create all same directories as origin
int main()
{
	const clock_t begin_time = clock();
	std::cout << "This is JIAO Jingguo's OSG Demo Program(------2023.5.28)!" << std::endl;
	string inputFolder = "E:\\KY_work\\Production_3", outputDir = "E:\\KY_work\\Production_3-JJG";
	if (!isDirExist(inputFolder)) return 0;
	/*
	  第一步，传入输入数据目录、解析metadata.xml文件，拿到ENU/EPSG后获取中心经度和中心纬度；
	*/
	std::cout << "第一步，传入输入数据目录、解析metadata.xml文件，拿到ENU/EPSG后获取中心经度和中心纬度；！" << endl;
	string mXMLFileName = inputFolder + "\\metadata.xml";
	if (!isDirExist(mXMLFileName)) return 0;
	tinyxml2::XMLDocument xml;
	xml.LoadFile(mXMLFileName.c_str());
	XMLElement *modelMetadata = xml.RootElement();
	XMLElement *SRS = modelMetadata->FirstChildElement("SRS");
	XMLElement *SRSOrigin = modelMetadata->FirstChildElement("SRSOrigin");
	const char* SRS_value = SRS->GetText();
	const char* SRSOrigin_value = SRSOrigin->GetText();
	ModelMetadata mmd;
	mmd.SRS = SRS_value;
	mmd.SRSOrigin = SRSOrigin_value;
	std::cout << "空间参考：" << mmd.SRS << endl;
	std::cout << "空间参考原点：" << mmd.SRSOrigin << endl;
	//可能存在 wkt 或 EPSG
	string srs = mmd.SRS, ENU = "ENU";
	double lon = 0;
	double lat = 0;
	if (srs.length() >= 3 && srs.substr(0, 3) == ENU)
	{
		std::cout << "The metadata.xml 文件的空间参考是北偏东坐标系！" << endl;
		string lonlatStr = srs.substr(4, srs.length());
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
			lat = stof(lonlatStr.substr(0, index).c_str());
			lon = stof(lonlatStr.substr(index + 1, lonlatStr.length()).c_str());// lon = stringToDouble(lonStr);lat = stringToDouble(latStr);
		}
		
	}

	/*
	  第二步，输出数据目录、最大层级、中心经度、中心纬度、区域偏移（模型离地面高度）、是否有pbr纹理，，调用osgb_batch_convert进行批量分块转换；
	*/
	std::cout << "第二步，输出数据目录、最大层级、中心经度、中心纬度、区域偏移（模型离地面高度）、是否有pbr纹理，，调用osgb_batch_convert进行批量分块转换！" << endl;
	unsigned int max_lvl = 20;
	double center_x = lon, center_y = lat, region_offset = 5;
	bool pbr_texture = true;
	string::size_type iPos = (mXMLFileName.find_last_of('\\') + 1) == 0 ? mXMLFileName.find_last_of('/') + 1 : mXMLFileName.find_last_of('\\') + 1;
	string fileName = mXMLFileName.substr(iPos, mXMLFileName.length() - iPos);//获取带后缀的文件名
	string filePath = mXMLFileName.substr(0, iPos);//获取文件路径
	string fileName_noSuffix = fileName.substr(0, fileName.rfind("."));//获取不带后缀的文件名
	string fileName_withSuffix = fileName.substr(fileName.rfind("."), fileName.length());//获取后缀名
	std::cout << "输出数据目录(outputDir)：" << filePath + "Data" << endl \
		      << "最大层级(max_lvl)：" << max_lvl << endl \
		      << "中心经度(center_x)：" << center_x << endl \
	          << "中心纬度(center_y)：" << center_y << endl \
			  << "区域偏移(region_offset)：" << region_offset << endl \
			  << "是否有pbr纹理(pbr_texture)：" << pbr_texture << endl;
	
	/*
	   第三步：判断输入目录下是否存在Data文件夹，如果存在，则对输出数据目录创建与输入数据目录同样的文件夹结构；
	*/
	std::cout << "第三步：判断输入目录下是否存在Data文件夹，如果存在，则对输出数据目录创建与输入数据目录同样的文件夹结构！" << endl;
	string dataDirectory = filePath + "Data", fileType = ".osgb", newFileType = ".b3dm";
	vector<OsgbInfo> osgb_dir_pair;
	if (!isDirExist(dataDirectory))
	{
		std::cout << "输入OSG数据目录" << dataDirectory << "不存在！" << endl;
		return 0;
	}

	std::cout << "OSG数据目录" << dataDirectory << "存在！" << endl;
	vector<string> temp;
	getAllFiles(dataDirectory, temp, fileType);

	for (int i = 0; i < temp.size(); ++i)
	{
		string tempStr = temp[i];
		string replaceNewDataFileName = replace_all_distinct(temp[i], dataDirectory, outputDir + "\\Data");
		string::size_type dotPos = replaceNewDataFileName.find_last_of('.') + 1;
		if (dotPos && replaceNewDataFileName.substr(dotPos - 1, replaceNewDataFileName.length() - fileType.length()) == fileType) //如果后缀名为.osgb
		{
			string newFilename = replace_all_distinct(replaceNewDataFileName, fileType, newFileType);
			OsgbInfo osgbInfo;
			osgbInfo.in_dir = tempStr;
			osgbInfo.out_dir = newFilename;
			osgb_dir_pair.push_back(osgbInfo);

		}
		else //如果是文件夹，则创建新目录
		{
			CreateMultiLevel(replaceNewDataFileName);
		}
	}

	/*
	   第四步：在遍历所有输入数据的同时，将.osgb数据文件的数量进行统计作为任务数，并建立.osgb输入数据和.b3dm输出数据的任务对数组；
	*/
	std::cout << "第四步：在遍历所有输入数据的同时，将.osgb数据文件的数量进行统计作为任务数，并建立.osgb输入数据和.b3dm输出数据的任务对数组！" << endl;
	std::cout << "任务总数为：" << osgb_dir_pair.size() << endl;

	/*
	   第五步：将经纬度转换为弧度值；
	*/
	std::cout << "第五步：将经纬度转换为弧度值！" << endl;
	double rad_x = degree2rad(center_x), rad_y = degree2rad(center_y);
	std::cout << "弧度经度（rad_x）" << rad_x << endl;
	std::cout << "弧度纬度（rad_y）" << rad_y << endl;

	/*
	   第六步：遍历所有的任务对，对于每个任务对调用osgb23dtile_path函数进行转换，对每个任务对的结果生成TileResult（包含输出数据路径、json_buf、root_box）;
	*/
	std::cout << "第六步：遍历所有的任务对，对于每个任务对调用osgb23dtile_path函数进行转换，对每个任务对的结果生成TileResult（包含输出数据路径、json_buf、root_box）！" << endl;
	for (int i = 0; i < osgb_dir_pair.size(); i++)
	{
		double *root_box = new double[6];
		*root_box = 0;
		*(root_box + 1) = 0;
		*(root_box + 2) = 0;
		*(root_box + 3) = 0;
		*(root_box + 4) = 0;
		*(root_box + 5) = 0;
		vector<int> json_buf;
	    int *json_len = 0;
		/*auto out_ptr = osgb23dtile_path(&osgb_dir_pair[i].in_dir[0],
			&osgb_dir_pair[i].out_dir[0],
			root_box,
			json_len,
			rad_x,
			rad_y,
			max_lvl,
			pbr_texture);
			
		if (!out_ptr)
			std::cout << "第"<<i << "个转换failed!" << endl;
		else*/
		std::cout << osgb_dir_pair[i].in_dir.c_str()<< " ----> " << osgb_dir_pair[i].out_dir.c_str()<< endl;
		bool res = osgb2glb(osgb_dir_pair[i].in_dir.c_str(), osgb_dir_pair[i].out_dir.c_str());
	}
	
	float seconds = float(clock() - begin_time) / 1000;    //最小精度到ms
	std::cout << "task over, cost " << seconds << "s" << endl;

	
	/*double lon = 117.0, lat = 39.0, tile_w = 250, tile_h = 250, height_min = 0, height_max = 100, geometricError = 50;
	double rad_lon = degree2rad(lon), rad_lat = degree2rad(lat);
	string full_path = "./tileset-First.json";
	const char* filename = "./tile+01_05/01_05.b3dm";
	write_tileset(rad_lon, rad_lat, tile_w, tile_h, height_min, height_max, geometricError, filename,full_path.data());*/


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

	

	
	
	

	//cout << "Start to Read Shapefile!" << std::endl;
	//GDALAllRegister();
	//GDALDataset   *poDS;
	//CPLSetConfigOption("SHAPE_ENCODING", "");  //解决中文乱码问题
	//										   //读取shp文件
	//										   //注：GDAL不能读取中文路径，所以记得要换成英文路径
	//poDS = (GDALDataset*)GDALOpenEx("../x64/Release/world.shp", GDAL_OF_VECTOR, NULL, NULL, NULL);

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

	//double* val = new double[2];
	//*val = 117;
	//*(val + 1) = 39;
	//epsg_convert(4326, val, "E:\\jing_zhong\\OSG_Demo\\x64\\Release\\gdal_data");
	//cout << *(val) << endl;
	//cout << *(val + 1) << endl;

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
	//filenameStr = "./cow.osg";
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
	//system("pause");
	//return 0;
}