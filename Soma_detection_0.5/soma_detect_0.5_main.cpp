// Created by program: Soma_Detection 
// Author: Guan-wei He

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <cmath>
#include "AM_format.h"
#include <time.h>
#include "PCA.h"
#include <iomanip> 
#include <vector>

//parameters///////////////////////////////////////////////////////////////////
//   you can modify by yourself and use your weights below

double dthreshold[6] = { 0.6, 0.7, 0.8, 0.9, 0.95, 0.99 };			// global threshold for 6 iterators
#define BF_th 2					// candidate need larger than BF_th

////////////////////////////////////////////////////////////////////////////////


using namespace std;

bool cout_flag = true;
void change_filename(int = 0);
void do_attributes(int, int);
void do_candidate(int);
double weight[15] = { 0 };

int count_candidate = 1;
int iSoma_ALL_index = 0;

struct_Voxel *iSoma_update;
struct_Voxel *iSoma_ALL;
struct_Attr *iSoma_ALL_attr;

string sFileName = "";
string sFileName_output = "";

string sFileName_bmp_z = "";
string sFileName_attr = "";

char header[54] = {
	0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,
	54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
};

char buffers[2000];
AM_data AM_all_data, csVoxels;

int icount_update = 0;

struct Soma_3D
{
	int nid;
	int x, y, z;
};

