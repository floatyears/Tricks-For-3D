#include <stddef.h>
#include "poly.h"
#include "t3dlib4.h"

void Transform_RENDERLIST4DV1(RENDERLIST4DV1_PTR render_list, MATRIX4X4_PTR mt, int coord_select){
	switch (coord_select)
	{
	//Ӧ���ĸ������е�������б任
	case TRANSFORM_LOCAL_ONLY:
		for (int poly = 0; poly < render_list->num_polys; poly++)
		{
			POLYF4DV1_PTR curr_poly = render_list->poly_ptrs[poly];
			if (curr_poly == NULL || !(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_CLIPPED) || (curr_poly->state & POLY4DV1_STATE_BACKFACE)){
				continue;
			}
			for (int vertex = 0; vertex < 3; vertex++)
			{
				POINT4D presult; //������ʱ�洢�任���
				//�Զ�����б任
				Mat_Mul_VECTOR4D_4X4(&curr_poly->vlist[vertex], mt, &presult);
				//���ṹ���ȥ
				VECTOR4D_COPY(&curr_poly->vlist[vertex], &presult);
			}
		}
		break;
	case TRANSFORM_TRANS_ONLY:
		//����Ⱦ�б���ÿ���任��Ķ�����б任
		//����tvlist[]���ڴ洢�ۻ��任���
		for (int poly = 0; poly < render_list->num_polys; poly++)
		{
			POLYF4DV1_PTR curr_poly = render_list->poly_ptrs[poly];
			if (curr_poly == NULL || !(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_CLIPPED) || (curr_poly->state & POLY4DV1_STATE_BACKFACE)){
				continue;
			}
			for (int vertex = 0; vertex < 3; vertex++)
			{
				POINT4D presult; //������ʱ�洢�任���
				//�Զ�����б任
				Mat_Mul_VECTOR4D_4X4(&curr_poly->tvlist[vertex], mt, &presult);
				//���ṹ���ȥ
				VECTOR4D_COPY(&curr_poly->tvlist[vertex], &presult);
			}
		}
		break;
	case TRANSFORM_LOCAL_TO_TRANS:
		//����Ⱦ�б��еľֲ�/ģ�Ͷ����б���б任
		//��������洢���任��Ķ����б���
		for (int poly = 0; poly < render_list->num_polys; poly++)
		{
			POLYF4DV1_PTR curr_poly = render_list->poly_ptrs[poly];
			if (curr_poly == NULL || !(curr_poly->state & POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_CLIPPED) || (curr_poly->state & POLY4DV1_STATE_BACKFACE)){
				continue;
			}
			for (int vertex = 0; vertex < 3; vertex++)
			{
				//ʹ�þ���mt�Զ�����б任
				Mat_Mul_VECTOR4D_4X4(&curr_poly->vlist[vertex], mt, &curr_poly->tvlist[vertex]);
			}
		}
		break;
	default:
		break;
	}
}

//
void Transform_OBJECT4DV1(OBJECT4DV1_PTR obj, 
	MATRIX4X4_PTR mt, 
	int coord_select,  //ָ��������������б任
	int transform_basis) //ָ���Ƿ�Ҫ�Գ����������б任
{
	switch (coord_select)
	{
	case TRANSFORM_LOCAL_ONLY:
		for (int vertex = 0; vertex < obj->num_vertices; vertex++)
		{
			POINT4D presult;
			Mat_Mul_VECTOR4D_4X4(&obj->vlist_local[vertex],mt, &presult);
			VECTOR4D_COPY(&obj->vlist_local[vertex], &presult);
		}
		break;
	case TRANSFORM_TRANS_ONLY:
		for (int vertex = 0; vertex < obj->num_vertices; vertex++)
		{
			POINT4D presult;
			Mat_Mul_VECTOR4D_4X4(&obj->vlist_trans[vertex], mt, &presult);
			VECTOR4D_COPY(&obj->vlist_trans[vertex], &presult);
		}
		break;
	case TRANSFORM_LOCAL_TO_TRANS:
		for (int vertex = 0; vertex < obj->num_vertices; vertex++)
		{
			POINT4D presult;
			Mat_Mul_VECTOR4D_4X4(&obj->vlist_trans[vertex], mt, &obj->vlist_local[vertex]);
		}
		break;
	default:
		break;
	}
	if (transform_basis){
		VECTOR4D vresult;
		Mat_Mul_VECTOR4D_4X4(&obj->ux, mt, &vresult);
		VECTOR4D_COPY(&obj->ux, &vresult);

		Mat_Mul_VECTOR4D_4X4(&obj->uy, mt, &vresult);
		VECTOR4D_COPY(&obj->uy, &vresult);

		Mat_Mul_VECTOR4D_4X4(&obj->uz, mt, &vresult);
		VECTOR4D_COPY(&obj->uz, &vresult);
	}
}

