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
	cam->attr = cam_attr;							//�������
	POINT4D_COPY(&cam->pos, cam_pos);				//λ��
	VECTOR4D_COPY(&cam->dir, cam_dir);				//ŷ������ķ���������Ƕ�

	VECTOR4D_INITXYZ(&cam->u, 1, 0, 0);				//����Ϊx�᷽��
	VECTOR4D_INITXYZ(&cam->v, 0, 1, 0);				//����Ϊy�᷽��
	VECTOR4D_INITXYZ(&cam->n, 0, 0, 1);				//����Ϊz�᷽��

	if (cam_target != NULL)
	{
		POINT4D_COPY(&cam->target, cam_target);	//UVNĿ��λ��
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

	//����ƽ���С����Ϊ2x(2/ar)
	cam->viewplane_width = 2.0;
	cam->viewplane_height = 2.0 / cam->aspect_ratio;

	//����fov����ƽ���С�����Ӿ�
	float tan_fov_div2 = tan(DEG_TO_RAD(fov / 2));

	cam->view_dist = 0.5 * cam->viewplane_width*tan_fov_div2;

	if (fov == 90.0){
		//�����ü���
		POINT3D pt_origin;
		VECTOR3D_INITXYZ(&pt_origin, 0, 0, 0);

		VECTOR3D vn; //�淨��

		//�Ҳü���
		VECTOR3D_INITXYZ(&vn, 1, 0, -1);
		PLANE3D_Init(&cam->rt_clip_plane, &pt_origin, &vn, 1);

		//��ü���
		VECTOR3D_INITXYZ(&vn, -1, 0, -1);
		PLANE3D_Init(&cam->lt_clip_plane, &pt_origin, &vn, 1);

		//�ϲü���
		VECTOR3D_INITXYZ(&vn, 0, 1, -1);
		PLANE3D_Init(&cam->tp_clip_plane, &pt_origin, &vn, 1);

		//�²ü���
		VECTOR3D_INITXYZ(&vn, 0, -1, -1);
		PLANE3D_Init(&cam->bt_clip_plane, &pt_origin, &vn, 1);
	}
	else
	{
		//����fov��Ϊ90��ʱ�Ĳü���
		POINT3D pt_origin; //
		VECTOR3D_INITXYZ(&pt_origin, 0, 0, 0);

		VECTOR3D vn; //�淨��

		//����fov��Ϊ90�ȣ����ȼ����ʾ�ü�����x-z��y-z�ϵ�2DͶӰ
		//Ȼ�������������������ֱ�������������ǲü���ķ���
		VECTOR3D_INITXYZ(&vn, cam->view_dist, 0, -cam->viewplane_width / 2.0);
		PLANE3D_Init(&cam->rt_clip_plane, &pt_origin, &vn, 1);

		//��ü��棬������Z�ᷴ���Ҳü���ķ��ߣ����õ���ü���ķ���
		VECTOR3D_INITXYZ(&vn, -cam->view_dist, 0, -cam->viewplane_width / 2.0);
		PLANE3D_Init(&cam->lt_clip_plane, &pt_origin, &vn, 1);

		//�ϲü���
		VECTOR3D_INITXYZ(&vn, 0, cam->view_dist, -cam->viewplane_width / 2.0);
		PLANE3D_Init(&cam->tp_clip_plane, &pt_origin, &vn, 1);

		//�²ü���
		VECTOR3D_INITXYZ(&vn, 0, -cam->view_dist, -cam->viewplane_width / 2.0);
		PLANE3D_Init(&cam->bt_clip_plane, &pt_origin, &vn, 1);
	}
}

void Build_CAM4DV1_Matrix_Euler(CAM4DV1_PTR cam, int cam_rot_seq)
{
	MATRIX4X4 mt_inv,	//���ƽ�ƾ���������
		mx_inv,			//�����x�����ת����������
		my_inv,			//�����y�����ת����������
		mz_inv,			//�����z�����ת����������
		mrot,			//��������ת����Ļ�
		mtmp;			//���ڴ洢��ʱ����

	//���������λ�ü�������ƽ�ƾ���������
	Mat_Init_4X4(&mt_inv, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-cam->pos.x, -cam->pos.y, -cam->pos.z, 1);

	//������ת����������
	//��ȡŷ���Ƕ�
	float theta_x = cam->dir.x;
	float theta_y = cam->dir.y;
	float theta_z = cam->dir.z;

	//����Ƕ�x�����Һ�����
	float cos_theta = Fast_Cos(theta_x);
	float sin_theta = -Fast_Sin(theta_x);

	//��������
	Mat_Init_4X4(&mx_inv, 1, 0, 0, 0,
		0, cos_theta, sin_theta, 0,
		0, -sin_theta, cos_theta, 0,
		0, 0, 0, 1);

	//������Ƕ�y�����Һ�����
	cos_theta = Fast_Cos(theta_y);
	sin_theta = -Fast_Sin(theta_y);
	
	//��������
	Mat_Init_4X4(&my_inv, cos_theta, 0, -sin_theta, 0,
		0, 1, 0, 0,
		sin_theta, 0, cos_theta, 0,
		0, 0, 0, 1);

	//����Ƕ�z�����Һ�����
	cos_theta = Fast_Cos(theta_z);
	sin_theta = -Fast_Sin(theta_z);

	//��������
	Mat_Init_4X4(&mz_inv, cos_theta, sin_theta, 0, 0,
		-sin_theta, cos_theta, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);

	//���ڼ�������ת����ĳ˻�
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