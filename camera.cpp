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
	MATRIX4X4 mt_inv,	//�����ƽ�ƾ���
		mt_uvn,			//uvn����任����
		mtmp;			//���ڴ洢��ʱ����

	Mat_Init_4X4(&mt_inv, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-cam->pos.x, -cam->pos.y, -cam->pos.z, 1);

	if (mode == UVN_MODE_SPHERICAL){
		//ʹ����������ģʽ

		float phi = cam->dir.x;		//����
		float theta = cam->dir.y;	//��λ��

		float sin_phi = Fast_Sin(phi);
		float cos_phi = Fast_Cos(phi);

		float sin_theta = Fast_Sin(theta);
		float cos_theta = Fast_Cos(theta);

		//����Ŀ����ڵ�λ�����ϵ�λ��(x,y,z)
		cam->target.x = -1 * sin_phi*sin_theta;
		cam->target.y = 1 * cos_phi;
		cam->target.z = 1 * sin_phi*cos_theta;
	}

	//��һ����n=<Ŀ��λ�� - �۲�λ��>
	VECTOR4D_Build(&cam->pos, &cam->target, &cam->n);

	//��v����Ϊ<0,1,0>
	VECTOR4D_INITXYZ(&cam->v, 0, 1, 0);

	// u = (v x n)
	VECTOR4D_Cross(&cam->v, &cam->n, &cam->u);

	// v = (n x u)
	VECTOR4D_Cross(&cam->n, &cam->u, &cam->v);

	//��һ��
	VECTOR4D_Normalize(&cam->u);
	VECTOR4D_Normalize(&cam->v);
	VECTOR4D_Normalize(&cam->n);

	Mat_Init_4X4(&mt_uvn, cam->u.x, cam->v.x, cam->n.x, 0,
		cam->u.y, cam->v.y, cam->n.y, 0,
		cam->u.z, cam->v.z, cam->n.z, 0,
		0, 0, 0, 1);

	Mat_Mul_4X4(&mt_inv, &mt_uvn, &cam->mcam);
}

//����һ�����ھ���ĺ��������������������任Ϊ�������
//�����Ƕ���α���ֻ�Ƕ�vlist_trans[]�еĶ�����б任������Ⱦ�б��еĶ���α�ʾ�ļ����嶼ͨ���˱����޳�
void World_To_Camera_OBJECT4DV1(CAM4DV1_PTR cam, OBJECT4DV1_PTR obj)
{
	for (int vertex = 0; vertex < obj->num_vertices; vertex++)
	{
		POINT4D_PTR presult;

		Mat_Mul_VECTOR4D_4X4(&obj->vlist_trans[vertex] ,& cam->mcam, presult);
		
		VECTOR4D_COPY(&obj->vlist_trans[vertex], presult);
	}
}

//�������ˮ�������Ѿ���ÿ������ת��Ϊ����Σ��������ǲ��뵽��Ⱦ�б���
//��ʹ����������������ǻ�������ĺ����Զ�����б任
//������任Ϊ����εĲ������������޳����ֲ��任���ֲ����굽��������任�Լ���������֮����е�
//��������޶ȼ�����ÿ�������б����뵽��Ⱦ�б��еĶ������Ŀ
//������������Ѿ������˾ֲ����굽��������任���Ҷ�������ݴ洢��POLYF4DV1�ı任����б�tvlist��
void World_To_Camera_RENDERLIST4DV1(CAM4DV1_PTR cam, RENDERLIST4DV1_PTR renderlist)
{
	for (int poly = 0; poly < renderlist->num_polys; poly++)
	{
		POLYF4DV1_PTR curr_poly = renderlist->poly_ptrs[poly];
		if (curr_poly == NULL || !(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_BACKFACE) || (curr_poly->state & POLY4DV1_STATE_CLIPPED))
		{
			for (int vertex = 0; vertex < 3; vertex++)
			{
				POINT4D presult;

				Mat_Mul_VECTOR4D_4X4(&curr_poly->tvlist[vertex], &cam->mcam, &presult);

				VECTOR4D_COPY(&curr_poly->tvlist[vertex], &presult);
			}
		}
	}
}

