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
	MATRIX4X4 mt_inv,	//逆相机平移矩阵
		mt_uvn,			//uvn相机变换矩阵
		mtmp;			//用于存储临时矩阵

	Mat_Init_4X4(&mt_inv, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-cam->pos.x, -cam->pos.y, -cam->pos.z, 1);

	if (mode == UVN_MODE_SPHERICAL){
		//使用球面坐标模式

		float phi = cam->dir.x;		//仰角
		float theta = cam->dir.y;	//方位角

		float sin_phi = Fast_Sin(phi);
		float cos_phi = Fast_Cos(phi);

		float sin_theta = Fast_Sin(theta);
		float cos_theta = Fast_Cos(theta);

		//计算目标点在单位球面上的位置(x,y,z)
		cam->target.x = -1 * sin_phi*sin_theta;
		cam->target.y = 1 * cos_phi;
		cam->target.z = 1 * sin_phi*cos_theta;
	}

	//第一步：n=<目标位置 - 观察位置>
	VECTOR4D_Build(&cam->pos, &cam->target, &cam->n);

	//将v设置为<0,1,0>
	VECTOR4D_INITXYZ(&cam->v, 0, 1, 0);

	// u = (v x n)
	VECTOR4D_Cross(&cam->v, &cam->n, &cam->u);

	// v = (n x u)
	VECTOR4D_Cross(&cam->n, &cam->u, &cam->v);

	//归一化
	VECTOR4D_Normalize(&cam->u);
	VECTOR4D_Normalize(&cam->v);
	VECTOR4D_Normalize(&cam->n);

	Mat_Init_4X4(&mt_uvn, cam->u.x, cam->v.x, cam->n.x, 0,
		cam->u.y, cam->v.y, cam->n.y, 0,
		cam->u.z, cam->v.z, cam->n.z, 0,
		0, 0, 0, 1);

	Mat_Mul_4X4(&mt_inv, &mt_uvn, &cam->mcam);
}

//这是一个基于矩阵的函数，将物体的世界坐标变换为相机坐标
//不考虑多边形本身，只是对vlist_trans[]中的顶点进行变换，而渲染列表中的多边形表示的几何体都通过了背面剔除
void World_To_Camera_OBJECT4DV1(CAM4DV1_PTR cam, OBJECT4DV1_PTR obj)
{
	for (int vertex = 0; vertex < obj->num_vertices; vertex++)
	{
		POINT4D_PTR presult;

		Mat_Mul_VECTOR4D_4X4(&obj->vlist_trans[vertex] ,& cam->mcam, presult);
		
		VECTOR4D_COPY(&obj->vlist_trans[vertex], presult);
	}
}

//如果在流水线上游已经将每个物体转换为多边形，并将它们插入到渲染列表中
//将使用这个函数，而不是基于物体的函数对顶点进行变换
//将物体变换为多边形的操作是在物体剔除、局部变换、局部坐标到世界坐标变换以及背面消除之后进行的
//这样最大限度减少了每个物体中被插入到渲染列表中的多边形数目
//这个函数至少已经进行了局部坐标到世界坐标变换，且多边形数据存储在POLYF4DV1的变换后的列表tvlist中
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

