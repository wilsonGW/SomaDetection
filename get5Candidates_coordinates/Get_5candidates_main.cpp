// Created by program: Soma_Detection 
// Author: Guan-wei He

#include<iostream>
#include <fstream>
#include<string>
#include<string.h>
#include <cmath>
#include <time.h>
#include "AM_format.h"

//parameters///////////////////////////////////////////////////////////////////
//Parameters   you can modify by yourself

#define TH 40					// global threshold (intensity value) percentage 

////////////////////////////////////////////////////////////////////////////////

using namespace std;

void change_filename();

struct_Voxel iSoma_ALL[50];

string sFileName="";
string sFileName_output="";
string sFileName_bmp_x="";
string sFileName_bmp_y="";
string sFileName_bmp_z="";


char header[54] = {
      0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,
        54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0
    };
    

int main(int argc, char* argv[])
{
	clock_t start_time = clock();

	AM_data AM_all_data;

	int ity;
	int iSoma_ALL_index=0;

	double th = TH;
	int hisgram[4096] = { 0 };
	
	sFileName = string(argv[1]);

	std::ifstream fin_in;
	fin_in.open(sFileName);

	while (fin_in >> sFileName)
	{
		if (strcmp(sFileName.c_str(), "") != 0)
		{
			AM_all_data.Load_AM(sFileName, 1);

			for (int i = 0; i < 4096; i++) hisgram[i] = 0;

			for (int z = 0; z < AM_all_data.idim_z; z++)
			for (int y = 0; y < AM_all_data.idim_y; y++)
			for (int x = 0; x < AM_all_data.idim_x; x++)
			{
				hisgram[(int)AM_all_data.Data[z][y][x].value]++;
			}
			
			th = TH;

			double sum = 0;
			int c = 1;
			for (c = 1; c < 4096; c++)
			{
				//foutattr << hisgram[c] << endl;
				sum += hisgram[c];
				if (sum / AM_all_data.iVoxels > th*0.01) break;			
			}
						
			AM_all_data.Re_SetValue(c);
			AM_all_data.Set_BFvalue();
			
			int iSoma_ALL_index = 0;
			struct_Voxel tmp_soma;
			tmp_soma.value = 0;
			
			cout << "BF MAX is " << AM_all_data.iBF_Max << endl;

			// find the maximum BF value voxels

			for (int i = 0; i < AM_all_data.idim_z; i++)
			for (int j = 0; j < AM_all_data.idim_y; j++)
			for (int k = 0; k<AM_all_data.idim_x; k++)
			{
				if (AM_all_data.Data[i][j][k].bf_value == AM_all_data.iBF_Max)
				{
					if (AM_all_data.Data[i][j][k].value>tmp_soma.value)
					{
						tmp_soma.value = AM_all_data.Data[i][j][k].value;
						tmp_soma.z = i;
						tmp_soma.y = j;
						tmp_soma.x = k;
					}
				}
			}
			
			iSoma_ALL[iSoma_ALL_index] = tmp_soma;
			iSoma_ALL_index++;

			int test_count = 0;
			int iwindow = AM_all_data.iBF_Max - 1;

			for (int i = -iwindow; i < iwindow; i++)			//cut first
			for (int j = -iwindow; j < iwindow; j++)
			for (int k = -iwindow; k < iwindow; k++)
			{
				if (AM_all_data.Data[tmp_soma.z + i][tmp_soma.y + j][tmp_soma.x + k].value == 0) test_count++;
				else{

					AM_all_data.Data[tmp_soma.z + i][tmp_soma.y + j][tmp_soma.x + k].value = 0;
					AM_all_data.iVoxels--;
				}

			}

			cout << "second point..." << endl;

			AM_all_data.Set_BFvalue();
			cout << "BF MAX" << AM_all_data.iBF_Max << endl;
			tmp_soma.value = 0;
			for (int i = 0; i < AM_all_data.idim_z; i++)
			for (int j = 0; j < AM_all_data.idim_y; j++)
			for (int k = 0; k<AM_all_data.idim_x; k++)
			{
				if (AM_all_data.Data[i][j][k].bf_value == AM_all_data.iBF_Max)
				{
					if (AM_all_data.Data[i][j][k].value>tmp_soma.value)
					{
						tmp_soma.value = AM_all_data.Data[i][j][k].value;
						tmp_soma.z = i;
						tmp_soma.y = j;
						tmp_soma.x = k;
					}
				}
			}

			iSoma_ALL[iSoma_ALL_index] = tmp_soma;
			iSoma_ALL_index++;

			test_count = 0;
			iwindow = AM_all_data.iBF_Max - 1;
			for (int i = -iwindow; i < iwindow; i++)
			for (int j = -iwindow; j < iwindow; j++)
			for (int k = -iwindow; k < iwindow; k++)
			{
				if (AM_all_data.Data[tmp_soma.z + i][tmp_soma.y + j][tmp_soma.x + k].value == 0) test_count++;
				else{

					AM_all_data.Data[tmp_soma.z + i][tmp_soma.y + j][tmp_soma.x + k].value = 0;
					AM_all_data.iVoxels--;
				}

			}

			cout << "third point..." << endl;	
			AM_all_data.Set_BFvalue();
			cout << "BF MAX" << AM_all_data.iBF_Max << endl;
			tmp_soma.value = 0;
			for (int i = 0; i < AM_all_data.idim_z; i++)
			for (int j = 0; j < AM_all_data.idim_y; j++)
			for (int k = 0; k<AM_all_data.idim_x; k++)
			{
				if (AM_all_data.Data[i][j][k].bf_value == AM_all_data.iBF_Max)
				{
					if (AM_all_data.Data[i][j][k].value>tmp_soma.value)
					{
						tmp_soma.value = AM_all_data.Data[i][j][k].value;
						tmp_soma.z = i;
						tmp_soma.y = j;
						tmp_soma.x = k;
					}
				}
			}


			iSoma_ALL[iSoma_ALL_index] = tmp_soma;
			iSoma_ALL_index++;

			test_count = 0;
			iwindow = AM_all_data.iBF_Max - 1;
			for (int i = -iwindow; i < iwindow; i++)
			for (int j = -iwindow; j < iwindow; j++)
			for (int k = -iwindow; k < iwindow; k++)
			{
				if (AM_all_data.Data[tmp_soma.z + i][tmp_soma.y + j][tmp_soma.x + k].value == 0) test_count++;
				else{

					AM_all_data.Data[tmp_soma.z + i][tmp_soma.y + j][tmp_soma.x + k].value = 0;
					AM_all_data.iVoxels--;
				}

			}
			cout << "fourth point..." << endl;

			AM_all_data.Set_BFvalue();
			cout << "BF MAX" << AM_all_data.iBF_Max << endl;
			tmp_soma.value = 0;
			for (int i = 0; i < AM_all_data.idim_z; i++)
			for (int j = 0; j < AM_all_data.idim_y; j++)
			for (int k = 0; k<AM_all_data.idim_x; k++)
			{
				if (AM_all_data.Data[i][j][k].bf_value == AM_all_data.iBF_Max)
				{
					if (AM_all_data.Data[i][j][k].value>tmp_soma.value)
					{
						tmp_soma.value = AM_all_data.Data[i][j][k].value;
						tmp_soma.z = i;
						tmp_soma.y = j;
						tmp_soma.x = k;
					}
				}
			}


			iSoma_ALL[iSoma_ALL_index] = tmp_soma;
			iSoma_ALL_index++;

			test_count = 0;
			iwindow = AM_all_data.iBF_Max - 1;
			for (int i = -iwindow; i < iwindow; i++)
			for (int j = -iwindow; j < iwindow; j++)
			for (int k = -iwindow; k < iwindow; k++)
			{
				if (AM_all_data.Data[tmp_soma.z + i][tmp_soma.y + j][tmp_soma.x + k].value == 0) test_count++;
				else{

					AM_all_data.Data[tmp_soma.z + i][tmp_soma.y + j][tmp_soma.x + k].value = 0;
					AM_all_data.iVoxels--;
				}

			}
			cout << "fifth point..." << endl;	
			AM_all_data.Set_BFvalue();
			cout << "BF MAX" << AM_all_data.iBF_Max << endl;
			tmp_soma.value = 0;
			for (int i = 0; i < AM_all_data.idim_z; i++)
			for (int j = 0; j < AM_all_data.idim_y; j++)
			for (int k = 0; k<AM_all_data.idim_x; k++)
			{
				if (AM_all_data.Data[i][j][k].bf_value == AM_all_data.iBF_Max)
				{
					if (AM_all_data.Data[i][j][k].value>tmp_soma.value)
					{
						tmp_soma.value = AM_all_data.Data[i][j][k].value;
						tmp_soma.z = i;
						tmp_soma.y = j;
						tmp_soma.x = k;
					}
				}
			}

			iSoma_ALL[iSoma_ALL_index] = tmp_soma;
			iSoma_ALL[iSoma_ALL_index].value = tmp_soma.value;
			iSoma_ALL_index++;

			std::cout << "done" << std::endl;

			test_count = 0;
			iwindow = AM_all_data.iBF_Max - 1;
			for (int i = -iwindow; i < iwindow; i++)
			for (int j = -iwindow; j < iwindow; j++)
			for (int k = -iwindow; k < iwindow; k++)
			{
				if (AM_all_data.Data[tmp_soma.z + i][tmp_soma.y + j][tmp_soma.x + k].value == 0) test_count++;
				else{

					AM_all_data.Data[tmp_soma.z + i][tmp_soma.y + j][tmp_soma.x + k].value = 0;
					AM_all_data.iVoxels--;
				}
			}						

			std::cout << "this data total candidates are " << iSoma_ALL_index << std::endl;
			change_filename();
			std::ofstream fsoma(sFileName_output.c_str());
			if (iSoma_ALL_index == 0)
			{
				fsoma << sFileName << "\r";
				fsoma << 0 << "\r";
			}
			else
			{
				fsoma << sFileName << endl;
				//fsoma << sFileName << " " << iSoma_ALL_index << "\n";

				for (int i = 0; i < iSoma_ALL_index; i++)
					fsoma << iSoma_ALL[i].x << " " << iSoma_ALL[i].y << " " << iSoma_ALL[i].z << " " << iSoma_ALL[i].value << "\n";
					//fsoma << iSoma_ALL[i].x*AM_all_data.dvxsize_x << " " << iSoma_ALL[i].y*AM_all_data.dvxsize_y << " " << iSoma_ALL[i].z*AM_all_data.dvxsize_z << " " << iSoma_ALL[i].value << "\n";

				int** z_plane = new int*[AM_all_data.idim_y];
				for (int i = 0; i < AM_all_data.idim_y; i++)		z_plane[i] = new int[AM_all_data.idim_x];

				for (int i = 0; i < AM_all_data.idim_y; i++)
				for (int j = 0; j < AM_all_data.idim_x; j++)
					z_plane[i][j] = 0;

				for (int i = 0; i < AM_all_data.idim_z; i++)
				for (int j = 0; j < AM_all_data.idim_y; j++)
				for (int k = 0; k < AM_all_data.idim_x; k++)
				if (z_plane[j][k] < AM_all_data.Data[i][j][k].value) z_plane[j][k] = AM_all_data.Data[i][j][k].value;


				int ysize = AM_all_data.idim_y;
				int xsize = AM_all_data.idim_x;
				if (xsize % 4 == 1)xsize = xsize + 3;
				else if (xsize % 4 == 2)xsize = xsize + 2;
				else if (xsize % 4 == 3)xsize = xsize + 1;

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

				char pixel = 0;
				double pp = 0;
				file_z.write(header, 54);
				// write body
				for (int i = ysize - 1; i >= 0; i--) {
					for (int j = 0; j < xsize; j++) {
						if (j < AM_all_data.idim_x)
						{
							bool find = false;
							for (int c = 0; c < iSoma_ALL_index; c++)
							{

								if (abs((float)(i - iSoma_ALL[c].y)) < 4 && abs((float)(j - iSoma_ALL[c].x)) < 4)
								{
									if (c == 0)	  file_z.put(0).put(255).put(0);
									else if (c == 1) file_z.put(255).put(0).put(0);
									else if (c == 2) file_z.put(0).put(0).put(255);
									else if (c == 3) file_z.put(255).put(255).put(0);
									else if (c == 4) file_z.put(0).put(255).put(255);
									else if (c == 5) file_z.put(255).put(0).put(255);
									else file_z.put(255).put(100).put(100);
									find = true;
									break;
								}
							}
							if (!find)
							{
								pp = z_plane[i][j];
								pixel = pp / AM_all_data.iIty_Max * 255;
								file_z.put(pixel).put(pixel).put(pixel);
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


				for (int i = 0; i < AM_all_data.idim_y; i++)  delete[] z_plane[i];
				delete[] z_plane;

			}
		}	//End all files
		else
			cout << "no such file   " << sFileName << endl;
		AM_all_data.AM_data_clear();
	}

	fin_in.close();
	clock_t end_time = clock();
	std::cout << "total time is " << difftime(end_time, start_time) / CLOCKS_PER_SEC << std::endl;

}

void change_filename()
{
	  sFileName_output=sFileName;
	  //sFileName_landmark=sFileName;

	  sFileName_bmp_x=sFileName;
	  sFileName_bmp_y=sFileName;
	  sFileName_bmp_z=sFileName;

	  int len=sFileName.length();
	  sFileName_output.resize(len+4);
	  sFileName_output[len-3]='.';
	  sFileName_output[len-2]='c';
	  sFileName_output[len-1]='o';
	  sFileName_output[len]='o';
	  sFileName_output[len+1]='r';
	  sFileName_output[len+2]='\0';


	  sFileName_bmp_z.resize(len+5);
	  sFileName_bmp_z[len-3]='_';
	  sFileName_bmp_z[len-2]='x';
	  sFileName_bmp_z[len-1]='y';
	  sFileName_bmp_z[len]='.';
	  sFileName_bmp_z[len+1]='b';
	  sFileName_bmp_z[len+2]='m';
	  sFileName_bmp_z[len+3]='p';
	  sFileName_bmp_z[len+4]='\0';


}