//����������ݴ���������Ϣ�ж������Ƿ����Ӿ�����
//����cull_flagsָ������Щ����ִ���޳�
//��ȡֵΪ�����޳���Ƶ�OR��������屻�޳�������Ӧ��������״̬
int Cull_OBJECT4DV1(OBJECT4DV1_PTR obj, 
	CAM4DV1_PTR cam, 
	int cull_flags)//Ҫ���ǵĲü���
{
	POINT4D sphere_pos;

	//�Ե���б任
	Mat_Mul_VECTOR4D_4X4(&obj->world_pos, &cam->mcam, &sphere_pos);

	if (cull_flags & CULL_OBJECT_Z_PLANE)
	{
		//ʹ��Զ���ü�����в���
		if ((sphere_pos.z - obj->max_raduis > cam->far_clip_z) || (sphere_pos.z + obj->max_raduis < cam->near_clip_z))
		{
			SET_BIT(obj->state, OBJECT4DV1_STATE_CULLED);
			return 1;
		}
	}

	if (cull_flags & CULL_OBJECT_X_PLANE)
	{
		//ֻ�������Ҳü�����������޳���������ʹ��ƽ�淽�̣���ʹ�����������Ƹ�����
		//��Ϊ����һ��2D����

		//ʹ���Ҳü������ü������Χ��������ߺ����ұߵĵ�
		float z_test = 0.5*cam->viewplane_width * sphere_pos.z / cam->view_dist;

		if ((sphere_pos.x - obj->max_raduis > z_test) || (sphere_pos.x + obj->max_raduis < -z_test))
		{
			SET_BIT(obj->state, OBJECT4DV1_STATE_CULLED);
			return 1;
		}
	}

	if (cull_flags & CULL_OBJECT_Y_PLANE)
	{
		float z_test = 0.5*cam->viewplane_height * sphere_pos.z / cam->view_dist;
		if ((sphere_pos.y - obj->max_raduis > z_test) || (sphere_pos.y + obj->max_raduis < -z_test))
		{
			SET_BIT(obj->state, OBJECT4DV1_STATE_CULLED);
			return 1;
		}
	}

	return 0;
}

void Reset_OBJECT4DV1(OBJECT4DV1_PTR obj)
{
	//���������ֲ����������״̬��Ϊ�任��׼��
	//ͨ�������ñ��޳������ü��ͱ���ȱ�ǣ�Ҳ����������������׼������
	RESET_BIT(obj->state, OBJECT4DV1_STATE_CULLED);

	//���ö���εı��õ��ͱ�����
	for (int poly = 0; poly < obj->num_polys; poly++)
	{
		POLY4DV1_PTR curr_poly = &obj->plist[poly];

		if (!(curr_poly->state & POLY4DV1_STATE_ACTIVE))
			continue;

		RESET_BIT(curr_poly->state, POLY4DV1_STATE_CLIPPED);
		RESET_BIT(curr_poly->state, POLY4DV1_STATE_BACKFACE);
	}
}

