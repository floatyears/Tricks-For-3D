#include <filesystem>
#include "resource.h"
#include "t3dlib4.h"
#include "poly.h"

char *Get_Line_PLG(char *buffer, int maxlength, FILE *fp)
{
	int index = 0;
	int length = 0;

	while (1)
	{
		//��ȡ��һ��
		if (!fgets(buffer, maxlength, fp))
			return NULL;
		//����ո���
		for (length = strlen(buffer), index = 0; isspace(buffer[index]); index++);

		//����Ƿ��ǿ��л���ע��
		if (index >= length || buffer[index] == '#')
			continue;
		return &buffer[index];
	}
}

//
int Load_OBJECT4DV1_PLG(OBJECT4DV1_PTR obj, char *filename, VECTOR4D_PTR scale, VECTOR4D_PTR pos, VECTOR4D_PTR rot)
{
	FILE *fp;			//�ļ�ָ��
	char buffer[256];	//������

	char *token_string; //ָ��Ҫ���������������ı���ָ��

	//��պͳ�ʼ��obj
	memset(obj, 0, sizeof(OBJECT4DV1));

	obj->state = OBJECT4DV1_STATE_ACTIVE | OBJECT4DV1_STATE_VISIBLE;

	obj->world_pos.x = pos->x;
	obj->world_pos.y = pos->y;
	obj->world_pos.z = pos->z;
	obj->world_pos.w = pos->w;

	if (!(fp = fopen(filename, "r"))){
		printf("Couldn't open PLG file %s. ", filename);
		return 0;
	}

	if (!(token_string = Get_Line_PLG(buffer, 255, fp))){
		printf("PLG file error with file %s (object descriptor invalid). ", filename);
		return 0;
	}
	printf("Object descriptor: %s", token_string);

	//��������
	sscanf(token_string, "%s %d %d", obj->name, &obj->num_vertices, &obj->num_polys);
	//�����б�
	for (int vertex = 0; vertex < obj->num_vertices; vertex++)
	{
		if (!(token_string = Get_Line_PLG(buffer, 255, fp))){
			printf("PLG file error with file %s (vertex list invalid). ", filename);
			return 0;
		}
		sscanf(token_string, "%f %f %f", &obj->vlist_local[vertex].x, &obj->vlist_local[vertex].y, &obj->vlist_local[vertex].z);
		obj->vlist_local[vertex].w = 1;
		
		//���Ŷ�������
		obj->vlist_local[vertex].x *= scale->x;
		obj->vlist_local[vertex].y *= scale->y;
		obj->vlist_local[vertex].z *= scale->z;

		printf("\nVertex %d = %f, %f, %f, %f", vertex, obj->vlist_local[vertex].x, obj->vlist_local[vertex].y, obj->vlist_local[vertex].z, obj->vlist_local[vertex].w);
	}

	//����ƽ���뾶�����뾶
	Compute_OBJECT4DV1_Radius(obj);
	printf("\nObject average radius = %f, max radius = %f", obj->avg_radius, obj->max_raduis);

	int poly_surface_desc = 0;
	int poly_num_verts = 0;
	char tmp_string[8];

	//������б�
	for (int poly = 0; poly < obj->num_polys; poly++)
	{
		//���������
		if (!(token_string = Get_Line_PLG(buffer, 255, fp))){
			printf("PLG file error with file %s (polygon descriptor invalid). ", filename);
			return 0;
		}
		printf("\n Pologon %d:", poly);

		sscanf(token_string, "%s %d %d %d %d", tmp_string, &poly_num_verts, &obj->plist[poly].vert[0], &obj->plist[poly].vert[1], &obj->plist[poly].vert[2]);

		//��������������ʮ������ֵ
		//�����Ҫ������������м��
		if (tmp_string[0] == '0' && toupper(tmp_string[1]) == 'X')
			sscanf(tmp_string, "%x", &poly_surface_desc);
		else
			poly_surface_desc = atoi(tmp_string);

		//�ö���ζ����б�ָ������Ķ����б����Ƕ���ģ���Ϊ������б�����������У�
		//��������μ�����ʱ�����û�����ʹ�þֲ������б��Ǳ任��Ķ����б�
		//������ж���ζ�����ͬһ�����壬��ָ������ΪNULL������
		obj->plist[poly].vlist = obj->vlist_local;
		printf("\nSurface Desc = 0x%.4x, num_verts = %d, vert_indices [%d, %d, %d]", poly_surface_desc, poly_num_verts, obj->plist[poly].vert[0], obj->plist[poly].vert[1], obj->plist[poly].vert[2]);
		//�洢�����б�Ͷ���ζ�������ֵ֮��
		//��������������������ݴ���Ӧ�����ö����
		
		//��ȡ������������е�ÿ��λ�ֶ�
		//�ӵ���/˫��λ��ʼ
		if (poly_surface_desc & PLX_COLOR_MODE_RGB_FLAG){
			SET_BIT(obj->plist[poly].attr, POLY4DV1_ATTR_2SIDED);
			printf("\n2 sided. ");
		}
		else
		{
			printf("\n1 sided. ");
		}

		//������ɫģʽ
		if (poly_surface_desc & PLX_COLOR_MODE_RGB_FLAG){
			//ΪRGB��ɫģʽ
			SET_BIT(obj->plist[poly].attr, POLY4DV1_ATTR_RGB16);
			//��ȡRGB��ɫ
			int red = (poly_surface_desc & 0x0f00) >> 8;
			int green = (poly_surface_desc & 0x00f0) >> 4;
			int blue = (poly_surface_desc & 0x000f);

			//�ļ�����ɫ����Ϊ4.4.4��ʽ����ͼ�ο���RGB��ɫ��ʽ��5.5.5��5.6.5����������ϵͳ��8.8.8��ɫ��ʽת��Ϊ5.5.5��5.6.5
			//�����Ҫ��4.4.4��RGBת��Ϊ8.8.8
			obj->plist[poly].color = _RGB16BIT555(red * 16, green * 16, blue * 16);
			printf("\nRGB color = {%d, %d, %d}", red, green, blue);
		}
		else
		{
			SET_BIT(obj->plist[poly].attr, POLY4DV1_ATTR_8BITCOLOR);
			obj->plist[poly].color = poly_surface_desc & 0x00ff;
			printf("\n8-bit color index=%d", obj->plist[poly].color);
		}

		//������ɫģʽ
		int shade_mode = poly_surface_desc & PLX_SHADE_MODE_MASK;
		switch (shade_mode)
		{
		case PLX_SHADE_MODE_PURE_FLAG:
			SET_BIT(obj->plist[poly].attr, POLY4DV1_ATTR_SHADE_MODE_PURE);
			printf("\nShade Mode = pure");
			break;
		case PLX_SHADE_MODE_FLAT_FLAG:
			SET_BIT(obj->plist[poly].attr, POLY4DV1_ATTR_SHADE_MODE_PURE);
			printf("\nShade Mode = flat");

			break;
		case PLX_SHADE_MODE_GOURAUD_FLAG:
			SET_BIT(obj->plist[poly].attr, POLY4DV1_ATTR_SHADE_MODE_PURE);
			printf("\nShade Mode = gouraud");
			break;
		case PLX_SHADE_MODE_PHONG_FLAG:
			SET_BIT(obj->plist[poly].attr, POLY4DV1_ATTR_SHADE_MODE_PURE);
			printf("\nShade Mode = phong");
			break;
		default:
			break;
		}

		obj->plist[poly].state = PLOY4DV1_STATE_ACTIVE;
	}

	fclose(fp);
	return 1;
}