void Model_To_World_OBJECT4DV1(OBJECT4DV1_PTR obj, int coord_select = TRANSFORM_LOCAL_TO_TRANS)
{
	//����û���þ���ֱ���ô��ݽ���������ľֲ�/ģ������任Ϊ�������꣬������洢�ڱ任��Ķ����б���
	if (coord_select == TRANSFORM_LOCAL_TO_TRANS)
	{
		//�Զ������ƽ��
		for (int vertex = 0; vertex < obj->num_vertices; vertex++)
		{
			VECTOR4D_Add(&obj->vlist_local[vertex], &obj->world_pos, &obj->vlist_trans[vertex]);
		}
	}
	else //trans only
	{
		for (int vertex = 0; vertex < obj->num_vertices; vertex++)
		{
			VECTOR4D_Add(&obj->vlist_trans[vertex], &obj->world_pos, &obj->vlist_trans[vertex]);
		}
	}
}

void Build_Model_To_World_MATRIX4X4(VECTOR4D_PTR vpos, MATRIX4X4_PTR m)
{
	//�����ֲ����굽��������任����
	Mat_Init_4X4(m, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		vpos->x, vpos->y, vpos->z, 1);
}

void Model_To_World_RENDERLIST4DV1(RENDERLIST4DV1_PTR render_list, VECTOR4D_PTR world_pos, int coord_select = TRANSFORM_LOCAL_TO_TRANS)
{
	if (coord_select == TRANSFORM_LOCAL_TO_TRANS)
	{
		for (int poly = 0; poly < render_list->num_polys; poly++)
		{
			POLYF4DV1_PTR curr_poly = render_list->poly_ptrs[poly];
			if (curr_poly == NULL || !(curr_poly->state == POLY4DV1_STATE_ACTIVE) || curr_poly->state == POLY4DV1_STATE_CLIPPED || curr_poly->state == POLY4DV1_STATE_BACKFACE)
			{
				continue;
			}

			for (int vertex = 0; vertex < 3; vertex++)
			{
				VECTOR4D_Add(&curr_poly->vlist[vertex], world_pos, &curr_poly->tvlist[vertex]);
			}
		}
	}
	else // trans only
	{
		for (int poly = 0; poly < render_list->num_polys; poly++)
		{
			POLYF4DV1_PTR curr_poly = render_list->poly_ptrs[poly];
			if (curr_poly == NULL || !(curr_poly->state == POLY4DV1_STATE_ACTIVE) || curr_poly->state == POLY4DV1_STATE_CLIPPED || curr_poly->state == POLY4DV1_STATE_BACKFACE)
			{
				continue;
			}

			for (int vertex = 0; vertex < 3; vertex++)
			{
				VECTOR4D_Add(&curr_poly->tvlist[vertex], world_pos, &curr_poly->tvlist[vertex]);
			}
		}
	}
}

void Build_Model_To_World_RENDERLIST4DV1(VECTOR4D_PTR vpos, MATRIX4X4_PTR m)
{
	Mat_Init_4X4(m, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		vpos->x, vpos->y, vpos->z, 1);
}

int Insert_POLY4DV1_RENDERLIST4DV1(RENDERLIST4DV1_PTR render_list, POLY4DV1_PTR obj)
{
	return -1;
}

int Insert_OBJECT4DV1_RENDERLIST4DV2(RENDERLIST4DV1_PTR render_list,
	OBJECT4DV1_PTR obj,
	int insert_local = 0,
	int lighting_on = 0)
{
	if (!(obj->state & OBJECT4DV1_STATE_ACTIVE) || (obj->state & OBJECT4DV1_STATE_CULLED) || !(obj->state & OBJECT4DV1_STATE_VISIBLE))
		return 0;

	for (int poly = 0; poly < obj->num_polys; poly++)
	{
		POLY4DV1_PTR curr_poly = &obj->plist[poly];

		//������Ƿ�ɼ�
		if (!(curr_poly->state || POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_CLIPPED) || (curr_poly->state & POLY4DV1_STATE_BACKFACE))
		{
			continue;
		}

		//���Ҫʹ�þֲ����꣬��ı�����ָ��Ķ����б�
		//���ȱ���ԭ����ָ��
		POINT4D_PTR vlist_old = curr_poly->vlist;
		if (insert_local)
			curr_poly->vlist = obj->vlist_local;
		else
		{
			curr_poly->vlist = obj->vlist_trans;
		}

		unsigned int base_color = 0;
		//�ж��Ƿ���Ҫǰ16λ�е���ɫ������ԭ������ɫ
		if (lighting_on)
		{
			base_color = (unsigned int)(curr_poly->color);
			curr_poly->color = (int)(base_color >> 16);
		}

		if (!Insert_POLY4DV1_RENDERLIST4DV1(render_list, curr_poly))
		{
			//�ָ������б�ָ��
			curr_poly->vlist = vlist_old;

			//����ʧ��
			return 0;
		}

		if (lighting_on)
		{
			//�ָ���ɫ
			curr_poly->color = (int)base_color;
		}
		//�ָ������б�ָ��
		curr_poly->vlist = vlist_old;
	}
	return 1;
}