//�����������vlist_trans�еĶ����Լ����λ�ã���������ı������Σ�����ֻ���ö���εı���״̬
void Remove_Backface_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam)
{
	//��������Ƿ��޳�
	if (obj->state & OBJECT4DV1_STATE_CULLED)
	{
		return;
	}

	for (int poly = 0; poly < obj->num_polys; poly++)
	{
		POLY4DV1_PTR curr_poly = &obj->plist[poly];

		//�ж϶�����Ƿ��б��ż�����û�б��޳������ڻ״̬���ɼ��Ҳ���˫���
		if (!(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_BACKFACE)
			|| (curr_poly->state & POLY4DV1_STATE_CLIPPED) || (curr_poly->state & POLY4DV1_ATTR_2SIDED))
			continue;

		int vindex_0 = curr_poly->vert[0];
		int vindex_1 = curr_poly->vert[1];
		int vindex_2 = curr_poly->vert[2];

		//�������ε��淨��
		VECTOR4D u, v, n;

		VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_1], &u);
		VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_2], &v);

		//������
		VECTOR4D_Cross(&u, &v, &n);

		//����ָ���ӵ������
		VECTOR4D view;
		VECTOR4D_Build(&obj->vlist_trans[vindex_0], &cam->pos, &view);

		//������
		float dp = VECTOR4D_Dot(&n, &view);
		if (dp <= 0.0)
		{
			SET_BIT(curr_poly->state, POLY4DV1_STATE_BACKFACE);
		}
	}
}

void Remove_Backface_RENDERLIST4DV1(RENDERLIST4DV1_PTR render_list, CAM4DV1_PTR cam)
{
	for (int poly = 0; poly < render_list->num_polys; poly++)
	{
		POLYF4DV1_PTR curr_poly = render_list->poly_ptrs[poly];

		//�ж϶�����Ƿ��б��ż�����û�б��޳������ڻ״̬���ɼ��Ҳ���˫���
		if (!(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_BACKFACE)
			|| (curr_poly->state & POLY4DV1_STATE_CLIPPED) || (curr_poly->state & POLY4DV1_ATTR_2SIDED))
			continue;

		//�������ε��淨��
		VECTOR4D u, v, n;

		VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[1], &u);
		VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[2], &v);

		//������
		VECTOR4D_Cross(&u, &v, &n);

		//����ָ���ӵ������
		VECTOR4D view;
		VECTOR4D_Build(&curr_poly->tvlist[0], &cam->pos, &view);

		//������
		float dp = VECTOR4D_Dot(&n, &view);
		if (dp <= 0.0)
		{
			SET_BIT(curr_poly->state, POLY4DV1_STATE_BACKFACE);
		}
	}
}

//����������ݴ�����������������������ת��Ϊ͸������
//�����Ķ���α�����ֻ�Ƕ�vlist_trans[]�еĶ�����б任
void Camera_To_Perspective_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam)
{
	for (int vertex = 0; vertex < obj->num_vertices; vertex++)
	{
		//��������Ĺ۲�����Զ�����б任
		float z = obj->vlist_trans[vertex].z;
		obj->vlist_trans[vertex].x = cam->view_dist_h*obj->vlist_trans[vertex].x / z;
		obj->vlist_trans[vertex].y = cam->view_dist_v*obj->vlist_trans[vertex].y * cam->aspect_ratio / z;

		//z���겻��
	}
}

void Build_Camera_To_Perspective_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m)
{
	Mat_Init_4X4(m,
		cam->view_dist_h, 0, 0, 0,
		0, cam->view_dist_v*cam->aspect_ratio, 0, 0,
		0, 0, 1, 1,
		0, 0, 0, 0);
}

//����������任��Ķ����б��е����ж����4D�������ת��Ϊ3D����
//�����ǽ�����x��y��z������w
void Convert_From_Homogeneous4D_OBJECT4DV1(OBJECT4DV1_PTR obj)
{
	for (int vertex = 0; vertex < obj->num_vertices; vertex++)
	{
		VECTOR4D_DIV_BY_W(&obj->vlist_trans[vertex]);
	}
}

void Camera_To_Perspective_RENDERLIST4DV1(RENDERLIST4DV1_PTR render_list, CAM4DV1_PTR cam)
{
	for (int poly = 0; poly < render_list->num_polys; poly++)
	{
		POLYF4DV1_PTR curr_poly = render_list->poly_ptrs[poly];
		if ((curr_poly == NULL) || !(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_BACKFACE)
			|| (curr_poly->state & POLY4DV1_STATE_CLIPPED))
			continue;

		for (int vertex = 0; vertex < 3; vertex++)
		{
			float z = curr_poly->tvlist[vertex].z;

			curr_poly->tvlist[vertex].x = cam->view_dist_h*curr_poly->tvlist[vertex].x / z;
			curr_poly->tvlist[vertex].y = cam->view_dist_v*curr_poly->tvlist[vertex].y * cam->aspect_ratio / z;
		}
	}
}