int main(int argc, char* argv[])
{
	// you can use your weights
	weight[0] = -2.43992;
	weight[1] = 0.346942;
	weight[2] = 0.303959;
	weight[3] = -1.81573;
	weight[4] = 0.307872;
	weight[5] = 1.71534;
	weight[6] = 0.705672;
	weight[7] = 1.0054;
	weight[8] = 0.486977;
	weight[9] = 1.27172;
	weight[10] = 0.197391;
	weight[11] = 0.321255;
	weight[12] = -1.56598;
	weight[13] = 0.173972;
	weight[14] = 0.385298;

	iSoma_update = new struct_Voxel[1000000];
	iSoma_ALL = new struct_Voxel[1000000];
	iSoma_ALL_attr = new struct_Attr[1000000];

	int raw_voxels = 0;
	//int icount_update = 0;				//control candidate to attributes

	sFileName = string(argv[1]);

	std::ifstream fin_in;
	fin_in.open(sFileName);

	int id;
	char tmp_s[50];

	while (fin_in >> sFileName)
	{
		if (strcmp(sFileName.c_str(), "") != 0)
		{

			
			clock_t start_time = clock();
			//cout << endl << endl;

			change_filename();

			AM_all_data.Load_AM(sFileName, 1);
			///////////////////////////////////////////////////////////////////////////////////////////////////////// output z plane for check
			int yy = AM_all_data.idim_y / 4 * 4;
			int xx = AM_all_data.idim_x / 4 * 4;

			int** z_plane = new int*[yy];
			for (int i = 0; i < yy; i++)		z_plane[i] = new int[xx];

			for (int i = 0; i < yy; i++)
			for (int j = 0; j < xx; j++)
				z_plane[i][j] = 0;

			for (int i = 0; i < AM_all_data.idim_z; i++)
			for (int j = 0; j < yy; j++)
			for (int k = 0; k < xx; k++)
			if (z_plane[j][k] < AM_all_data.Data_Ori[i][j][k]) z_plane[j][k] = AM_all_data.Data_Ori[i][j][k];

			///////////////////////////////////////////////////////////////////////////////////////////////////////// output z plane for check

			int hisgram[4096] = { 0 };

			for (int z = 0; z < AM_all_data.idim_z; z++)
			for (int y = 0; y < AM_all_data.idim_y; y++)
			for (int x = 0; x < AM_all_data.idim_x; x++)
			{
				hisgram[(int)AM_all_data.Data[z][y][x].value]++;
			}

			std::ofstream foutattr(sFileName_attr.c_str());
			cout << AM_all_data.iVoxels << endl;

			//
			int cvalue[6] = { 0 };

			for (int dotime = 0; dotime < 6; dotime++)
			{
				double sum = 0;
				int c = 1;
				for (c = 1; c < 4096; c++)
				{
					//foutattr << hisgram[c] << endl;
					sum += hisgram[c];
					if (sum / AM_all_data.iVoxels > dthreshold[dotime]) break;			// first time 0.6
				}
				cvalue[dotime] = c;
			}



			bool stop = false;
			for (int dotime = 0; dotime < 6 && !stop; dotime++)				//10 50 100 150
			{
				if (cout_flag) std::cout << "run " << dotime + 1 << endl;

				cout << "run " << dotime + 1 << endl;
				cout << "threshold is " << dthreshold[dotime] << endl;

				AM_all_data.Re_SetValue(cvalue[dotime]);
	

				int components;
				int bound_count = 0;

				components = AM_all_data.Find_Component(500);               //drop CC smaller then 10000 voxels

				icount_update = 0;
				iSoma_ALL_index = 0;

				do_candidate(components);
				do_attributes(components, raw_voxels);


				double judge = 0;
				double x_train[4] = { 0 };
				cout << dotime + 1 << "run and total " << iSoma_ALL_index << " candidates" << endl;
				foutattr << dotime + 1 << " run and total " << iSoma_ALL_index << " candidates" << endl;

				for (int th =5; th <6;  th++) //for (int th = 1; th < 10; th++)
				{
					//		cout << th << endl;
					icount_update = 0;
					for (int i = 0; i < iSoma_ALL_index; i++)
					{
						x_train[0] = iSoma_ALL_attr[i].attr1;
						x_train[1] = iSoma_ALL_attr[i].attr2;
						x_train[2] = iSoma_ALL_attr[i].attr3;
						x_train[3] = iSoma_ALL_attr[i].attr4;

						judge = weight[0] + weight[1] * x_train[0] + weight[2] * x_train[1] + weight[3] * x_train[2] + weight[4] * x_train[3];
						judge += weight[5] * x_train[0] * x_train[0] + 2 * weight[6] * x_train[0] * x_train[1] + 2 * weight[7] * x_train[0] * x_train[2];
						judge += 2 * weight[8] * x_train[0] * x_train[3] + weight[9] * x_train[1] * x_train[1] + 2 * weight[10] * x_train[1] * x_train[2];
						judge += 2 * weight[11] * x_train[1] * x_train[3] + weight[12] * x_train[2] * x_train[2] + 2 * weight[13] * x_train[2] * x_train[3];
						judge += weight[14] * x_train[3] * x_train[3];

						judge = 1 / (1 + pow(exp(1), -1 * judge));

						//if (th == 1) 
							foutattr << iSoma_ALL_attr[i].attr1 << "\t" << iSoma_ALL_attr[i].attr2 << "\t" << iSoma_ALL_attr[i].attr3 << "\t" << iSoma_ALL_attr[i].attr4 << "\t" << iSoma_ALL[i].x * 4 + 2 << "\t" << iSoma_ALL[i].y * 4 + 2 << "\t" << iSoma_ALL[i].z << "\t" << iSoma_ALL[i].value << "\t" << judge << "\t" << iSoma_ALL[i].bf_value << std::endl;

						if (judge > 0.1*th && iSoma_ALL_attr[i].attr1 > 0.3)
						{
							iSoma_update[icount_update] = iSoma_ALL[i];
							icount_update++;
						}

					}
						cout << "icount_update is ... " << icount_update << endl;

					change_filename(th);

					//for (int i = 0; i < icount_update; i++)
					//{
					//	iSoma_ALL[i] = iSoma_update[i];

					//}

					std::ofstream fsoma(sFileName_output.c_str());

					if (icount_update == 0)
					{
						fsoma << sFileName << "\r";
						fsoma << 0 << "\r";
						fsoma.close();
					}
					else
					{
						//cout << "start checking repeats" << endl;

						int total_soma = 0;
						struct_Voxel tmp_maxsoma;
						struct_Voxel *repeats;
						repeats = new struct_Voxel[icount_update];

						for (int i = 0; i < icount_update; i++){
							int repeat_index = 0;
							if (iSoma_update[i].x != -1)
							{
								repeats[repeat_index++] = iSoma_update[i];
								iSoma_update[i].x = -1;

								for (int j = i + 1; j < icount_update; j++)
								{
									if (iSoma_update[j].x != -1)
									{
										for (int k = 0; k < repeat_index; k++)
										{
											if ((abs(iSoma_update[j].x - repeats[k].x) < 6) && (abs(iSoma_update[j].y - repeats[k].y) < 6) && (abs(iSoma_update[j].z - repeats[k].z) < 6))
											{

												repeats[repeat_index++] = iSoma_update[j];

												iSoma_update[j].x = -1;

												break;
											}

										}
									}


								}
								tmp_maxsoma = repeats[0];

								for (int k = 1; k < repeat_index; k++)
								{

									if (repeats[k].connect_flag > tmp_maxsoma.connect_flag)
										tmp_maxsoma = repeats[k];

								}

								//if (tmp_maxsoma.x > 6 && tmp_maxsoma.y > 6 && tmp_maxsoma.z > 3 && tmp_maxsoma.x < 249 && tmp_maxsoma.y<249 && tmp_maxsoma.z< AM_all_data.idim_z-3)
								iSoma_update[total_soma++] = tmp_maxsoma;

								//cout << total_soma << endl;
							}
						}
						//	cout << "done repeats" << endl;
						delete[] repeats;
						icount_update = total_soma;
						//		cout << "total " << icount_update << endl;
						fsoma << sFileName << "\n" << icount_update << "\n";

						for (int i = 0; i < icount_update; i++)
							fsoma << iSoma_update[i].x  << " " << iSoma_update[i].y  << " " << iSoma_update[i].z << " " << iSoma_update[i].value << "\n";
						fsoma.close();


						int ysize = AM_all_data.idim_y;
						int xsize = AM_all_data.idim_x;
						if (xsize % 4 == 1)xsize = xsize + 3;
						else if (xsize % 4 == 2)xsize = xsize + 2;
						else if (xsize % 4 == 3)xsize = xsize + 1;

						if (ysize % 4 == 1)ysize = ysize + 3;
						else if (ysize % 4 == 2)ysize = ysize + 2;
						else if (ysize % 4 == 3)ysize = ysize + 1;

						int file_size = ysize * xsize * 3 + 54;
						header[2] = (unsigned char)(file_size & 0x000000ff);
						header[3] = (file_size >> 8) & 0x000000ff;
						header[4] = (file_size >> 16) & 0x000000ff;
						header[5] = (file_size >> 24) & 0x000000ff;

						int width = xsize;
						header[18] = width & 0x000000ff;
						header[19] = (width >> 8) & 0x000000ff;
						header[20] = (width >> 16) & 0x000000ff;
						header[21] = (width >> 24) & 0x000000ff;

						int height = ysize;
						header[22] = height & 0x000000ff;
						header[23] = (height >> 8) & 0x000000ff;
						header[24] = (height >> 16) & 0x000000ff;
						header[25] = (height >> 24) & 0x000000ff;

						std::ofstream file_z(sFileName_bmp_z.c_str(), std::ofstream::binary);

						std::cout << "log1";
						char pixel = 0;
						double pp = 0;
						file_z.write(header, 54);
						// write body
						//for (int i = 0; i < ysize; i++) {
						for (int i = ysize - 1; i >= 0; i--) {
							for (int j = 0; j < xsize; j++) {
								if (j < AM_all_data.idim_x && i < AM_all_data.idim_y)
								{

									bool find = false;
									bool find_human = false;
									for (int c = 0; c < icount_update; c++)
									{
										if ((abs((float)(i - iSoma_update[c].y)) + abs((float)(j - iSoma_update[c].x))) < 2)
											//if( abs((float)(i-iSoma_ALL[c].y*4+2))<3 && abs((float)(j-iSoma_ALL[c].x*4+2))<3 )
										{

											find = true;
											break;
										}
									}	

									if (find)
									{
										file_z.put(0).put(0).put(255);
									}
									else
									{
										if (j < xx && i < yy)
										{
											pp = z_plane[i][j];
											pixel = pp / AM_all_data.iIty_Max * 255;
											file_z.put(pixel).put(pixel).put(pixel);
										}
										else{
											pixel = 0;
											file_z.put(pixel).put(pixel).put(pixel);
										}
									}


								}
								else
								{
									pixel = 0;
									file_z.put(pixel).put(pixel).put(pixel);
								}
							}
						}

						file_z.close();

						std::cout << "done pic" << endl;
						stop = true;
					//	break; //break to  dotime


					}

				}		//end for do times

			}
			clock_t end_time = clock();
			std::cout << "total time is " << difftime(end_time, start_time) / CLOCKS_PER_SEC << std::endl;

			foutattr.close();


			for (int i = 0; i < yy; i++)  delete[] z_plane[i];
			delete[] z_plane;


		}
	}	//End all files



	delete[] iSoma_update;
	delete[] iSoma_ALL;
	delete[] iSoma_ALL_attr;



}

