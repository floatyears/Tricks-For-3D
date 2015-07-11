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
	int state;					//相机状态
	int attr;					//相机属性
	POINT4D pos;				//相机在世界坐标系中的位置
	VECTOR4D dir;				//欧拉角度
	VECTOR4D u;					//相机的uvn朝向向量
	VECTOR4D v;
	VECTOR4D n;
	POINT4D  target;			//UVN模型的目标向量

	float view_dist;			//视距

	float fov;					//水平方向和垂直方向的视野

	float near_clip_z;			//近裁剪平面
	float far_clip_z;			//远裁剪平面

	float viewplane_width;
	float viewplane_height;

	PLANE3D rt_clip_plane;		//右裁剪平面
	PLANE3D lt_clip_plane;		//左裁剪平面
	PLANE3D tp_clip_plane;		//上裁剪平面
	PLANE3D bt_clip_plane;		//下裁剪平面

	float viewport_width;	    //屏幕/视口的大小
	float viewport_height;
	float viewport_center_x;    //视口的中心
	float viewport_center_y;

	float aspect_ratio;			//屏幕的宽高比

	MATRIX4X4 mcam;				//用于存储世界坐标到相机坐标变换矩阵
	MATRIX4X4 mper;				//用于存储相机坐标到透视坐标变换矩阵
	MATRIX4X4 mscr;				//用于存储透视坐标到屏幕坐标变换矩阵
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