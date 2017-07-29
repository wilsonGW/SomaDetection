// Created by program: Soma_Detection 
// Author: Guan-wei He

#include "AM_format.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <string.h>

//#define INT_MAX 999999
//#define INT_MIN -999999
using namespace std;
bool output_flag = true;
char buffer[200];
//////////////////////////////////////////////////////////////////////////////
//Constructor initial all
//////////////////////////////////////////////////////////////////////////////
AM_data::AM_data()
{
	idim_x = 0;	 idim_y = 0;	 idim_z = 0;

	iVoxels = 0;
	iThreshold = 0;
	iIty_Max = INT_MIN;	 iIty_Min = INT_MAX;
	iIty_Avg = 0;
	iBF_Max = 0;

	dbounding_X_Min = 0;	 dbounding_X_Max = 0;
	dbounding_Y_Min = 0;	 dbounding_Y_Max = 0;
	dbounding_Z_Min = 0;  dbounding_Z_Max = 0;

	dvxsize_x = 0;	 dvxsize_y = 0;	 dvxsize_z = 0;
	bdata_flag = false;
	bcomp_flag = false;

	connect_index = 0;
}

//////////////////////////////////////////////////////////////////////////////
//Copy Constructor
//////////////////////////////////////////////////////////////////////////////
AM_data::AM_data(const AM_data& rhs)
{
	idim_x = rhs.idim_x;	 	idim_y = rhs.idim_y;	 	idim_z = rhs.idim_z;

	iVoxels = rhs.iVoxels;
	iThreshold = rhs.iThreshold;
	iIty_Max = rhs.iIty_Max;	 iIty_Min = rhs.iIty_Min;
	iIty_Avg = rhs.iIty_Avg;

	iBF_Max = rhs.iBF_Max;

	dbounding_X_Min = rhs.dbounding_X_Min;	dbounding_X_Max = rhs.dbounding_X_Max;
	dbounding_Y_Min = rhs.dbounding_Y_Min;	dbounding_Y_Max = rhs.dbounding_Y_Max;
	dbounding_Z_Min = rhs.dbounding_Z_Min;	dbounding_Z_Max = rhs.dbounding_Z_Max;

	dvxsize_x = rhs.dvxsize_x;	 dvxsize_y = rhs.dvxsize_y;	 dvxsize_z = rhs.dvxsize_z;
	bdata_flag = rhs.bdata_flag;
	bcomp_flag = rhs.bcomp_flag;
	connect_index = rhs.connect_index;

	if (rhs.bdata_flag)
	{
		Data = new struct_Voxel**[idim_z];
		Data_Ori = new short**[idim_z];

		for (int i = 0; i<idim_z; i++)
		{
			Data[i] = new struct_Voxel*[idim_y];
			Data_Ori[i] = new short*[idim_y];
			for (int j = 0; j<idim_y; j++)
			{
				Data[i][j] = new struct_Voxel[idim_x];
				Data_Ori[i][j] = new short[idim_x];
			}
		}

		for (int i = 0; i<idim_z; i++)
		for (int j = 0; j<idim_y; j++)
		for (int k = 0; k<idim_x; k++)
		{
			Data[i][j][k].z = i; Data[i][j][k].y = j; Data[i][j][k].x = k;		// i j k  =  z y x
			Data[i][j][k].value = rhs.Data[i][j][k].value;
			Data[i][j][k].bf_value = rhs.Data[i][j][k].bf_value;
			Data[i][j][k].connect_flag = rhs.Data[i][j][k].connect_flag;

			Data_Ori[i][j][k] = rhs.Data_Ori[i][j][k];
		}
	}

	if (rhs.bcomp_flag)
	{
		iComponents = new int*[2];
		for (int i = 0; i<2; i++) iComponents[i] = new int[connect_index + 1];
		for (int i = 0; i<connect_index + 1; i++) {
			iComponents[0][i] = rhs.iComponents[0][i];
			iComponents[1][i] = rhs.iComponents[1][i];
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//Destructor	reset variables and delete Data storage
//////////////////////////////////////////////////////////////////////////////
void AM_data::AM_data_clear()
{
	if (bdata_flag)
	{
		for (int i = 0; i<idim_z; i++)
		{
			for (int j = 0; j<idim_y; j++)
			{
				delete[] Data[i][j];
				delete[] Data_Ori[i][j];
			}
			delete[] Data[i];
			delete[] Data_Ori[i];
		}
		delete[] Data;
		delete[] Data_Ori;
	}

	if (bcomp_flag)
	{
		for (int i = 0; i<2; i++) delete[] iComponents[i];
		delete[] iComponents;
	}
	idim_x = 0;	idim_y = 0;	idim_z = 0;

	iVoxels = 0;
	iThreshold = 0;
	iIty_Max = INT_MIN;	iIty_Min = INT_MAX;	iIty_Avg = 0;
	iBF_Max = 0;

	dbounding_X_Min = 0;	 dbounding_X_Max = 0;
	dbounding_Y_Min = 0;	 dbounding_Y_Max = 0;
	dbounding_Z_Min = 0;	 dbounding_Z_Max = 0;

	dvxsize_x = 0; 	 dvxsize_y = 0; 	 dvxsize_z = 0;
	bdata_flag = false;
	bcomp_flag = false;
	connect_index = 0;
}
//////////////////////////////////////////////////////////////////////////////
//Load Am
//Input: File_Name
//Call Read Header to continue
//////////////////////////////////////////////////////////////////////////////
void AM_data::Load_AM(std::string sFile_name, int Threshold)
{
	if (bdata_flag) AM_data::AM_data_clear();

	iThreshold = Threshold;
	Read_Header(sFile_name);
	bdata_flag = true;

}

//////////////////////////////////////////////////////////////////////////////
//Load Am file context
//Input: File_name
//////////////////////////////////////////////////////////////////////////////
void AM_data::Read_Header(string sFile_name)
{
	FILE* hRead;
	string sTarget;
	std::cout << "loading...file     " << sFile_name << std::endl;

	hRead = fopen(sFile_name.c_str(), "r");
	if (hRead == 0)
	{
		if (output_flag) std::cout << "no file named " << sFile_name << std::endl;
		return;
	}

	char ch;

	do{
		fscanf(hRead, "%c", &ch);
		sTarget = sTarget + ch;
	} while (ch != '@');

	const char* p = strstr(sTarget.c_str(), "define Lattice");
	sscanf(p, "define Lattice %d %d %d", &idim_x, &idim_y, &idim_z);
	if (output_flag) std::cout << "dimension :" << idim_x << "  " << idim_y << "  " << idim_z << std::endl;
	//cout<<"dimension_y:"<<idim_y<<endl;
	//cout<<"dimension_z:"<<idim_z<<endl;

	p = strstr(sTarget.c_str(), "BoundingBox");
	sscanf(p, "BoundingBox %lf %lf %lf %lf %lf %lf ", &dbounding_X_Min, &dbounding_X_Max, &dbounding_Y_Min, &dbounding_Y_Max, &dbounding_Z_Min, &dbounding_Z_Max);

	//cout<<"BoundingBox x :"<<dbounding_X_Min<<"\t~ "<<dbounding_X_Max<<endl;
	//cout<<"BoundingBox y :"<<dbounding_Y_Min<<"\t~ "<<dbounding_Y_Max<<endl;
	//cout<<"BoundingBox z :"<<dbounding_Z_Min<<"\t~ "<<dbounding_Z_Max<<endl;


	//idim_z=idim_z*4;

	dvxsize_x = (dbounding_X_Max - dbounding_X_Min) / (idim_x - 1);
	dvxsize_y = (dbounding_Y_Max - dbounding_Y_Min) / (idim_y - 1);
	dvxsize_z = (dbounding_Z_Max - dbounding_Z_Min) / (idim_z - 1);
	//cout<<"vsize_x="<<dvxsize_x<<endl;
	//cout<<"vsize_y="<<dvxsize_y<<endl;
	//cout<<"vsize_z="<<dvxsize_z<<endl;

	do{
		fscanf(hRead, "%c", &ch);
		sTarget = sTarget + ch;
	} while (ch != '@');

	fscanf(hRead, "%c", &ch);
	sTarget = sTarget + ch;			//load @1


	Data = new struct_Voxel**[idim_z];
	Data_Ori = new short**[idim_z];

	for (int i = 0; i<idim_z; i++)
	{
		Data[i] = new struct_Voxel*[idim_y];
		Data_Ori[i] = new short*[idim_y];
		for (int j = 0; j<idim_y; j++)
		{
			Data[i][j] = new struct_Voxel[idim_x];
			Data_Ori[i][j] = new short[idim_x];
		}
	}

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	{
		Data[i][j][k].z = i; Data[i][j][k].y = j; Data[i][j][k].x = k;		// i j k  =  z y x
		Data[i][j][k].value = 0;
		Data[i][j][k].bf_value = -1;
		Data[i][j][k].connect_flag = 0;

		Data_Ori[i][j][k] = 0;
	}

	//	cout<<"123AAAA"<<iThreshold;
	short tmp_value = 0;
	long long sum = 0;
	for (int i = 0; i<idim_z; i++)
	{
		for (int j = 0; j<idim_y; j++)
		for (int k = 0; k<idim_x; k++)
		{
			fscanf(hRead, "%hd", &tmp_value);

			if (tmp_value >= iThreshold)
			{
				Data_Ori[i][j][k] = Data[i][j][k].value = tmp_value;
				sum += tmp_value;
				iVoxels++;
				if (tmp_value>iIty_Max) iIty_Max = tmp_value;
				if (tmp_value<iIty_Min) iIty_Min = tmp_value;
			}
		}
		//        std::cout<<setw(4)<<i+1; if((i+1)%20==0) std::cout<<endl;
		// if((i+1)/4%20==0) std::cout<<"."<<std::flush;
	}

	iIty_Avg = (int)(sum / iVoxels);


	fclose(hRead);
	if (output_flag) std::cout << "load (" << iThreshold << ") complate!!!  total " << iVoxels << " voxels" << std::endl;

}
//////////////////////////////////////////////////////////////////////////////
//Save to Am file
//Input Save_Filename
//////////////////////////////////////////////////////////////////////////////
void AM_data::Save_AM(string sFile_name)
{
	if (output_flag) std::cout << "saving AM data..." << std::endl;

	FILE* hWrite;
	hWrite = fopen(sFile_name.c_str(), "w");

	fprintf(hWrite, "# AmiraMesh 3D ASCII 2.0\n\n");
	fprintf(hWrite, "# Created by program: Soma_Detection \n");
	fprintf(hWrite, "# Author: Guan-wei He\n\n");

	fprintf(hWrite, "define Lattice %d %d %d\n\n", idim_x, idim_y, idim_z);
	fprintf(hWrite, "Parameters {\n");
	fprintf(hWrite, "    Content \"%dx%dx%d ushort, uniform coordinates\",\n", idim_x, idim_y, idim_z);

	fprintf(hWrite, "    DataWindow %d %d,\n", iIty_Min, iIty_Max);

	fprintf(hWrite, "    BoundingBox %10.6f %10.6f %10.6f %10.6f %10.6f %10.6f,\n", dbounding_X_Min, dbounding_X_Min + (idim_x - 1) * dvxsize_x,
		dbounding_Y_Min, dbounding_Y_Min + (idim_y - 1) * dvxsize_y,
		dbounding_Z_Min, dbounding_Z_Min + (idim_z - 1) * dvxsize_z);

	fprintf(hWrite, "    CoordType \"uniform\"\n");
	fprintf(hWrite, "}\n\n");

	fprintf(hWrite, "Lattice { ushort Data } @1\n\n");
	fprintf(hWrite, "# Data section follows\n");
	fprintf(hWrite, "@1\n");


	for (int i = 0; i<idim_z; i++)
	{
		for (int j = 0; j<idim_y; j++)
		for (int k = 0; k<idim_x; k++)
		{
			fprintf(hWrite, "%hd\n", Data[i][j][k].value);
		}
		if (output_flag) std::cout << setw(3) << i + 1; if ((i + 1) % 20 == 0)std::cout << endl;
	}

	fprintf(hWrite, "\n\n");

	fclose(hWrite);
	if (output_flag) std::cout << endl;
}
//////////////////////////////////////////////////////////////////////////////
//Find Connected Component
//Input comp_threshold is intensity threshold
//Input comp_voxels is voxels threshold
//////////////////////////////////////////////////////////////////////////////
int AM_data::Find_Component(int comp_threshold, int comp_voxels)
{
	if (bcomp_flag)
	{
		for (int i = 0; i<2; i++) delete[] iComponents[i];
		delete[] iComponents;
	}

	if (output_flag) std::cout << "find connect component!!!";
	struct_Voxel *union_find = new struct_Voxel[iVoxels];
	int union_index = 0;
	int start;
	int xx, yy, zz, flag;							// temp x y z for union
	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	{
		for (int k = 0; k<idim_x; k++)
		{
			start = union_index;
			if (Data[i][j][k].value >= comp_threshold && Data[i][j][k].connect_flag == 0)
			{
				Data[i][j][k].connect_flag = ++connect_index;
				union_find[union_index++] = Data[i][j][k];

			}
			for (int c = start; c<union_index; c++)
			{
				zz = union_find[c].z, yy = union_find[c].y, xx = union_find[c].x; flag = union_find[c].connect_flag;
				if (zz>1 && Data[zz - 1][yy][xx].value > comp_threshold  && Data[zz - 1][yy][xx].connect_flag == 0) { Data[zz - 1][yy][xx].connect_flag = flag; union_find[union_index++] = Data[zz - 1][yy][xx]; }
				if (zz<idim_z - 2 && Data[zz + 1][yy][xx].value > comp_threshold  && Data[zz + 1][yy][xx].connect_flag == 0) { Data[zz + 1][yy][xx].connect_flag = flag; union_find[union_index++] = Data[zz + 1][yy][xx]; }
				if (yy>1 && Data[zz][yy - 1][xx].value > comp_threshold  && Data[zz][yy - 1][xx].connect_flag == 0) { Data[zz][yy - 1][xx].connect_flag = flag; union_find[union_index++] = Data[zz][yy - 1][xx]; }
				if (yy< idim_y - 2 && Data[zz][yy + 1][xx].value > comp_threshold  && Data[zz][yy + 1][xx].connect_flag == 0) { Data[zz][yy + 1][xx].connect_flag = flag; union_find[union_index++] = Data[zz][yy + 1][xx]; }
				if (xx>1 && Data[zz][yy][xx - 1].value > comp_threshold  && Data[zz][yy][xx - 1].connect_flag == 0) { Data[zz][yy][xx - 1].connect_flag = flag; union_find[union_index++] = Data[zz][yy][xx - 1]; }
				if (xx< idim_x - 2 && Data[zz][yy][xx + 1].value > comp_threshold  && Data[zz][yy][xx + 1].connect_flag == 0) { Data[zz][yy][xx + 1].connect_flag = flag; union_find[union_index++] = Data[zz][yy][xx + 1]; }
			}
			//if(union_index)
			//cout<<"UID="<<connect_index<<" "<<union_index<<endl;
			union_index = 0;
		}

	}
	//cout<<"connect_index "<<connect_index<<endl;

	int **iComponents_temp = new int*[2];
	for (int i = 0; i<2; i++) iComponents_temp[i] = new int[connect_index + 1];

	for (int i = 0; i<connect_index + 1; i++) { iComponents_temp[0][i] = i; iComponents_temp[1][i] = 0; }

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
		iComponents_temp[1][Data[i][j][k].connect_flag]++;

	int comps = 0;
	for (int i = 1; i<connect_index + 1; i++)
	if (iComponents_temp[1][i]>comp_voxels) comps++;

	if (output_flag) std::cout << " ,  total comps=" << comps << std::endl;
	iComponents = new int*[2];
	for (int i = 0; i<2; i++) iComponents[i] = new int[comps + 1];

	for (int i = 0; i<comps + 1; i++) { iComponents[0][i] = i; iComponents[1][i] = 0; }

	int jj = 1;
	for (int i = 1; i<connect_index + 1; i++)
	{
		if (iComponents_temp[1][i]>comp_voxels)
		{
			iComponents[0][jj] = iComponents_temp[0][i];
			iComponents[1][jj] = iComponents_temp[1][i];
			jj++;
		}
	}



	//cout<<"Q"<<connect_index;
	int temp = 0, temp1 = 0;					//sort by component
	for (int j = 2; j<comps + 1; j++)
	{
		for (int i = 1; i<comps; i++)
		{
			if (iComponents[1][i]<iComponents[1][i + 1])
			{
				temp = iComponents[1][i];
				iComponents[1][i] = iComponents[1][i + 1];
				iComponents[1][i + 1] = temp;

				temp1 = iComponents[0][i];
				iComponents[0][i] = iComponents[0][i + 1];
				iComponents[0][i + 1] = temp1;
			}
		}
	}


	bcomp_flag = true;


	delete[] union_find;
	for (int i = 0; i<2; i++) delete[] iComponents_temp[i];
	delete[] iComponents_temp;


	return comps;
}
//////////////////////////////////////////////////////////////////////////////
//Find Connected Component
//Input only comp_voxels without intensity threshold
//cut 35% as threshold
//////////////////////////////////////////////////////////////////////////////
int AM_data::Find_Component(int comp_voxels)
{
	if (bcomp_flag)
	{
		for (int i = 0; i<2; i++) delete[] iComponents[i];
		delete[] iComponents;
		connect_index = 0;
	}
	if (output_flag) std::cout << "find connect component!!!   ";

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
		Data[i][j][k].connect_flag = 0;

	int comp_threshold = 0;               //useless parameter
	struct_Voxel *union_find = new struct_Voxel[iVoxels];
	int union_index = 0;
	int start;
	int xx, yy, zz, flag;							// temp x y z for union

	for (int i = 0; i<idim_z; i++)
	{
		for (int j = 0; j<idim_y; j++)
		{

			for (int k = 0; k<idim_x; k++)
			{
				start = union_index;
				if (Data[i][j][k].value >comp_threshold && Data[i][j][k].connect_flag == 0)
				{
					Data[i][j][k].connect_flag = ++connect_index;
					union_find[union_index++] = Data[i][j][k];

				}
				for (int c = start; c<union_index; c++)
				{
					zz = union_find[c].z, yy = union_find[c].y, xx = union_find[c].x; flag = union_find[c].connect_flag;
					if (zz>0 && Data[zz - 1][yy][xx].value > comp_threshold  && Data[zz - 1][yy][xx].connect_flag == 0) { Data[zz - 1][yy][xx].connect_flag = flag; union_find[union_index++] = Data[zz - 1][yy][xx]; }
					if (zz<idim_z - 1 && Data[zz + 1][yy][xx].value > comp_threshold  && Data[zz + 1][yy][xx].connect_flag == 0) { Data[zz + 1][yy][xx].connect_flag = flag; union_find[union_index++] = Data[zz + 1][yy][xx]; }
					if (yy>0 && Data[zz][yy - 1][xx].value > comp_threshold  && Data[zz][yy - 1][xx].connect_flag == 0) { Data[zz][yy - 1][xx].connect_flag = flag; union_find[union_index++] = Data[zz][yy - 1][xx]; }
					if (yy< idim_y - 1 && Data[zz][yy + 1][xx].value > comp_threshold  && Data[zz][yy + 1][xx].connect_flag == 0) { Data[zz][yy + 1][xx].connect_flag = flag; union_find[union_index++] = Data[zz][yy + 1][xx]; }
					if (xx>0 && Data[zz][yy][xx - 1].value > comp_threshold  && Data[zz][yy][xx - 1].connect_flag == 0) { Data[zz][yy][xx - 1].connect_flag = flag; union_find[union_index++] = Data[zz][yy][xx - 1]; }
					if (xx< idim_x - 1 && Data[zz][yy][xx + 1].value > comp_threshold  && Data[zz][yy][xx + 1].connect_flag == 0) { Data[zz][yy][xx + 1].connect_flag = flag; union_find[union_index++] = Data[zz][yy][xx + 1]; }
					if (union_index > iVoxels - 10) break;
				}
				//if(union_index)
				//cout<<"UID="<<connect_index<<" "<<union_index<<endl;
				//if (union_index > iVoxels - 10) 
				//system("pause");
				union_index = 0;
			}

		}
	}
	//cout<<"connect_index "<<connect_index<<endl;
	int **iComponents_temp = new int*[2];
	for (int i = 0; i<2; i++) iComponents_temp[i] = new int[connect_index + 1];

	for (int i = 0; i<connect_index + 1; i++) { iComponents_temp[0][i] = i; iComponents_temp[1][i] = 0; }

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
		iComponents_temp[1][Data[i][j][k].connect_flag]++;

	int comps = 0;
	for (int i = 1; i<connect_index + 1; i++)
	if (iComponents_temp[1][i]>comp_voxels) comps++;

	iComponents = new int*[2];
	for (int i = 0; i<2; i++) iComponents[i] = new int[comps + 1];

	for (int i = 0; i<comps + 1; i++) { iComponents[0][i] = i; iComponents[1][i] = 0; }

	int jj = 1;
	for (int i = 1; i<connect_index + 1; i++)
	{
		if (iComponents_temp[1][i]>comp_voxels)
		{
			iComponents[0][jj] = iComponents_temp[0][i];
			iComponents[1][jj] = iComponents_temp[1][i];
			jj++;
		}
	}
	int temp = 0, temp1 = 0;					//sort by component
	for (int j = 2; j<comps + 1; j++)
	{
		for (int i = 1; i<comps; i++)
		{
			if (iComponents[1][i]<iComponents[1][i + 1])
			{
				temp = iComponents[1][i];
				iComponents[1][i] = iComponents[1][i + 1];
				iComponents[1][i + 1] = temp;

				temp1 = iComponents[0][i];
				iComponents[0][i] = iComponents[0][i + 1];
				iComponents[0][i + 1] = temp1;
			}
		}
	}

	bcomp_flag = true;

	delete[] union_find;

	for (int i = 0; i<2; i++) delete[] iComponents_temp[i];
	delete[] iComponents_temp;

	if (output_flag) cout << " total are " << comps << " components" << std::endl;

	return comps;


}
AM_data& AM_data::operator=(const AM_data& rhs)
{
	if (bdata_flag) AM_data::AM_data_clear();

	idim_x = rhs.idim_x;	 	idim_y = rhs.idim_y;	 	idim_z = rhs.idim_z;

	iVoxels = rhs.iVoxels;
	iThreshold = rhs.iThreshold;
	iIty_Max = rhs.iIty_Max;	 iIty_Min = rhs.iIty_Min;	  iIty_Avg = rhs.iIty_Avg;
	iBF_Max = rhs.iBF_Max;

	dbounding_X_Min = rhs.dbounding_X_Min;	dbounding_X_Max = rhs.dbounding_X_Max;
	dbounding_Y_Min = rhs.dbounding_Y_Min;	dbounding_Y_Max = rhs.dbounding_Y_Max;
	dbounding_Z_Min = rhs.dbounding_Z_Min;	dbounding_Z_Max = rhs.dbounding_Z_Max;

	dvxsize_x = rhs.dvxsize_x;	 dvxsize_y = rhs.dvxsize_y;	 dvxsize_z = rhs.dvxsize_z;

	bdata_flag = rhs.bdata_flag;
	bcomp_flag = rhs.bcomp_flag;

	connect_index = rhs.connect_index;


	if (rhs.bdata_flag)
	{
		Data = new struct_Voxel**[idim_z];
		Data_Ori = new short**[idim_z];

		for (int i = 0; i<idim_z; i++)
		{
			Data[i] = new struct_Voxel*[idim_y];
			Data_Ori[i] = new short*[idim_y];

			for (int j = 0; j<idim_y; j++)  {
				Data[i][j] = new struct_Voxel[idim_x];
				Data_Ori[i][j] = new short[idim_x];
			}
		}

		for (int i = 0; i<idim_z; i++)
		for (int j = 0; j<idim_y; j++)
		for (int k = 0; k<idim_x; k++)
		{
			Data[i][j][k].z = i; Data[i][j][k].y = j; Data[i][j][k].x = k;		// i j k  =  z y x
			Data[i][j][k].value = rhs.Data[i][j][k].value;
			Data[i][j][k].bf_value = rhs.Data[i][j][k].bf_value;
			Data[i][j][k].connect_flag = rhs.Data[i][j][k].connect_flag;

			Data_Ori[i][j][k] = rhs.Data_Ori[i][j][k];
		}
	}

	if (rhs.bcomp_flag)
	{
		iComponents = new int*[2];
		for (int i = 0; i<2; i++) iComponents[i] = new int[connect_index + 1];
		for (int i = 0; i<connect_index + 1; i++) {
			iComponents[0][i] = rhs.iComponents[0][i];
			iComponents[1][i] = rhs.iComponents[1][i];
		}
	}


	return *this;
}
AM_data& AM_data::Get_Component(const AM_data& rhs, int com_number, int& boundary_count)
{
	if (bdata_flag) AM_data::AM_data_clear();
	int min_z = rhs.idim_z, max_z = 0;
	int min_y = rhs.idim_y, max_y = 0;
	int min_x = rhs.idim_x, max_x = 0;

	for (int i = 0; i<rhs.idim_z; i++)
	for (int j = 0; j<rhs.idim_y; j++)
	for (int k = 0; k<rhs.idim_x; k++)
	if (rhs.Data[i][j][k].connect_flag == rhs.iComponents[0][com_number])
	{
		if (i<min_z) min_z = i;	if (i>max_z) max_z = i;
		if (j<min_y) min_y = j;	if (j>max_y) max_y = j;
		if (k<min_x) min_x = k;	if (k>max_x) max_x = k;
	}

	idim_x = max_x - min_x + 1;
	idim_y = max_y - min_y + 1;
	idim_z = max_z - min_z + 1;

	dvxsize_x = rhs.dvxsize_x; dvxsize_y = rhs.dvxsize_y; dvxsize_z = rhs.dvxsize_z;


	Data = new struct_Voxel**[idim_z];
	Data_Ori = new short**[idim_z];
	for (int i = 0; i<idim_z; i++)
	{
		Data[i] = new struct_Voxel*[idim_y];
		Data_Ori[i] = new short*[idim_y];
		for (int j = 0; j<idim_y; j++)  {
			Data[i][j] = new struct_Voxel[idim_x];
			Data_Ori[i][j] = new short[idim_x];
		}
	}
	int tmp_value = 0;
	long long sum = 0;
	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	{
		Data[i][j][k].z = i; Data[i][j][k].y = j; Data[i][j][k].x = k;		// i j k  =  z y x
		Data[i][j][k].bf_value = -1;
		Data[i][j][k].connect_flag = 0;
		Data[i][j][k].value = 0;
		if (rhs.Data[i + min_z][j + min_y][k + min_x].connect_flag == rhs.iComponents[0][com_number])
		{
			tmp_value = rhs.Data[i + min_z][j + min_y][k + min_x].value;
			Data[i][j][k].value = tmp_value;

			if (tmp_value)
			{
				sum += tmp_value;
				if (tmp_value>iIty_Max) iIty_Max = tmp_value;
				if (tmp_value<iIty_Min) iIty_Min = tmp_value;
				iVoxels++;
			}

		}

	}
	int bc_1 = 0, bc_2 = 0, bc_3 = 0, bc_4 = 0, bc_5 = 0, bc_6 = 0;

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	{
		if (i + min_z < 10)
		{
			if (Data[i][j][k].value> rhs.iIty_Max*0.9) bc_1++;
		}
		else if (j + min_y < 10)
		{
			if (Data[i][j][k].value> rhs.iIty_Max*0.9)bc_2++;
		}
		else if (k + min_x < 10)
		{
			if (Data[i][j][k].value> rhs.iIty_Max*0.9)bc_3++;
		}
		else if (i + min_z > rhs.idim_z - 10)
		{
			if (Data[i][j][k].value> rhs.iIty_Max*0.9)bc_4++;
		}
		else if (j + min_y > rhs.idim_y - 10)
		{
			if (Data[i][j][k].value> rhs.iIty_Max*0.9)bc_5++;
		}
		else if (k + min_x > rhs.idim_x - 10)
		{
			if (Data[i][j][k].value> rhs.iIty_Max*0.9)bc_6++;
		}
	}
	// boundary_count = abs(bc_1-bc_4)+ abs(bc_2-bc_5)+ abs(bc_3-bc_6);
	boundary_count = bc_1 + bc_2 + bc_3 + bc_4 + bc_5 + bc_6;
	//if(bc_2 > boundary_count) boundary_count = bc_2;
	//if(bc_3 > boundary_count) boundary_count = bc_3;
	//if(bc_4 > boundary_count) boundary_count = bc_4;
	//if(bc_5 > boundary_count) boundary_count = bc_5;
	//if(bc_6 > boundary_count) boundary_count = bc_6;

	iIty_Avg = (int)(sum / iVoxels);

	dbounding_X_Min = rhs.dbounding_X_Min + rhs.dvxsize_x*min_x;
	dbounding_X_Max = dbounding_X_Min + dvxsize_x*(idim_x - 1);
	dbounding_Y_Min = rhs.dbounding_Y_Min + rhs.dvxsize_y*min_y;
	dbounding_Y_Max = dbounding_Y_Min + dvxsize_y*(idim_y - 1);
	dbounding_Z_Min = rhs.dbounding_Z_Min + rhs.dvxsize_z*min_z;
	dbounding_Z_Max = dbounding_Z_Min + dvxsize_z*(idim_z - 1);
	bdata_flag = true;

	if (output_flag) cout << " Get " << com_number << " Component Complete!!!  total " << iVoxels << " voxels" << std::endl;
	return *this;
}
AM_data& AM_data::Set_BFvalue()
{

	//cout<<endl<<"total"<<iVoxels<<endl;
	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
		Data[i][j][k].bf_value = -1;

	int count = 0;
	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	{
		if (Data[i][j][k].value == 0) Data[i][j][k].bf_value = 0;
	}
	int bf = 0;

	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	{
		if (Data[0][j][k].value>0)  { Data[0][j][k].bf_value = 1; count++; }
		if (Data[idim_z - 1][j][k].value>0)  { Data[idim_z - 1][j][k].bf_value = 1; count++; }
	}

	for (int i = 0; i<idim_z; i++)
	for (int k = 0; k<idim_x; k++)
	{
		if (Data[i][0][k].value>0)  { Data[i][0][k].bf_value = 1; count++; }
		if (Data[i][idim_y - 1][k].value>0)  { Data[i][idim_y - 1][k].bf_value = 1; count++; }
	}

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	{
		if (Data[i][j][0].value>0)  { Data[i][j][0].bf_value = 1; count++; }
		if (Data[i][j][idim_x - 1].value>0)  { Data[i][j][idim_x - 1].bf_value = 1; count++; }
	}

	while (count<iVoxels)
	{
		bf++;
		for (int i = 0; i<idim_z; i++)
		for (int j = 0; j<idim_y; j++)
		for (int k = 0; k<idim_x; k++)
		{
			if (i>0 && Data[i - 1][j][k].bf_value == bf - 1 && Data[i][j][k].bf_value == -1 && Data[i][j][k].value>0) { Data[i][j][k].bf_value = bf; count++; }
			if (i<idim_z - 1 && Data[i + 1][j][k].bf_value == bf - 1 && Data[i][j][k].bf_value == -1 && Data[i][j][k].value>0) { Data[i][j][k].bf_value = bf; count++; }
			if (j>0 && Data[i][j - 1][k].bf_value == bf - 1 && Data[i][j][k].bf_value == -1 && Data[i][j][k].value>0) { Data[i][j][k].bf_value = bf; count++; }
			if (j<idim_y - 1 && Data[i][j + 1][k].bf_value == bf - 1 && Data[i][j][k].bf_value == -1 && Data[i][j][k].value>0) { Data[i][j][k].bf_value = bf; count++; }
			if (k>0 && Data[i][j][k - 1].bf_value == bf - 1 && Data[i][j][k].bf_value == -1 && Data[i][j][k].value>0) { Data[i][j][k].bf_value = bf; count++; }
			if (k<idim_x - 1 && Data[i][j][k + 1].bf_value == bf - 1 && Data[i][j][k].bf_value == -1 && Data[i][j][k].value>0) { Data[i][j][k].bf_value = bf; count++; }
		}
		//cout<<"count="<<count<<endl;
	}
	iBF_Max = bf;

	return *this;
	//cout<<"QQQ"<<bf;
}

AM_data& AM_data::Re_SetValue(int threshold)
{
	// iThreshold=threshold;

	if (iThreshold<iIty_Min)
		iIty_Min = iThreshold;

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	if (Data[i][j][k].value != 0 && Data[i][j][k].value<threshold)
		Data[i][j][k].value = 0;
	//std::cout<<setw(3)<<i+1; if((i+1)%20==0) std::cout<<std::endl;

	long long sum = 0;
	iVoxels = 0;
	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	{
		if (Data[i][j][k].value >= threshold)
		{
			sum += Data[i][j][k].value;
			iVoxels++;
		}
	}

	if (iVoxels == 0)
		iIty_Avg = 0;
	else
		iIty_Avg = (int)(sum / iVoxels);

	if (output_flag) std::cout << "reset value (" << threshold << ") complete!!! total " << iVoxels << " voxels" << std::endl;

	return *this;
}

AM_data& AM_data::Match_Filter()
{

	short*** fData = new short**[idim_z];
	for (int i = 0; i<idim_z; i++)
	{
		fData[i] = new short*[idim_y];
		for (int j = 0; j<idim_y; j++)  fData[i][j] = new short[idim_x];
	}

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
		fData[i][j][k] = Data[i][j][k].value;

	int i_max = iIty_Max;
	iIty_Max = INT_MIN;	 iIty_Min = INT_MAX;
	iVoxels = 0;

	for (int i = 0; i<idim_z; i++)
	{
		///if(i% (am_file.dimz/10)==0) std::cout<<". ";
		for (int j = 3; j<idim_y - 3; j++)
		{
			double tmp = 0;
			for (int k = 3; k<idim_x - 3; k++)
			{
				tmp = Data[i][j - 3][k - 3].value + Data[i][j - 3][k + 3].value + Data[i][j + 3][k - 3].value + Data[i][j + 3][k + 3].value;
				tmp += (Data[i][j - 3][k - 2].value + Data[i][j - 3][k + 2].value + Data[i][j - 2][k - 3].value + Data[i][j - 2][k + 3].value + Data[i][j + 2][k - 3].value + Data[i][j + 2][k + 3].value + Data[i][j + 3][k - 2].value + Data[i][j + 3][k + 2].value) * 1.5;
				tmp += (Data[i][j - 2][k - 2].value + Data[i][j - 2][k + 2].value + Data[i][j + 2][k - 2].value + Data[i][j + 2][k + 2].value) * 2;
				tmp += (Data[i][j - 3][k].value + Data[i][j - 2][k - 1].value + Data[i][j - 2][k + 1].value + Data[i][j - 1][k - 2].value + Data[i][j - 1][k + 2].value + Data[i][j][k - 3].value + Data[i][j][k + 3].value + Data[i][j + 1][k - 2].value + Data[i][j + 1][k + 2].value + Data[i][j + 2][k - 1].value + Data[i][j + 2][k + 1].value + Data[i][j + 3][k].value)*0.5;
				tmp -= (Data[i][j - 1][k - 1].value + Data[i][j - 1][k + 1].value + Data[i][j + 1][k - 1].value + Data[i][j + 1][k + 1].value);
				tmp -= Data[i][j][k].value;
				fData[i][j][k] = (short)(tmp / 18);
				if (fData[i][j][k] < Data[i][j][k].value) fData[i][j][k] = Data[i][j][k].value;
				else if (fData[i][j][k]>i_max) fData[i][j][k] = i_max;
				if (fData[i][j][k]>iIty_Max) iIty_Max = fData[i][j][k];
				if (fData[i][j][k] != 0 && fData[i][j][k]<iIty_Min) iIty_Min = fData[i][j][k];
			}
		}
	}

	double sum = 0;
	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	{
		Data[i][j][k].value = fData[i][j][k];
		if (Data[i][j][k].value != 0) { sum += Data[i][j][k].value; iVoxels++; }
	}

	iIty_Avg = (int)(sum / iVoxels);

	for (int i = 0; i<idim_z; i++)
	{
		for (int j = 0; j<idim_y; j++)  delete[] fData[i][j];
		delete[] fData[i];
	}
	delete[] fData;

	return *this;
}

AM_data& AM_data::Median_Filter(int window)
{
	//int median[125]={0};
	int window_size = window*window*window;
	int* median = new int[window_size];

	int r = window / 2;

	short*** fData = new short**[idim_z];
	for (int i = 0; i<idim_z; i++)
	{
		fData[i] = new short*[idim_y];
		for (int j = 0; j<idim_y; j++)  fData[i][j] = new short[idim_x];
	}

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
		fData[i][j][k] = Data[i][j][k].value;

	int i_max = iIty_Max;
	iIty_Max = INT_MIN;	 iIty_Min = INT_MAX;
	iVoxels = 0;



	for (int i = r; i<idim_z - r; i++)
	for (int j = r; j<idim_y - r; j++)
	for (int k = r; k<idim_x - r; k++)
	{
		int index = 0;
		for (int ii = -r; ii <= r; ii++)
		for (int jj = -r; jj <= r; jj++)
		for (int kk = -r; kk <= r; kk++)
			median[index++] = Data[i + ii][j + jj][k + kk].value;

		std::sort(median, median + window_size);
		fData[i][j][k] = median[window_size / 2];
		if (fData[i][j][k]>i_max) fData[i][j][k] = i_max;
		if (fData[i][j][k]>iIty_Max) iIty_Max = fData[i][j][k];
		if (fData[i][j][k] != 0 && fData[i][j][k]<iIty_Min) iIty_Min = fData[i][j][k];
	}

	double sum = 0;
	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	{
		Data[i][j][k].value = fData[i][j][k];
		//Data_Ori[i][j][k]=fData[i][j][k];
		if (Data[i][j][k].value != 0) { sum += Data[i][j][k].value; iVoxels++; }
	}

	iIty_Avg = (int)(sum / iVoxels);

	for (int i = 0; i<idim_z; i++)
	{
		for (int j = 0; j<idim_y; j++)  delete[] fData[i][j];
		delete[] fData[i];
	}
	delete[] fData;
	delete[] median;
	if (output_flag) std::cout << "median filter (" << window << ") done!!! total " << iVoxels << " voxels" << std::endl;
	return *this;
}

AM_data& AM_data::Mean_Filter()
{
	//int median[125]={0};

	short*** fData = new short**[idim_z];
	for (int i = 0; i<idim_z; i++)
	{
		fData[i] = new short*[idim_y];
		for (int j = 0; j<idim_y; j++)  fData[i][j] = new short[idim_x];
	}

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
		fData[i][j][k] = Data[i][j][k].value;

	int i_max = iIty_Max;
	iIty_Max = INT_MIN;	 iIty_Min = INT_MAX;
	iVoxels = 0;



	for (int i = 2; i<idim_z - 2; i++)
	for (int j = 2; j<idim_y - 2; j++)
	for (int k = 2; k<idim_x - 2; k++)
	{
		int index = 0;
		//int d_index=0;
		for (int ii = -2; ii <= 2; ii++)
		for (int jj = -2; jj <= 2; jj++)
		for (int kk = -2; kk <= 2; kk++)
		{
			index += Data[i + ii][j + jj][k + kk].value;

		}
		fData[i][j][k] = index / 125;
		if (fData[i][j][k] < Data[i][j][k].value) fData[i][j][k] = Data[i][j][k].value;
		if (fData[i][j][k]>i_max) fData[i][j][k] = i_max;
		if (fData[i][j][k]>iIty_Max) iIty_Max = fData[i][j][k];
		if (fData[i][j][k] != 0 && fData[i][j][k]<iIty_Min) iIty_Min = fData[i][j][k];

	}

	double sum = 0;
	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	{
		Data[i][j][k].value = fData[i][j][k];
		if (Data[i][j][k].value != 0) { sum += Data[i][j][k].value; iVoxels++; }
	}

	iIty_Avg = (int)(sum / iVoxels);

	for (int i = 0; i<idim_z; i++)
	{
		for (int j = 0; j<idim_y; j++)  delete[] fData[i][j];
		delete[] fData[i];
	}
	delete[] fData;

	return *this;
}
/*
void AM_data::Output_Landmark(string Filename, struct_Voxel* iSoma_update, int n)
{
std::ofstream fout1(Filename);
fout1<<"# AmiraMesh 3D ASCII 2.0"<<endl<<endl;
fout1<<"# Soma Point Landmark"<<endl<<endl;
fout1<<"define Markers "<<n<<endl;
fout1<<"Parameters {"<<endl;
fout1<<"    ContentType \"LandmarkSet\","<<endl;
fout1<<"    NumSets 1"<<endl;
fout1<<"}"<<endl;
fout1<<"Markers {float[3] Coordinates} @1"<<endl<<endl;
fout1<<"# Data section follows"<<endl;
fout1<<"@1"<<endl;

for(int i=0;i<n;i++)
{

fout1<<iSoma_update[i].x * dvxsize_x + dbounding_X_Min<<" ";
fout1<<iSoma_update[i].y * dvxsize_y + dbounding_Y_Min<<" ";
fout1<<iSoma_update[i].z * dvxsize_z + dbounding_Z_Min<<" ";
fout1<<endl;
//	cout<<"value="<<iSoma_update[i].bf_value<<endl;
}
}

void AM_data::Output_Picture_X(string Filename, struct_Voxel* iSoma_update, int n)
{

int** x_plane= new int*[idim_z];
for(int i=0; i<idim_z; i++ )		x_plane[i] = new int [idim_y];

for(int i=0; i<idim_z; i++ )
for(int j=0; j<idim_y; j++ )
x_plane[i][j]=0;

for(int i=0; i<idim_z; i++ )
for(int j=0; j<idim_y; j++ )
for(int k=0; k<idim_x; k++ )
if( x_plane[i][j] < Data[i][j][k].value ) x_plane[i][j]=Data[i][j][k].value;

IplImage *Image1,*Image2;
CvSize ImageSize1 = cvSize(idim_y,idim_z);				//Width Height
Image1 = cvCreateImage(ImageSize1,IPL_DEPTH_8U,3);

double pp=0;
char pixel=0;
for(int i=0;i<Image1->height;i++)
{
for(int j=0;j<Image1->widthStep;j=j+3)
{
pp=x_plane[i][j/3];
pixel=pp/iIty_Max*255;
Image1->imageData[i*Image1->widthStep+j]=pixel;
Image1->imageData[i*Image1->widthStep+j+1]=pixel;
Image1->imageData[i*Image1->widthStep+j+2]=pixel;
}
}

CvPoint TextPosition;
CvFont Font1=cvFont(0.7,1);
for(int i=0;i<n;i++)
{
sprintf(buffer,"%d,%d",iSoma_update[i].connect_flag,iSoma_update[i].bf_value);
cvCircle(Image1, cvPoint(iSoma_update[i].y,iSoma_update[i].z), 2, CV_RGB(0,255,255),-1, 3, 0);
if(iSoma_update[i].y>950)
cvPutText(Image1,buffer,cvPoint(iSoma_update[i].y-50,iSoma_update[i].z+5),&Font1,CV_RGB(255,0,0));
else
cvPutText(Image1,buffer,cvPoint(iSoma_update[i].y+5,iSoma_update[i].z+5),&Font1,CV_RGB(255,0,0));
}

CvSize ImageSize2 = cvSize(1024,512);
Image2 = cvCreateImage(ImageSize2,IPL_DEPTH_8U,3);
cvResize(Image1, Image2, CV_INTER_LINEAR);

sprintf(buffer,"%s_YZ.jpg",Filename.c_str());
cvSaveImage(buffer, Image2 );

for(int i=0; i<idim_z; i++ )  delete [] x_plane[i];
delete [] x_plane;


cvReleaseImage(&Image1);
cvReleaseImage(&Image2);
}

void AM_data::Output_Picture_Y(string Filename, struct_Voxel* iSoma_update, int n)
{

int** y_plane= new int*[idim_z];
for(int i=0; i<idim_z; i++ )		y_plane[i] = new int [idim_x];

for(int i=0; i<idim_z; i++ )
for(int j=0; j<idim_x; j++ )
y_plane[i][j]=0;

for(int i=0; i<idim_z; i++ )
for(int j=0; j<idim_y; j++ )
for(int k=0; k<idim_x; k++ )
if( y_plane[i][k] < Data[i][j][k].value ) y_plane[i][k]=Data[i][j][k].value;

IplImage *Image1,*Image2;
CvSize ImageSize1 = cvSize(idim_x,idim_z);				//Width Height
Image1 = cvCreateImage(ImageSize1,IPL_DEPTH_8U,3);

double pp=0;
char pixel=0;
for(int i=0;i<Image1->height;i++)
{
for(int j=0;j<Image1->widthStep;j=j+3)
{
pp=y_plane[i][j/3];
pixel=pp/iIty_Max*255;
Image1->imageData[i*Image1->widthStep+j]=pixel;
Image1->imageData[i*Image1->widthStep+j+1]=pixel;
Image1->imageData[i*Image1->widthStep+j+2]=pixel;
}
}

CvPoint TextPosition;
CvFont Font1=cvFont(0.7,1);
for(int i=0;i<n;i++)
{
sprintf(buffer,"%d,%d",iSoma_update[i].connect_flag,iSoma_update[i].bf_value);
cvCircle(Image1, cvPoint(iSoma_update[i].x,iSoma_update[i].z), 2, CV_RGB(0,255,255),-1, 3, 0);
if(iSoma_update[i].x>950)
cvPutText(Image1,buffer,cvPoint(iSoma_update[i].x-50,iSoma_update[i].z+5),&Font1,CV_RGB(255,0,0));
else
cvPutText(Image1,buffer,cvPoint(iSoma_update[i].x+5,iSoma_update[i].z+5),&Font1,CV_RGB(255,0,0));
}

CvSize ImageSize2 = cvSize(1024,512);
Image2 = cvCreateImage(ImageSize2,IPL_DEPTH_8U,3);
cvResize(Image1, Image2, CV_INTER_LINEAR);

sprintf(buffer,"%s_XZ.jpg",Filename.c_str());
cvSaveImage(buffer, Image2 );

for(int i=0; i<idim_z; i++ )  delete [] y_plane[i];
delete [] y_plane;


cvReleaseImage(&Image1);
cvReleaseImage(&Image2);
}

void AM_data::Output_Picture_Z(string Filename, struct_Voxel* iSoma_update, int n)
{

int** z_plane= new int*[idim_y];
for(int i=0; i<idim_y; i++ )		z_plane[i] = new int [idim_x];

for(int i=0; i<idim_y; i++ )
for(int j=0; j<idim_x; j++ )
z_plane[i][j]=0;

for(int i=0; i<idim_z; i++ )
for(int j=0; j<idim_y; j++ )
for(int k=0; k<idim_x; k++ )
if( z_plane[j][k] < Data[i][j][k].value ) z_plane[j][k]= Data[i][j][k].value;

IplImage *Image1; *Image2;
CvSize ImageSize1 = cvSize(idim_x,idim_y);				//Width Height
Image1 = cvCreateImage(ImageSize1,IPL_DEPTH_8U,3);

double pp=0;
char pixel=0;
for(int i=0;i<Image1->height;i++)
{
for(int j=0;j<Image1->widthStep;j=j+3)
{
pp=z_plane[i][j/3];
pixel=pp/iIty_Max*255;
Image1->imageData[i*Image1->widthStep+j]=pixel;
Image1->imageData[i*Image1->widthStep+j+1]=pixel;
Image1->imageData[i*Image1->widthStep+j+2]=pixel;
}
}

//CvPoint TextPosition;
CvFont Font1=cvFont(2,2);
for(int i=0;i<n;i++)
{
sprintf(buffer,"%d,%d",iSoma_update[i].connect_flag,iSoma_update[i].bf_value);
cvCircle(Image1, cvPoint(iSoma_update[i].x,iSoma_update[i].y), 5, CV_RGB(0,255,255),-1, 3, 0);
if(iSoma_update[i].x>950)
cvPutText(Image1,buffer,cvPoint(iSoma_update[i].x-50,iSoma_update[i].y+5),&Font1,CV_RGB(255,0,0));
else
cvPutText(Image1,buffer,cvPoint(iSoma_update[i].x+5,iSoma_update[i].y+5),&Font1,CV_RGB(255,0,0));
}

//CvSize ImageSize2 = cvSize(1024,1024);
//Image2 = cvCreateImage(ImageSize2,IPL_DEPTH_8U,3);
//cvResize(Image1, Image2, CV_INTER_LINEAR);

sprintf(buffer,"%s_XY.jpg",Filename.c_str());
cvSaveImage(buffer, Image1 );

for(int i=0; i<idim_z; i++ )  delete [] z_plane[i];
delete [] z_plane;


cvReleaseImage(&Image1);
//cvReleaseImage(&Image2);
}

*/

AM_data& AM_data::Replace_Ori(){

	long long sum = 0;
	iVoxels = 0;
	iIty_Max = -1;
	iIty_Min = 10000;
	short tmp_value;
	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
	{
		tmp_value = Data_Ori[i][j][k];

		Data[i][j][k].value = tmp_value;
		Data[i][j][k].bf_value = 0;
		Data[i][j][k].connect_flag = 0;
		if (tmp_value>iIty_Max) iIty_Max = tmp_value;
		if (tmp_value<iIty_Min) iIty_Min = tmp_value;
		if (tmp_value >= iThreshold) iVoxels++;
		else Data[i][j][k].value = 0;
		sum += tmp_value;


	}
	iIty_Avg = (int)(sum / iVoxels);
	// iThreshold=10;
	// iVoxels=rhs.iVoxels;            //resotre previous
	if (output_flag) std::cout << endl << " REPLACE (" << iThreshold << ") complate!!!  total " << iVoxels << " voxels" << std::endl;

	return *this;
}

AM_data& AM_data::downsample(int xrate, int yrate, int zrate)
{
	int idim_xnew = idim_x / xrate, idim_ynew = idim_y / yrate, idim_znew = idim_z / zrate;

	for (int i = 0; i<idim_z; i++)
	{
		for (int j = 0; j<idim_y; j++)
		{
			delete[] Data[i][j];
		}
		delete[] Data[i];
	}
	delete[] Data;


	Data = new struct_Voxel**[idim_znew];

	for (int i = 0; i<idim_znew; i++)
	{
		Data[i] = new struct_Voxel*[idim_ynew];
		for (int j = 0; j<idim_ynew; j++)
		{
			Data[i][j] = new struct_Voxel[idim_xnew];
		}
	}

	for (int i = 0; i<idim_znew; i++)
	for (int j = 0; j<idim_ynew; j++)
	for (int k = 0; k<idim_xnew; k++)
	{
		Data[i][j][k].z = i; Data[i][j][k].y = j; Data[i][j][k].x = k;		// i j k  =  z y x
		Data[i][j][k].value = 0;
		Data[i][j][k].bf_value = -1;
		Data[i][j][k].connect_flag = 0;

	}

	//cout << "wilson_1" << endl;
	iVoxels = 0;
	for (int i = 0; i<idim_z; i += zrate)
	for (int j = 0; j<idim_y; j += yrate)
	for (int k = 0; k<idim_x; k += xrate)
	{
		long long sum = 0;
		int voxels = 0;
		for (int ii = 0; ii < zrate; ii++)
		{
			if (i + ii >= idim_z) continue;
			for (int jj = 0; jj < yrate; jj++)
			{
				if (j + jj >= idim_y) continue;
				for (int kk = 0; kk < xrate; kk++)
				{
					if (k + kk >= idim_x) continue;
					sum += Data_Ori[i + ii][j + jj][k + kk];
					voxels++;
				}

			}
		}

		if (i / zrate >= idim_znew) continue;
		if (j / yrate >= idim_ynew) continue;
		if (k / xrate >= idim_xnew) continue;
		Data[i / zrate][j / yrate][k / xrate].value = (unsigned short)(sum / voxels);
		if (voxels>0) iVoxels++;

	}
	//	cout << "wilson_1" << endl;
	for (int i = 0; i<idim_z; i++)
	{
		for (int j = 0; j<idim_y; j++)
		{
			delete[] Data_Ori[i][j];
		}
		delete[] Data_Ori[i];
	}
	delete[] Data_Ori;

	idim_x = idim_xnew;
	idim_y = idim_ynew;
	idim_z = idim_znew;

	dvxsize_x = (dbounding_X_Max - dbounding_X_Min) / (idim_x - 1);
	dvxsize_y = (dbounding_Y_Max - dbounding_Y_Min) / (idim_y - 1);
	dvxsize_z = (dbounding_Z_Max - dbounding_Z_Min) / (idim_z - 1);

	iIty_Max = INT_MIN;	 iIty_Min = INT_MAX;

	long long sum = 0;
	unsigned short tmp_value;
	for (int i = 0; i<idim_z; i++)
	{
		for (int j = 0; j<idim_y; j++)
		for (int k = 0; k<idim_x; k++)
		{
			tmp_value = Data[i][j][k].value;
			if (tmp_value>0)
			{
				sum += tmp_value;
				if (tmp_value>iIty_Max) iIty_Max = tmp_value;
				if (tmp_value<iIty_Min) iIty_Min = tmp_value;

			}
		}
	}


	Data_Ori = new short**[idim_z];

	for (int i = 0; i<idim_z; i++)
	{
		Data_Ori[i] = new short*[idim_y];
		for (int j = 0; j<idim_y; j++)
		{
			Data_Ori[i][j] = new short[idim_x];
		}
	}

	for (int i = 0; i<idim_z; i++)
	for (int j = 0; j<idim_y; j++)
	for (int k = 0; k<idim_x; k++)
		Data_Ori[i][j][k] = Data[i][j][k].value;




	iIty_Avg = (int)(sum / iVoxels);



	if (output_flag) std::cout << "downsample (" << xrate << " " << yrate << " " << zrate << ") complate!!!  total " << iVoxels << " voxels";

	if (output_flag) std::cout << "  new dimension :" << idim_x << "  " << idim_y << "  " << idim_z << std::endl;

	return *this;
}