//�Զ�����б������������Zֵ������С��Zֵ����ƽ��Zֵ
void Sort_RENDERLIST4DV1(RENDERLIST4DV1_PTR render_list, int sort_method)
{
	switch (sort_method)
	{
	case SORT_POLYLIST_AVGZ:
		qsort((void *)render_list->poly_ptrs, render_list->num_polys, sizeof(POLYF4DV1_PTR), Compare_AvgZ_POLYLIST4DV1);
		break;
	case SORT_POLYLIST_NEARZ:
		qsort((void *)render_list->poly_ptrs, render_list->num_polys, sizeof(POLYF4DV1_PTR), Compare_NearZ_POLYLIST4DV1);
		break;
	case SORT_POLYLIST_FARZ:
		qsort((void *)render_list->poly_ptrs, render_list->num_polys, sizeof(POLYF4DV1_PTR), Compare_FarZ_POLYLIST4DV1);
		break;
	default:
		break;
	}
}

int Compare_AvgZ_POLYLIST4DV1(const void *arg1, const void *arg2)
{
	float z1, z2;
	POLYF4DV1_PTR poly1, poly2;
	poly1 = *((POLYF4DV1_PTR *)(arg1));
	poly2 = *((POLYF4DV1_PTR *)(arg2));

	z1 = 0.33333 * (poly1->tvlist[0].z + poly1->tvlist[1].z + poly1->tvlist[2].z);
	z2 = 0.33333 * (poly2->tvlist[0].z + poly2->tvlist[1].z + poly2->tvlist[2].z);
	if (z1 > z2) return -1;
	else if (z1 == z2) return 0;
	else if (z1 < z2) return 1;
}

int Compare_NearZ_POLYLIST4DV1(const void *arg1, const void *arg2)
{
	float z1, z2;
	POLYF4DV1_PTR poly1, poly2;
	poly1 = *((POLYF4DV1_PTR *)(arg1));
	poly2 = *((POLYF4DV1_PTR *)(arg2));

	if (poly1->tvlist[0].z > poly1->tvlist[1].z)
	{
		if (poly1->tvlist[2].z > poly1->tvlist[1].z)
		{
			z1 = poly1->tvlist[1].z;
		}
		else
		{
			z1 = poly1->tvlist[2].z;
		}
	}
	else
	{
		if (poly1->tvlist[2].z > poly1->tvlist[0].z)
		{
			z1 = poly1->tvlist[0].z;
		}
		else
		{
			z1 = poly1->tvlist[2].z;
		}
	}
	if (z1 > z2) return -1;
	else if (z1 == z2) return 0;
	else if (z1 < z2) return 1;
}

int Compare_FarZ_POLYLIST4DV1(const void *arg1, const void *arg2)
{
	float z1, z2;
	POLYF4DV1_PTR poly1, poly2;
	poly1 = *((POLYF4DV1_PTR *)(arg1));
	poly2 = *((POLYF4DV1_PTR *)(arg2));

	if (poly1->tvlist[0].z < poly1->tvlist[1].z)
	{
		if (poly1->tvlist[2].z < poly1->tvlist[1].z)
		{
			z1 = poly1->tvlist[1].z;
		}
		else
		{
			z1 = poly1->tvlist[2].z;
		}
	}
	else
	{
		if (poly1->tvlist[2].z < poly1->tvlist[0].z)
		{
			z1 = poly1->tvlist[0].z;
		}
		else
		{
			z1 = poly1->tvlist[2].z;
		}
	}

	if (z1 > z2) return -1;
	else if (z1 == z2) return 0;
	else if (z1 < z2) return 1;
}