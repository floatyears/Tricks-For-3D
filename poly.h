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

//����κͶ�����������
#define POLY4DV1_ATTR_2SIDED		0x0001
#define PLOY4DV1_ATTR_TRANSPARENT	0x0002
#define POLY4DV1_ATTR_8BITCOLOR		0x0004
#define POLY4DV1_ATTR_RGB16			0x0008
#define POLY4DV1_ATTR_RGB24			0x0010

#define POLY4DV1_ATTR_SHADE_MODE_PURE		0x0020
#define POLY4DV1_ATTR_SHADE_MODE_FLAT		0x0040
#define POLY4DV1_ATTR_SHADE_MODE_GOURAUD	0x0080
#define POLY4DV1_ATTR_SHADE_MODE_PHONG		0x0100

//����κ����״ֵ̬
#define POLY4DV1_STATE_ACTIVE		0x0001
#define POLY4DV1_STATE_VISIBLE		0x0002
#define POLY4DV1_STATE_BACKFACE		0x0004
#define POLY4DV1_STATE_CLIPPED		0x0008

#define OBJECT4DV1_STATE_ACTIVE			0x0001

#define RENDERLIST4DV1_MAX_POLYS 256

//�任
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

//���ڶ����б�Ķ����
typedef struct POLY4DV1_TYP
{
	int state;
	int attr;
	int color;
	POINT4D_PTR vlist;
	int vert[3]; //�����б���Ԫ�ص�����
} POLY4DV1, *POLY4DV1_PTR;

//�԰����Ķ�������ݣ�����Ⱦ�б�ʹ��
typedef struct POLYF4DV1_TYP
{
	int state;
	int attr;
	int color;
	
	POINT4D vlist[3];  //�����ζ���
	POINT4D tvlist[3]; //�任��������ζ���

	POLYF4DV1_TYP *next;
	POLYF4DV1_TYP *prev;
} POLYF4DV1, *POLYF4DV1_PTR;

//���ڶ����б�Ͷ�����б������
typedef struct OBJECT4DV1_TYP
{
	int id;
	char name[64];
	int state;
	int attr;
	float avg_radius;
	float max_raduis;

	POINT4D world_pos;
	VECTOR4D dir;			//�����ھֲ�����ϵ�е���ת�Ƕ�
	VECTOR4D ux, uy, uz;    //�����ھֲ�����ϵ�е���ת

	int num_vertices;		//����Ķ�����
	POINT4D vlist_local[OBJECT4DV1_MAX_VERTICES]; //�洢����ֲ����������
	POINT4D vlist_trans[OBJECT4DV1_MAX_VERTICES]; //�洢�任��Ķ������������

	int num_polys; //��������Ķ������
	POLY4DV1 plist[OBJECT4DV1_MAX_POLYS]; //���������
} OBJECT4DV1, *OBJECT4DV1_PTR;

typedef struct RENDERLIST4DV1_TYP{
	int state;
	int attr;
	//ָ�����飬����������������Ĳ���
	POLYF4DV1_PTR poly_ptrs[RENDERLIST4DV1_MAX_POLYS];
	//ԭʼ���ݣ�Ϊ�˱���ÿ֡Ϊ����η�����ͷŴ洢�ռ䣬�洢��������
	POLYF4DV1 poly_data[RENDERLIST4DV1_MAX_POLYS];

	int num_polys; //��Ⱦ�б��еĶ������Ŀ
} RENDERLIST4DV1,*RENDERLIST4DV1_PTR;

void Compute_OBJECT4DV1_Radius(OBJECT4DV1_PTR obj){
	int max_raduis = 0;
	for (int i = 0; i < obj->num_vertices; i++)
	{
		
	}
}