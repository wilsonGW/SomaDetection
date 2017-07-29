// Created by program: Soma_Detection 
// Author: Guan-wei He

#ifndef AM_FORMAT_H
#define AM_FORMAT_H

#include<iostream>
//#include <QString>

struct struct_Voxel
{
    short x,y,z;
    short value;
    short bf_value;
    int connect_flag;
};

struct struct_Attr
{
    double attr1,attr2,attr3,attr4;
};

class AM_data
{
public:
    int idim_x;			//x y z dimension
    int idim_y;
    int idim_z;

    int iVoxels;				//total voxels
    int iThreshold;				//load threshold
    int iIty_Max;				//intensity max
    int iIty_Min;				//intensity min
    int iIty_Avg;
    int iBF_Max;				//bf value max

    double dbounding_X_Min;		//bounding box	xmin xmax
    double dbounding_X_Max;		//				ymin ymax
    double dbounding_Y_Min;		//				zmin zmax
    double dbounding_Y_Max;
    double dbounding_Z_Min;
    double dbounding_Z_Max;

    double dvxsize_x;
    double dvxsize_y;
    double dvxsize_z;



    struct_Voxel*** Data;
    short*** Data_Ori;

    int **iComponents;				//connect component [i][numbers]

    /////////////////////////////////////////////////////member function
    AM_data();
    AM_data(const AM_data&);
    void AM_data_clear();
    void Load_AM(std::string,int=1);		//file name and threshold
    void Save_AM(std::string);			//output to am file
    AM_data& operator=(const AM_data&);

    AM_data& Get_Component(const AM_data&,int,int&);


    ////////////////////////////////////////////////////// Isolation find connect component
    int connect_index;
    int Find_Component(int,int);	//cut first parameter threshold and second voxel size
    int Find_Component(int);		//cut voxel size
    AM_data& Set_BFvalue();
    AM_data& Re_SetValue(int);			//reset threshold
    AM_data& Match_Filter();
    AM_data& Median_Filter(int);
    AM_data& Mean_Filter();


	AM_data& downsample(int,int,int);			//downsample 1/x 1/y 1/z
    AM_data& Replace_Ori();



private:
    void Read_Header(std::string);
    bool bdata_flag;


    bool bcomp_flag;				//flag for component
};

#endif // AM_FORMAT_H
