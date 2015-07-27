#include "poly.h"
#include "t3dlib4.h"

typedef unsigned char  UCHAR;

//材质属性，尽可能使其与多边形属性一致
#define MATV1_ATTR_2SIDED						0x0001
#define MATV1_ATTR_TRANSPARENT					0x0002
#define MATV1_ATTR_8BITCOLOR					0x0004
#define MATV1_ATTR_RGB16						0x0008
#define MATV1_ATTR_RGB24						0x0010

#define MATV1_ATTR_SHADER_MODE_CONSTANT			0x0020
#define MATV1_ATTR_SHADER_MODE_EMMISIVE			0x0020 //别名
#define MATV1_ATTR_SHADER_MODE_FLAT				0x0040 
#define MATV1_ATTR_SHADER_MODE_GOURAUD			0x0080
#define MATV1_ATTR_SHADER_MODE_FASTPHONG		0x0100
#define MATV1_ATTR_SHADER_MODE_TEXTURE			0x0200

//材质状态
#define MATV1_STATE_ACTIVE						0x0001

#define MAX_MATERIALS							256

typedef struct RGBAV1_TYP
{
	union
	{
		int rgba;				//压缩格式
		UCHAR rgba_M[4];		//数组格式
		struct					//显式名称格式
		{
			UCHAR a, b, g, r;
		};
	};
} RGBAV1, *RGBAV1_PTR;

// the simple bitmap image
typedef struct BITMAP_IMAGE_TYP
{
	int state;          // state of bitmap
	int attr;           // attributes of bitmap
	int x, y;            // position of bitmap
	int width, height;  // size of bitmap
	int num_bytes;      // total bytes of bitmap
	int bpp;            // bits per pixel
	UCHAR *buffer;      // pixels of bitmap

} BITMAP_IMAGE, *BITMAP_IMAGE_PTR;

typedef	struct MATV1_TYP
{
	int state;					//材质的状态
	int id;						//材质id，执行材质数组的索引
	char name[64];				//材质名称
	int attr;					//属性：着色模式、着色方法、环境、纹理及其他特殊标记

	RGBAV1 color;				//材质颜色
	float ka, kd, ks, power;	//对环境光、散射光和镜面反射光的反射系数和镜面反射指数

	RGBAV1 ra, rd, rs;			//预先计算得到的颜色和反射系数的值，旨在使该数据结构与我们的光照模型更为一致
	char texture_file[80];		//包含纹理的文件的位置
	BITMAP_IMAGE texture;		//纹理图
} MATV1, *MATV1_PTR;


//有关广元的常量
#define LIGHTV1_ATTR_AMBIENT			0x0001
#define LIGHTV1_ATTR_INFINITE			0x0002
#define LIGHTV1_ATTR_POINT				0x0004
#define LIGHTV1_ATTR_SPOTLIGHT1			0x0008
#define LIGHTV1_ATTR_SPOTLIGHT2			0x0010

#define LIGHTV1_STATE_ON				0
#define LIGHTV1_STATE_OFF				1

#define MAX_LIGHTS						8

typedef	struct LIGHTV1_TYP
{
	int state;
	int id;
	int attr;

	RGBAV1 c_ambient;	//环境光强度
	RGBAV1 c_diffuse;	//散射光强度
	RGBAV1 c_specular;	//镜面反射光强度
	POINT4D pos;		//光源位置
	VECTOR4D dir;		//光源方向
	float kc, kl, kq;	//衰减因子
	float spot_inner;	//聚光灯内锥角
	float spot_outer;	//聚光灯外锥角

	float pf;
} LIGHTV1, *LIGHT_PTR;