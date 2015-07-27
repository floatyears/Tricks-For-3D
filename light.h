#include "poly.h"
#include "t3dlib4.h"

typedef unsigned char  UCHAR;

//�������ԣ�������ʹ������������һ��
#define MATV1_ATTR_2SIDED						0x0001
#define MATV1_ATTR_TRANSPARENT					0x0002
#define MATV1_ATTR_8BITCOLOR					0x0004
#define MATV1_ATTR_RGB16						0x0008
#define MATV1_ATTR_RGB24						0x0010

#define MATV1_ATTR_SHADER_MODE_CONSTANT			0x0020
#define MATV1_ATTR_SHADER_MODE_EMMISIVE			0x0020 //����
#define MATV1_ATTR_SHADER_MODE_FLAT				0x0040 
#define MATV1_ATTR_SHADER_MODE_GOURAUD			0x0080
#define MATV1_ATTR_SHADER_MODE_FASTPHONG		0x0100
#define MATV1_ATTR_SHADER_MODE_TEXTURE			0x0200

//����״̬
#define MATV1_STATE_ACTIVE						0x0001

#define MAX_MATERIALS							256

typedef struct RGBAV1_TYP
{
	union
	{
		int rgba;				//ѹ����ʽ
		UCHAR rgba_M[4];		//�����ʽ
		struct					//��ʽ���Ƹ�ʽ
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
	int state;					//���ʵ�״̬
	int id;						//����id��ִ�в������������
	char name[64];				//��������
	int attr;					//���ԣ���ɫģʽ����ɫ��������������������������

	RGBAV1 color;				//������ɫ
	float ka, kd, ks, power;	//�Ի����⡢ɢ���;��淴���ķ���ϵ���;��淴��ָ��

	RGBAV1 ra, rd, rs;			//Ԥ�ȼ���õ�����ɫ�ͷ���ϵ����ֵ��ּ��ʹ�����ݽṹ�����ǵĹ���ģ�͸�Ϊһ��
	char texture_file[80];		//����������ļ���λ��
	BITMAP_IMAGE texture;		//����ͼ
} MATV1, *MATV1_PTR;


//�йع�Ԫ�ĳ���
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

	RGBAV1 c_ambient;	//������ǿ��
	RGBAV1 c_diffuse;	//ɢ���ǿ��
	RGBAV1 c_specular;	//���淴���ǿ��
	POINT4D pos;		//��Դλ��
	VECTOR4D dir;		//��Դ����
	float kc, kl, kq;	//˥������
	float spot_inner;	//�۹����׶��
	float spot_outer;	//�۹����׶��

	float pf;
} LIGHTV1, *LIGHT_PTR;