//����������任��Ķ����б���������Ч�Ķ���ζ����4D�������ת��Ϊ3D����
void Convert_From_Homogeneous4D_RENDERLIST4DV1(RENDERLIST4DV1_PTR render_list)
{
	for (int poly = 0; poly < render_list->num_polys; poly++)
	{
		POLYF4DV1_PTR curr_poly = render_list->poly_ptrs[poly];
		if ((curr_poly == NULL) || !(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_BACKFACE)
			|| (curr_poly->state & POLY4DV1_STATE_CLIPPED))
			continue;

		for (int vertex = 0; vertex < 3; vertex++)
		{
			//ת��Ϊ�������
			VECTOR4D_DIV_BY_W(&curr_poly->tvlist[vertex]);
		}
	}
}


void Perspective_To_Screen_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam)
{
	float alpha = 0.5*cam->viewport_width - 0.5;
	float beta = 0.5*cam->viewport_height - 0.5;

	for (int vertex = 0; vertex < obj->num_vertices; vertex++)
	{
		obj->vlist_trans[vertex].x = alpha + alpha*obj->vlist_trans[vertex].x;
		obj->vlist_trans[vertex].y = beta - beta*obj->vlist_trans[vertex].y;
	}
}

//�����������͸�����굽��Ļ����任��������ٶ�Ҫ�����������б任�����ڹ�դ���ڼ�ִ��4D������굽3D����ת��
//��Ȼִ��2D��ȾʱֻҪ���ǵ��x�����y���꣬������������ľ������ڶ�͸������������ź�ƽ�ƣ�����任Ϊ��Ļ����
void Build_Perspective_To_Screen_4D_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m)
{
	float alpha = 0.5*cam->viewport_width - 0.5;
	float beta = 0.5*cam->viewport_height - 0.5;

	Mat_Init_4X4(m, alpha, 0, 0, 0,
		0, -beta, 0, 0,
		alpha, beta, 1, 0,
		0, 0, 0, 1);
}

//�����������͸�����굽��Ļ����任�����������͸�������Ѿ���4D�������ת��Ϊ3D����
//������������ľ������ڶ�͸������������ź�ƽ�ƣ�����任Ϊ��Ļ����
//��ǰһ��������Ψһ������ڣ���������һ��û�н�w����Ϊz
void Build_Perspective_To_Screen_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m)
{
	float alpha = 0.5*cam->viewport_width - 0.5;
	float beta = 0.5*cam->viewport_height - 0.5;

	Mat_Init_4X4(m, alpha, 0, 0, 0,
		0, -beta, 0, 0,
		alpha, beta, 1, 0,
		0, 0, 0, 1);
}

void Perspective_To_Screen_RENDERLIST4DV1(RENDERLIST4DV1_PTR render_list, CAM4DV1_PTR cam)
{
	for (int poly = 0; poly < render_list->num_polys; poly++)
	{
		POLYF4DV1_PTR curr_poly = render_list->poly_ptrs[poly];
		if ((curr_poly == NULL) || !(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_BACKFACE)
			|| (curr_poly->state & POLY4DV1_STATE_CLIPPED))
			continue;

		float alpha = 0.5*cam->viewport_width - 0.5;
		float beta = 0.5*cam->viewport_height - 0.5;
		for (int vertex = 0; vertex < 3; vertex++)
		{
			curr_poly->tvlist[vertex].x = alpha + alpha*curr_poly->tvlist[vertex].x;
			curr_poly->tvlist[vertex].y = beta - beta*curr_poly->tvlist[vertex].y;
		}
	}
}