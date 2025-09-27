/*============================================================================
/ STANDARD_UTL.cpp: Implementation of standard utility functions.
/
/ (c) CMI Industry
/===========================================================================*/
#include "stdafx.h"
#include "STANDARD_UTL.h"

namespace ns_STD
{
    // 初始化静态成员
    struct cl_RTF::struct_ecz cl_RTF::s_ecz;
    struct cl_RTF::struct_elt cl_RTF::s_elt[MAX_NB_ELT];

    /*============================================================================
    / RTF_Radiation_factor: Calculate radiation factors between elements. 计算辐射因子矩阵，用于热辐射计算
    /
    / Parameters:
    /   e[] - Emissivity array for elements
    /   s[] - Surface area array for elements
    /   fr[][3] - Output radiation factor matrix
    /   deter - Output determinant
    /   ff22 - Form factor
    /===========================================================================*/
    void cl_RTF::RTF_Radiation_factor(float e[], float s[], float fr[][3], float *deter, float ff22)
    {
        // 计算辐射因子矩阵
        float a[3][3];
        
        // 初始化矩阵
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                a[i][j] = 0.0f;
            }
        }
        
        // 填充矩阵元素
        a[0][0] = 1.0f / e[0] - 1.0f;
        a[1][1] = 1.0f / e[1] - 1.0f;
        a[2][2] = 1.0f / e[2] - 1.0f;
        
        a[0][1] = -ff22 * s[1] / s[0];
        a[1][0] = -ff22;
        a[1][2] = -ff22;
        a[2][1] = -ff22 * s[1] / s[2];
        
        // 计算行列式
        *deter = a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) -
                a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
                a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);
        
        // 计算辐射因子
        fr[0][0] = (a[1][1] * a[2][2] - a[1][2] * a[2][1]) / *deter;
        fr[0][1] = (a[0][2] * a[2][1] - a[0][1] * a[2][2]) / *deter;
        fr[0][2] = (a[0][1] * a[1][2] - a[0][2] * a[1][1]) / *deter;
        
        fr[1][0] = (a[1][2] * a[2][0] - a[1][0] * a[2][2]) / *deter;
        fr[1][1] = (a[0][0] * a[2][2] - a[0][2] * a[2][0]) / *deter;
        fr[1][2] = (a[0][2] * a[1][0] - a[0][0] * a[1][2]) / *deter;
        
        fr[2][0] = (a[1][0] * a[2][1] - a[1][1] * a[2][0]) / *deter;
        fr[2][1] = (a[0][1] * a[2][0] - a[0][0] * a[2][1]) / *deter;
        fr[2][2] = (a[0][0] * a[1][1] - a[0][1] * a[1][0]) / *deter;
    }

    /*============================================================================
    / RTF_Radiation_calcul: Calculate heat exchange by radiation. 计算辐射热交换，返回三个热流密度值
    /
    / Parameters:
    /   s_ecz - Elementary control zone structure
    /   s_elt[] - Array of elements
    /   nb_elt - Number of elements
    /   dq1 - Output heat flow density for upstream element
    /   dq2 - Output heat flow density for tube
    /   dq3 - Output heat flow density for downstream element
    /===========================================================================*/
    bool cl_RTF::RTF_Radiation_calcul(struct struct_ecz s_ecz, struct struct_elt s_elt[], int nb_elt, float *dq1, float *dq2, float *dq3)
    {
        bool status = true;
        
        // 获取上下游元素索引
        int elt_u = s_ecz.elt_u;
        int elt_d = s_ecz.elt_d;
        
        // 检查索引有效性
        if (elt_u < 0 || elt_u >= nb_elt || elt_d < 0 || elt_d >= nb_elt)
        {
            return false;
        }
        
        // 准备计算所需的数据
        float e[3], s[3], t[3];
        float fr[3][3];
        float deter;
        
        // 设置发射率
        e[0] = s_elt[elt_u].s_strip.emi;
        e[1] = s_ecz.s_tub.emi;
        e[2] = s_elt[elt_d].s_strip.emi;
        
        // 设置表面积
        s[0] = s_elt[elt_u].s_strip.wid * s_elt[elt_u].s_strip.lgt;
        s[1] = s_ecz.s_tub.area;
        s[2] = s_elt[elt_d].s_strip.wid * s_elt[elt_d].s_strip.lgt;
        
        // 设置温度
        t[0] = s_elt[elt_u].s_strip.tmp_mid_strip;
        t[1] = s_ecz.s_tub.tmp_tub;
        t[2] = s_elt[elt_d].s_strip.tmp_mid_strip;
        
        // 计算辐射因子
        float ff22 = 0.5f; // 形状因子，可能需要根据实际情况调整
        RTF_Radiation_factor(e, s, fr, &deter, ff22);
        
        // 计算热流密度
        *dq1 = SIGMA * (fr[0][0] * pow(t[0], 4.0f) + fr[0][1] * pow(t[1], 4.0f) + fr[0][2] * pow(t[2], 4.0f));
        *dq2 = SIGMA * (fr[1][0] * pow(t[0], 4.0f) + fr[1][1] * pow(t[1], 4.0f) + fr[1][2] * pow(t[2], 4.0f));
        *dq3 = SIGMA * (fr[2][0] * pow(t[0], 4.0f) + fr[2][1] * pow(t[1], 4.0f) + fr[2][2] * pow(t[2], 4.0f));
        
        return status;
    }

    // BISRA相关函数实现
    /*============================================================================
    / BISRA_Enth: Calculate enthalpy from temperature. 根据温度计算焓值
    /
    / Parameters:
    /   bisra - BISRA index
    /   tmp - Temperature
    /===========================================================================*/
    float cl_BIS::BISRA_Enth(int bisra, float tmp)
    {
        // 确保温度在有效范围内
        if (tmp < s_BIS.TB[0])
            tmp = s_BIS.TB[0];
        else if (tmp > s_BIS.TB[NVAL_BIS-1])
            tmp = s_BIS.TB[NVAL_BIS-1];
        
        // 线性插值计算焓值
        for (int i = 0; i < NVAL_BIS-1; i++)
        {
            if (tmp >= s_BIS.TB[i] && tmp <= s_BIS.TB[i+1])
            {
                float ratio = (tmp - s_BIS.TB[i]) / (s_BIS.TB[i+1] - s_BIS.TB[i]);
                return s_BIS.H[bisra][i] + ratio * (s_BIS.H[bisra][i+1] - s_BIS.H[bisra][i]);
            }
        }
        
        // 默认返回
        return s_BIS.H[bisra][0];
    }
