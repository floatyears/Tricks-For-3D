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
		//读取下一行
		if (!fgets(buffer, maxlength, fp))
			return NULL;
		//计算空格数
		for (length = strlen(buffer), index = 0; isspace(buffer[index]); index++);

		//检查是否是空行或者注释
		if (index >= length || buffer[index] == '#')
			continue;
		return &buffer[index];
	}
}

//
int Load_OBJECT4DV1_PLG(OBJECT4DV1_PTR obj, char *filename, VECTOR4D_PTR scale, VECTOR4D_PTR pos, VECTOR4D_PTR rot)
{
	FILE *fp;			//文件指针
	char buffer[256];	//缓冲区

	char *token_string; //指向要分析的物体数据文本的指针

	//清空和初始化obj
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

	//物体描述
	sscanf(token_string, "%s %d %d", obj->name, &obj->num_vertices, &obj->num_polys);
	//顶点列表
	for (int vertex = 0; vertex < obj->num_vertices; vertex++)
	{
		if (!(token_string = Get_Line_PLG(buffer, 255, fp))){
			printf("PLG file error with file %s (vertex list invalid). ", filename);
			return 0;
		}
		sscanf(token_string, "%f %f %f", &obj->vlist_local[vertex].x, &obj->vlist_local[vertex].y, &obj->vlist_local[vertex].z);
		obj->vlist_local[vertex].w = 1;
		
		//缩放顶点坐标
		obj->vlist_local[vertex].x *= scale->x;
		obj->vlist_local[vertex].y *= scale->y;
		obj->vlist_local[vertex].z *= scale->z;

		printf("\nVertex %d = %f, %f, %f, %f", vertex, obj->vlist_local[vertex].x, obj->vlist_local[vertex].y, obj->vlist_local[vertex].z, obj->vlist_local[vertex].w);
	}

	//计算平均半径和最大半径
	Compute_OBJECT4DV1_Radius(obj);
	printf("\nObject average radius = %f, max radius = %f", obj->avg_radius, obj->max_raduis);

	int poly_surface_desc = 0;
	int poly_num_verts = 0;
	char tmp_string[8];

	//多边形列表
	for (int poly = 0; poly < obj->num_polys; poly++)
	{
		//多边形描述
		if (!(token_string = Get_Line_PLG(buffer, 255, fp))){
			printf("PLG file error with file %s (polygon descriptor invalid). ", filename);
			return 0;
		}
		printf("\n Pologon %d:", poly);

		sscanf(token_string, "%s %d %d %d %d", tmp_string, &poly_num_verts, &obj->plist[poly].vert[0], &obj->plist[poly].vert[1], &obj->plist[poly].vert[2]);

		//面描述符可以是十六进制值
		//因此需要对这种情况进行检测
		if (tmp_string[0] == '0' && toupper(tmp_string[1]) == 'X')
			sscanf(tmp_string, "%x", &poly_surface_desc);
		else
			poly_surface_desc = atoi(tmp_string);

		//让多边形顶点列表指向物体的顶点列表，这是多余的，因为多边形列表包含在物体中，
		//建立多边形几何体时，由用户决定使用局部顶点列表还是变换后的顶点列表
		//如果所有多边形都属于同一个物体，该指针设置为NULL更合适
		obj->plist[poly].vlist = obj->vlist_local;
		printf("\nSurface Desc = 0x%.4x, num_verts = %d, vert_indices [%d, %d, %d]", poly_surface_desc, poly_num_verts, obj->plist[poly].vert[0], obj->plist[poly].vert[1], obj->plist[poly].vert[2]);
		//存储顶点列表和多边形顶点索引值之后
		//分析多边形描述符，并据此相应地设置多边形
		
		//提取多边形描述符中的每个位字段
		//从单面/双面位开始
		if (poly_surface_desc & PLX_COLOR_MODE_RGB_FLAG){
			SET_BIT(obj->plist[poly].attr, POLY4DV1_ATTR_2SIDED);
			printf("\n2 sided. ");
		}
		else
		{
			printf("\n1 sided. ");
		}

		//设置颜色模式
		if (poly_surface_desc & PLX_COLOR_MODE_RGB_FLAG){
			//为RGB颜色模式
			SET_BIT(obj->plist[poly].attr, POLY4DV1_ATTR_RGB16);
			//提取RGB颜色
			int red = (poly_surface_desc & 0x0f00) >> 8;
			int green = (poly_surface_desc & 0x00f0) >> 4;
			int blue = (poly_surface_desc & 0x000f);

			//文件中颜色总是为4.4.4格式，而图形卡的RGB颜色格式是5.5.5或5.6.5，但是虚拟系统把8.8.8颜色格式转换为5.5.5或5.6.5
			//因此需要将4.4.4的RGB转换为8.8.8
			obj->plist[poly].color = _RGB16BIT555(red * 16, green * 16, blue * 16);
			printf("\nRGB color = {%d, %d, %d}", red, green, blue);
		}
		else
		{
			SET_BIT(obj->plist[poly].attr, POLY4DV1_ATTR_8BITCOLOR);
			obj->plist[poly].color = poly_surface_desc & 0x00ff;
			printf("\n8-bit color index=%d", obj->plist[poly].color);
		}

		//处理着色模式
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