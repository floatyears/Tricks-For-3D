#include "light.h"

int Reset_Materials_MATV1(void)
{
	static int first_time = 1;
	//如果是第一次调用该函数，则重置所有的材质
	if (first_time)
	{
		memset(materials, 0, MAX_MATERIALS*sizeof(MATV1));
		first_time = 0;
	}

	for (int curr_matt = 0; curr_matt < MAX_MATERIALS; curr_matt++)
	{
		//不管材质是否处于活动状态，都释放与之相关的纹理图 
		//Destroy_Bitmap(&materials[curr_matt],0,sizeof(MATV1));
		memset(&materials[curr_matt], 0, sizeof(MATV1));
	}
	return 1;
}

int Reset_Lights_LIGHTV1(void)
{
	static int first_time = 1;
	//重置系统中的所有光源
	memset(lights, 0, MAX_LIGHTS * sizeof(LIGHTV1));

	num_lights = 0;

	first_time = 0;

	return 1;
}


//根据传入的参数初始化光源
int Init_Light_LIGHTV1(int index,
	int _state,
	int _attr,
	RGBAV1 _c_ambient,		//环境光强度
	RGBAV1 _c_diffuse,		//散射光强度
	RGBAV1 _c_specular,
	POINT4D_PTR _pos,
	POINT4D_PTR _dir,
	float _kc, float _kl, float _kq, //衰减因子
	float _spot_inner,		//聚光灯内锥角
	float _spot_outer,		//聚光灯外锥角
	float _pf)				//聚光灯指数因子
{
	if (index < 0 || index >= MAX_LIGHTS) return 0;

	lights[index].state = _state;
	lights[index].attr	= _attr;
	lights[index].c_ambient = _c_ambient;
	lights[index].c_diffuse = _c_diffuse;
	lights[index].c_specular = _c_specular;
	lights[index].kc = _kc;
	lights[index].kq = _kq;
	lights[index].kl = _kl;

	if (_pos)
	{
		VECTOR4D_COPY(&lights[index].pos, _pos);
	}

	if (_dir)
	{
		VECTOR4D_COPY(&lights[index].dir, _dir);
		//归一化
		VECTOR4D_Normalize(&lights[index].dir);
	}
	lights[index].spot_inner = _spot_inner;
	lights[index].spot_outer = _spot_outer;
	lights[index].pf = _pf;

	//返回索引光源
	return index;
}

//该函数指定为哪种RGB颜色格式创建查找表，RGB格式为5.5.5时，只有32k种颜色（不考虑第一位的值
int RGB_16_8_IndexedRGB_Table_Builder(int rgb_format, //rgb格式，5.6.5或者5.5.5
	LPPALETTEENTRY src_palette, //调色板
	UCHAR *rgblookup) //查找
{
	if (!src_palette || !rgblookup)
	{
		return -1;
	}

	if (rgb_format == DD_PIXEL_FORMAT565)
	{
		for (int rgbindex = 0; rgbindex < 65536; rgbindex++)
		{
			int curr_index = -1; //当前最接近的颜色索引
			long curr_error = INT_MAX; //最接近的颜色和当前颜色之间的距离 
			for (int  color_index = 0; color_index < 256; color_index++)
			{
				int r = (rgbindex >> 11) << 3;
				int g = ((rgbindex >> 5) & 0x3f) << 2;
				int b = (rgbindex & 0x1f) << 3;

				//计算距离
				long delta_red = abs(src_palette[color_index].peRed - r);
				long delta_green = abs(src_palette[color_index].peGreen - g);
				long delta_blue = abs(src_palette[color_index].peBlue - b);
				long error = (delta_red * delta_red) + (delta_green*delta_green) + (delta_blue*delta_blue);

				//是否更近
				if (error < curr_error)
				{
					curr_index = color_index;
					curr_error = error;
				}
			}

			rgblookup[rgbindex] = curr_index;
			
		}

	}
	else if (rgb_format == DD_PIXEL_FORMAT555)
	{
		for (int rgbindex = 0; rgbindex < 32768; rgbindex++)
		{
			int curr_index = -1;
			int curr_error = INT_MAX;
			for (int color_index = 0; color_index < 256; color_index++)
			{
				int r = (rgbindex >> 10) << 3;
				int g = ((rgbindex >> 5) & 0x1f) << 3;
				int b = (rgbindex & 0x1f) << 3;

				//计算距离
				long delta_red = abs(src_palette[color_index].peRed - r);
				long delta_green = abs(src_palette[color_index].peGreen - g);
				long delta_blue = abs(src_palette[color_index].peBlue - b);

				long error = (delta_red * delta_red) + (delta_green*delta_green) + (delta_blue*delta_blue);

				//是否更近
				if (error < curr_error)
				{
					curr_index = color_index;
					curr_error = error;
				}
			}
			rgblookup[rgbindex] = curr_index;
		}
	}
	else
	{
		return -1;
	}
	return 1;
}

