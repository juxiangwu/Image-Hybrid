//
//  Util.cpp
//  HybridCpp
//
//  Created by Wang on 16/5/29.
//  Copyright © 2016年 Wang. All rights reserved.
//

#include "Util.hpp"
extern const string SrcDir = "/Users/wang/Code/Cpp/HybridCpp/HybridCpp/Image/";
extern const string DataPath = SrcDir + "randomBit";
extern const string GreyImg[] = {
    "Im1.bmp", "Im2.bmp", "chest.bmp"
};
BMP_FILE_HEADER MY_FILE_HEADER;
BMP_INFO MY_BMP_INFO;
vector<RGB_QUAD> MY_QUAD;

bool
writeFile(const string& filePath,
          const size_t& bitNum){
    /*  Write Random Bit File
     *
     */
    ofstream fopen(filePath.c_str());
    if (!fopen) {
        printf("Error (writeFile): File Open Exception\n\t: %s\n",
               filePath.c_str());
        return false;
    }
    if (bitNum <= 0) {
        return false;
    }
    for (size_t i = 0; i < bitNum; i++){
        BYTE ch = (rand() & 1 );
        fopen.put(ch);
    }
    fopen.close();
    fopen.clear();
    return true;
}

vector<BYTE>
readFile(const string& filePath,
         const size_t& bitNum){
    /*  Read Random File
     *
     */
    
    ifstream fin(filePath.c_str());
    vector<BYTE> res;
    if (!fin){
        printf("Error (readFile): Read File Exception\n\t:%s\n",
               filePath.c_str());
        return res;
    }
    if (bitNum <= 0) {
        return res;
    }
    char ch;
    for (size_t i = 0; (i < bitNum); i++) {
        fin.get(ch);
        res.push_back(ch);
    }
    
    fin.close();
    fin.clear();
    return res;
}


static bool
readBMP(const string& filePath,
        vector<vector<BYTE>> &image,
        BMP_FILE_HEADER *bmpHeader,
        BMP_INFO *bmpInfo,
        vector<RGB_QUAD> &bmpQuad){
    /*  Read Grey BMP Image from Storage
     *
     *
     */
    
    const char* cPath = filePath.c_str();
    FILE *fp;
    if((fp = fopen(cPath, "rb")) == NULL)
    {
        cerr << "Error (readBMP): Cannot Open File: " << filePath << endl;
        exit(-1);
    }
    fread(bmpHeader, sizeof(BMP_FILE_HEADER), 1, fp);
    fread(bmpInfo, sizeof(BMP_INFO), 1, fp);
    for (size_t i = 0; i < bmpInfo -> bClrUsed; i++){
        RGB_QUAD quad;
        fread(&quad, sizeof(RGB_QUAD), 1, fp);
        bmpQuad.push_back(quad);
    }
    size_t col = bmpInfo -> bWidth;
    size_t row = bmpInfo -> bHeight;
    for (size_t r = 0; r < row; r++) {
        vector<BYTE> row;
        for (size_t c = 0; c < col; c++) {
            BYTE pixel;
            fread(&pixel, sizeof(BYTE), 1, fp);
            row.push_back(pixel);
        }
        image.push_back(row);
    }
    
    fclose(fp);
    
    return true;
}

static bool
writeBMP(const string& filePath,
         const vector<vector<BYTE>>& image,
         const BMP_FILE_HEADER* bmpHeader,
         const BMP_INFO* bmpInfo,
         const vector<RGB_QUAD>& bmpQuad){
    /* Write Grey BMP Image into Storage
     *
     */
    
    const char* cPath = filePath.c_str();
    FILE *fp;
    if ((fp = fopen(cPath, "wb")) == NULL) {
        cerr << "Error (writeBMP): Cannot Write File: " << filePath << endl;
        return false;
    }
    fwrite(bmpHeader, sizeof(BMP_FILE_HEADER), 1, fp);
    fwrite(bmpInfo, sizeof(BMP_INFO), 1, fp);
    for (size_t i = 0; i < bmpQuad.size(); i++) {
        RGB_QUAD quad = bmpQuad[i];
        fwrite(&quad, sizeof(RGB_QUAD), 1, fp);
    }
    size_t col = bmpInfo -> bWidth;
    size_t row = bmpInfo -> bHeight;
    for (size_t r = 0; r < row; r++) {
        for (size_t c = 0; c < col; c++) {
            BYTE pixel = image[r][c];
            fwrite(&pixel, sizeof(BYTE), 1, fp);
        }
    }
    fclose(fp);
    return true;
}

