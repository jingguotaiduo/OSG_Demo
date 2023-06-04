# OSG_Demo
Demo Program for OSG!

## 1、read OSGB file to view the scene

<pre>
<code>
  osg::Group* root = new osg::Group;
	osg::ref_ptr<osg::Node> n = osgDB::readNodeFile("E:\\KY_work\\Production_3\\Data\\Tile_+000_+000\\Tile_+000_+000.osgb");
	root->addChild(n);
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
	viewer->setSceneData(root);
	viewer->setUpViewInWindow(20, 20, 800, 600);
	viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
	viewer->addEventHandler(new osgViewer::WindowSizeHandler);
	viewer->addEventHandler(new osgViewer::StatsHandler);
	viewer->realize();
	return viewer->run();
</code>
</pre>