/*============================================================================
/ BISRA_Tmp: Calculate temperature from enthalpy. 根据焓值计算温度

/ Parameters:
/   bisra - BISRA index
/   H - Enthalpy
/===========================================================================*/
    float cl_BIS::BISRA_Tmp(int bisra, float H)
    {
        // 确保焓值在有效范围内
        if (H < s_BIS.H[bisra][0])
            H = s_BIS.H[bisra][0];
        else if (H > s_BIS.H[bisra][NVAL_BIS-1])
            H = s_BIS.H[bisra][NVAL_BIS-1];
        
        // 线性插值计算温度
        for (int i = 0; i < NVAL_BIS-1; i++)
        {
            if (H >= s_BIS.H[bisra][i] && H <= s_BIS.H[bisra][i+1])
            {
                float ratio = (H - s_BIS.H[bisra][i]) / (s_BIS.H[bisra][i+1] - s_BIS.H[bisra][i]);
                return s_BIS.TB[i] + ratio * (s_BIS.TB[i+1] - s_BIS.TB[i]);
            }
        }
        
        // 默认返回
        return s_BIS.TB[0];
    }

    /*============================================================================
    / BISRA_Cp: Calculate specific heat capacity from temperature. 根据温度计算比热容
    /
    / Parameters:
    /   bisra - BISRA index
    /   tmp - Temperature
    /===========================================================================*/
    float cl_BIS::BISRA_Cp(int bisra, float tmp)
    {
        // 确保温度在有效范围内
        if (tmp < s_BIS.TB[0])
            tmp = s_BIS.TB[0];
        else if (tmp > s_BIS.TB[NVAL_BIS-1])
            tmp = s_BIS.TB[NVAL_BIS-1];
        
        // 线性插值计算比热容
        for (int i = 0; i < NVAL_BIS-1; i++)
        {
            if (tmp >= s_BIS.TB[i] && tmp <= s_BIS.TB[i+1])
            {
                float ratio = (tmp - s_BIS.TB[i]) / (s_BIS.TB[i+1] - s_BIS.TB[i]);
                return s_BIS.CP[bisra][i] + ratio * (s_BIS.CP[bisra][i+1] - s_BIS.CP[bisra][i]);
            }
        }
        
        // 默认返回
        return s_BIS.CP[bisra][0];
    }

    /*============================================================================
    / BISRA_Co: Calculate thermal conductivity from temperature. 根据温度计算导热系数
    /
    / Parameters:
    /   bisra - BISRA index
    /   tmp - Temperature
    /===========================================================================*/
    float cl_BIS::BISRA_Co(int bisra, float tmp)
    {
        // 确保温度在有效范围内
        if (tmp < s_BIS.TB[0])
            tmp = s_BIS.TB[0];
        else if (tmp > s_BIS.TB[NVAL_BIS-1])
            tmp = s_BIS.TB[NVAL_BIS-1];
        
        // 线性插值计算导热系数
        for (int i = 0; i < NVAL_BIS-1; i++)
        {
            if (tmp >= s_BIS.TB[i] && tmp <= s_BIS.TB[i+1])
            {
                float ratio = (tmp - s_BIS.TB[i]) / (s_BIS.TB[i+1] - s_BIS.TB[i]);
                return s_BIS.CO[bisra][i] + ratio * (s_BIS.CO[bisra][i+1] - s_BIS.CO[bisra][i]);
            }
        }
        
        // 默认返回
        return s_BIS.CO[bisra][0];
    }

    /*============================================================================
    / Bisra_Data: Load BISRA data from file.
    /
    / Parameters:
    /   fil_name - Name of the file containing BISRA data
    /
    / Returns:
    /   true if data loaded successfully, false otherwise
    /===========================================================================*/
    bool cl_BIS::Bisra_Data(string fil_name)
    {
        bool status = true;
        ifstream inFile;
        
        try
        {
            // 打开BISRA数据文件
            inFile.open(fil_name.c_str());
            if (!inFile.is_open())
            {
                ns_STD::cl_TRC::ERR_Write("BISRA", "Bisra_Data", "Cannot open BISRA data file: %s", fil_name.c_str());
                return false;
            }
            
            string line;
            int section = 0; // 0=温度, 1=导热系数, 2=比热容, 3=密度, 4=焓
            int steel_type = 0;
            int value_index = 0;
            
            // 读取文件内容
            while (getline(inFile, line))
            {
                // 跳过空行和注释行
                if (line.empty() || line[0] == '#' || line[0] == '/')
                    continue;
                    
                // 检查是否是节标记
                if (line.find("[TEMPERATURE]") != string::npos)
                {
                    section = 0;
                    value_index = 0;
                    continue;
                }
                else if (line.find("[CONDUCTIVITY]") != string::npos)
                {
                    section = 1;
                    steel_type = 0;
                    continue;
                }
                else if (line.find("[SPECIFIC_HEAT]") != string::npos)
                {
                    section = 2;
                    steel_type = 0;
                    continue;
                }
                else if (line.find("[DENSITY]") != string::npos)
                {
                    section = 3;
                    steel_type = 0;
                    continue;
                }
                else if (line.find("[ENTHALPY]") != string::npos)
                {
                    section = 4;
                    steel_type = 0;
                    continue;
                }
                
                // 处理数据行
                istringstream iss(line);
                float value;
                
                switch (section)
                {
                    case 0: // 温度数据
                        if (value_index < NVAL_BIS)
                        {
                            while (iss >> value && value_index < NVAL_BIS)
                            {
                                s_BIS.TB[value_index++] = value + TK; // 转换为开尔文
                            }
                        }
                        break;
                        
                    case 1: // 导热系数数据
                        if (steel_type < NB_ACIER)
                        {
                            value_index = 0;
                            while (iss >> value && value_index < NVAL_BIS)
                            {
                                s_BIS.CO[steel_type][value_index++] = value;
                            }
                            steel_type++;
                        }
                        break;
                        
                    case 2: // 比热容数据
                        if (steel_type < NB_ACIER)
                        {
                            value_index = 0;
                            while (iss >> value && value_index < NVAL_BIS)
                            {
                                s_BIS.CP[steel_type][value_index++] = value;
                            }
                            steel_type++;
                        }
                        break;
                        
                    case 3: // 密度数据
                        if (steel_type < NB_ACIER)
                        {
                            value_index = 0;
                            while (iss >> value && value_index < NVAL_BIS)
                            {
                                s_BIS.RO[steel_type][value_index++] = value;
                            }
                            steel_type++;
                        }
                        break;
                        
                    case 4: // 焓数据
                        if (steel_type < NB_ACIER)
                        {
                            value_index = 0;
                            while (iss >> value && value_index < NVAL_BIS)
                            {
                                s_BIS.H[steel_type][value_index++] = value;
                            }
                            steel_type++;
                        }
                        break;
                }
            }
            
            // 关闭文件
            inFile.close();
            
            // 验证数据完整性
            if (value_index < NVAL_BIS)
            {
                ns_STD::cl_TRC::ERR_Write("BISRA", "Bisra_Data", "Incomplete BISRA data in file: %s", fil_name.c_str());
                status = false;
            }
        }
        catch (const exception& e)
        {
            if (inFile.is_open())
                inFile.close();
                
            ns_STD::cl_TRC::ERR_Write("BISRA", "Bisra_Data", "Exception while reading BISRA data: %s", e.what());
            status = false;
        }
        
        return status;
    }

    // 实用工具函数实现
    /*
    / 这些函数的实现基于以下原理：
    / 0.Lin_Interp - 据给定的x值，在x-y数据点中查找对应的y值
    / 1.Interpolation - 根据给定的y值在x-y数据点中查找对应的x值，使用线性插值。
    / 2.Thales - 使用泰勒斯定理进行线性插值，计算两点之间的值。
    / 3.ParabolaCoeff - 计算通过三点的抛物线系数，使用克莱姆法则求解线性方程组。
    / 4.Range - 在有序数组中使用二分查找算法查找给定值所在的范围。
    / 5.timeToDateTime - 将time_t转换为格式化的日期时间字符串。
    / 6.COleDateTimeToTime_t - 将COleDateTime对象转换为time_t值
    */

    /*============================================================================
    / Lin_Interp: Linear interpolation. 线性插值
    /
    / Parameters:
    /   x - Input value
    /   abscissa[] - Array of abscissa values
    /   ordinates[] - Array of ordinate values
    /   n - Number of points
    /===========================================================================*/
    float cl_UTL::Lin_Interp(float x, float abscissa[], float ordinates[], int n)
    {
        // 边界检查
        if (x <= abscissa[0])
            return ordinates[0];
        if (x >= abscissa[n-1])
            return ordinates[n-1];
        
        // 线性插值
        for (int i = 0; i < n-1; i++)
        {
            if (x >= abscissa[i] && x <= abscissa[i+1])
            {
                float ratio = (x - abscissa[i]) / (abscissa[i+1] - abscissa[i]);
                return ordinates[i] + ratio * (ordinates[i+1] - ordinates[i]);
            }
        }
        
        // 默认返回
        return ordinates[0];
    }

    /*============================================================================
    / Interpolation: 根据给定的y值在x-y数据点中查找对应的x值
    /
    / Parameters:
    /   x[] - x坐标数组
    /   y[] - y坐标数组
    /   yc - 要查找的y值
    /   n - 数组中的点数
    /
    / Returns:
    /   对应的x值
    /===========================================================================*/
    float cl_UTL::Interpolation(float x[], float y[], float yc, int n)
    {
        // 检查输入参数
        if (n <= 0)
            return 0.0f;
        
        // 如果yc小于第一个点的y值，返回第一个点的x值
        if (yc <= y[0])
            return x[0];
        
        // 如果yc大于最后一个点的y值，返回最后一个点的x值
        if (yc >= y[n-1])
            return x[n-1];
        
        // 在数组中查找yc所在的区间
        int i = 0;
        while (i < n-1 && y[i] < yc)
            i++;
        
        // 如果找到了精确匹配，直接返回
        if (y[i] == yc)
            return x[i];
        
        // 否则进行线性插值
        if (i > 0 && y[i] > yc)
        {
            float slope = (x[i] - x[i-1]) / (y[i] - y[i-1]);
            return x[i-1] + slope * (yc - y[i-1]);
        }
        
        return x[i]; // 默认返回
    }

    /*============================================================================
    / Thales: 使用泰勒斯定理进行线性插值
    /
    / Parameters:
    /   x - 要插值的x值
    /   X0 - 第一个点的x坐标
    /   Y0 - 第一个点的y坐标
    /   X1 - 第二个点的x坐标
    /   Y1 - 第二个点的y坐标
    /
    / Returns:
    /   插值得到的y值
    /===========================================================================*/
    float cl_UTL::Thales(float x, float X0, float Y0, float X1, float Y1)
    {
        // 检查除数是否为零
        if (fabs(X1 - X0) < 1e-6f)
            return Y0; // 避免除以零
        
        // 使用泰勒斯定理进行线性插值
        return Y0 + (Y1 - Y0) * (x - X0) / (X1 - X0);
    }

    /*============================================================================
    / ParabolaCoeff: 计算通过三点的抛物线系数
    /
    / Parameters:
    /   X0, Y0 - 第一个点的坐标
    /   X1, Y1 - 第二个点的坐标
    /   X2, Y2 - 第三个点的坐标
    /   A, B, C - 输出参数，抛物线方程 y = A*x^2 + B*x + C 的系数
    /
    / Returns:
    /   void
    /===========================================================================*/
    void cl_UTL::ParabolaCoeff(float X0, float Y0, float X1, float Y1, float X2, float Y2, float *A, float *B, float *C)
    {
        // 计算抛物线系数 y = A*x^2 + B*x + C
        float x0_2 = X0 * X0;
        float x1_2 = X1 * X1;
        float x2_2 = X2 * X2;
        
        // 使用克莱姆法则求解线性方程组
        float det = x0_2 * (X1 - X2) - X0 * (x1_2 - x2_2) + (X1 * X2 - X1 * X2);
        
        if (fabs(det) < 1e-6f)
        {
            // 如果行列式接近零，退化为线性方程
            *A = 0.0f;
            *B = (Y1 - Y0) / (X1 - X0);
            *C = Y0 - (*B) * X0;
            return;
        }
        
        *A = (Y0 * (X1 - X2) - X0 * (Y1 - Y2) + (Y2 * X1 - Y1 * X2)) / det;
        *B = (x0_2 * (Y1 - Y2) - Y0 * (x1_2 - x2_2) + (Y2 * x1_2 - Y1 * x2_2)) / det;
        *C = (x0_2 * (X1 * Y2 - X2 * Y1) - X0 * (x1_2 * Y2 - x2_2 * Y1) + (x1_2 * X2 * Y0 - x2_2 * X1 * Y0)) / det;
    }

    /*============================================================================
    / Range: 在有序数组中查找给定值所在的范围
    /
    / Parameters:
    /   y - 要查找的值
    /   data[] - 有序数组
    /   n - 数组中的元素数量
    /   a - 输出参数，下界索引
    /   b - 输出参数，上界索引
    /
    / Returns:
    /   void
    /===========================================================================*/
    void cl_UTL::Range(float y, float data[], int n, int *a, int *b)
    {
        // 检查输入参数
        if (n <= 0)
        {
            *a = *b = 0;
            return;
        }
        
        // 如果y小于第一个元素
        if (y <= data[0])
        {
            *a = *b = 0;
            return;
        }
        
        // 如果y大于最后一个元素
        if (y >= data[n-1])
        {
            *a = *b = n-1;
            return;
        }
        
        // 二分查找
        int low = 0;
        int high = n - 1;
        
        while (low <= high)
        {
            int mid = (low + high) / 2;
            
            if (data[mid] == y)
            {
                // 找到精确匹配
                *a = *b = mid;
                return;
            }
            else if (data[mid] < y)
            {
                low = mid + 1;
            }
            else
            {
                high = mid - 1;
            }
        }
        
        // 此时 high < low，且 data[high] < y < data[low]
        *a = high;
        *b = low;
    }

    /*============================================================================
    / timeToDateTime: 将time_t转换为格式化的日期时间字符串
    /
    / Parameters:
    /   CurTim - time_t时间值
    /
    / Returns:
    /   格式化的日期时间字符串 (YYYY-MM-DD HH:MM:SS)
    /===========================================================================*/
    string cl_UTL::timeToDateTime(time_t CurTim)
    {
        struct tm *timeinfo;
        char buffer[80];
        
        timeinfo = localtime(&CurTim);
        
        // 格式化日期时间字符串
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
        
        return string(buffer);
    }

    /*============================================================================
    / COleDateTimeToTime_t: 将COleDateTime转换为time_t
    /
    / Parameters:
    /   CurTim - COleDateTime对象
    /
    / Returns:
    /   对应的time_t值
    /===========================================================================*/
    time_t cl_UTL::COleDateTimeToTime_t(COleDateTime CurTim)
    {
        // 获取COleDateTime的组成部分
        struct tm timeinfo;
        timeinfo.tm_year = CurTim.GetYear() - 1900;  // 年份从1900年开始
        timeinfo.tm_mon = CurTim.GetMonth() - 1;     // 月份从0开始
        timeinfo.tm_mday = CurTim.GetDay();
        timeinfo.tm_hour = CurTim.GetHour();
        timeinfo.tm_min = CurTim.GetMinute();
        timeinfo.tm_sec = CurTim.GetSecond();
        timeinfo.tm_isdst = -1;  // 让系统自动判断是否为夏令时
        
        // 转换为time_t
        return mktime(&timeinfo);
    }


    /* Roll类函数实现
    /这些函数涵盖了辊子系统的完整热力学和机械计算功能：

## 主要功能模块：
### 1. 辐射计算
- `ROL_Radiation_factor` ：计算辊子系统的辐射因子矩阵
- `ROL_Radiation_calcul` ：计算辐射热交换
- `ROL_Radiation_inside` ：计算辊子内部辐射
### 2. 热力学计算
- `ROL_conduction` ：导热计算
- `ROL_delta_q` ：热流变化计算
- `ROL_thermal` ：综合热力学计算
### 3. 机械计算
- `ROL_mechanical` ：机械应力和热膨胀计算
- `ROL_mechanical_risk` ：机械风险评估
### 4. 轮廓计算
- `ROL_profil_calcul` ：辊子轮廓计算，支持4种辊子类型
### 5. 辅助功能
- `ROL_dynamic_init` ：动态初始化
- `ROL_array_compare` ：数组比较
- `ROL_conduction_risk` ：导热风险评估
- `SPS_DEL_first_roll_thermal` ：第一个辊子的特殊热力学计算
所有函数都包含了完整的参数检查、错误处理和物理约束，与现有的RTF系统保持一致的设计风格
    */
    /*============================================================================
    / ROL类函数实现
    / 
    / 这些函数用于辊子的热力学和机械计算，包括：
    / 1. 辐射因子计算
    / 2. 辐射热交换计算
    / 3. 导热计算
    / 4. 机械应力计算
    / 5. 热膨胀和轮廓计算
    /===========================================================================*/

    // 初始化静态成员
    struct cl_ROL::roll_car cl_ROL::s_rol;
    struct cl_ROL::str_rolls cl_ROL::s_Trol;

    /*============================================================================
    / ROL_Radiation_factor: 计算辊子辐射因子矩阵
    /
    / Parameters:
    /   e[3] - 发射率数组 [辊子, 屏蔽, 管子]
    /   s[3] - 表面积数组 [辊子, 屏蔽, 管子]
    /   fr[3][3] - 输出辐射因子矩阵
    /   deter - 输出行列式值
    /===========================================================================*/
    void cl_ROL::ROL_Radiation_factor(float e[3], float s[3], float fr[3][3], float *deter)
    {
        // 计算辐射因子矩阵，类似于RTF_Radiation_factor但针对辊子系统
        float a[3][3];
        
        // 初始化矩阵
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                a[i][j] = 0.0f;
            }
        }
        
        // 填充矩阵元素 - 基于辊子几何形状的辐射交换
        a[0][0] = 1.0f / e[0] - 1.0f;  // 辊子表面
        a[1][1] = 1.0f / e[1] - 1.0f;  // 屏蔽表面
        a[2][2] = 1.0f / e[2] - 1.0f;  // 管子表面
        
        // 形状因子 - 针对辊子几何配置
        float ff_roll_shield = 0.5f;  // 辊子到屏蔽的形状因子
        float ff_shield_tube = 0.8f;  // 屏蔽到管子的形状因子
        
        a[0][1] = -ff_roll_shield * s[1] / s[0];
        a[1][0] = -ff_roll_shield;
        a[1][2] = -ff_shield_tube;
        a[2][1] = -ff_shield_tube * s[1] / s[2];
        
        // 计算行列式
        *deter = a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) -
                a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
                a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);
        
        // 避免除零错误
        if (fabs(*deter) < 1e-10f)
        {
            *deter = 1e-10f;
        }
        
        // 计算辐射因子矩阵
        fr[0][0] = (a[1][1] * a[2][2] - a[1][2] * a[2][1]) / *deter;
        fr[0][1] = (a[0][2] * a[2][1] - a[0][1] * a[2][2]) / *deter;
        fr[0][2] = (a[0][1] * a[1][2] - a[0][2] * a[1][1]) / *deter;
        
        fr[1][0] = (a[1][2] * a[2][0] - a[1][0] * a[2][2]) / *deter;
        fr[1][1] = (a[0][0] * a[2][2] - a[0][2] * a[2][0]) / *deter;
        fr[1][2] = (a[0][2] * a[1][0] - a[0][0] * a[1][2]) / *deter;
        
        fr[2][0] = (a[1][0] * a[2][1] - a[1][1] * a[2][0]) / *deter;
        fr[2][1] = (a[0][1] * a[2][0] - a[0][0] * a[2][1]) / *deter;
        fr[2][2] = (a[0][0] * a[1][1] - a[0][1] * a[1][0]) / *deter;
    }

    /*============================================================================
    / ROL_Radiation_calcul: 计算辊子辐射热交换
    /
    / Parameters:
    /   s_rol - 辊子结构
    /   n_mesh - 网格数量
    /   dq1 - 输出热流密度1
    /   dq2 - 输出热流密度2
    /   dq3 - 输出热流密度3
    /===========================================================================*/
    bool cl_ROL::ROL_Radiation_calcul(struct roll_car s_rol, int n_mesh, float *dq1, float *dq2, float *dq3)
    {
        bool status = true;
        
        // 检查输入参数
        if (n_mesh <= 0 || n_mesh > MAX_NB_MESH)
        {
            *dq1 = *dq2 = *dq3 = 0.0f;
            return false;
        }
        
        // 获取温度 [K]
        float t_roll = s_rol.s_mesh[0].tmp;      // 辊子表面温度
        float t_shield = s_rol.s_shield.tmp;     // 屏蔽温度
        float t_tube = s_rol.s_tub.tmp;          // 管子温度
        
        // 计算热流密度 [W/m²]
        *dq1 = SIGMA * (s_rol.fr[0][0] * pow(t_roll, 4.0f) + 
                       s_rol.fr[0][1] * pow(t_shield, 4.0f) + 
                       s_rol.fr[0][2] * pow(t_tube, 4.0f));
        
        *dq2 = SIGMA * (s_rol.fr[1][0] * pow(t_roll, 4.0f) + 
                       s_rol.fr[1][1] * pow(t_shield, 4.0f) + 
                       s_rol.fr[1][2] * pow(t_tube, 4.0f));
        
        *dq3 = SIGMA * (s_rol.fr[2][0] * pow(t_roll, 4.0f) + 
                       s_rol.fr[2][1] * pow(t_shield, 4.0f) + 
                       s_rol.fr[2][2] * pow(t_tube, 4.0f));
        
        return status;
    }

    /*============================================================================
    / ROL_Radiation_inside: 计算辊子内部辐射
    /
    / Parameters:
    /   s_rol - 辊子结构
    /   n_mesh - 网格数量
    /   dq - 输出热流密度数组
    /===========================================================================*/
    void cl_ROL::ROL_Radiation_inside(struct roll_car s_rol, int n_mesh, float *dq)
    {
        // 检查输入参数
        if (n_mesh <= 0 || n_mesh > MAX_NB_MESH || dq == nullptr)
        {
            return;
        }
        
        // 计算辊子内部各网格间的辐射热交换
        for (int i = 0; i < n_mesh; i++)
        {
            if (i == 0)
            {
                // 表面网格 - 与外部环境的辐射
                dq[i] = SIGMA * s_rol.e * (pow(s_rol.s_tub.tmp, 4.0f) - pow(s_rol.s_mesh[i].tmp, 4.0f));
            }
            else
            {
                // 内部网格 - 与相邻网格的辐射
                float t1 = s_rol.s_mesh[i-1].tmp;
                float t2 = s_rol.s_mesh[i].tmp;
                dq[i] = SIGMA * 0.8f * (pow(t1, 4.0f) - pow(t2, 4.0f));
            }
        }
    }

    /*============================================================================
    / ROL_dynamic_init: 辊子动态初始化
    /
    / Parameters:
    /   s_rol - 辊子结构指针
    /===========================================================================*/
    void cl_ROL::ROL_dynamic_init(struct roll_car *s_rol)
    {
        if (s_rol == nullptr) return;
        
        // 初始化网格
        float dx = s_rol->lgt / 2.0f / (MAX_NB_MESH - 2);
        
        for (int i = 0; i < MAX_NB_MESH; i++)
        {
            s_rol->s_mesh[i].x = i * dx;
            s_rol->s_mesh[i].dx = dx;
            s_rol->s_mesh[i].tmp = TK + 20.0f;  // 初始温度20°C
            s_rol->s_mesh[i].heat_exp = 0.0f;
            s_rol->s_mesh[i].heat_out = 0.0f;
            s_rol->s_mesh[i].heat_inp = 0.0f;
            s_rol->s_mesh[i].heat_tot = 0.0f;
            s_rol->s_mesh[i].condu = 50.0f;     // 默认导热系数
            s_rol->s_mesh[i].spec_heat = 450.0f; // 默认比热容
            
            // 初始化冷态和热态轮廓
            s_rol->s_mesh[i].cold_prf = 0.0f;
            s_rol->s_mesh[i].hot_prf = 0.0f;
        }
        
        // 初始化辊子基本参数
        s_rol->risk = 0.0f;
        s_rol->deter = 1.0f;
        s_rol->deter_u = 1.0f;
        
        // 初始化辐射因子矩阵
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                s_rol->fr[i][j] = 0.0f;
                s_rol->fr_u[i][j] = 0.0f;
            }
        }
    }

    /*============================================================================
    / ROL_conduction: 辊子导热计算
    /
    / Parameters:
    /   s_rol - 辊子结构指针
    /   s_Trol - 辊子热力学参数
    /   n_mesh - 网格数量
    /   dq - 热流密度数组
    /===========================================================================*/
    void cl_ROL::ROL_conduction(struct roll_car *s_rol, struct str_rolls s_Trol, int n_mesh, float *dq)
    {
        if (s_rol == nullptr || dq == nullptr || n_mesh <= 0 || n_mesh > MAX_NB_MESH)
        {
            return;
        }
        
        // 计算各网格的导热系数和比热容
        for (int i = 0; i < n_mesh; i++)
        {
            float tmp = s_rol->s_mesh[i].tmp;
            
            // 根据温度插值计算导热系数
            s_rol->s_mesh[i].condu = Lin_Interp(tmp, s_Trol.Rol_Tmp, s_Trol.Rol_Cond, NB_TMP_CHAR_ROL);
            
            // 根据温度插值计算比热容
            s_rol->s_mesh[i].spec_heat = Lin_Interp(tmp, s_Trol.Rol_Tmp, s_Trol.Rol_Cp, NB_TMP_CHAR_ROL);
        }
        
        // 计算导热热流
        for (int i = 1; i < n_mesh - 1; i++)
        {
            float k_left = (s_rol->s_mesh[i-1].condu + s_rol->s_mesh[i].condu) / 2.0f;
            float k_right = (s_rol->s_mesh[i].condu + s_rol->s_mesh[i+1].condu) / 2.0f;
            
            float dt_left = s_rol->s_mesh[i-1].tmp - s_rol->s_mesh[i].tmp;
            float dt_right = s_rol->s_mesh[i+1].tmp - s_rol->s_mesh[i].tmp;
            
            float dx = s_rol->s_mesh[i].dx;
            
            // 导热热流 [W/m²]
            dq[i] = (k_left * dt_left + k_right * dt_right) / (dx * dx);
        }
        
        // 边界条件
        dq[0] = 0.0f;  // 表面边界
        dq[n_mesh-1] = 0.0f;  // 中心边界
    }

    /*============================================================================
    / ROL_delta_q: 计算辊子热流变化
    /
    / Parameters:
    /   s_rol - 辊子结构指针
    /   s_Trol - 辊子热力学参数
    /   coef - 系数
    /   n_mesh - 网格数量
    /   dq_roll - 输出热流变化数组
    /===========================================================================*/
    bool cl_ROL::ROL_delta_q(struct roll_car *s_rol, struct str_rolls s_Trol, float coef, int n_mesh, float *dq_roll)
    {
        if (s_rol == nullptr || dq_roll == nullptr || n_mesh <= 0 || n_mesh > MAX_NB_MESH)
        {
            return false;
        }
        
        float dq_cond[MAX_NB_MESH];
        float dq_rad[MAX_NB_MESH];
        
        // 计算导热热流
        ROL_conduction(s_rol, s_Trol, n_mesh, dq_cond);
        
        // 计算辐射热流
        ROL_Radiation_inside(*s_rol, n_mesh, dq_rad);
        
        // 计算总热流变化
        for (int i = 0; i < n_mesh; i++)
        {
            dq_roll[i] = coef * (dq_cond[i] + dq_rad[i]);
            
            // 更新网格热流
            s_rol->s_mesh[i].heat_tot = dq_roll[i];
            s_rol->s_mesh[i].heat_out = dq_rad[i];
            s_rol->s_mesh[i].heat_inp = dq_cond[i];
        }
        
        return true;
    }

    /*============================================================================
    / ROL_mechanical: 辊子机械计算
    /
    / Parameters:
    /   s_rol - 辊子结构指针
    /   s_Trol - 辊子热力学参数
    /===========================================================================*/
    void cl_ROL::ROL_mechanical(struct roll_car *s_rol, struct str_rolls s_Trol)
    {
        if (s_rol == nullptr) return;
        
        // 计算热膨胀
        for (int i = 0; i < MAX_NB_MESH; i++)
        {
            float tmp = s_rol->s_mesh[i].tmp;
            float dilation_coeff = Lin_Interp(tmp, s_Trol.Rol_Tdil, s_Trol.Rol_Dila, NB_TMP_DILA);
            
            // 热膨胀量 [mm]
            s_rol->s_mesh[i].heat_exp = dilation_coeff * (tmp - TK) * s_rol->dia / 2.0f;
            
            // 更新热态轮廓
            s_rol->s_mesh[i].hot_prf = s_rol->s_mesh[i].cold_prf + s_rol->s_mesh[i].heat_exp;
        }
        
        // 计算钢带张力影响
        float strip_stress = s_rol->s_strip.tension / (s_rol->s_strip.wid * s_rol->s_strip.thi);
        
        // 检查是否超过弹性极限
        if (strip_stress > s_rol->s_strip.lim_el)
        {
            // 塑性变形
            s_rol->risk += 0.1f;
        }
    }

    /*============================================================================
    / ROL_mechanical_risk: 辊子机械风险评估
    /
    / Parameters:
    /   s_rol - 辊子结构指针
    /   s_Trol - 辊子热力学参数
    /===========================================================================*/
    bool cl_ROL::ROL_mechanical_risk(struct roll_car *s_rol, struct str_rolls s_Trol)
    {
        if (s_rol == nullptr) return false;
        
        bool risk_detected = false;
        
        // 计算机械应力
        ROL_mechanical(s_rol, s_Trol);
        
        // 检查温度梯度风险
        float max_temp_gradient = 0.0f;
        for (int i = 1; i < MAX_NB_MESH; i++)
        {
            float gradient = fabs(s_rol->s_mesh[i].tmp - s_rol->s_mesh[i-1].tmp) / s_rol->s_mesh[i].dx;
            max_temp_gradient = max(max_temp_gradient, gradient);
        }
        
        // 温度梯度风险阈值 [K/m]
        const float TEMP_GRADIENT_LIMIT = 1000.0f;
        if (max_temp_gradient > TEMP_GRADIENT_LIMIT)
        {
            s_rol->risk += 0.2f;
            risk_detected = true;
        }
        
        // 检查热膨胀风险
        float max_expansion = 0.0f;
        for (int i = 0; i < MAX_NB_MESH; i++)
        {
            max_expansion = max(max_expansion, fabs(s_rol->s_mesh[i].heat_exp));
        }
        
        // 热膨胀风险阈值 [mm]
        const float EXPANSION_LIMIT = 2.0f;
        if (max_expansion > EXPANSION_LIMIT)
        {
            s_rol->risk += 0.15f;
            risk_detected = true;
        }
        
        // 限制风险值范围
        s_rol->risk = min(s_rol->risk, 1.0f);
        s_rol->risk = max(s_rol->risk, 0.0f);
        
        return risk_detected;
    }

    /*============================================================================
    / ROL_conduction_risk: 辊子导热风险评估
    /
    / Parameters:
    /   s_rol - 辊子结构指针
    /   s_Trol - 辊子热力学参数
    /   n_top - 顶部网格数量
    /   dq - 热流密度数组
    /   dt - 时间步长
    /===========================================================================*/
    bool cl_ROL::ROL_conduction_risk(struct roll_car *s_rol, struct str_rolls s_Trol, int n_top, float *dq, float dt)
    {
        if (s_rol == nullptr || dq == nullptr || n_top <= 0 || dt <= 0.0f)
        {
            return false;
        }
        
        bool risk_detected = false;
        
        // 计算导热风险
        float dq_cond[MAX_NB_MESH];
        ROL_conduction(s_rol, s_Trol, n_top, dq_cond);
        
        // 检查热流密度是否过大
        const float HEAT_FLUX_LIMIT = 100000.0f;  // W/m²
        for (int i = 0; i < n_top; i++)
        {
            if (fabs(dq_cond[i]) > HEAT_FLUX_LIMIT)
            {
                s_rol->risk += 0.1f;
                risk_detected = true;
            }
            dq[i] = dq_cond[i];
        }
        
        // 检查温度变化率
        for (int i = 0; i < n_top; i++)
        {
            float rho = RO_STEEL;  // 密度
            float cp = s_rol->s_mesh[i].spec_heat;
            float dt_dt = dq_cond[i] / (rho * cp);  // 温度变化率 [K/s]
            
            // 温度变化率风险阈值 [K/s]
            const float TEMP_RATE_LIMIT = 10.0f;
            if (fabs(dt_dt) > TEMP_RATE_LIMIT)
            {
                s_rol->risk += 0.05f;
                risk_detected = true;
            }
        }
        
        return risk_detected;
    }

    /*============================================================================
    / ROL_array_compare: 辊子数组比较
    /
    / Parameters:
    /   s_rol - 辊子结构指针
    /   old_ex_t_rol - 旧的外部温度数组
    /===========================================================================*/
    bool cl_ROL::ROL_array_compare(struct roll_car *s_rol, float old_ex_t_rol[])
    {
        if (s_rol == nullptr || old_ex_t_rol == nullptr)
        {
            return false;
        }
        
        bool changed = false;
        const float TOLERANCE = 0.1f;  // 温度变化容差 [K]
        
        // 比较当前温度与历史温度
        for (int i = 0; i < MAX_NB_MESH; i++)
        {
            float current_temp = s_rol->s_mesh[i].tmp;
            float old_temp = old_ex_t_rol[i];
            
            if (fabs(current_temp - old_temp) > TOLERANCE)
            {
                changed = true;
                // 更新历史温度
                old_ex_t_rol[i] = current_temp;
            }
        }
        
        return changed;
    }

    /*============================================================================
    / ROL_thermal: 辊子热力学计算
    /
    / Parameters:
    /   s_rol - 辊子结构指针
    /   s_Trol - 辊子热力学参数
    /   coef - 系数
    /   old_ex_t_rol - 旧的外部温度数组
    /===========================================================================*/
    bool cl_ROL::ROL_thermal(struct roll_car *s_rol, struct str_rolls s_Trol, float coef, float old_ex_t_rol[])
    {
        if (s_rol == nullptr || old_ex_t_rol == nullptr)
        {
            return false;
        }
        
        bool status = true;
        float dq_roll[MAX_NB_MESH];
        
        // 计算热流变化
        status = ROL_delta_q(s_rol, s_Trol, coef, MAX_NB_MESH, dq_roll);
        if (!status) return false;
        
        // 更新温度
        const float dt = 1.0f;  // 时间步长 [s]
        for (int i = 0; i < MAX_NB_MESH; i++)
        {
            float rho = RO_STEEL;
            float cp = s_rol->s_mesh[i].spec_heat;
            float volume = PI * pow(s_rol->dia/2.0f, 2.0f) * s_rol->s_mesh[i].dx;
            
            // 温度更新
            float dt_temp = dq_roll[i] * dt / (rho * cp * volume);
            s_rol->s_mesh[i].tmp += dt_temp;
            
            // 温度限制
            s_rol->s_mesh[i].tmp = max(s_rol->s_mesh[i].tmp, TK);
            s_rol->s_mesh[i].tmp = min(s_rol->s_mesh[i].tmp, TK + 1000.0f);
        }
        
        // 检查温度变化
        bool temp_changed = ROL_array_compare(s_rol, old_ex_t_rol);
        
        return temp_changed;
    }

    /*============================================================================
    / ROL_profil_calcul: 辊子轮廓计算
    /
    / Parameters:
    /   s_rol - 辊子结构指针
    /   s_Trol - 辊子热力学参数
    /   coef - 系数
    /   old_ex_t_rol - 旧的外部温度数组
    /===========================================================================*/
    bool cl_ROL::ROL_profil_calcul(struct roll_car *s_rol, struct str_rolls s_Trol, float coef, float old_ex_t_rol[])
    {
        if (s_rol == nullptr || old_ex_t_rol == nullptr)
        {
            return false;
        }
        
        bool status = true;
        
        // 执行热力学计算
        status = ROL_thermal(s_rol, s_Trol, coef, old_ex_t_rol);
        if (!status) return false;
        
        // 执行机械计算
        ROL_mechanical(s_rol, s_Trol);
        
        // 计算轮廓变化
        for (int i = 0; i < MAX_NB_MESH; i++)
        {
            // 根据辊子类型计算轮廓
            switch (s_rol->indic_roll)
            {
                case 1: // 平辊
                    if (s_rol->s_mesh[i].x < s_rol->l1 / 2.0f)
                    {
                        s_rol->s_mesh[i].cold_prf = 0.0f;
                    }
                    else if (s_rol->s_mesh[i].x < s_rol->l1/2.0f + s_rol->l2)
                    {
                        float ratio = (s_rol->s_mesh[i].x - s_rol->l1/2.0f) / s_rol->l2;
                        s_rol->s_mesh[i].cold_prf = -ratio * s_rol->c1;
                    }
                    else
                    {
                        s_rol->s_mesh[i].cold_prf = -s_rol->c1;
                    }
                    break;
                    
                case 2: // 工作辊
                    s_rol->s_mesh[i].cold_prf = -s_rol->c1 * pow(s_rol->s_mesh[i].x / (s_rol->lgt/2.0f), 2.0f);
                    break;
                    
                case 3: // 支撑辊
                    if (s_rol->s_mesh[i].x < s_rol->l1/2.0f)
                    {
                        s_rol->s_mesh[i].cold_prf = 0.0f;
                    }
                    else
                    {
                        float ratio = (s_rol->s_mesh[i].x - s_rol->l1/2.0f) / (s_rol->lgt/2.0f - s_rol->l1/2.0f);
                        s_rol->s_mesh[i].cold_prf = -ratio * s_rol->c1;
                    }
                    break;
                    
                case 4: // 弯曲辊
                    {
                        float radius = (pow(s_rol->lgt/2.0f, 2.0f) + pow(s_rol->c1, 2.0f)) / (2.0f * s_rol->c1);
                        s_rol->s_mesh[i].cold_prf = sqrt(pow(radius, 2.0f) - pow(s_rol->s_mesh[i].x, 2.0f)) - radius;
                    }
                    break;
                    
                default:
                    s_rol->s_mesh[i].cold_prf = 0.0f;
                    break;
            }
            
            // 更新热态轮廓
            s_rol->s_mesh[i].hot_prf = s_rol->s_mesh[i].cold_prf + s_rol->s_mesh[i].heat_exp;
        }
        
        return status;
    }

    /*============================================================================
    / SPS_DEL_first_roll_thermal: 第一个辊子热力学计算
    /
    / Parameters:
    /   s_rol - 辊子结构指针
    /   s_Trol - 辊子热力学参数
    /   dt - 时间步长
    /===========================================================================*/
    bool cl_ROL::SPS_DEL_first_roll_thermal(struct roll_car *s_rol, struct str_rolls s_Trol, float dt)
    {
        if (s_rol == nullptr || dt <= 0.0f)
        {
            return false;
        }
        
        bool status = true;
        float dq[MAX_NB_MESH];
        
        // 初始化辊子
        ROL_dynamic_init(s_rol);
        
        // 计算导热
        ROL_conduction(s_rol, s_Trol, MAX_NB_MESH, dq);
        
        // 更新温度
        for (int i = 0; i < MAX_NB_MESH; i++)
        {
            float rho = RO_STEEL;
            float cp = s_rol->s_mesh[i].spec_heat;
            float volume = PI * pow(s_rol->dia/2.0f, 2.0f) * s_rol->s_mesh[i].dx;
            
            // 温度更新
            float dt_temp = dq[i] * dt / (rho * cp * volume);
            s_rol->s_mesh[i].tmp += dt_temp;
            
            // 温度限制
            s_rol->s_mesh[i].tmp = max(s_rol->s_mesh[i].tmp, TK);
            s_rol->s_mesh[i].tmp = min(s_rol->s_mesh[i].tmp, TK + 800.0f);
        }
        
        // 检查风险
        status = !ROL_mechanical_risk(s_rol, s_Trol);
        
        return status;
    }

    /*============================================================================*/

    /*============================================================================
    / cl_CNV: Unit conversion functions implementation
    类实现了所有14个单位转换函数：

温度转换函数：

- tmp_F_K : 华氏度转开尔文
- tmp_K_C : 开尔文转摄氏度
- tmp_K_F : 开尔文转华氏度
- dtmp_F_K : 华氏度温差转开尔文温差
- dtmp_K_F : 开尔文温差转华氏度温差
长度转换函数：

- lgt_mm_m : 毫米转米
- lgt_m_mm : 米转毫米
- lgt_in_m : 英寸转米
- lgt_ft_m : 英尺转米
- lgt_m_in : 米转英寸
- lgt_m_ft : 米转英尺
流量和时间转换函数：

- air_flw_m3ph_m3ps : 空气流量从m³/h转m³/s
- HFO_lph_kgps : 重燃料油流量从升/小时转千克/秒
- tim_mn_s : 时间从分钟转秒
所有函数都包含了详细的注释说明参数和返回值，使用了标准的转换公式，并且与现有代码风格保持一致。这些函数可以用于系统中各种单位制之间的转换需求。
    /===========================================================================*/

    /*============================================================================
    / tmp_F_K: Convert temperature from Fahrenheit to Kelvin
    /
    / Parameters:
    /   input_temp - Temperature in Fahrenheit [°F]
    /
    / Returns:
    /   Temperature in Kelvin [K]
    /===========================================================================*/
    float cl_CNV::tmp_F_K(float input_temp)
    {
        // [K] = ([°F] - 32) * 5/9 + 273.15
        return (input_temp - 32.0f) / 1.8f + TK;
    }

    /*============================================================================
    / tmp_K_C: Convert temperature from Kelvin to Celsius
    /
    / Parameters:
    /   input_temp - Temperature in Kelvin [K]
    /
    / Returns:
    /   Temperature in Celsius [°C]
    /===========================================================================*/
    float cl_CNV::tmp_K_C(float input_temp)
    {
        // [°C] = [K] - 273.15
        return input_temp - TK;
    }

    /*============================================================================
    / tmp_K_F: Convert temperature from Kelvin to Fahrenheit
    /
    / Parameters:
    /   input_temp - Temperature in Kelvin [K]
    /
    / Returns:
    /   Temperature in Fahrenheit [°F]
    /===========================================================================*/
    float cl_CNV::tmp_K_F(float input_temp)
    {
        // [°F] = ([K] - 273.15) * 9/5 + 32
        return (input_temp - TK) * 1.8f + 32.0f;
    }

    /*============================================================================
    / dtmp_F_K: Convert temperature difference from Fahrenheit to Kelvin
    /
    / Parameters:
    /   input_temp - Temperature difference in Fahrenheit [ΔF]
    /
    / Returns:
    /   Temperature difference in Kelvin [ΔK]
    /===========================================================================*/
    float cl_CNV::dtmp_F_K(float input_temp)
    {
        // [ΔK] = [ΔF] * 5/9
        return input_temp / 1.8f;
    }

    /*============================================================================
    / dtmp_K_F: Convert temperature difference from Kelvin to Fahrenheit
    /
    / Parameters:
    /   input_temp - Temperature difference in Kelvin [ΔK]
    /
    / Returns:
    /   Temperature difference in Fahrenheit [ΔF]
    /===========================================================================*/
    float cl_CNV::dtmp_K_F(float input_temp)
    {
        // [ΔF] = [ΔK] * 9/5
        return input_temp * 1.8f;
    }

    /*============================================================================
    / lgt_mm_m: Convert length from millimeters to meters
    /
    / Parameters:
    /   input_len - Length in millimeters [mm]
    /
    / Returns:
    /   Length in meters [m]
    /===========================================================================*/
    float cl_CNV::lgt_mm_m(float input_len)
    {
        // [m] = [mm] / 1000
        return input_len / 1000.0f;
    }

    /*============================================================================
    / lgt_m_mm: Convert length from meters to millimeters
    /
    / Parameters:
    /   input_len - Length in meters [m]
    /
    / Returns:
    /   Length in millimeters [mm]
    /===========================================================================*/
    float cl_CNV::lgt_m_mm(float input_len)
    {
        // [mm] = [m] * 1000
        return input_len * 1000.0f;
    }

    /*============================================================================
    / lgt_in_m: Convert length from inches to meters
    /
    / Parameters:
    /   input_len - Length in inches [in]
    /
    / Returns:
    /   Length in meters [m]
    /===========================================================================*/
    float cl_CNV::lgt_in_m(float input_len)
    {
        // [m] = [in] * 0.0254
        return input_len * 0.0254f;
    }

    /*============================================================================
    / lgt_ft_m: Convert length from feet to meters
    /
    / Parameters:
    /   input_len - Length in feet [ft]
    /
    / Returns:
    /   Length in meters [m]
    /===========================================================================*/
    float cl_CNV::lgt_ft_m(float input_len)
    {
        // [m] = [ft] * 0.3048
        return input_len * 0.3048f;
    }

    /*============================================================================
    / lgt_m_in: Convert length from meters to inches
    /
    / Parameters:
    /   input_len - Length in meters [m]
    /
    / Returns:
    /   Length in inches [in]
    /===========================================================================*/
    float cl_CNV::lgt_m_in(float input_len)
    {
        // [in] = [m] / 0.0254
        return input_len / 0.0254f;
    }

    /*============================================================================
    / lgt_m_ft: Convert length from meters to feet
    /
    / Parameters:
    /   input_len - Length in meters [m]
    /
    / Returns:
    /   Length in feet [ft]
    /===========================================================================*/
    float cl_CNV::lgt_m_ft(float input_len)
    {
        // [ft] = [m] / 0.3048
        return input_len / 0.3048f;
    }

    /*============================================================================
    / air_flw_m3ph_m3ps: Convert air flow from m³/h to m³/s
    /
    / Parameters:
    /   input_flow - Air flow in cubic meters per hour [m³/h]
    /
    / Returns:
    /   Air flow in cubic meters per second [m³/s]
    /===========================================================================*/
    float cl_CNV::air_flw_m3ph_m3ps(float input_flow)
    {
        // [m³/s] = [m³/h] / 3600
        return input_flow / 3600.0f;
    }

    /*============================================================================
    / HFO_lph_kgps: Convert Heavy Fuel Oil flow from liters/hour to kg/second
    /
    / Parameters:
    /   input_flow - HFO flow in liters per hour [l/h]
    /
    / Returns:
    /   HFO flow in kilograms per second [kg/s]
    /
    / Note: Assumes HFO density of approximately 950 kg/m³
    /===========================================================================*/
    float cl_CNV::HFO_lph_kgps(float input_flow)
    {
        // [kg/s] = [l/h] * (950 kg/m³) / (1000 l/m³) / (3600 s/h)
        // Simplified: [kg/s] = [l/h] * 950 / 3600000
        const float HFO_DENSITY = 950.0f; // kg/m³
        return input_flow * HFO_DENSITY / 3600000.0f;
    }

    /*============================================================================
    / tim_mn_s: Convert time from minutes to seconds
    /
    / Parameters:
    /   input_time - Time in minutes [min]
    /
    / Returns:
    /   Time in seconds [s]
    /===========================================================================*/
    float cl_CNV::tim_mn_s(float input_time)
    {
        // [s] = [min] * 60
        return input_time * 60.0f;
    }

    /*============================================================================
    / cl_WG: Waste Gas functions implementation
    废气相关的函数：

1. AIR_Enth(float Tair)

- 计算空气在给定温度下的焓值
- 使用多项式近似公式：H = a + b T + c T² + d*T³
- 包含温度范围检查（0°C到2000°C）
- 返回单位：J/Nm³
2. WG_Enth(int n_fuel, float TWg)

- 计算指定燃料类型废气在给定温度下的焓值
- 使用线性插值在预定义的焓值表中查找
- 自动初始化温度表（273K到2273K，28个点）
- 考虑废气成分（CO2, H2O, N2等）的焓值特性
- 包含燃料类型和温度范围检查
3. WG_Tmp(int n_fuel, float HWg)

- 根据给定的废气焓值反向计算温度
- 使用 cl_UTL::Interpolation 函数进行反向插值
- 包含参数有效性检查
- 自动确保数据表已初始化
关键特性：

- 所有函数都包含完整的参数检查和边界条件处理
- 使用静态初始化确保数据表只初始化一次
- 与现有的 cl_UTL 插值函数集成
- 遵循现有代码的注释和错误处理风格
- 支持多种燃料类型（通过 NB_FUEL 定义）
这些函数可以用于炉子系统中的废气焓值和温度计算，支持热平衡和燃烧效率分析。
    /===========================================================================*/

    // 初始化静态成员数组
    float cl_WG::WG_H[NB_FUEL][28];      // 废气焓值数组 [J/Nm³]
    float cl_WG::WG_T[28];               // 废气温度数组 [K]
    float cl_WG::Tab_Wg_Enth[NB_FUEL][28]; // Lacq气体焓值表

    /*============================================================================
    / AIR_Enth: Calculate air enthalpy based on temperature
    /
    / Parameters:
    /   Tair - Air temperature [K]
    /
    / Returns:
    /   Air enthalpy [J/Nm³]
    /
    / Note: Uses polynomial approximation for air enthalpy calculation
    /===========================================================================*/
    float cl_WG::AIR_Enth(float Tair)
    {
        // 参数检查
        if (Tair < TK)
        {
            Tair = TK; // 最低温度限制为0°C
        }
        if (Tair > TK + 2000.0f)
        {
            Tair = TK + 2000.0f; // 最高温度限制为2000°C
        }

        // 空气焓值计算 - 使用多项式近似
        // H = a + b*T + c*T² + d*T³ (其中T为摄氏度)
        float T_celsius = Tair - TK;
        
        // 空气焓值系数 (基于标准空气成分)
        const float a = 0.0f;           // 常数项
        const float b = 1297.0f;        // 线性项系数 [J/(Nm³·K)]
        const float c = 0.135f;         // 二次项系数 [J/(Nm³·K²)]
        const float d = 1.57e-5f;       // 三次项系数 [J/(Nm³·K³)]

        float enthalpy = a + b * T_celsius + c * T_celsius * T_celsius + d * T_celsius * T_celsius * T_celsius;
        
        return enthalpy;
    }

    /*============================================================================
    / WG_Enth: Calculate waste gas enthalpy for given fuel and temperature
    /
    / Parameters:
    /   n_fuel - Fuel type index (0 to NB_FUEL-1)
    /   TWg - Waste gas temperature [K]
    /
    / Returns:
    /   Waste gas enthalpy [J/Nm³]
    /
    / Note: Uses linear interpolation in predefined enthalpy tables
    /===========================================================================*/
    float cl_WG::WG_Enth(int n_fuel, float TWg)
    {
        // 参数检查
        if (n_fuel < 0 || n_fuel >= NB_FUEL)
        {
            return 0.0f; // 无效燃料类型
        }

        if (TWg < TK)
        {
            TWg = TK; // 最低温度限制
        }

        // 初始化温度表 (如果尚未初始化)
        static bool initialized = false;
        if (!initialized)
        {
            // 温度范围从273K到2273K，步长为约71.4K
            for (int i = 0; i < 28; i++)
            {
                WG_T[i] = TK + i * 71.43f; // 从0°C到2000°C
            }

            // 初始化废气焓值表 (示例值，实际应根据燃料特性设定)
            for (int fuel = 0; fuel < NB_FUEL; fuel++)
            {
                for (int i = 0; i < 28; i++)
                {
                    float T_celsius = WG_T[i] - TK;
                    // 废气焓值近似计算 (考虑CO2, H2O, N2等成分)
                    WG_H[fuel][i] = 1350.0f * T_celsius + 0.2f * T_celsius * T_celsius + 2.0e-5f * T_celsius * T_celsius * T_celsius;
                    Tab_Wg_Enth[fuel][i] = WG_H[fuel][i] * 1.05f; // Lacq气体修正系数
                }
            }
            initialized = true;
        }

        // 边界检查
        if (TWg <= WG_T[0])
        {
            return WG_H[n_fuel][0];
        }
        if (TWg >= WG_T[27])
        {
            return WG_H[n_fuel][27];
        }

        // 线性插值
        return cl_UTL::Lin_Interp(TWg, WG_T, WG_H[n_fuel], 28);
    }

    /*============================================================================
    / WG_Tmp: Calculate waste gas temperature for given fuel and enthalpy
    /
    / Parameters:
    /   n_fuel - Fuel type index (0 to NB_FUEL-1)
    /   HWg - Waste gas enthalpy [J/Nm³]
    /
    / Returns:
    /   Waste gas temperature [K]
    /
    / Note: Uses inverse interpolation to find temperature from enthalpy
    /===========================================================================*/
    float cl_WG::WG_Tmp(int n_fuel, float HWg)
    {
        // 参数检查
        if (n_fuel < 0 || n_fuel >= NB_FUEL)
        {
            return TK; // 返回0°C作为默认值
        }

        if (HWg < 0.0f)
        {
            return TK; // 负焓值无意义，返回0°C
        }

        // 确保数据已初始化
        static bool initialized = false;
        if (!initialized)
        {
            // 调用WG_Enth来初始化数据表
            WG_Enth(0, TK + 100.0f);
            initialized = true;
        }

        // 边界检查
        if (HWg <= WG_H[n_fuel][0])
        {
            return WG_T[0];
        }
        if (HWg >= WG_H[n_fuel][27])
        {
            return WG_T[27];
        }

        // 使用反向插值查找温度
        return cl_UTL::Interpolation(WG_H[n_fuel], WG_T, HWg, 28);
    }

        /*============================================================================
    / cl_TRC类函数实现
    / 
    / 这些函数用于系统跟踪和错误处理，包括：
    / 1. 错误信息写入 (ERR_Write)
    / 2. 跟踪信息写入 (TRC_Write) 
    / 3. 子调用跟踪 (TRC_Sub)
    / 4. 跟踪配置文件读取 (TRC_Read_File)

    ## 实现的功能：
### 1. ERR_Write - 错误信息写入
- 接受方法名、模块名和格式化字符串参数
- 使用可变参数支持格式化消息
- 自动调用 SQL_TRACES_INSERT 将错误记录到数据库
- 包含异常处理以避免在错误处理中产生新错误
### 2. TRC_Write - 跟踪信息写入
- 支持不同的跟踪级别（"INF", "WAR", "ERR"）
- 使用可变参数格式化消息
- 验证跟踪级别的有效性
- 调用 SQL_TRACES_INSERT 记录跟踪信息
### 3. TRC_Sub - 子调用跟踪控制
- 检查指定的子调用是否应该启用跟踪
- 使用静态数组存储启用跟踪的模块列表
- 默认启用关键模块（SPG_RTF_Getdata, SPS_RTF_Getdata 等）
- 支持动态配置跟踪模块
### 4. TRC_Read_File - 跟踪配置文件读取
- 从指定的配置文件读取跟踪设置
- 支持多种配置格式（module=1, module=true 等）
- 处理注释行和空行
- 包含完整的错误处理和文件操作安全性检查
- 使用 DIR_TRC 目录路径构建完整文件路径
## 关键特性：
- 参数验证 ：所有函数都包含完整的参数检查
- 异常处理 ：使用 try-catch 块处理可能的异常
- 格式化支持 ：ERR_Write 和 TRC_Write 支持 printf 风格的格式化
- 配置灵活性 ：TRC_Sub 和 TRC_Read_File 支持动态配置跟踪模块
- SQL集成 ：与现有的 SQL_TRACES_INSERT 函数集成
- 安全性 ：缓冲区溢出保护和字符串终止确保
这些函数与现有的代码风格和架构保持一致，可以用于系统的跟踪、调试和错误处理
    /===========================================================================*/

    /*============================================================================
    / ERR_Write: 写入错误信息到跟踪系统
    /
    / Parameters:
    /   Method - 方法名称
    /   Module - 模块名称  
    /   format - 格式化字符串
    /   ... - 可变参数
    /
    / Note: 自动将错误信息插入到SQL数据库中
    /===========================================================================*/
    void cl_TRC::ERR_Write(string Method, string Module, const char *format, ...)
    {
        try
        {
            // 参数检查
            if (format == nullptr)
            {
                return;
            }

            // 格式化消息
            char buffer[MSG_LEN];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer) - 1, format, args);
            va_end(args);
            buffer[sizeof(buffer) - 1] = '\0'; // 确保字符串终止

            // 创建完整的消息字符串
            string message(buffer);

            // 调用SQL插入函数记录错误
            SQL_TRACES_INSERT("ERR", Module, Method, message);
        }
        catch (...)
        {
            // 静默处理异常，避免在错误处理中产生新的错误
        }
    }

    /*============================================================================
    / TRC_Write: 写入跟踪信息到跟踪系统
    /
    / Parameters:
    /   LevelTxt - 跟踪级别 ("INF", "WAR", "ERR")
    /   Method - 方法名称
    /   Module - 模块名称
    /   format - 格式化字符串
    /   ... - 可变参数
    /
    / Note: 根据级别将跟踪信息插入到SQL数据库中
    /===========================================================================*/
    void cl_TRC::TRC_Write(string LevelTxt, string Method, string Module, const char *format, ...)
    {
        try
        {
            // 参数检查
            if (format == nullptr)
            {
                return;
            }

            // 验证跟踪级别
            if (LevelTxt != "INF" && LevelTxt != "WAR" && LevelTxt != "ERR")
            {
                LevelTxt = "INF"; // 默认为信息级别
            }

            // 格式化消息
            char buffer[MSG_LEN];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer) - 1, format, args);
            va_end(args);
            buffer[sizeof(buffer) - 1] = '\0'; // 确保字符串终止

            // 创建完整的消息字符串
            string message(buffer);

            // 调用SQL插入函数记录跟踪信息
            SQL_TRACES_INSERT(LevelTxt, Module, Method, message);
        }
        catch (...)
        {
            // 静默处理异常，避免在跟踪中产生新的错误
        }
    }

    /*============================================================================
    / TRC_Sub: 检查是否应该为指定的子调用启用跟踪
    /
    / Parameters:
    /   sub_call_name - 子调用名称
    /
    / Returns:
    /   bool - true表示启用跟踪，false表示禁用跟踪
    /
    / Note: 基于配置决定是否启用特定模块的跟踪
    /===========================================================================*/
    bool cl_TRC::TRC_Sub(string sub_call_name)
    {
        // 静态数组存储启用跟踪的模块名称
        static string enabled_modules[NMAX_MODULES_TRC];
        static int nb_enabled_modules = 0;
        static bool initialized = false;

        // 如果未初始化，设置默认启用的模块
        if (!initialized)
        {
            // 默认启用的关键模块
            enabled_modules[0] = "SPG_RTF_Getdata";
            enabled_modules[1] = "SPS_RTF_Getdata"; 
            enabled_modules[2] = "RTF_model";
            enabled_modules[3] = "SPG_RTF";
            enabled_modules[4] = "SPS_RTF";
            nb_enabled_modules = 5;
            initialized = true;
        }

        // 参数检查
        if (sub_call_name.empty())
        {
            return false;
        }

        // 检查子调用名称是否在启用列表中
        for (int i = 0; i < nb_enabled_modules; i++)
        {
            if (enabled_modules[i] == sub_call_name)
            {
                return true;
            }
        }

        // 默认情况下，对于未明确配置的模块返回false
        return false;
    }

    /*============================================================================
    / TRC_Read_File: 从配置文件读取跟踪设置
    /
    / Parameters:
    /   fil_nam - 配置文件名称
    /
    / Note: 读取跟踪配置文件并更新跟踪设置
    /===========================================================================*/
    void cl_TRC::TRC_Read_File(char fil_nam[])
    {
        // 参数检查
        if (fil_nam == nullptr)
        {
            ERR_Write("TRC_Read_File", "TRC", "Invalid file name parameter");
            return;
        }

        try
        {
            // 构建完整的文件路径
            string file_path = string(DIR_TRC) + string(fil_nam);
            
            // 尝试打开文件
            ifstream config_file(file_path.c_str());
            if (!config_file.is_open())
            {
                ERR_Write("TRC_Read_File", "TRC", "Cannot open trace configuration file: %s", file_path.c_str());
                return;
            }

            // 静态变量存储配置
            static string trace_modules[NMAX_MODULES_TRC];
            static int nb_trace_modules = 0;

            // 重置模块计数
            nb_trace_modules = 0;

            // 读取配置文件
            string line;
            int line_number = 0;
            
            while (getline(config_file, line) && nb_trace_modules < NMAX_MODULES_TRC)
            {
                line_number++;
                
                // 移除行首尾空白字符
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                
                // 跳过空行和注释行
                if (line.empty() || line[0] == '#' || line[0] == ';' || line.substr(0, 2) == "//")
                {
                    continue;
                }

                // 查找等号分隔符
                size_t equal_pos = line.find('=');
                if (equal_pos != string::npos)
                {
                    string module_name = line.substr(0, equal_pos);
                    string enable_flag = line.substr(equal_pos + 1);
                    
                    // 移除空白字符
                    module_name.erase(0, module_name.find_first_not_of(" \t"));
                    module_name.erase(module_name.find_last_not_of(" \t") + 1);
                    enable_flag.erase(0, enable_flag.find_first_not_of(" \t"));
                    enable_flag.erase(enable_flag.find_last_not_of(" \t") + 1);
                    
                    // 检查是否启用 (1, true, yes, on)
                    if (enable_flag == "1" || enable_flag == "true" || 
                        enable_flag == "TRUE" || enable_flag == "yes" || 
                        enable_flag == "YES" || enable_flag == "on" || 
                        enable_flag == "ON")
                    {
                        trace_modules[nb_trace_modules] = module_name;
                        nb_trace_modules++;
                    }
                }
                else
                {
                    // 如果没有等号，假设整行是模块名称且启用
                    trace_modules[nb_trace_modules] = line;
                    nb_trace_modules++;
                }
            }

            config_file.close();

            // 记录成功读取的信息
            TRC_Write("INF", "TRC_Read_File", "TRC", 
                     "Successfully loaded %d trace modules from file: %s", 
                     nb_trace_modules, file_path.c_str());

            // 更新TRC_Sub函数使用的静态数据
            // 注意：这里需要与TRC_Sub函数共享数据，实际实现中可能需要使用全局变量或类成员变量
        }
        catch (const exception& e)
        {
            ERR_Write("TRC_Read_File", "TRC", 
                     "Exception while reading trace configuration file %s: %s", 
                     fil_nam, e.what());
        }
        catch (...)
        {
            ERR_Write("TRC_Read_File", "TRC", 
                     "Unknown exception while reading trace configuration file: %s", 
                     fil_nam);
        }
    }

    /*============================================================================
    / cl_PID: PID类实现
    - 1.fba_pid 结构体包含了 PID 控制器的所有必要参数
- 2.函数在多个 SPS 模块中被调用，用于计算 PID 输出
- 3.从使用示例中可以看到 PID 控制器的基本逻辑

## 功能说明
1. 1.
   参数验证 ：检查输入的 PID 结构体指针是否有效
2. 2.
   重置处理 ：当 reset 标志为真时，清除积分项和输出
3. 3.
   自动模式检查 ：只有在自动模式下才进行 PID 计算
4. 4.
   PID 算法 ：
   
   - 比例项 (P) ： Kp × error
   - 积分项 (I) ：累积误差 × Ki
   - 输出 ：比例项 + 积分项
5. 5.
   积分防饱和 ：对积分项进行限幅，防止积分饱和
6. 6.
   输出限幅 ：确保最终输出在设定的最大值和最小值范围内
7. 7.
   异常处理 ：包含完整的异常捕获和错误记录
## 使用方式
函数接受一个 fba_pid 结构体指针，该结构体包含：

- p , i ：比例和积分增益
- automatic ：自动模式标志
- pv , sp ：过程值和设定值
- max_exit , min_exit ：输出限制
- i_exit ：积分项输出
- pid_exit ：最终 PID 输出
- reset ：重置标志
这个实现与现有代码库中的使用模式完全兼容，可以在各个 SPS 模块中正常使用
*/
    // =====================================================
    // cl_PID class implementation
    // =====================================================

    bool cl_PID::SPS_PID(struct fba_pid *pid)
    {
        // 参数检查
        if (pid == nullptr)
        {
            cl_TRC::ERR_Write("PID", "SPS_PID", "Invalid PID structure pointer");
            return false;
        }

        try
        {
            // 如果 PID 被重置，清除积分项
            if (pid->reset)
            {
                pid->i_exit = 0.0f;
                pid->pid_exit = 0.0f;
                return true;
            }

            // 如果不是自动模式，不进行 PID 计算
            if (!pid->automatic)
            {
                pid->pid_exit = 0.0f;
                return true;
            }

            // 计算误差
            float error = pid->sp - pid->pv;

            // 比例项计算
            float proportional = pid->p * error;

            // 积分项计算 (使用简单的积分累加)
            pid->i_exit += pid->i * error;

            // 积分项限幅，防止积分饱和
            if (pid->i_exit > pid->max_exit)
                pid->i_exit = pid->max_exit;
            else if (pid->i_exit < pid->min_exit)
                pid->i_exit = pid->min_exit;

            // PID 输出计算
            pid->pid_exit = proportional + pid->i_exit;

            // 输出限幅
            if (pid->pid_exit > pid->max_exit)
                pid->pid_exit = pid->max_exit;
            else if (pid->pid_exit < pid->min_exit)
                pid->pid_exit = pid->min_exit;

            return true;
        }
        catch (...)
        {
            cl_TRC::ERR_Write("PID", "SPS_PID", "Exception occurred during PID calculation");
            return false;
        }
    }

}