#include "light.h"

int Reset_Lights_LIGHTV1(void)
{
	static int first_time = 1;
	//如果是第一次调用该函数，则重置所有的材质
	if (first_time)
	{
		memset(materials, 0, MAX_MATERIALS*sizeof(MATV1));
		first_time = 0;
	}

	for (int curr_matt = 0; curr_matt < MAX_MATERIALS; curr_matt++)
	{
		//不管材质是否处于活动状态，都释放与之相关的纹理图 
		//Destroy_Bitmap(&materials[curr_matt],0,sizeof(MATV1));
		memset(&materials[curr_matt], 0, sizeof(MATV1));
	}
	return 1;
}

int Reset_Lights_LIGHTV1(void)
{
	static int first_time = 1;
	//重置系统中的所有光源
	memset(lights, 0, MAX_LIGHTS * sizeof(LIGHTV1));

	num_lights = 0;

	first_time = 0;

	return 1;
}

//根据传入的参数初始化光源
int Init_Light_LIGHTV1(int index,
	int _state,
	int _attr,
	RGBAV1 _c_ambient,		//环境光强度
	RGBAV1 _c_diffuse,		//散射光强度
	RGBAV1 _c_specular,
	POINT4D_PTR _pos,
	POINT4D_PTR _dir,
	float _kc, float _kl, float _kq, //衰减因子
	float _spot_inner,		//聚光灯内锥角
	float _spot_outer,		//聚光灯外锥角
	float _pf)				//聚光灯指数因子
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
		//归一化
		VECTOR4D_Normalize(&lights[index].dir);
	}
	lights[index].spot_inner = _spot_inner;
	lights[index].spot_outer = _spot_outer;
	lights[index].pf = _pf;

	//返回索引光源
	return index;
}