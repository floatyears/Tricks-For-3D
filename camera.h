#define CAM_MODEL_ELUER		1
#define CAM_MODEL_UVN		2

#define CAM_ROT_SEQ_XYZ		0
#define CAM_ROT_SEQ_YXZ		1
#define CAM_ROT_SEQ_XZY		2
#define CAM_ROT_SEQ_YZX		3
#define CAM_ROT_SEQ_ZYX		4
#define CAM_ROT_SEQ_ZXY		5


#include "t3dlib4.h"
#include "poly.h"

typedef struct CAM4DV1_TYP{
	int state;					//���״̬
	int attr;					//�������
	POINT4D pos;				//�������������ϵ�е�λ��
	VECTOR4D dir;				//ŷ���Ƕ�
	VECTOR4D u;					//�����uvn��������
	VECTOR4D v;
	VECTOR4D n;
	POINT4D  target;			//UVNģ�͵�Ŀ������

	float view_dist;			//�Ӿ�

	float fov;					//ˮƽ����ʹ�ֱ�������Ұ

	float near_clip_z;			//���ü�ƽ��
	float far_clip_z;			//Զ�ü�ƽ��

	float viewplane_width;
	float viewplane_height;

	PLANE3D rt_clip_plane;		//�Ҳü�ƽ��
	PLANE3D lt_clip_plane;		//��ü�ƽ��
	PLANE3D tp_clip_plane;		//�ϲü�ƽ��
	PLANE3D bt_clip_plane;		//�²ü�ƽ��

	float viewport_width;	    //��Ļ/�ӿڵĴ�С
	float viewport_height;
	float viewport_center_x;    //�ӿڵ�����
	float viewport_center_y;

	float aspect_ratio;			//��Ļ�Ŀ�߱�

	MATRIX4X4 mcam;				//���ڴ洢�������굽�������任����
	MATRIX4X4 mper;				//���ڴ洢������굽͸������任����
	MATRIX4X4 mscr;				//���ڴ洢͸�����굽��Ļ����任����
} CAM4DV1, *CAM4DV1_PTR;

void Init_CAM4DV1(CAM4DV1_PTR cam,
	int cam_attr,
	POINT4D_PTR cam_pos,
	VECTOR4D_PTR cam_dir,
	POINT4D_PTR cam_target,
	float near_clip_z,
	float far_clip_z,
	float fov,
	float viewport_width,
	float viewport_height)