int RGB_16_8_Indexed_Intensity_Table_Builder(LPPALETTEENTRY src_palette,
	UCHAR rgbilookup[256][256], //查找表
	int intensity_normalization = 1)
{
	int ri, gi, bi; //初始颜色
	int rw, gw, bw; //用于储存临时颜色值
	float ratio;
	float dl, dr, db, dg; //256种颜色之间的强度梯度

	if (!src_palette || !rgbilookup)
		return -1;

	for (int color_index = 0; color_index < 256; color_index++)
	{
		ri = src_palette[color_index].peRed;
		gi = src_palette[color_index].peGreen;
		bi = src_palette[color_index].peBlue;

		//找出最大的分量，将其设置为255
		//然后向下计算出256种颜色强度
		if (intensity_normalization == 1)
		{
			if (ri >= gi && ri >= bi)
			{
				ratio = (float)255 / (float)ri;

				ri = 255;
				gi = (int)((float)gi*ratio + 0.5);
				bi = (int)((float)bi*ratio + 0.5);
			}
			else if (gi >= ri && gi >= bi)
			{
				ratio = (float)255 / (float)gi;

				gi = 255;
				ri = (int)((float)ri*ratio + 0.5);
				bi = (int)((float)bi*ratio + 0.5);
			}
			else
			{
				ratio = (float)255 / (float)bi;

				bi = 255;
				ri = (int)((float)ri*ratio + 0.5);
				gi = (int)((float)gi*ratio + 0.5);
			}
		}

		//计算强度梯度，以计算该颜色256种着色度对应的RGB值
		dl = sqrt(ri*ri + gi*gi + bi*bi) / (float)256;
		dr = ri / dl;
		db = gi / dl;
		dg = bi / dl;

		//初始化临时颜色值变量
		rw = 0;
		gw = 0;
		bw = 0;

		for (int intensity_index = 0; intensity_index < 256; intensity_index++)
		{
			int curr_index = -1;
			long curr_error = INT_MAX;
			for (int color_index = 0; color_index < 256; color_index++)
			{
				long delta_red = abs(src_palette[color_index].peRed - rw);
				long delta_green = abs(src_palette[color_index].peGreen - rw);
				long delta_blue = abs(src_palette[color_index].peBlue - rw);

				long delta_error = abs(delta_red*delta_red + delta_blue * delta_blue + delta_green*delta_green);
				if (delta_error < curr_error)
				{
					curr_index = color_index;
					curr_error = delta_error;
				}
			}

			//找到最近的颜色值索引后，将其存储到查找表的相应位置中
			rgbilookup[color_index][intensity_index] = curr_index;

			//计算下一种着色度对应的RGB值，并查找它们是否溢出
			if (rw += dr > 255) rw = 255;
			if (gw += db > 255) gw = 255;
			if (bw += dg > 255) bw = 255;
		}
	}
	return -1;
}

