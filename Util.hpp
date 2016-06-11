//
//  Util.hpp
//  HybridCpp
//
//  Created by Wang on 16/5/29.
//  Copyright © 2016年 Wang. All rights reserved.
//

#ifndef Util_hpp
#define Util_hpp

#include <stdio.h>
#include "vector"
#include "string"
#include "iostream"
#include "fstream"
#include "map"
#include "math.h"
#include "numeric"
#pragma pack(2)
using namespace std;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

extern const string SrcDir;
extern const string GreyImg[];
extern const string DataPath;

struct BMP_FILE_HEADER{
    WORD bType; /* 文件标识符 */
    DWORD bSize; /* 文件的大小 */
    WORD bReserved1; /* 保留值,必须设置为0 */
    WORD bReserved2; /* 保留值,必须设置为0 */
    DWORD bOffset; /* 文件头的最后到图像数据位开始的偏移量 */
};

struct BMP_INFO{
    DWORD bInfoSize; /* 信息头的大小 */
    DWORD bWidth; /* 图像的宽度 */
    DWORD bHeight; /* 图像的高度 */
    WORD bPlanes; /* 图像的位面数 */
    WORD bBitCount; /* 每个像素的位数 */
    DWORD bCompression; /* 压缩类型 */
    DWORD bmpImageSize; /* 图像的大小,以字节为单位 */
    DWORD bXPelsPerMeter; /* 水平分辨率 */
    DWORD bYPelsPerMeter; /* 垂直分辨率 */
    DWORD bClrUsed; /* 使用的色彩数 */
    DWORD bClrImportant; /* 重要的颜色数 */
};

struct RGB_QUAD{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReversed;
};

struct RGB_PIXEL{
    BYTE bBlue;
    BYTE bGreen;
    BYTE bRed;
    RGB_PIXEL(BYTE bl, BYTE gr, BYTE re): bBlue(bl), bGreen(gr), bRed(re){}
};

struct pe_val_bit;

// PE grey image
struct pe_threshold{
    int totalBit; // To be embedded data number
    int T;  // Embedding threshold
    // {4, -4}, {3, -3}, {2, -2}, {1, -1}, {0}
    // {-4, 3}, {-3, 2}, {-2, 1}, {-1, 0}
    int TF; // Fluctuation threshold
    int TL; // Left histogram shrinking threshold
    // OR tn(pe_img_embed_mixEmb)
    int TR; // Right histogram shrinking threshold
    // OR tp(pe_img_embed_mixEmb)
    int S;  // Stop embedding threshold
    int P;  // Stop pixel
    // OR Already embeded data number(pe_embed_mixEmb)
    int circle; // Stop Circle
    vector<pe_val_bit> bookkeeping_left;
    vector<pe_val_bit> bookkeeping_right;
    // Bookkeeping data in shrinking histogram
    // Shrink from left to right
    pe_threshold(int tbit = -888,int t = -888,
                 int tf = -888, int tl = -888,
                 int tr = -888, int s = -888,
                 int p = -888, int c = -888):
    totalBit(tbit),
    T(t), TF(tf),TR(tr), TL(tl),S(s), P(p), circle(c){};
    bool isReadyEmbed(){
        return (totalBit >= 0) && (T != -888) && (TF != -888)
        && (TR != -888) && (TL != -888);
    }
    bool isReadyRecover(){
        return isReadyEmbed() && (S != -888)
        && (P != -888) && (circle != -888);
    }
    bool isInital(){
        return totalBit != -888;
    }
    void print(string str = "pe_threshold"){
        printf("%s: [T, TF, TL, TR] = [%d, %d, %d, %d]\n",
               str.c_str(), T, TF, TL, TR);
        printf("%s: [S, P, circle, BitNum] = [%d, %d, %d, %d]\n",
               str.c_str(), S, P, circle, totalBit);
    }
};

// PE grey image
struct pe_val_bit{
    int val;
    int bit;
    pe_val_bit():val(-1), bit(-1){};
    pe_val_bit(int v, int b): val(v), bit(b){};
    bool operator<(const pe_val_bit &obj) const{
        return (bit < obj.bit);
    }
};

// HS Grey Image
struct hs_threshold{
    vector<int> peakPoint;  // Selected peak value in every round
    vector<int> zeroPoint;  // Selected zero value in every round
    vector<int> bookkeeping; // Bookkeeping bits in every round
    vector<int> bitNum;   // To be embedded bits in every round
    int totalBit;   // To be embedded bits in all round
};

