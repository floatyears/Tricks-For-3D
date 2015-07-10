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