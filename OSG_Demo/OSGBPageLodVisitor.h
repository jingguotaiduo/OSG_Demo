#include<string>
#include<vector>
#include<map>
#include<set>

#include <osg/PagedLOD>
#include <osgDB/ReadFile>
#include <osgDB/ConvertUTF>
#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

using namespace std;


class OSGBPageLodVisitor : public osg::NodeVisitor
{
public:
	OSGBPageLodVisitor(const std::string& path) :osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), path(path)
	{}

	virtual ~OSGBPageLodVisitor() {}

	void apply(osg::Geometry& geometry);
	void apply(osg::PagedLOD& node);

public:
	std::string path;
	vector<osg::Geometry*>  geometryArray;
	set<osg::Texture*>  textureArray;
	map<osg::Geometry*, osg::Texture*> textureMap;
	vector<std::string> subNodeNames;
};