// Image Blocking
struct block_threshold{
    double Tdev;
    int blockSize;
    vector<int> bookkeeping;
    block_threshold(double d1, int i1):
    Tdev(d1), blockSize(i1){};
};
// Write Random Bit File
bool
writeFile(const string& filePath,
          const size_t& bitNum);

// Read File
vector<BYTE>
readFile(const string& filePath,
         const size_t& bitNum);

/*
 *  Grey BMP image
 */

// Read Grey Image API
vector<vector<BYTE>>
readBMP(const string& filePath);

// Write Grey Image API
bool
writeBMP(const vector<vector<BYTE>>& image, const string& filePath);

// Calculate Histogram (for PE smaller)
template <typename T>
inline vector<int>
calHistogram(const vector<vector<T>> &img){
    size_t bit = 8; // 8 bit depth
    vector<int> hist(1 << bit, 0);
    size_t row = img.size();
    size_t col = img[0].size();
    for (size_t r = 1; r < row - 1; r++)
        for (size_t c = 1; c < col - 1; c++)
            hist[img[r][c]]++;
    return hist;
}

// Calculate Histogram (for PE smaller)
template <typename T>
inline vector<int>
calHistogram(const vector<vector<T>> &img,
             const vector<vector<BYTE>> ref){
    size_t bit = 8; // 8 bit depth
    vector<int> hist(1 << bit, 0);
    size_t row = img.size();
    size_t col = img[0].size();
    for (size_t r = 1; r < row - 1; r++)
        for (size_t c = 1; c < col - 1; c++)
            if (ref[r][c] == 0)
                hist[img[r][c]]++;
    return hist;
}

// Calculate Standard Histogram
template <typename T>
vector<int>
calStdHistogram(const vector<vector<T>> &img){
    size_t bit = 8; // 8 bit depth
    vector<int> hist(1 << bit, 0);
    size_t row = img.size();
    size_t col = img[0].size();
    for (size_t r = 0; r < row; r++)
        for (size_t c = 0; c < col; c++)
            hist[img[r][c]]++;
    return hist;
}

// Calculate Block Histogram
vector<int>
calBlockHistogram(const vector<vector<BYTE>>& img,
                  const vector<vector<BYTE>>& ref);

// Calculate Grey Image PSNR
double
calPSNR(const vector<vector<BYTE>>&,
        const vector<vector<BYTE>>&);

// Convert Byte to Double Grey Image
vector<vector<double>>
byte2Double(const vector<vector<BYTE>>&);

// Convert Double to Byte Grey Image
vector<vector<BYTE>>
double2Byte(const vector<vector<double>>&);

// Convert Byte to Int Grey Image
vector<vector<int>>
byte2Int(const vector<vector<BYTE>>&);

// Convert Int to Byte Grey Image
vector<vector<BYTE>>
int2Byte(const vector<vector<int>>&);

// Max Value
template <typename T>
inline T
maxValue(const vector<vector<T>>& img){
    T max = img[0][0];
    int row = int(img.size());
    int col = int(img[0].size());
    for (int r = 0; r < row; r++)
        for (int c = 0; c < col; c++)
            if (img[r][c] > max)
                max = img[r][c];
    return max;
}

// Min Value
template <typename T>
inline T
minValue(const vector<vector<T>>& img){
    T min = img[0][0];
    int row = int(img.size());
    int col = int(img[0].size());
    for (int r = 0; r < row; r++)
        for (int c = 0; c < col; c++)
            if (img[r][c] < min)
                min = img[r][c];
    return min;
}

// Difference Pixel
template <typename T>
inline void
diffPixel(const vector<vector<T>>& img1,
          const vector<vector<T>>& img2){
    const string TAG = "diffPixel";
    int row = static_cast<int>(img1.size());
    int col = static_cast<int>(img1[0].size());
    for (int r = 0; r < row; r++) {
        for (int c = 0; c < col; c++) {
            int p1 = static_cast<int>(img1[r][c]);
            int p2 = static_cast<int>(img2[r][c]);
            if (p1 != p2) {
                printf("%s[%d,%d][%d,%d]\n",
                       TAG.c_str(), r, c, p1, p2);
            }
        }
    }
}

// Generate linearly spaced vector
vector<int>
linspace(const int start,
         const int end,
         const int N = 10);

// Block Image
vector<int>
blockSequence(const vector<vector<BYTE>>& image,
              const block_threshold& thres);

block_threshold
getBlockThreshold(const vector<vector<BYTE>>& image,
                  const vector<int>& seq,
                  block_threshold thres);
#endif /* Util_hpp */
