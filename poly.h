// this builds a 16 bit color value in 5.5.5 format (1-bit alpha mode)
#define _RGB16BIT555(r,g,b) ((b & 31) + ((g & 31) << 5) + ((r & 31) << 10))

// this builds a 16 bit color value in 5.6.5 format (green dominate mode)
#define _RGB16BIT565(r,g,b) ((b & 31) + ((g & 63) << 5) + ((r & 31) << 11))

// this builds a 24 bit color value in 8.8.8 format 
#define _RGB24BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) )

// this builds a 32 bit color value in A.8.8.8 format (8-bit alpha mode)
#define _RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

// bit manipulation macros
#define SET_BIT(word,bit_flag)   ((word)=((word) | (bit_flag)))
#define RESET_BIT(word,bit_flag) ((word)=((word) & (~bit_flag)))

// initializes a direct draw struct, basically zeros it and sets the dwSize field
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// used to compute the min and max of two expresions
#define MIN(a, b)  (((a) < (b)) ? (a) : (b)) 
#define MAX(a, b)  (((a) > (b)) ? (a) : (b)) 

// used for swapping algorithm
#define SWAP(a,b,t) {t=a; a=b; b=t;}

// some math macros
#define DEG_TO_RAD(ang) ((ang)*PI/180.0)
#define RAD_TO_DEG(rads) ((rads)*180.0/PI)

#define RAND_RANGE(x,y) ( (x) + (rand()%((y)-(x)+1)))

#define OBJECT4DV1_MAX_VERTICES		64
#define OBJECT4DV1_MAX_POLYS		1024

//多边形和多边形面的属性
#define POLY4DV1_ATTR_2SIDED		0x0001
#define PLOY4DV1_ATTR_TRANSPARENT	0x0002
#define POLY4DV1_ATTR_8BITCOLOR		0x0004
#define POLY4DV1_ATTR_RGB16			0x0008
#define POLY4DV1_ATTR_RGB24			0x0010

#define POLY4DV1_ATTR_SHADE_MODE_PURE		0x0020
#define POLY4DV1_ATTR_SHADE_MODE_FLAT		0x0040
#define POLY4DV1_ATTR_SHADE_MODE_GOURAUD	0x0080
#define POLY4DV1_ATTR_SHADE_MODE_PHONG		0x0100

//多边形和面的状态值
#define POLY4DV1_STATE_ACTIVE		0x0001
#define POLY4DV1_STATE_VISIBLE		0x0002
#define POLY4DV1_STATE_BACKFACE		0x0004
#define POLY4DV1_STATE_CLIPPED		0x0008

#define OBJECT4DV1_STATE_ACTIVE			0x0001

#define RENDERLIST4DV1_MAX_POLYS 256

//变换
#define TRANSFORM_LOCAL_ONLY		1
#define TRANSFORM_TRANS_ONLY		2
#define TRANSFORM_LOCAL_TO_TRANS	3

typedef struct POINT4D_TYP
{
	int x;
	int y;
	int z;
	int w;
} POINT4D, *POINT4D_PTR;

typedef struct VECTOR4D_TYP
{
	int x;
	int y;
	int z;
	int w;
} VECTOR4D, *VECTOR4D_PTR;

//基于顶点列表的多边形
typedef struct POLY4DV1_TYP
{
	int state;
	int attr;
	int color;
	POINT4D_PTR vlist;
	int vert[3]; //顶点列表中元素的索引
} POLY4DV1, *POLY4DV1_PTR;

//自包含的多边形数据，供渲染列表使用
typedef struct POLYF4DV1_TYP
{
	int state;
	int attr;
	int color;
	
	POINT4D vlist[3];  //三角形顶点
	POINT4D tvlist[3]; //变换后的三角形定点

	POLYF4DV1_TYP *next;
	POLYF4DV1_TYP *prev;
} POLYF4DV1, *POLYF4DV1_PTR;

//基于顶点列表和多边形列表的物体
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
	VECTOR4D ux, uy, uz;    //物体在局部坐标系中的旋转

	int num_vertices;		//物体的顶点数
	POINT4D vlist_local[OBJECT4DV1_MAX_VERTICES]; //存储顶点局部坐标的数组
	POINT4D vlist_trans[OBJECT4DV1_MAX_VERTICES]; //存储变换后的顶点坐标的数组

	int num_polys; //物体网格的多边形数
	POLY4DV1 plist[OBJECT4DV1_MAX_POLYS]; //多边形数组
} OBJECT4DV1, *OBJECT4DV1_PTR;

typedef struct RENDERLIST4DV1_TYP{
	int state;
	int attr;
	//指针数组，用来排序或者其他的操作
	POLYF4DV1_PTR poly_ptrs[RENDERLIST4DV1_MAX_POLYS];
	//原始数据，为了避免每帧为多边形分配和释放存储空间，存储在这里面
	POLYF4DV1 poly_data[RENDERLIST4DV1_MAX_POLYS];

	int num_polys; //渲染列表中的多边形数目
} RENDERLIST4DV1,*RENDERLIST4DV1_PTR;

void Compute_OBJECT4DV1_Radius(OBJECT4DV1_PTR obj){
	int max_raduis = 0;
	for (int i = 0; i < obj->num_vertices; i++)
	{
		
	}
}