void change_filename(int value)
{
	sFileName_output = sFileName;
	//sFileName_landmark=sFileName;

	sFileName_attr = sFileName;
	sFileName_bmp_z = sFileName;

	int len = sFileName.length();

	sFileName_output.resize(len + 6);
	sFileName_output[len - 3] = '.';
	sFileName_output[len - 2] = 's';
	sFileName_output[len - 1] = 'o';
	sFileName_output[len] = 'm';
	sFileName_output[len + 1] = 'a';
	sFileName_output[len + 2] = '\0';

	sFileName_attr.resize(len + 4);
	sFileName_attr[len - 3] = '.';
	sFileName_attr[len - 2] = 'a';
	sFileName_attr[len - 1] = 't';
	sFileName_attr[len] = 't';
	sFileName_attr[len + 1] = 'r';
	sFileName_attr[len + 2] = '\0';

	sFileName_bmp_z.resize(len + 6);
	sFileName_bmp_z[len - 3] = '_';
	sFileName_bmp_z[len - 2] = 'x';
	sFileName_bmp_z[len - 1] = 'y';
	sFileName_bmp_z[len] = '.';
	sFileName_bmp_z[len + 1] = 'b';
	sFileName_bmp_z[len + 2] = 'm';
	sFileName_bmp_z[len + 3] = 'p';
	sFileName_bmp_z[len + 4] = '\0';

}