//这个函数根据传入的相机信息判断物体是否在视景体内
//参数cull_flags指定在哪些轴上执行剔除
//其取值为各种剔除表计的OR，如果物体被剔除，将相应地设置其状态
int Cull_OBJECT4DV1(OBJECT4DV1_PTR obj, 
	CAM4DV1_PTR cam, 
	int cull_flags)//要考虑的裁剪面
{
	POINT4D sphere_pos;

	//对点进行变换
	Mat_Mul_VECTOR4D_4X4(&obj->world_pos, &cam->mcam, &sphere_pos);

	if (cull_flags & CULL_OBJECT_Z_PLANE)
	{
		//使用远近裁剪面进行测试
		if ((sphere_pos.z - obj->max_raduis > cam->far_clip_z) || (sphere_pos.z + obj->max_raduis < cam->near_clip_z))
		{
			SET_BIT(obj->state, OBJECT4DV1_STATE_CULLED);
			return 1;
		}
	}

	if (cull_flags & CULL_OBJECT_X_PLANE)
	{
		//只根据左右裁剪面进行物体剔除，本可以使用平面方程，但使用三角形相似更容易
		//因为这是一种2D问题

		//使用右裁剪面和左裁剪面检测包围球上最左边和最右边的点
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
	//这个函数种植传入的物体的状态，为变换做准备
	//通常是重置被剔除、被裁减和背面等标记，也可以在这里做其他准备工作
	RESET_BIT(obj->state, OBJECT4DV1_STATE_CULLED);

	//重置多边形的被裁掉和背面标记
	for (int poly = 0; poly < obj->num_polys; poly++)
	{
		POLY4DV1_PTR curr_poly = &obj->plist[poly];

		if (!(curr_poly->state & POLY4DV1_STATE_ACTIVE))
			continue;

		RESET_BIT(curr_poly->state, POLY4DV1_STATE_CLIPPED);
		RESET_BIT(curr_poly->state, POLY4DV1_STATE_BACKFACE);
	}
}

//这个函数根据vlist_trans中的顶点以及相机位置，消除物体的背面多边形，这里只设置多边形的背面状态
void Remove_Backface_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam)
{
	//检查物体是否被剔除
	if (obj->state & OBJECT4DV1_STATE_CULLED)
	{
		return;
	}

	for (int poly = 0; poly < obj->num_polys; poly++)
	{
		POLY4DV1_PTR curr_poly = &obj->plist[poly];

		//判断多边形是否有被才减掉、没有被剔除、处于活动状态、可见且不是双面的
		if (!(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_BACKFACE)
			|| (curr_poly->state & POLY4DV1_STATE_CLIPPED) || (curr_poly->state & POLY4DV1_ATTR_2SIDED))
			continue;

		int vindex_0 = curr_poly->vert[0];
		int vindex_1 = curr_poly->vert[1];
		int vindex_2 = curr_poly->vert[2];

		//计算多边形的面法线
		VECTOR4D u, v, n;

		VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_1], &u);
		VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_2], &v);

		//计算叉积
		VECTOR4D_Cross(&u, &v, &n);

		//创建指向视点的向量
		VECTOR4D view;
		VECTOR4D_Build(&obj->vlist_trans[vindex_0], &cam->pos, &view);

		//计算点积
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

		//判断多边形是否有被才减掉、没有被剔除、处于活动状态、可见且不是双面的
		if (!(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_BACKFACE)
			|| (curr_poly->state & POLY4DV1_STATE_CLIPPED) || (curr_poly->state & POLY4DV1_ATTR_2SIDED))
			continue;

		//计算多边形的面法线
		VECTOR4D u, v, n;

		VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[1], &u);
		VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[2], &v);

		//计算叉积
		VECTOR4D_Cross(&u, &v, &n);

		//创建指向视点的向量
		VECTOR4D view;
		VECTOR4D_Build(&curr_poly->tvlist[0], &cam->pos, &view);

		//计算点积
		float dp = VECTOR4D_Dot(&n, &view);
		if (dp <= 0.0)
		{
			SET_BIT(curr_poly->state, POLY4DV1_STATE_BACKFACE);
		}
	}
}

//这个函数根据传入的相机对象将物体的相机坐标转换为透视坐标
//不关心多边形本身，而只是对vlist_trans[]中的顶点进行变换
void Camera_To_Perspective_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam)
{
	for (int vertex = 0; vertex < obj->num_vertices; vertex++)
	{
		//根据相机的观察参数对顶点进行变换
		float z = obj->vlist_trans[vertex].z;
		obj->vlist_trans[vertex].x = cam->view_dist_h*obj->vlist_trans[vertex].x / z;
		obj->vlist_trans[vertex].y = cam->view_dist_v*obj->vlist_trans[vertex].y * cam->aspect_ratio / z;

		//z坐标不变
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

//这个函数将变换后的顶点列表中的所有顶点从4D其次坐标转换为3D坐标
//方法是将分量x、y、z都除以w
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

//这个函数将变换后的顶点列表中所有有效的多边形顶点从4D齐次坐标转换为3D坐标
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
			//转换为齐次坐标
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

//这个函数创建透视坐标到屏幕坐标变换矩阵，这里假定要对齐次坐标进行变换，并在光栅化期间执行4D齐次坐标到3D坐标转换
//当然执行2D渲染时只要考虑点的x坐标和y坐标，这个函数创建的矩阵用于对透视坐标进行缩放和平移，将其变换为屏幕坐标
void Build_Perspective_To_Screen_4D_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m)
{
	float alpha = 0.5*cam->viewport_width - 0.5;
	float beta = 0.5*cam->viewport_height - 0.5;

	Mat_Init_4X4(m, alpha, 0, 0, 0,
		0, -beta, 0, 0,
		alpha, beta, 1, 0,
		0, 0, 0, 1);
}

//这个函数创建透视坐标到屏幕坐标变换矩阵，这里假设透视坐标已经从4D齐次坐标转换为3D坐标
//这个函数创建的矩阵用于对透视坐标进行缩放和平移，将其变换为屏幕坐标
//和前一个函数的唯一差别在于，矩阵的最后一列没有将w设置为z
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