// print BMP_FILE_HEADER
void
printBMP_FILE_HEADER(const BMP_FILE_HEADER *bmp_header){
    
    const string info = "BMP_HEADER";
    printf("\n%s Type:\t%X\n", info.c_str(), bmp_header -> bType);
    printf("%s Size:\t%u\n", info.c_str(), bmp_header -> bSize);
    printf("%s Reserved 1:\t%u\n", info.c_str(), bmp_header -> bReserved1);
    printf("%s Reserved 2:\t%u\n", info.c_str(), bmp_header -> bReserved2);
    printf("%s Offset:\t%u\n", info.c_str(), bmp_header -> bOffset);
}

// print BMP_INFO
void
printBMP_INFO(const BMP_INFO *bmp_info){
    
    const string info = "BMP_INFO";
    printf("\n%s Info Size:\t%u\n", info.c_str(), bmp_info -> bInfoSize);
    printf("%s Width:\t%u\n", info.c_str(), bmp_info -> bWidth);
    printf("%s Height:\t%u\n", info.c_str(), bmp_info -> bHeight);
    printf("%s Planes Per Pixel:\t%u\n", info.c_str(), bmp_info -> bPlanes);
    printf("%s Bit Per Pixel:\t%u\n", info.c_str(), bmp_info -> bBitCount);
    printf("%s Compression:\t%u\n", info.c_str(), bmp_info -> bCompression);
    printf("%s Image Size(Byte):\t%u\n", info.c_str(), bmp_info -> bmpImageSize);
    printf("%s X Pels Per Meter:\t%u\n", info.c_str(), bmp_info -> bXPelsPerMeter);
    printf("%s Y Pels Per Meter:\t%u\n", info.c_str(), bmp_info -> bYPelsPerMeter);
    printf("%s Color Used:\t%u\n", info.c_str(), bmp_info -> bClrUsed);
    printf("%s Color Important:\t%u\n", info.c_str(), bmp_info -> bClrImportant);
}

// Calculate PE Histogram
vector<int>
calPeHistogram(const vector<vector<BYTE>> &img){
    size_t bit = 5;
    int cnt_lvl = (1 << bit) + 1;
    vector<int> hist(cnt_lvl, 0);
    size_t row = img.size();
    size_t col = img[0].size();
    vector<int> B = {1, 2, 1, 2, 0, 2, 1, 2, 1};
    int C = accumulate(B.begin(), B.end(), 0);
    for (size_t r = 1; r < row - 1; r++) {
        for (size_t c= 1; c < col - 1; c++) {
            int curAvg = 0, num = 0;
            for (size_t i = r - 1; i <= r + 1; i++)
                for (size_t j = c - 1; j <= c + 1; j++)
                    curAvg += img[i][j] * B[num++];
            curAvg /= C;
            int curPe = img[r][c] - curAvg;
            if (curPe >=  -(cnt_lvl >> 1) &&
                curPe <= (cnt_lvl >> 1)) {
                hist[curPe + (cnt_lvl >> 1)]++;
            }
            
        }
    }
    return hist;
}

