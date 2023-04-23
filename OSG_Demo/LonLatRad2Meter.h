#ifndef LONLATRAD2METER_H
	#define LONLATRAD2METER_H
    static const double pi = std::acos(-1);

	struct Transform // ����任�ṹ��
	{
		double radian_x;
		double radian_y;
		double min_height;
	};

	struct Box //������ӽṹ�壬��12�������������а������ĵ���ά���꣨P_x,P_y,P_z�����Լ�������0,0������0����0���ͣ�0��0���ߣ�
	{
		double matrix[12];
	};

	struct Region //��������ṹ�壬�ֱ�Ϊˮƽx��ˮƽy����ֱh����Сֵ�����ֵ������6��������
	{
		double min_x;
		double min_y;
		double max_x;
		double max_y;
		double min_height;
		double max_height;
	};

	struct TileBox //������Ƭ���ӽṹ��
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

	double degree2rad(double val);//ʮ���ƶ� ת ����

	double lati_to_meter(double diff);//ʮ����γ�� ת ���ȣ���λ��m)

	double longti_to_meter(double diff, double lati);//ʮ���ƾ��� ת ���ȣ���λ��m)

	double meter_to_lati(double m);//���ȣ���λ��m��ת ʮ����γ��

	double meter_to_longti(double m, double lati);//���ȣ���λ��m�� ת ʮ���ƾ���

	bool mkdirs(const char* path);//����Ŀ¼�ĺ���

	bool write_file(const char* filename, const char* buf, unsigned long buf_len);//д�ļ��ĺ���
	bool write_file(std::string filename, std::string content);//д�ļ��ĺ���

	void log_error(const char* msg);//��ӡ������־�ĺ���

	#define LOG_E(fmt,...) \
			char buf[512];\
			sprintf_s(buf,fmt,##__VA_ARGS__);\
			log_error(buf);

	bool write_tileset_region(Transform* trans, Region& region, double geometricError, const char* b3dm_file, const char* json_file);

	bool write_tileset_box(Transform* trans, Box& box, double geometricError, const char* b3dm_file, const char* json_file);

	bool write_tileset(double longti, double lati, double tile_w, double tile_h, double height_min, double height_max, double geometricError, const char* filename, const char* full_path);
	
	bool write_tileset(double radian_x, double radian_y, double tile_w, double tile_h, double height_min, double height_max, double geometricError, const char* filename, const char* full_path);//���ĵ㻡�Ⱦ��ȡ����ĵ㻡��γ�ȡ���Ƭ��ȡ���Ƭ�߶ȡ���С�߶ȡ����߶ȡ��������ļ���
#endif