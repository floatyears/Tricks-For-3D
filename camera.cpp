#include "camera.h"

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
{
	cam->attr = cam_attr;							//相机属性
	POINT4D_COPY(&cam->pos, cam_pos);				//位置
	VECTOR4D_COPY(&cam->dir, cam_dir);				//欧拉相机的方向向量或角度

	VECTOR4D_INITXYZ(&cam->u, 1, 0, 0);				//设置为x轴方向
	VECTOR4D_INITXYZ(&cam->v, 0, 1, 0);				//设置为y轴方向
	VECTOR4D_INITXYZ(&cam->n, 0, 0, 1);				//设置为z轴方向

	if (cam_target != NULL)
	{
		POINT4D_COPY(&cam->target, cam_target);	//UVN目标位置
	}
	else
	{
		POINT4D_ZERO(&cam->target);
	}

	cam->near_clip_z = near_clip_z;
	cam->far_clip_z = far_clip_z;

	cam->viewport_height = viewport_height;
	cam->viewport_width = viewport_width;

	cam->viewport_center_x = (viewport_width - 1) / 2;
	cam->viewport_center_y = (viewport_height - 1) / 2;

	cam->aspect_ratio = (float)viewport_width / (float)viewport_height;

	MAT_IDENTITY_4X4(&cam->mcam);
	MAT_IDENTITY_4X4(&cam->mper);
	MAT_IDENTITY_4X4(&cam->mscr);

	cam->fov = fov;

	//将视平面大小设置为2x(2/ar)
	cam->viewplane_width = 2.0;
	cam->viewplane_height = 2.0 / cam->aspect_ratio;

	//根据fov和视平面大小计算视距
	float tan_fov_div2 = tan(DEG_TO_RAD(fov / 2));

	cam->view_dist = 0.5 * cam->viewplane_width*tan_fov_div2;

	if (fov == 90.0){
		//建立裁剪面
		POINT3D pt_origin;
		VECTOR3D_INITXYZ(&pt_origin, 0, 0, 0);

		VECTOR3D vn; //面法线

		//右裁剪面
		VECTOR3D_INITXYZ(&vn, 1, 0, -1);
		PLANE3D_Init(&cam->rt_clip_plane, &pt_origin, &vn, 1);

		//左裁剪面
		VECTOR3D_INITXYZ(&vn, -1, 0, -1);
		PLANE3D_Init(&cam->lt_clip_plane, &pt_origin, &vn, 1);

		//上裁剪面
		VECTOR3D_INITXYZ(&vn, 0, 1, -1);
		PLANE3D_Init(&cam->tp_clip_plane, &pt_origin, &vn, 1);

		//下裁剪面
		VECTOR3D_INITXYZ(&vn, 0, -1, -1);
		PLANE3D_Init(&cam->bt_clip_plane, &pt_origin, &vn, 1);
	}
	else
	{
		//计算fov不为90度时的裁剪面
		POINT3D pt_origin; //
		VECTOR3D_INITXYZ(&pt_origin, 0, 0, 0);

		VECTOR3D vn; //面法线

		//由于fov不为90度，首先计算表示裁剪面在x-z和y-z上的2D投影
		//然后计算与这两个向量垂直的向量，它就是裁剪面的法线
		VECTOR3D_INITXYZ(&vn, cam->view_dist, 0, -cam->viewplane_width / 2.0);
		PLANE3D_Init(&cam->rt_clip_plane, &pt_origin, &vn, 1);

		//左裁剪面，可以绕Z轴反射右裁剪面的法线，来得到左裁剪面的法线
		VECTOR3D_INITXYZ(&vn, -cam->view_dist, 0, -cam->viewplane_width / 2.0);
		PLANE3D_Init(&cam->lt_clip_plane, &pt_origin, &vn, 1);

		//上裁剪面
		VECTOR3D_INITXYZ(&vn, 0, cam->view_dist, -cam->viewplane_width / 2.0);
		PLANE3D_Init(&cam->tp_clip_plane, &pt_origin, &vn, 1);

		//下裁剪面
		VECTOR3D_INITXYZ(&vn, 0, -cam->view_dist, -cam->viewplane_width / 2.0);
		PLANE3D_Init(&cam->bt_clip_plane, &pt_origin, &vn, 1);
	}
}

void Build_CAM4DV1_Matrix_Euler(CAM4DV1_PTR cam, int cam_rot_seq)
{
	MATRIX4X4 mt_inv,	//相机平移矩阵的逆矩阵
		mx_inv,			//相机绕x轴的旋转矩阵的逆矩阵
		my_inv,			//相机绕y轴的旋转矩阵的逆矩阵
		mz_inv,			//相机绕z轴的旋转矩阵的逆矩阵
		mrot,			//所有逆旋转矩阵的积
		mtmp;			//用于存储临时矩阵

	//根据相机的位置计算出相机平移矩阵的逆矩阵
	Mat_Init_4X4(&mt_inv, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-cam->pos.x, -cam->pos.y, -cam->pos.z, 1);

	//创建旋转矩阵的逆矩阵
	//提取欧拉角度
	float theta_x = cam->dir.x;
	float theta_y = cam->dir.y;
	float theta_z = cam->dir.z;

	//计算角度x的正弦和余弦
	float cos_theta = Fast_Cos(theta_x);
	float sin_theta = -Fast_Sin(theta_x);

	//建立矩阵
	Mat_Init_4X4(&mx_inv, 1, 0, 0, 0,
		0, cos_theta, sin_theta, 0,
		0, -sin_theta, cos_theta, 0,
		0, 0, 0, 1);

	//计算余角度y的正弦和余弦
	cos_theta = Fast_Cos(theta_y);
	sin_theta = -Fast_Sin(theta_y);
	
	//建立矩阵
	Mat_Init_4X4(&my_inv, cos_theta, 0, -sin_theta, 0,
		0, 1, 0, 0,
		sin_theta, 0, cos_theta, 0,
		0, 0, 0, 1);

	//计算角度z的正弦和余弦
	cos_theta = Fast_Cos(theta_z);
	sin_theta = -Fast_Sin(theta_z);

	//建立矩阵
	Mat_Init_4X4(&mz_inv, cos_theta, sin_theta, 0, 0,
		-sin_theta, cos_theta, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);

	//现在计算逆旋转矩阵的乘积
	switch (cam_rot_seq)
	{
	case CAM_ROT_SEQ_XYZ:
		Mat_Mul_4X4(&mx_inv, &my_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &mz_inv, &mrot);
		break;
	case CAM_ROT_SEQ_YXZ:
		Mat_Mul_4X4(&my_inv, &mx_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &mz_inv, &mrot);
		break;
	case CAM_ROT_SEQ_XZY:
		Mat_Mul_4X4(&mx_inv, &mz_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &my_inv, &mrot);
		break;
	case CAM_ROT_SEQ_YZX:
		Mat_Mul_4X4(&my_inv, &mz_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &mx_inv, &mrot);
		break;
	case CAM_ROT_SEQ_ZYX:
		Mat_Mul_4X4(&mz_inv, &my_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &mx_inv, &mrot);
		break;
	case CAM_ROT_SEQ_ZXY:
		Mat_Mul_4X4(&mz_inv, &mx_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &my_inv, &mrot);
		break;
	default:
		break;
	}

	Mat_Mul_4X4(&mt_inv, &mrot, &cam->mcam);
	
}

void Build_CAM4DV1_Matrix_UVN(CAM4DV1_PTR cam, int mode)
{

}