// Calculate Gery Image MSE (Mean Square Error)
double
calMSE(const vector<vector<BYTE>>& img1,
       const vector<vector<BYTE>>& img2){
    
    const string TAG = "calMSE";
    if (img1.size() != img2.size() || img1[0].size() != img2[0].size()){
        printf("Error (CalMSE): Images' Size Must Same!");
        exit(-1);
    }
    size_t row = img1.size();
    size_t col = img1[0].size();
    double myMse = 0.0;
    for (size_t r = 0; r < row; r++) {
        for (size_t c = 0; c < col; c++) {
            double p1 = img1[r][c];
            double p2 = img2[r][c];
            myMse += pow( p1 - p2, 2.0);
        }
    }
    myMse /= row * col;
    return myMse;
}

// Calculate Grey Image PSNR
double
calPSNR(const vector<vector<BYTE>>& img1,
        const vector<vector<BYTE>>& img2){
    
    double myPsnr = 0.0;
    myPsnr = 10 * log10(pow(255.0, 2.0) / calMSE(img1, img2));
    return myPsnr;
}

// Calculate Grey Image Entropy
double
calEntropy(const vector<vector<BYTE>>& img){
    
    double myEntropy = 0.0;
    vector<int> hist = calHistogram(img);
    double pixelNum = accumulate(hist.begin(), hist.end(), 0);
    for (size_t i = 0; i < hist.size(); i++) {
        
        double temp = hist[i] / pixelNum;
        temp = temp * log2(temp);
        if (isnan(temp)) {
            temp = 0;
        }
        myEntropy += temp;
    }
    return -1.0 * myEntropy;
}

// Calculate Grey Image PE Entropy
double
calPeEntropy(const vector<int>& hist){
    double myEntropy = 0.0;
    double pixelNum = accumulate(hist.begin(), hist.end(), 0);
    for (size_t i = 0; i < hist.size(); i++) {
        
        double temp = hist[i] / pixelNum;
        temp = temp * log2(temp);
        if (isnan(temp)) {
            temp = 0;
        }
        myEntropy += temp;
    }
    return -1.0 * myEntropy;
}

// Byte -> Double
vector<vector<double>>
byte2Double(const vector<vector<BYTE>>& img){
    
    size_t row = img.size();
    size_t col = img[0].size();
    vector<vector<double>> res;
    for (size_t i = 0; i < row; i++) {
        vector<double> temp;
        for (size_t j = 0; j < col; j++) {
            temp.push_back(img[i][j]);
        }
        res.push_back(temp);
    }
    return res;
}

// Double -> Byte
vector<vector<BYTE>>
double2Byte(const vector<vector<double>>& img){
    
    size_t row = img.size();
    size_t col = img[0].size();
    vector<vector<BYTE>> res;
    for (size_t i = 0; i < row; i++) {
        vector<BYTE> temp;
        for (size_t j = 0; j < col; j++) {
            double p = img[i][j];
            if (p > 0xff){
                continue;
                //                printf("Warning (Double2Byte): Could Cause Overflow!\n");
            }
            else if (p < 0){
                continue;
                //                printf("Warning (Double2Byte): Could Cause Underflow!\n");
            }
            temp.push_back(BYTE(p));
        }
        res.push_back(temp);
    }
    return res;
}

// Byte -> Int
vector<vector<int>>
byte2Int(const vector<vector<BYTE>>& img){
    size_t row = img.size();
    size_t col = img[0].size();
    vector<vector<int>> res;
    for (size_t i = 0; i < row; i++) {
        vector<int> temp;
        for (size_t j = 0; j < col; j++) {
            temp.push_back(img[i][j]);
        }
        res.push_back(temp);
    }
    return res;
}

// Int -> Byte
vector<vector<BYTE>>
int2Byte(const vector<vector<int>>& img){
    size_t row = img.size();
    size_t col = img[0].size();
    vector<vector<BYTE>> res;
    for (size_t i = 0; i < row; i++) {
        vector<BYTE> temp;
        for (size_t j = 0; j < col; j++) {
            int p = img[i][j];
            if (p > 0xff){
                continue;
                //                printf("Warning (int2Byte): Could Cause Overflow!\n");
                //                exit(-1);
            }
            else if (p < 0){
                continue;
                //                printf("Warning (int2Byte): Could Cause Underflow!\n");
                //               exit(-1);
            }
            temp.push_back(static_cast<BYTE>(p));
        }
        res.push_back(temp);
    }
    return res;
}

