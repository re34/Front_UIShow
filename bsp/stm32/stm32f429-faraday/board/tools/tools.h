#ifndef __TOOLS_H
#define __TOOLS_H

#define TOOL_USING_KALMAN


#if (TOOL_USING_KALMAN)
/*以下为卡尔曼滤波参数*/
#define LASTP 0.050F   // 上次估算协方差
#define COVAR_Q 0.055F // 过程噪声协方差
#define COVAR_R 0.067F // 测噪声协方差

    typedef struct
    {
        float Last_Covariance; // 上次估算协方差 初始化值为0.02
        float Now_Covariance;  // 当前估算协方差 初始化值为0
        float Output;          // 卡尔曼滤波器输出 初始化值为0
        float Kg;              // 卡尔曼增益 初始化值为0
        float Q;               // 过程噪声协方差 初始化值为0.001
        float R;               // 观测噪声协方差 初始化值为0.543
    } KFP;
    extern float kalmanFilter(KFP *kfp, float input);
#else
/*左移次数*/
#define FILTER_SHIFT 4U

typedef struct
{
    bool First_Flag;
    float SideBuff[1 << FILTER_SHIFT];
    float *Head;
    float Sum;
} SideParm;
extern float sidefilter(SideParm *side, float input);
#endif

#endif

