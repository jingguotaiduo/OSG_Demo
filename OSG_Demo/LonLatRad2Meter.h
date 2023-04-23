#ifndef LONLATRAD2METER_H
	#define LONLATRAD2METER_H
    static const double pi = std::acos(-1);

	struct Transform // 定义变换结构体
	{
		double radian_x;
		double radian_y;
		double min_height;
	};

	struct Box //定义盒子结构体，共12个浮点数，其中包含中心点三维坐标（P_x,P_y,P_z）、以及（长，0,0）、（0、宽、0）和（0、0、高）
	{
		double matrix[12];
	};

	struct Region //定义区域结构体，分别为水平x、水平y、竖直h的最小值和最大值，共计6个浮点数
	{
		double min_x;
		double min_y;
		double max_x;
		double max_y;
		double min_height;
		double max_height;
	};

	struct TileBox //定义切片盒子结构体
	{
		std::vector<double> max;
		std::vector<double> min;

		void extend(double ratio) {
			ratio /= 2;
			double x = max[0] - min[0];
			double y = max[1] - min[1];
			double z = max[2] - min[2];
			max[0] += x * ratio;
			max[1] += y * ratio;
			max[2] += z * ratio;

			min[0] -= x * ratio;
			min[1] -= y * ratio;
			min[2] -= z * ratio;
		}
	};

	double degree2rad(double val);//十进制度 转 弧度

	double lati_to_meter(double diff);//十进制纬度 转 长度（单位：m)

	double longti_to_meter(double diff, double lati);//十进制经度 转 长度（单位：m)

	double meter_to_lati(double m);//长度（单位：m）转 十进制纬度

	double meter_to_longti(double m, double lati);//长度（单位：m） 转 十进制经度

	bool mkdirs(const char* path);//创建目录的函数

	bool write_file(const char* filename, const char* buf, unsigned long buf_len);//写文件的函数
	bool write_file(std::string filename, std::string content);//写文件的函数

	void log_error(const char* msg);//打印错误日志的函数

	#define LOG_E(fmt,...) \
			char buf[512];\
			sprintf_s(buf,fmt,##__VA_ARGS__);\
			log_error(buf);

	bool write_tileset_region(Transform* trans, Region& region, double geometricError, const char* b3dm_file, const char* json_file);

	bool write_tileset_box(Transform* trans, Box& box, double geometricError, const char* b3dm_file, const char* json_file);

	bool write_tileset(double longti, double lati, double tile_w, double tile_h, double height_min, double height_max, double geometricError, const char* filename, const char* full_path);
	
	bool write_tileset(double radian_x, double radian_y, double tile_w, double tile_h, double height_min, double height_max, double geometricError, const char* filename, const char* full_path);//中心点弧度经度、中心点弧度纬度、切片宽度、切片高度、最小高度、最大高度、几何误差、文件名
#endif