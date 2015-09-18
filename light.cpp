#include "light.h"

int Reset_Materials_MATV1(void)
{
	static int first_time = 1;
	//����ǵ�һ�ε��øú��������������еĲ���
	if (first_time)
	{
		memset(materials, 0, MAX_MATERIALS*sizeof(MATV1));
		first_time = 0;
	}

	for (int curr_matt = 0; curr_matt < MAX_MATERIALS; curr_matt++)
	{
		//���ܲ����Ƿ��ڻ״̬�����ͷ���֮��ص�����ͼ 
		//Destroy_Bitmap(&materials[curr_matt],0,sizeof(MATV1));
		memset(&materials[curr_matt], 0, sizeof(MATV1));
	}
	return 1;
}

int Reset_Lights_LIGHTV1(void)
{
	static int first_time = 1;
	//����ϵͳ�е����й�Դ
	memset(lights, 0, MAX_LIGHTS * sizeof(LIGHTV1));

	num_lights = 0;

	first_time = 0;

	return 1;
}


//���ݴ���Ĳ�����ʼ����Դ
int Init_Light_LIGHTV1(int index,
	int _state,
	int _attr,
	RGBAV1 _c_ambient,		//������ǿ��
	RGBAV1 _c_diffuse,		//ɢ���ǿ��
	RGBAV1 _c_specular,
	POINT4D_PTR _pos,
	POINT4D_PTR _dir,
	float _kc, float _kl, float _kq, //˥������
	float _spot_inner,		//�۹����׶��
	float _spot_outer,		//�۹����׶��
	float _pf)				//�۹��ָ������
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
		//��һ��
		VECTOR4D_Normalize(&lights[index].dir);
	}
	lights[index].spot_inner = _spot_inner;
	lights[index].spot_outer = _spot_outer;
	lights[index].pf = _pf;

	//����������Դ
	return index;
}