void do_attributes(int components, int raw_voxels)
{
	int bound_count;

	for (int com = 0; com <components; com++)								//isolation
	{
		//std::cout << "go c=" << c << endl;
		csVoxels.Get_Component(AM_all_data, com + 1, bound_count);
		csVoxels.Set_BFvalue();
		//std::cout<<"set value ok"<<std::endl;
		//std::cout << csVoxels.idim_x << " " << csVoxels.idim_y << " " << csVoxels.idim_z << " bbbx " << endl;
		struct_Voxel *union_find = new struct_Voxel[csVoxels.iVoxels / 5];

		for (int num = 0; num<icount_update; num++)
		{
			int z = iSoma_update[num].z - ((csVoxels.dbounding_Z_Min - AM_all_data.dbounding_Z_Min) / csVoxels.dvxsize_z);
			int y = iSoma_update[num].y - ((csVoxels.dbounding_Y_Min - AM_all_data.dbounding_Y_Min) / csVoxels.dvxsize_y);
			int x = iSoma_update[num].x - ((csVoxels.dbounding_X_Min - AM_all_data.dbounding_X_Min) / csVoxels.dvxsize_x);
			//std::cout << " num " << num;
			if (x>0 && x<csVoxels.idim_x && y>0 && y<csVoxels.idim_y && z>0 && z<csVoxels.idim_z && csVoxels.Data[z][y][x].value > 0 && csVoxels.Data[z][y][x].bf_value > 1)
			{
				//std::cout<<"real "<<iSoma_update[num].x<<" "<<iSoma_update[num].y<<" "<<iSoma_update[num].z<<std::endl;
				//std::cout<<"num "<<num<<"   "<<x<<" "<<y<<" "<<z<<" hit component "<<c<<" value is "<<csVoxels.Data[z][y][x].value<<std::endl;
				int intensity = csVoxels.Data[z][y][x].value;
				int dfield_value = csVoxels.Data[z][y][x].bf_value;
				bool soma = true;
	
				double tmp_value_average = 0;
				for (int new_x = -1; new_x <= 1; new_x++)
				for (int new_y = -1; new_y <= 1; new_y++)
				for (int new_z = -1; new_z <= 1; new_z++)
					tmp_value_average += csVoxels.Data[z + new_z][y + new_y][x + new_x].value;
		
				double train_ity;
				if (csVoxels.iIty_Max > 255) train_ity = (double)tmp_value_average / 27 / 4096;
				else train_ity = (double)tmp_value_average / 27 / 255;

		
				float x1 = fabs((float)x / csVoxels.idim_x - 0.5);
				float y1 = fabs((float)y / csVoxels.idim_y - 0.5);
				float z1 = fabs((float)z / csVoxels.idim_z - 0.5);
				float train_coord = x1 > y1 ? x1 > z1 ? x1 : z1 : y1 > z1 ? y1 : z1;

				train_coord *= 2;

		
				double tmp_density_average = 0;
				for (int new_x = -1; new_x <= 1; new_x++)
				{
					for (int new_y = -1; new_y <= 1; new_y++)
					{
						for (int new_z = -1; new_z <= 1; new_z++)
						{
							if (csVoxels.Data[z + new_z][y + new_y][x + new_x].value > csVoxels.iIty_Avg)
								tmp_density_average++;
						}
					}
				}
			
				double train_den = tmp_density_average / 27;
				double ratio_x = 0, ratio_y = 0, ratio_z = 0, ratio_xyz = 0;

				for (int rz = 0; rz < csVoxels.idim_z; rz++)
				for (int ry = 0; ry < csVoxels.idim_y; ry++)
				for (int rx = 0; rx < csVoxels.idim_x; rx++)
					csVoxels.Data[rz][ry][rx].connect_flag = 0;


				int threshold_voxel = csVoxels.iVoxels / 10;
			
				int union_index = 0;
				union_find[union_index++] = csVoxels.Data[z][y][x];
				csVoxels.Data[z][y][x].connect_flag = 1;

				int B_value = union_find[0].bf_value;					//windows size bf_value ^ 3

				int zz, yy, xx;
				int real_upbz = csVoxels.idim_z - 2, real_upby = csVoxels.idim_y - 2, real_upbx = csVoxels.idim_x - 2;
				int real_dnbz = 1, real_dnby = 1, real_dnbx = 1;
				int rela_upbz = z + B_value * 2 + 1, rela_upby = y + B_value * 2 + 1, rela_upbx = x + B_value * 2 + 1;
				int rela_dnbz = z - B_value * 2 - 1, rela_dnby = y - B_value * 2 - 1, rela_dnbx = x - B_value * 2 - 1;

				for (int i = 0; i<union_index && soma; i++)
				{
					zz = union_find[i].z, yy = union_find[i].y, xx = union_find[i].x;
					if (zz<real_upbz && zz<rela_upbz && csVoxels.Data[zz + 1][yy][xx].bf_value>0 && csVoxels.Data[zz + 1][yy][xx].connect_flag == 0) { csVoxels.Data[zz + 1][yy][xx].connect_flag = 1; union_find[union_index++] = csVoxels.Data[zz + 1][yy][xx]; }
					if (yy<real_upby && yy<rela_upby && csVoxels.Data[zz][yy + 1][xx].bf_value>0 && csVoxels.Data[zz][yy + 1][xx].connect_flag == 0) { csVoxels.Data[zz][yy + 1][xx].connect_flag = 1; union_find[union_index++] = csVoxels.Data[zz][yy + 1][xx]; }
					if (xx<real_upbx && xx<rela_upbx && csVoxels.Data[zz][yy][xx + 1].bf_value>0 && csVoxels.Data[zz][yy][xx + 1].connect_flag == 0) { csVoxels.Data[zz][yy][xx + 1].connect_flag = 1; union_find[union_index++] = csVoxels.Data[zz][yy][xx + 1]; }

					if (zz>real_dnbz && zz>rela_dnbz && csVoxels.Data[zz - 1][yy][xx].bf_value>0 && csVoxels.Data[zz - 1][yy][xx].connect_flag == 0) { csVoxels.Data[zz - 1][yy][xx].connect_flag = 1; union_find[union_index++] = csVoxels.Data[zz - 1][yy][xx]; }
					if (yy>real_dnby && zz > rela_dnby && csVoxels.Data[zz][yy - 1][xx].bf_value > 0 && csVoxels.Data[zz][yy - 1][xx].connect_flag == 0) { csVoxels.Data[zz][yy - 1][xx].connect_flag = 1; union_find[union_index++] = csVoxels.Data[zz][yy - 1][xx]; }
					if (xx > real_dnbx && zz > rela_dnbx && csVoxels.Data[zz][yy][xx - 1].bf_value > 0 && csVoxels.Data[zz][yy][xx - 1].connect_flag == 0) { csVoxels.Data[zz][yy][xx - 1].connect_flag = 1; union_find[union_index++] = csVoxels.Data[zz][yy][xx - 1]; }

					if (union_index > threshold_voxel) break;
				}

				//cout << "total PCA number is " << union_index << endl;

				int M = 3, N = union_index;

				double** data = new double*[N];
				for (int i = 0; i < N; i++)
					data[i] = new double[M];
				int c = 0;
				for (int ci = 0; ci < N; ci++)
				{
					data[ci][0] = union_find[c].x;
					data[ci][1] = union_find[c].y;
					data[ci][2] = union_find[c].z;
					c++;
				}

				double** result = new double*[M];
				for (int i = 0; i < M; i++)
					result[i] = new double[M + 1];

				//	for(int i=0;i<N+1;i++) result[i][0]=result[i][1]=0;

				result = PCA(data, M, N, true);

				//cout << result[0][0] << " " << result[0][1] << " " << result[0][2] << " " << result[0][3] << endl;

				rela_upbz = z + B_value * 4 + 1, rela_upby = y + B_value * 4 + 1, rela_upbx = x + B_value * 4 + 1;
				rela_dnbz = z - B_value * 4 - 1, rela_dnby = y - B_value * 4 - 1, rela_dnbx = x - B_value * 4 - 1;

				union_index = 1;
				csVoxels.Data[z][y][x].connect_flag = 2;
				for (int i = 0; i<union_index && soma; i++)
				{
					zz = union_find[i].z, yy = union_find[i].y, xx = union_find[i].x;
					if (zz<real_upbz  && csVoxels.Data[zz + 1][yy][xx].bf_value>0 && csVoxels.Data[zz + 1][yy][xx].connect_flag != 2) { csVoxels.Data[zz + 1][yy][xx].connect_flag = 2; union_find[union_index++] = csVoxels.Data[zz + 1][yy][xx]; }
					if (yy<real_upby  && csVoxels.Data[zz][yy + 1][xx].bf_value>0 && csVoxels.Data[zz][yy + 1][xx].connect_flag != 2) { csVoxels.Data[zz][yy + 1][xx].connect_flag = 2; union_find[union_index++] = csVoxels.Data[zz][yy + 1][xx]; }
					if (xx<real_upbx  && csVoxels.Data[zz][yy][xx + 1].bf_value>0 && csVoxels.Data[zz][yy][xx + 1].connect_flag != 2) { csVoxels.Data[zz][yy][xx + 1].connect_flag = 2; union_find[union_index++] = csVoxels.Data[zz][yy][xx + 1]; }

					if (zz>real_dnbz &&  csVoxels.Data[zz - 1][yy][xx].bf_value > 0 && csVoxels.Data[zz - 1][yy][xx].connect_flag != 2) { csVoxels.Data[zz - 1][yy][xx].connect_flag = 2; union_find[union_index++] = csVoxels.Data[zz - 1][yy][xx]; }
					if (yy > real_dnby &&  csVoxels.Data[zz][yy - 1][xx].bf_value > 0 && csVoxels.Data[zz][yy - 1][xx].connect_flag != 2) { csVoxels.Data[zz][yy - 1][xx].connect_flag = 2; union_find[union_index++] = csVoxels.Data[zz][yy - 1][xx]; }
					if (xx > real_dnbx &&  csVoxels.Data[zz][yy][xx - 1].bf_value > 0 && csVoxels.Data[zz][yy][xx - 1].connect_flag != 2) { csVoxels.Data[zz][yy][xx - 1].connect_flag = 2; union_find[union_index++] = csVoxels.Data[zz][yy][xx - 1]; }
					if (union_index > threshold_voxel) break;
				}

				//cout << "union_index" << union_index << endl;



				double v1, v2, v3;
				double b_positive = 0, b_negative = 0;
				double value;
				for (int i = 1; i < union_index; i++)
				{
					v1 = union_find[i].x - x;
					v2 = union_find[i].y - y;
					v3 = union_find[i].z - z;
					value = v1*result[0][0] + v2*result[0][1] + v3*result[0][2];
					if (value>0) b_positive++;
					else if (value < 0) b_negative++;
				}
				//cout << "pos" << b_positive << "  neg" << b_negative << endl;

				for (int i = 0; i < N; i++) delete[] data[i];
				delete[] data;

				for (int i = 0; i < M; i++) delete result[i];
				delete[] result;

				if (b_positive > b_negative) ratio_xyz = b_negative / b_positive;
				else ratio_xyz = b_positive / b_negative;

				ratio_xyz = 1 - ratio_xyz;
				double x_train[4] = { 0 };
				x_train[0] = train_ity; x_train[1] = train_coord; x_train[2] = train_den; x_train[3] = ratio_xyz;


				iSoma_ALL[iSoma_ALL_index].value = iSoma_update[num].value;
				iSoma_ALL[iSoma_ALL_index].connect_flag = com + 1;
				iSoma_ALL[iSoma_ALL_index].z = iSoma_update[num].z;
				iSoma_ALL[iSoma_ALL_index].y = iSoma_update[num].y;
				iSoma_ALL[iSoma_ALL_index].x = iSoma_update[num].x;
				iSoma_ALL[iSoma_ALL_index].bf_value = iSoma_update[num].bf_value;

				iSoma_ALL_attr[iSoma_ALL_index].attr1 = x_train[0];
				iSoma_ALL_attr[iSoma_ALL_index].attr2 = x_train[1];
				iSoma_ALL_attr[iSoma_ALL_index].attr3 = x_train[2];
				iSoma_ALL_attr[iSoma_ALL_index].attr4 = x_train[3];
				iSoma_ALL_index++;

			}

		}		// end of icount_update
		//cout << "// end of icount_update" << endl;
		delete[] union_find;
		//cout << "// end of union" << endl;
	}		//end of components

}
void do_candidate(int components)
{
	int bound_count;

	for (int c = 0; c<components; c++)								//isolation
	{
		sprintf(buffers, "%s_%d.am", sFileName.c_str(), c + 1);
		if (cout_flag) std::cout << buffers << std::endl;

		csVoxels.Get_Component(AM_all_data, c + 1, bound_count);

		if (csVoxels.iVoxels == 0) {
			if (cout_flag) std::cout << "iVoxels = 0" << std::endl;
			break;
		}

		//cout<<"checking for boundary...";

		if (bound_count>100)       
		{
			if (cout_flag) std::cout << "boundary fail" << std::endl;
			  cout<<"boundary fail"<<std::endl;
			continue;
		}
		//
		csVoxels.Set_BFvalue();

		if (cout_flag) std::cout << "find maximal points...bfvalue= ";
		struct_Voxel *iSoma_loc = new struct_Voxel[1000000];
		struct_Voxel *union_find = new struct_Voxel[csVoxels.iVoxels];
		int union_index = 0, component_index = 1;
		int bvalue = csVoxels.iBF_Max;

		int icount = 0;
		int x, y, z;
		bool find = false;
		// std::cout<<bvalue;
		int union_thres = 5;
		int can_value;
		if (cout_flag) cout << "bf= " << bvalue << endl;

		int ity_th = 0;
		if (csVoxels.iIty_Max > 255) ity_th = 0.3 * 4096;
		else ity_th = 0.3 * 255;

		for (int i = 0; i< csVoxels.idim_z; i++)
		for (int j = 0; j<csVoxels.idim_y; j++)
		for (int k = 0; k<csVoxels.idim_x; k++)
		if (csVoxels.Data[i][j][k].value<ity_th)
			csVoxels.Data[i][j][k].connect_flag = -1;

		while (bvalue>csVoxels.iBF_Max / 5 && bvalue> BF_th)
			//while (bvalue>2)
		{
			find = false;
			int tmp_ity = 0;

			for (int i = 0; i< csVoxels.idim_z && !find; i++)
			for (int j = 0; j<csVoxels.idim_y && !find; j++)
			for (int k = 0; k<csVoxels.idim_x && !find; k++)
			{
				if (csVoxels.Data[i][j][k].bf_value == bvalue && csVoxels.Data[i][j][k].connect_flag == 0 && csVoxels.Data[i][j][k].value> csVoxels.iIty_Avg)
				{
					tmp_ity = csVoxels.Data[i][j][k].value;
					csVoxels.Data[i][j][k].connect_flag = component_index;
					union_find[union_index++] = csVoxels.Data[i][j][k];
					find = true;
					break;
				}
			}

			if (find)
			{
				can_value = union_find[0].value;
				for (int i = 0; i<union_index; i++)
				{
					z = union_find[i].z;  y = union_find[i].y; x = union_find[i].x;
					int t_value = union_find[i].bf_value;

					if (csVoxels.Data[z - 1][y][x].connect_flag == 0 && csVoxels.Data[z - 1][y][x].value < can_value + 10 && z > 0 && csVoxels.Data[z - 1][y][x].bf_value > 1 && csVoxels.Data[z - 1][y][x].bf_value <= t_value) { csVoxels.Data[z - 1][y][x].connect_flag = component_index; union_find[union_index++] = csVoxels.Data[z - 1][y][x]; }
					if (csVoxels.Data[z + 1][y][x].connect_flag == 0 && csVoxels.Data[z + 1][y][x].value < can_value + 10 && z < csVoxels.idim_z - 1 && csVoxels.Data[z + 1][y][x].bf_value >1 && csVoxels.Data[z + 1][y][x].bf_value <= t_value) { csVoxels.Data[z + 1][y][x].connect_flag = component_index; union_find[union_index++] = csVoxels.Data[z + 1][y][x]; }
					if (csVoxels.Data[z][y - 1][x].connect_flag == 0 && csVoxels.Data[z][y - 1][x].value < can_value + 10 && y > 0 && csVoxels.Data[z][y - 1][x].bf_value > 1 && csVoxels.Data[z][y - 1][x].bf_value <= t_value) { csVoxels.Data[z][y - 1][x].connect_flag = component_index; union_find[union_index++] = csVoxels.Data[z][y - 1][x]; }
					if (csVoxels.Data[z][y + 1][x].connect_flag == 0 && csVoxels.Data[z][y + 1][x].value < can_value + 10 && y < csVoxels.idim_y - 1 && csVoxels.Data[z][y + 1][x].bf_value >1 && csVoxels.Data[z][y + 1][x].bf_value <= t_value) { csVoxels.Data[z][y + 1][x].connect_flag = component_index; union_find[union_index++] = csVoxels.Data[z][y + 1][x]; }
					if (csVoxels.Data[z][y][x - 1].connect_flag == 0 && csVoxels.Data[z][y][x - 1].value < can_value + 10 && x > 0 && csVoxels.Data[z][y][x - 1].bf_value > 1 && csVoxels.Data[z][y][x - 1].bf_value <= t_value) { csVoxels.Data[z][y][x - 1].connect_flag = component_index; union_find[union_index++] = csVoxels.Data[z][y][x - 1]; }
					if (csVoxels.Data[z][y][x + 1].connect_flag == 0 && csVoxels.Data[z][y][x + 1].value < can_value + 10 && x < csVoxels.idim_x - 1 && csVoxels.Data[z][y][x + 1].bf_value >1 && csVoxels.Data[z][y][x + 1].bf_value <= t_value) { csVoxels.Data[z][y][x + 1].connect_flag = component_index; union_find[union_index++] = csVoxels.Data[z][y][x + 1]; }

				}
				//std::cout<<"QQ "<<union_find[0].x<<" "<<union_find[0].y<<" "<<union_find[0].z<<"   ";

				int tmp_x = 0, tmp_y = 0, tmp_z = 0, tmp_c = 0;

				if (union_index > union_thres)
				{


					for (int i = 0; i<union_index; i++)
					{
						if ((union_find[i].bf_value > bvalue - 3)) 
						{
							tmp_x += union_find[i].x;
							tmp_y += union_find[i].y;
							tmp_z += union_find[i].z;
							tmp_c++;
						}
						else if (sqrt((union_find[i].x - union_find[0].x)*(union_find[i].x - union_find[0].x) + (union_find[i].y - union_find[0].y)*(union_find[i].y - union_find[0].y) + (union_find[i].z - union_find[0].z)*(union_find[i].z - union_find[0].z)) < 3)
						{
							tmp_x += union_find[i].x;
							tmp_y += union_find[i].y;
							tmp_z += union_find[i].z;
							tmp_c++;
						}
					}

					tmp_x /= tmp_c; tmp_y /= tmp_c; tmp_z /= tmp_c;

					iSoma_loc[icount++] = csVoxels.Data[tmp_z][tmp_y][tmp_x];
				}

				union_index = 0;
			}
			else         //not find
			{
				if (cout_flag) std::cout << bvalue << " ";
				bvalue--;

			}
		}

		if (cout_flag) std::cout << "total candidate is " << icount << std::endl;
		double temp_z = 0, temp_y = 0, temp_x = 0;
		temp_z = (csVoxels.dbounding_Z_Min - AM_all_data.dbounding_Z_Min) / csVoxels.dvxsize_z;
		temp_y = (csVoxels.dbounding_Y_Min - AM_all_data.dbounding_Y_Min) / csVoxels.dvxsize_y;
		temp_x = (csVoxels.dbounding_X_Min - AM_all_data.dbounding_X_Min) / csVoxels.dvxsize_x;

		if ((temp_z - (int)temp_z) >  0.5) temp_z++;
		if ((temp_y - (int)temp_y) >  0.5) temp_y++;
		if ((temp_x - (int)temp_x) >  0.5) temp_x++;

		for (int i = 0; i<icount; i++)
		{

			iSoma_update[icount_update].value = iSoma_loc[i].value;
			iSoma_update[icount_update].connect_flag = c + 1;
			iSoma_update[icount_update].bf_value = iSoma_loc[i].bf_value;
			iSoma_update[icount_update].z = iSoma_loc[i].z + temp_z;
			iSoma_update[icount_update].y = iSoma_loc[i].y + temp_y;
			iSoma_update[icount_update].x = iSoma_loc[i].x + temp_x;

			icount_update++;
			// }
		}
		delete[] union_find;
		delete[] iSoma_loc;


		////////////////////////////////////////////////////////////// all candidate

	}		// END for(int c=0;c<components; c++)								//isolation

	if (cout_flag) std::cout << "total candidates " << icount_update << std::endl;
}
