//
//  NAC_DTM_ExtractBoundaryPoints.c
//  NAC_DTM_ExtractBoundary
//
//  Created by MingleZhao on 2019/1/4.
//  Copyright © 2019 Peking University. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <sys/types.h>
#include <dirent.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NAC_DTM_ExtractBoundaryPoints(const char *DataPath, const char *OutputPath, int Res_X_m);  // Res_X_m: Resolution in METERS; //
char *int2str(char *str, int n);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, const char *argv[])
{
    NAC_DTM_ExtractBoundaryPoints("/Users/minglezhao/Documents/Data/LRO/LROC/NAC/NAC_DTM/NAC_DTM_TOTAL/LROC_NAC_DTM", "/Users/minglezhao/Desktop/NAC_DTM_TEST", 2);
    
    return((void) printf("\nDone. \n"), 0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int NAC_DTM_ExtractBoundaryPoints(const char *DataPath, const char *OutputPath, int Res_X_m)
{
    //__Directory_Path_End_Check__//
    unsigned short DataPathLen = strlen(DataPath);
    unsigned short OutputPathLen = strlen(OutputPath);
    
    char *NewDataPath = (char *) calloc(256, sizeof(char));
    char *NewDataPath0 = (char *) calloc(256, sizeof(char));
    char *NewOutputPath = (char *) calloc(256, sizeof(char));
    char *NewOutputPath0 = (char *) calloc(256, sizeof(char));
    if(NewDataPath == NULL || NewDataPath0 == NULL || NewOutputPath == NULL || NewOutputPath0 == NULL)
    {
        printf("\nError in allocating memory. \n");
        return -1;
    }
    
    strcpy(NewDataPath, DataPath);
    if(*((char *) (NewDataPath + DataPathLen - 1)) != '/')
    {
        *((char *) (NewDataPath + DataPathLen)) = '/';
        *((char *) (NewDataPath + DataPathLen + 1)) = '\0';
    }
    strcpy(NewOutputPath, OutputPath);
    if(*((char *) (NewOutputPath + OutputPathLen - 1)) != '/')
    {
        *((char *) (NewOutputPath + OutputPathLen)) = '/';
        *((char *) (NewOutputPath + OutputPathLen + 1)) = '\0';
    }
    strcpy(NewDataPath0, NewDataPath);
    strcpy(NewOutputPath0, NewOutputPath);
    //
    
    //__Definations_and_Initializations_of_Pointers__//
    //__Directory_and_Data_Name_List__//
    struct dirent *DataDirInfo = NULL;
    DIR *DataDP = opendir(NewDataPath);
    if(DataDP == NULL)
    {
        printf("\nError in opening directory. \n");
        return -1;
    }
    char *DataName = (char *) calloc(256, sizeof(char));
    char **DataNameList = (char **) calloc(1, sizeof(char *));
    if((DataName == NULL) || (DataNameList == NULL))
    {
        printf("\nError in allocating memory. \n");
        return -1;
    }
    //
    
    //__Data_and_Output_Files__//
    char str[10] = {0};
    FILE *fd_Data = NULL;
    FILE *fd_xm = fopen(strcat(strcat(strcat(NewOutputPath0, "NAC_DTM_BoundaryPoints_Res_"), int2str(str, Res_X_m)), "m.txt"), "a");
    memset(NewOutputPath0, 0, 256);
    strcpy(NewOutputPath0, NewOutputPath);
    FILE *fd_DataNameList = fopen(strcat(strcat(strcat(NewOutputPath0, "NAC_DTM_DataNameList_Res_"), str), "m.txt"), "a");
    memset(NewOutputPath0, 0, 256);
    strcpy(NewOutputPath0, NewOutputPath);
    //
    //
    
    //__Counting_the_Number_of_Data__//
    //__Writing_the_Data_Name_List__//
    unsigned int DataNum = 0;
    while((DataDirInfo = readdir(DataDP)))
    {
        if(((strstr(DataDirInfo -> d_name, ".img")) || (strstr(DataDirInfo -> d_name, ".IMG"))) && (strstr(DataDirInfo -> d_name, "NAC_DTM")))
        {
            DataNum += 1;
            strcpy(DataName, DataDirInfo -> d_name);
            
            if(!(DataNameList = (char **) realloc(DataNameList, DataNum * sizeof(char *))))
            {
                printf("\nError in reallocating memory. \n");
                return -1;
            }
            
            if(!(*(DataNameList + DataNum - 1) = (char *) calloc(strlen(DataName) + 1, sizeof(char))))
            {
                printf("\nError in allocating memory. \n");
                return -1;
            }
            
            strcpy(*(DataNameList + DataNum - 1), DataName);
        }
    }
    free(DataDirInfo);
    if((closedir(DataDP)))
    {
        printf("\nError in closing directory. \n");
        return -1;
    }
    DataDirInfo = NULL;
    DataDP = NULL;
    //
    //
    
    //__Reading_Data_and_Label_Files__//
    unsigned int nx = 0, Nth = 0;
    for(Nth = 1; Nth <= DataNum; Nth += 1)
    {
        printf("\nSum = %d, Now = %d\n", DataNum, Nth);
        
        //__Declarations_of_Label_Parameters__//
        unsigned char Flag = 0;
        char *p_inline = NULL;
        int map_scale = 0;
        double latmax = 0, latmin = 0, lonwst = 0, lonest = 0, map_scale_double = 0;
        char line[1024] = {'\0'};
        //
        
        //__Opening_Data_Files__//
        strcpy(DataName, *(DataNameList + Nth - 1));
        memset(NewDataPath0, 0, 256);
        strcpy(NewDataPath0, NewDataPath);
        if((fd_Data = fopen(strcat(NewDataPath0, DataName), "r")) == NULL)
        {
            printf("\nError in opening data. => Data = %s\n", DataName);
            return -1;
        }
        printf("Data:\t%s\n", DataName);
        //
        
        //__Label_Parameters__//
        fseek(fd_Data, 0, SEEK_SET);
        while((fgets(line, sizeof(line), fd_Data)) && (Flag ^ 0x1F))  //  //  (strcmp(fgets(line, sizeof(line), fd_Data), "END\r\n"))  //  //
        {
            if((strstr(line, "MAP_SCALE")) && (strstr(line, "=")))
            {
                if((strstr(line, "<METERS/PIXEL>")) || (strstr(line, "<METERS>")))
                {
                    p_inline = strchr(line, '=');
                    sscanf(p_inline + 1, "%lf", &map_scale_double);
                    map_scale = (int) (map_scale_double + 0.5);
                    printf("MAP_SCALE\t=\t%d\n", map_scale);
                    
                    Flag |= 0x01;
                }
                else if(strstr(line, "<KM/PIXEL>"))
                {
                    p_inline = strchr(line, '=');
                    sscanf(p_inline + 1, "%lf", &map_scale_double);
                    map_scale = (int) (map_scale_double * 1000 + 0.5);
                    printf("MAP_SCALE\t=\t%d\n", map_scale);
                    
                    Flag |= 0x01;
                }
            }
            
            if((strstr(line, "MAXIMUM_LATITUDE")) && (strstr(line, "=")))
            {
                p_inline = strchr(line, '=');
                sscanf(p_inline + 1, "%lf", &latmax);
                printf("MAXIMUM_LATITUDE\t=\t%lf\n", latmax);
                
                Flag |= 0x02;
            }
            
            if((strstr(line, "MINIMUM_LATITUDE")) && (strstr(line, "=")))
            {
                p_inline = strchr(line, '=');
                sscanf(p_inline + 1, "%lf", &latmin);
                printf("MINIMUM_LATITUDE\t=\t%lf\n", latmin);
                
                Flag |= 0x04;
            }
            
            if((strstr(line, "EASTERNMOST_LONGITUDE")) && (strstr(line, "=")))
            {
                p_inline = strchr(line, '=');
                sscanf(p_inline + 1, "%lf", &lonest);
                printf("EASTERNMOST_LONGITUDE\t=\t%lf\n", lonest);
                
                Flag |= 0x08;
            }
            
            if((strstr(line, "WESTERNMOST_LONGITUDE")) && (strstr(line, "=")))
            {
                p_inline = strchr(line, '=');
                sscanf(p_inline + 1, "%lf", &lonwst);
                printf("WESTERNMOST_LONGITUDE\t=\t%lf\n", lonwst);
                
                Flag |= 0x10;
            }
            
            memset(line, 0, sizeof(line));
        }
        
        if((fclose(fd_Data)))
        {
            printf("\nError in closing data files. \n");
            return -1;
        }
        fd_Data = NULL;
        //
        
        //__Writing_Coordinates_of_Boundary_Points_in_Files__//
        if(map_scale == Res_X_m)
        {
            fprintf(fd_xm, "%lf\t%lf\n%lf\t%lf\n%lf\t%lf\n%lf\t%lf\n%lf\t%lf\n<\n", lonest, latmax, lonwst, latmax, lonwst, latmin, lonest, latmin, lonest, latmax);
            fprintf(fd_DataNameList, "%s\n", DataName);
            nx += 1;
        }
        
        printf("Sum = %d, N_%sm = %d\n", DataNum, str, nx);
        //
    }
    //
    
    //__Closing_Files_and_Release_Pointers__//
    if((fclose(fd_xm)) || (fclose(fd_DataNameList)))
    {
        printf("\nError in closing output files. \n");
    }
    fd_xm = NULL;
    fd_DataNameList = NULL;
    
    for(Nth = 1; Nth <= DataNum; Nth += 1)
    {
        free(*(DataNameList + Nth - 1));
    }
    free(DataNameList);
    DataNameList = NULL;
    
    free(DataName);
    free(NewDataPath);
    free(NewDataPath0);
    free(NewOutputPath);
    free(NewOutputPath0);
    DataName = NULL;
    NewDataPath = NULL;
    NewDataPath0 = NULL;
    NewOutputPath = NULL;
    NewOutputPath0 = NULL;
    //
    
    printf("\nNAC_DTM_ExtractBoundaryPoints => Done. \n");
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *int2str(char *str, int n)
{
    char buf[10] = {0};
    int i = 0, len = 0, temp = (n < 0) ? (-n) : n;
    
    if(str == NULL)
    {
        printf("\n'char *str' is NULL. \n");
        return NULL;
    }
    
    while(temp)
    {
        buf[i++] = (temp % 10) + '0';
        temp /= 10;
    }
    
    len = (n < 0) ? (++i) : i;
    str[i] = 0;
    
    while(1)
    {
        i--;
        if(buf[len - i - 1] == 0)
            break;

        str[i] = buf[len - i - 1];
    }
    
    if(i == 0)
        str[i] = '-';
    
    return str;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