//�ú���ָ��Ϊ����RGB��ɫ��ʽ�������ұ�RGB��ʽΪ5.5.5ʱ��ֻ��32k����ɫ�������ǵ�һλ��ֵ
int RGB_16_8_IndexedRGB_Table_Builder(int rgb_format, //rgb��ʽ��5.6.5����5.5.5
	LPPALETTEENTRY src_palette, //��ɫ��
	UCHAR *rgblookup) //����
{
	if (!src_palette || !rgblookup)
	{
		return -1;
	}

	if (rgb_format == DD_PIXEL_FORMAT565)
	{
		for (int rgbindex = 0; rgbindex < 65536; rgbindex++)
		{
			int curr_index = -1; //��ǰ��ӽ�����ɫ����
			long curr_error = INT_MAX; //��ӽ�����ɫ�͵�ǰ��ɫ֮��ľ��� 
			for (int  color_index = 0; color_index < 256; color_index++)
			{
				int r = (rgbindex >> 11) << 3;
				int g = ((rgbindex >> 5) & 0x3f) << 2;
				int b = (rgbindex & 0x1f) << 3;

				//�������
				long delta_red = abs(src_palette[color_index].peRed - r);
				long delta_green = abs(src_palette[color_index].peGreen - g);
				long delta_blue = abs(src_palette[color_index].peBlue - b);
				long error = (delta_red * delta_red) + (delta_green*delta_green) + (delta_blue*delta_blue);

				//�Ƿ����
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

				//�������
				long delta_red = abs(src_palette[color_index].peRed - r);
				long delta_green = abs(src_palette[color_index].peGreen - g);
				long delta_blue = abs(src_palette[color_index].peBlue - b);

				long error = (delta_red * delta_red) + (delta_green*delta_green) + (delta_blue*delta_blue);

				//�Ƿ����
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
	UCHAR rgbilookup[256][256], //���ұ�
	int intensity_normalization = 1)
{
	int ri, gi, bi; //��ʼ��ɫ
	int rw, gw, bw; //���ڴ�����ʱ��ɫֵ
	float ratio;
	float dl, dr, db, dg; //256����ɫ֮���ǿ���ݶ�

	if (!src_palette || !rgbilookup)
		return -1;

	for (int color_index = 0; color_index < 256; color_index++)
	{
		ri = src_palette[color_index].peRed;
		gi = src_palette[color_index].peGreen;
		bi = src_palette[color_index].peBlue;

		//�ҳ����ķ�������������Ϊ255
		//Ȼ�����¼����256����ɫǿ��
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

		//����ǿ���ݶȣ��Լ������ɫ256����ɫ�ȶ�Ӧ��RGBֵ
		dl = sqrt(ri*ri + gi*gi + bi*bi) / (float)256;
		dr = ri / dl;
		db = gi / dl;
		dg = bi / dl;

		//��ʼ����ʱ��ɫֵ����
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

			//�ҵ��������ɫֵ�����󣬽���洢�����ұ����Ӧλ����
			rgbilookup[color_index][intensity_index] = curr_index;

			//������һ����ɫ�ȶ�Ӧ��RGBֵ�������������Ƿ����
			if (rw += dr > 255) rw = 255;
			if (gw += db > 255) gw = 255;
			if (bw += dg > 255) bw = 255;
		}
	}
	return -1;
}

int Light_OBJECT4DV1_World16(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam,
	LIGHTV1_PTR lights,  //��Դ�б�
	int max_lights,		 //����Դ��
	int dd_pixel_format) 
{
	unsigned int r_base, g_base, b_base,	//ԭ������ɫֵ 
		r_sum, g_sum, b_sum,				//ȫ����Դ���������Ч��
		shaded_color;						//������ɫ

	float dp,		//���
		dist,		//�����͹⻬֮��ľ���
		i,			//ǿ��
		nl,			//���߳���
		atten;		//˥��������
	
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

		//��ȡָ�����б�Ķ�������
		//����β����԰����ģ��ǻ�������Ķ����б��
		int vindex_0 = curr_poly->vert[0];
		int vindex_1 = curr_poly->vert[1];
		int vindex_2 = curr_poly->vert[2];

		//�����ɫģʽ
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

			//��ʼ�����������ɫ
			r_sum = 0;
			g_sum = 0;
			b_sum = 0;

			for (int curr_light = 0; curr_light < max_lights; curr_light++)
			{
				//��Դ�Ƿ񱻴�
				if (lights[curr_light].state)
					continue;
				//�жϹ�Դ������
				if (lights[curr_light].attr & LIGHTV1_ATTR_AMBIENT) //������
				{
					r_sum += (lights[curr_light].c_ambient.r*r_base) / 256;
					g_sum += (lights[curr_light].c_ambient.g*g_base) / 256;
					b_sum += (lights[curr_light].c_ambient.b*b_base) / 256;
				}
				else if (lights[curr_light].attr & LIGHTV1_ATTR_INFINITE) //����Զ��Դ
				{
					//��Ҫ֪���淨�ߺ͹�Դ����
					VECTOR4D u, v, n;
					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_1], &u);
					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_2], &v);

					//������
					VECTOR4D_Cross(&u, &v, &n);

					//��Ϊ�Ż���Ԥ��������ж���η��ߵĳ���
					nl = VECTOR4D_Length_Fast(&n);

					//����Զ��Դ����ģ�����£�I(d)dir = IOdir * Cldir;
					//ɢ����ļ��㹫ʽ���£� Itotal = Rsdiffuse * (n . l)
					//���ֻ��Ҫ�����ǳ���������
					dp = VECTOR4D_Dot(&n, &lights[curr_light].dir);

					//����dp����0ʱ������Ҫ���Ǹù�Դ��Ӱ��
					if (dp > 0)
					{
						//����128���⸡������
						i = 128 * dp / nl;
						r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / (256 * 128);
					}
				}
				else if (lights[curr_light].attr & LIGHTV1_ATTR_POINT) //���Դ
				{
					VECTOR4D u, v, n, l;
					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_1], &u);
					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &obj->vlist_trans[vindex_2], &v);

					//������
					VECTOR4D_Cross(&u, &v, &n);

					//��Ϊ�Ż���Ԥ��������ж���η��ߵĳ���
					nl = VECTOR4D_Length_Fast(&n);

					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &lights[curr_light].pos, &l);

					//��������˥��
					dist = VECTOR4D_Length_Fast(&l);

					dp = VECTOR4D_Dot(&l, &n);

					if (dp > 0)
					{
						atten = (lights[curr_light].kc + lights[curr_light].kl *dist + lights[curr_light].kq*dist*dist);
						//����128���⸡�����㣬������������Ϊ������������������Ϊ������ת������С�Ŵ���CPU����
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

					//������
					VECTOR4D_Cross(&u, &v, &n);

					//��Ϊ�Ż���Ԥ��������ж���η��ߵĳ���
					nl = VECTOR4D_Length_Fast(&n);

					VECTOR4D_Build(&obj->vlist_trans[vindex_0], &lights[curr_light].pos, &l);

					//��������˥��
					dist = VECTOR4D_Length_Fast(&l);

					//����ɢ��� Itotal = Rsdiffuse * Idiffuse * (n . l)
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

					//������
					VECTOR4D_Cross(&u, &v, &n);

					//��Ϊ�Ż���Ԥ��������ж���η��ߵĳ���
					nl = VECTOR4D_Length_Fast(&n);


					//����ɢ��� Itotal = Rsdiffuse * Idiffuse * (n . l)
					dp = VECTOR4D_Dot(&lights[curr_light].dir, &n);

					if (dp > 0)
					{
						//�����������Դ������
						VECTOR4D_Build(&lights[curr_light].pos, &obj->vlist_trans[vindex_0], &s);

						//����s�ĳ��ȣ��Թ�һ��
						dist = VECTOR4D_Length_Fast(&s);

						//������ s . l
						float dpsl = VECTOR4D_Dot(&s, &lights[curr_light].dir) / dist;

						if (dpsl > 0)
						{
							atten = (lights[curr_light].kc + lights[curr_light].kl*dist + lights[curr_light].kq*dist*dist);

							//Ϊ����ٶȣ�pf�����Ǵ���1.0������
							float dpsl_exp = dpsl;

							for (int e_index = 0; e_index < (int)lights[curr_light].pf; e_index++)
							{
								dpsl_exp *= dpsl;
							}

								//����dpsl_exp�洢����(dpsl)^pf, �� (s.l)^pf;
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
		else //���ù̶���ɫ����ԭ������ɫ���Ƶ�ǰ16λ��
		{
			curr_poly->color = (int)(curr_poly->state << 16 | curr_poly->color);
		}
	}
	return 1;
}