int Light_OBJECT4DV1_World16(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam,
	LIGHTV1_PTR lights,  //光源列表
	int max_lights,		 //最大光源数
	int dd_pixel_format) 
{
	unsigned int r_base, g_base, b_base,	//原来的颜色值 
		r_sum, g_sum, b_sum,				//全部光源的总体光照效果
		shaded_color;						//最后的颜色

	float dp,		//点积
		dist,		//表明和光滑之间的距离
		i,			//强度
		nl,			//法线长度
		atten;		//衰减计算结果
	
	if (!(obj->state & OBJECT4DV1_STATE_ACTIVE) || (obj->state & OBJECT4DV1_STATE_CULLED) || !(obj->state & OBJECT4DV1_STATE_VISIBLE))
		return 0;

	for (int poly = 0; poly < obj->num_polys; poly++)
	{
		POLY4DV1_PTR curr_poly = &obj->plist[poly];

		//多边形是否可见
		if (!(curr_poly->state || POLY4DV1_STATE_ACTIVE) || (curr_poly->state & POLY4DV1_STATE_CLIPPED) || (curr_poly->state & POLY4DV1_STATE_BACKFACE))
		{
			continue;
		}

		//提取指向主列表的顶点索引
		//多边形不是自包含的，是基于物体的顶点列表的
		int vindex_0 = curr_poly->vert[0];
		int vindex_1 = curr_poly->vert[1];
		int vindex_2 = curr_poly->vert[2];

		//检查着色模式
		if (curr_poly->attr & POLY4DV1_ATTR_SHADE_MODE_FLAT || curr_poly->attr & POLY4DV1_ATTR_SHADE_MODE_GOURAUD)
		{
			if (dd_pixel_format == DD_PIXEL_FORMAT565)
			{
				_RGB565FORMAT16BIT(curr_poly->color, &r_base, &g_base, &b_base);
				r_base <<= 3;
				g_base <<= 2;
				b_base <<= 3;
			}
			else
			{
				_RGB555FORMAT16BIT(curr_poly->color, &r_base, &g_base, &b_base);
				r_base <<= 3;
				g_base <<= 3;
				b_base <<= 3;
			}

			//初始化总体光照颜色
			r_sum = 0;
			g_sum = 0;
			b_sum = 0;

			for (int curr_light = 0; curr_light < max_lights; curr_light++)
			{
				//光源是否被打开
				if (lights[curr_light].state)
					continue;
				//判断光源的类型
				if (lights[curr_light].attr & LIGHTV1_ATTR_AMBIENT) //环境光
				{
					r_sum += (lights[curr_light].c_ambient.r*r_base) / 256;
					g_sum += (lights[curr_light].c_ambient.g*g_base) / 256;
					b_sum += (lights[curr_light].c_ambient.b*b_base) / 256;
				}
				else if (lights[curr_light].attr & LIGHTV1_ATTR_INFINITE) //无穷远光源
				{
					//需要知道面法线和光源方向
					VECTOR4D u, v, n;
					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_1], &u);
					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_2], &v);

					//计算叉积
					VECTOR4D_Cross(&u, &v, &n);

					//作为优化，预计算出所有多边形法线的长度
					nl = VECTOR4D_Length_Fast(&n);

					//无穷远光源光照模型如下：I(d)dir = IOdir * Cldir;
					//散射项的计算公式如下： Itotal = Rsdiffuse * (n . l)
					//因此只需要把它们乘起来即可
					dp = VECTOR4D_Dot(&n, &lights[curr_light].dir);

					//仅当dp大于0时，才需要考虑该光源的影响
					if (dp > 0)
					{
						//乘以128避免浮点运算
						i = 128 * dp / nl;
						r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / (256 * 128);
					}
				}
				else if (lights[curr_light].attr & LIGHTV1_ATTR_POINT) //点光源
				{
					VECTOR4D u, v, n, l;
					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_1], &u);
					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_2], &v);

					//计算叉积
					VECTOR4D_Cross(&u, &v, &n);

					//作为优化，预计算出所有多边形法线的长度
					nl = VECTOR4D_Length_Fast(&n);

					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &lights[curr_light].pos, &l);

					//计算距离和衰减
					dist = VECTOR4D_Length_Fast(&l);

					dp = VECTOR4D_Dot(&l, &n);

					if (dp > 0)
					{
						atten = (lights[curr_light].kc + lights[curr_light].kl *dist + lights[curr_light].kq*dist*dist);
						//乘以128避免浮点运算，这样做不是因为浮点数运算慢，是因为浮点数转换可能小号大量CPU周期
						i = 128 * dp / (nl*dist*atten);
						r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / (256 * 128);
					}
				}
				else if (lights[curr_light].attr & LIGHTV1_ATTR_SPOTLIGHT1)
				{
					VECTOR4D u, v, n, l;

					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_1], &u);
					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_2], &v);

					//计算叉积
					VECTOR4D_Cross(&u, &v, &n);

					//作为优化，预计算出所有多边形法线的长度
					nl = VECTOR4D_Length_Fast(&n);

					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &lights[curr_light].pos, &l);

					//计算距离和衰减
					dist = VECTOR4D_Length_Fast(&l);

					//对于散射光 Itotal = Rsdiffuse * Idiffuse * (n . l)
					dp = VECTOR4D_Dot(&l, &n);

					if (dp > 0)
					{
						atten = (lights[curr_light].kc + lights[curr_light].kl*dist + lights[curr_light].kq*dist*dist);

						i = 128 * dp / (nl*atten);
						r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / (256 * 128);
					}
				}
				else if (lights[curr_light].attr & LIGHTV1_ATTR_SPOTLIGHT2)
				{
					VECTOR4D u, v, n, d,s;

					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_1], &u);
					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_2], &v);

					//计算叉积
					VECTOR4D_Cross(&u, &v, &n);

					//作为优化，预计算出所有多边形法线的长度
					nl = VECTOR4D_Length_Fast(&n);


					//对于散射光 Itotal = Rsdiffuse * Idiffuse * (n . l)
					dp = VECTOR4D_Dot(&lights[curr_light].dir, &n);

					if (dp > 0)
					{
						//计算表明到光源的向量
						VECTOR4D_Build(&lights[curr_light].pos, &obj->vlist_trans[vindex_0], &s);

						//计算s的长度，以归一化
						dist = VECTOR4D_Length_Fast(&s);

						//计算点积 s . l
						float dpsl = VECTOR4D_Dot(&s, &lights[curr_light].dir) / dist;

						if (dpsl > 0)
						{
							atten = (lights[curr_light].kc + lights[curr_light].kl*dist + lights[curr_light].kq*dist*dist);

							//为提高速度，pf必须是大于1.0的整数
							float dpsl_exp = dpsl;

							for (int e_index = 0; e_index < (int)lights[curr_light].pf; e_index++)
							{
								dpsl_exp *= dpsl;
							}

								//现在dpsl_exp存储的是(dpsl)^pf, 即 (s.l)^pf;
								i = 128 * dp * dpsl_exp / (nl * atten);

								r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / (256 * 128);
								g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / (256 * 128);
								b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / (256 * 128);
						}
						
					}
				}

				
			}
			if (r_sum > 255) r_sum = 255;
			if (g_sum > 255) g_sum = 255;
			if (b_sum > 255) b_sum = 255;

			shaded_color = _RGB16BIT565(r_sum, g_sum, b_sum);
			curr_poly->color = (int)(shaded_color << 16 | curr_poly->color);
		}
		else //采用固定着色，将原来的颜色复制到前16位中
		{
			curr_poly->color = (int)(curr_poly->state << 16 | curr_poly->color);
		}
	}
	return 1;
}