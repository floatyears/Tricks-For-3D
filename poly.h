#define OBJECT4DV1_MAX_VERTICES 64;

typedef struct POINT4D_TYP
{

} POINT4D, *POINt4D_PTR;

typedef struct VECTOR4D_TYP
{

} VECTOR4D, *VECTOR4D_PTR;

typedef struct POLY4DV1_TYP
{
	int state;
	int attr;
	int color;
	
	POINT4D vlist[3];  //三角形顶点
	POINT4D tvlist[3]; //变换后的三角形定点

	POLY4DV1_TYP *next;
	POLY4DV1_TYP *prev;
} POLY4DV1, *POLY4DV1_PTR;

typedef struct OBJECT4DV1_TYP
{
	int id;
	char name[64];
	int state;
	int attr;
	float avg_radius;
	float max_raduis;

	POINT4D world_pos;
	VECTOR4D dir;			//物体在局部坐标系中的旋转角度
	VECTOR4D ux, uy, uz;
};