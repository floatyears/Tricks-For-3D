#include "light.h"

int Reset_Lights_LIGHTV1(void)
{
	static int first_time = 1;
	//����ǵ�һ�ε��øú��������������еĲ���
	if (first_time)
	{
		memset(materials, 0, MAX_MATERIALS*sizeof(MATV1));
		first_time = 0;
	}

	for (int curr_matt = 0; curr_matt < MAX_MATERIALS; curr_matt++)
	{
		//���ܲ����Ƿ��ڻ״̬�����ͷ���֮��ص�����ͼ 
		//Destroy_Bitmap(&materials[curr_matt],0,sizeof(MATV1));
		memset(&materials[curr_matt], 0, sizeof(MATV1));
	}
	return 1;
}

int Reset_Lights_LIGHTV1(void)
{
	static int first_time = 1;
	//����ϵͳ�е����й�Դ
	memset(lights, 0, MAX_LIGHTS * sizeof(LIGHTV1));

	num_lights = 0;

	first_time = 0;

	return 1;
}

//���ݴ���Ĳ�����ʼ����Դ
int Init_Light_LIGHTV1(int index,
	int _state,
	int _attr,
	RGBAV1 _c_ambient,		//������ǿ��
	RGBAV1 _c_diffuse,		//ɢ���ǿ��
	RGBAV1 _c_specular,
	POINT4D_PTR _pos,
	POINT4D_PTR _dir,
	float _kc, float _kl, float _kq, //˥������
	float _spot_inner,		//�۹����׶��
	float _spot_outer,		//�۹����׶��
	float _pf)				//�۹��ָ������
{
	if (index < 0 || index >= MAX_LIGHTS) return 0;

	lights[index].state = _state;
	lights[index].attr	= _attr;
	lights[index].c_ambient = _c_ambient;
	lights[index].c_diffuse = _c_diffuse;
	lights[index].c_specular = _c_specular;
	lights[index].kc = _kc;
	lights[index].kq = _kq;
	lights[index].kl = _kl;

	if (_pos)
	{
		VECTOR4D_COPY(&lights[index].pos, _pos);
	}

	if (_dir)
	{
		VECTOR4D_COPY(&lights[index].dir, _dir);
		//��һ��
		VECTOR4D_Normalize(&lights[index].dir);
	}
	lights[index].spot_inner = _spot_inner;
	lights[index].spot_outer = _spot_outer;
	lights[index].pf = _pf;

	//����������Դ
	return index;
}