// Generate linearly spaced vector
vector<int>
linspace(const int start,
         const int end,
         const int N){
    vector<int> res(N);
    int step = (end - start + 1) / N;
    if (step < 0) {
        return res;
    }
    for (size_t i = 0; i < N; i++) {
        res[i] = start + int(i) * step;
    }
    return res;
}

// Read Grey Image API
vector<vector<BYTE>>
readBMP(const string& filePath){
    const string TAG = "readBMP";
    vector<vector<BYTE>> img;
    readBMP(filePath, img, &MY_FILE_HEADER, &MY_BMP_INFO, MY_QUAD);
    return img;
};

// Write Grey Image API
bool
writeBMP(const vector<vector<BYTE>>& image, const string& filePath){
    return writeBMP(filePath, image, &MY_FILE_HEADER, &MY_BMP_INFO, MY_QUAD);
}

// Calculate Block Standard Deviation
static double
calBlockStd(const vector<vector<BYTE>>& image,
                const pair<int, int> rowRange,
                const pair<int, int> colRange){
    vector<double> value;
    for (int r = rowRange.first; r <= rowRange.second; r++) {
        for (int c = colRange.first; c <= colRange.second; c++) {
            value.push_back((double)image[r][c]);
        }
    }
    double avg =
    accumulate(value.begin(), value.end(), 0) / (double)value.size() ;
    double res = 0;
    for (int i = 0; i < value.size(); i++) {
        res += pow((value[i] - avg), 2.0);
    }
    res /= value.size();
    return sqrt(res);
}

// Calculate Block Average
static double
calBlockAvg(const vector<vector<BYTE>>& image,
            const pair<int, int> rowRange,
            const pair<int, int> colRange){
    vector<double> value;
    for (int r = rowRange.first; r <= rowRange.second; r++) {
        for (int c = colRange.first; c <= colRange.second; c++) {
            value.push_back((double)image[r][c]);
        }
    }
    double avg =
    accumulate(value.begin(), value.end(), 0) / (double)value.size() ;
    return avg;
}

// Block Image
vector<int>
blockSequence(const vector<vector<BYTE>>& image,
              const block_threshold& thres){
    vector<int> res;
    int row = (int)image.size();
    int col = (int)image[0].size();
    int rstep = thres.blockSize;
    int cstep = thres.blockSize;
    int hs = 0, pe = 0;
    for(int r = 0; r < row; r += rstep){
        int rmax = r + rstep - 1;
        if (rmax >= row) {
            rmax = row - 1;
        }
        for (int c = 0; c < col; c += cstep) {
            int cmax = c + cstep - 1;
            if (cmax >= col) {
                cmax = col - 1;
            }
            double block_std =
            calBlockStd(image, make_pair(r, rmax), make_pair(c, cmax));
            if (block_std > thres.Tdev) {
                res.push_back(0);   // PE block
                pe++;
            }else{
                res.push_back(1);   // HS block
                hs++;
            }
        }
    }
    for (int i = 0; i < thres.bookkeeping.size(); i++) {
        res[thres.bookkeeping[i]] = !(res[thres.bookkeeping[i]]);
    }
    return res;
}



// Get Block Threshold
block_threshold
getBlockThreshold(const vector<vector<BYTE>>& image,
                  const vector<int>& seq,
                  block_threshold thres){
    int row = (int)image.size();
    int col = (int)image[0].size();
    int rstep = thres.blockSize;
    int cstep = thres.blockSize;
    int cnt = 0;
    vector<int> new_seq = blockSequence(image, thres);
    for (int i = 0; i < new_seq.size(); i++) {
        if (new_seq[i] != seq[i]) {
            thres.bookkeeping.push_back(i);
        }
    }
    return thres;
}