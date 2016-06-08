//
//  HS.cpp
//  HybridCpp
//
//  Created by Wang on 16/5/29.
//  Copyright © 2016年 Wang. All rights reserved.
//

#include "HS.hpp"

// Histogram Shifting Method
bool
hs_img_main(vector<vector<BYTE>> img,
            vector<BYTE> data,
            int bitNum,
            const vector<pair<int, int>> emb_val){
    string TAG = "hs_img_main";
    vector<vector<BYTE>> Io = img; // Original Image
    vector<BYTE> Do = data;        // Original Embedded Data
    
    cout<< "--------Embed-----------"<<endl;
    const vector<vector<BYTE>> Iref(Io.size(), vector<BYTE>(Io[0].size(), 255));
    hs_threshold myThres = hs_img_embed_main(img, Iref, data, bitNum, emb_val);
    cout<< "--------End-----------"<<endl;
    // Embed Data Function
    vector<vector<BYTE>> Im = img;  // Marked Image
    printf("%s: PSNR(Io,Im) = %f\n", TAG.c_str(), calPSNR(Io, Im));
    vector<vector<BYTE>> Ir = Im;   // Copy Marked Image
    vector<BYTE> Dr;                // Recoverd Data
    cout<< "--------recover-----------"<<endl;
    hs_img_recover_main(Ir, Iref, Dr, myThres);   // Recover Data Process
    cout<< "--------End-----------"<<endl;
    printf("%s: PSNR(Io,Ir) = %f\n", TAG.c_str(), calPSNR(Io, Ir));
    cout <<"Data Recover:" <<boolalpha << equal(Dr.begin(), Dr.end(), Do.begin()) <<endl;
    return true;
}

// Embed Main
hs_threshold
hs_img_embed_main(vector<vector<BYTE>> &img,
                  const vector<vector<BYTE>> &ref,
                  const vector<BYTE> &data,
                  int bitNum,
                  const vector<pair<int, int>> emb_val){
    // Get bitNum bits data
    string TAG = "hs_img_embed_main";
    vector<BYTE> myData(data.begin(), data.begin() + bitNum);// Get Embed Data
    cout<< TAG <<": Bit="<<bitNum <<endl;
    hs_threshold myThres;   // Initial Threshold
    myThres.totalBit = bitNum;
    
    // Check can be embedded bitNum bits data
    if(bitNum > hs_img_embed_bits(calBlockHistogram(img, ref), emb_val)){
        printf("Error (hs_img_embed_main): Too Much Bits to be Embedded.\n");
        exit(-1);
    }
    vector<vector<BYTE>> I0 = img;
    int cnt = 2;
    for (int i = 0; i < emb_val.size(); i++) {
        if (bitNum <=
            accumulate(myThres.bitNum.begin(), myThres.bitNum.end(), 0)) {
            break;
        }
        vector<BYTE> myRecord;
        // Initial Record Data
        hs_img_embed_shift(img, ref, myThres, myRecord, emb_val[i]);
        // Histogram Shifting
        hs_img_embed_round(img, ref, myData, myRecord, myThres);
        // Embed Data
        cnt--;
        printf("%s:[Peak=%d,Zero=%d,BitNum=%d,Book=%d]\n", TAG.c_str(),
               myThres.peakPoint.back(),
               myThres.zeroPoint.back(),
               myThres.bitNum.back(),
               myThres.bookkeeping.back()
               );
    }
    return myThres;
}

// Get Embed Bits Number
int
hs_img_embed_bits(vector<int> hist,
                  const vector<pair<int, int>> emb_val){
    int emb_bits = 0;
    for (int i = 0; i < emb_val.size(); i++) {
        emb_bits += hist[emb_val[i].first];
        emb_bits -= hist[emb_val[i].second];
        if (emb_val[i].first > emb_val[i].second) {
            emb_bits -= hist[emb_val[i].second + 1];
        }
        else{
            emb_bits -= hist[emb_val[i].second - 1];
        }
    }
    return emb_bits;
}

// Embed Shift Histogram
bool
hs_img_embed_shift(vector<vector<BYTE>> &img,
                   const vector<vector<BYTE>> &ref,
                   hs_threshold &thrs,
                   vector<BYTE> &record,
                   const pair<int, int> emb_val){
    string TAG = "hs_img_embed_shift";
    size_t row = img.size();
    size_t col = img[0].size();
    int peak = emb_val.first;
    int zero = emb_val.second;
    int bookCnt = 0;
    vector<int> hist = calStdHistogram(img);
    
    bool bookFlag = (hist[zero] != 0? true: false);
    // Shifting
    for (size_t r = 0; r < row; r++) {
        for (size_t c = 0; c < col; c++) {
            if (ref[r][c] != 255) {
                continue;
            }
            BYTE &curPixel = img[r][c];
            if (peak > zero) {
                // Record bookkeeping data
                if (bookFlag) {
                    if (curPixel == (zero + 1)) {
                        record.push_back(0);
                        bookCnt++;
                    }
                    else if (curPixel == zero){
                        record.push_back(1);
                        bookCnt++;
                    }
                }
                // Shift histogram
                if (curPixel > zero && curPixel < peak){
                    curPixel--;
                }
            }
            else if (peak < zero){
                // Record bookkeeping data
                if (bookFlag) {
                    if (curPixel == (zero - 1)) {
                        record.push_back(0);
                        bookCnt++;
                    }
                    else if (curPixel == zero){
                        record.push_back(1);
                        bookCnt++;
                    }
                }
                // Shift histogram
                if (curPixel < zero && curPixel > peak) {
                    curPixel++;
                }
            }
        }
    }
    if (bookCnt != record.size()) {
        printf("Error (%s): Bookkeeping Number mismatch.", TAG.c_str());
        exit(-1);
    }
    thrs.peakPoint.push_back(peak);
    thrs.zeroPoint.push_back(zero);
    thrs.bookkeeping.push_back(bookCnt);
    return true;
}

// Embed One Round
bool
hs_img_embed_round(vector<vector<BYTE>> &img,
                   const vector<vector<BYTE>> &ref,
                   const vector<BYTE> &data,
                   const vector<BYTE> &record,
                   hs_threshold &thrs){
    size_t row = img.size();
    size_t col = img[0].size();
    string TAG = "hs_img_embed_round";
    if ((thrs.peakPoint.size() != thrs.zeroPoint.size()) ||
        (thrs.peakPoint.size() != thrs.bookkeeping.size())) {
        printf("Error (%s): hs_threshold mismatch.", TAG.c_str());
        exit(-1);
    }
    int peak = thrs.peakPoint.back();
    int zero = thrs.zeroPoint.back();
    int dataCnt = accumulate(thrs.bitNum.begin(), thrs.bitNum.end(), 0);
    int dataBase = dataCnt;
    int recordCnt = 0;
    for (size_t r = 0; r < row; r++) {
        for (size_t c = 0; c < col; c++) {
            if (ref[r][c] != 255) {
                continue;
            }
            BYTE &curPixel = img[r][c];
            if (curPixel == peak) {
                if (recordCnt < record.size()) {
                    // Embed record data
                    BYTE bit = record[recordCnt++];
                    if (zero < peak)
                        curPixel -= bit;
                    else if(zero > peak)
                        curPixel += bit;
                    else{
                        printf("Error (%s): Peak Equals to Zero. %d,%d\n",
                               TAG.c_str(), peak, zero);
                    }
                }
                else if(dataCnt < thrs.totalBit) {
                    // Embed random data
                    BYTE bit = data[dataCnt++];
                    if (zero < peak)
                        curPixel -= bit;
                    else if(zero > peak)
                        curPixel += bit;
                    else{
                        printf("Error (%s): Peak Equals to Zero. %d,%d\n",
                               TAG.c_str(), peak, zero);
                    }
                }
                else{
                    thrs.bitNum.push_back(dataCnt - dataBase);
                    return true;
                }
            }
        }
    }
    thrs.bitNum.push_back(dataCnt - dataBase);
    return true;
}

// Recover Main
bool
hs_img_recover_main(vector<vector<BYTE>> &img,
                    const vector<vector<BYTE>> &ref,
                    vector<BYTE> &data_re,
                    hs_threshold thres){
    const string TAG = "hs_img_recover_main";
    
    while (data_re.size() < thres.totalBit) {
        printf("%s:[data_re.size = %d, thres.totalBit = %d]\n ",
               TAG.c_str(),
               data_re.size(),
               thres.totalBit);
        vector<BYTE> record;    // recover record in current round
        vector<BYTE> mydata_re; // recover data in current round
        hs_img_recover_round(img, ref, mydata_re, record, thres);
        hs_img_recover_shift(img, ref, thres, record);
        data_re.insert(data_re.begin(),
                       mydata_re.begin(), mydata_re.end());
        thres.peakPoint.pop_back();
        thres.zeroPoint.pop_back();
        thres.bitNum.pop_back();
        thres.bookkeeping.pop_back();
        printf("%s:[TotalBit=%d,Record=%lu,MydataRe=%lu]\n",
               TAG.c_str(),
               thres.totalBit,
               record.size(),
               mydata_re.size());
    }
    
    return true;
}

// Recover One Round
bool
hs_img_recover_round(vector<vector<BYTE>> &img,
                     const vector<vector<BYTE>> &ref,
                     vector<BYTE> &data,
                     vector<BYTE> &record,
                     hs_threshold &thrs){
    size_t row = img.size();
    size_t col = img[0].size();
    string TAG = "hs_img_recover_round";
    if ((thrs.peakPoint.size() != thrs.zeroPoint.size()) ||
        (thrs.peakPoint.size() != thrs.bookkeeping.size())) {
        printf("Error (%s): hs_threshold mismatch.", TAG.c_str());
        exit(-1);
    }
    int peak = thrs.peakPoint.back();
    int zero = thrs.zeroPoint.back();
    int recordCnt = thrs.bookkeeping.back();
    int dataCnt = thrs.bitNum.back();
    for (size_t r = 0; r < row; r++) {
        for (size_t c = 0; c < col; c++) {
            if (ref[r][c] != 255) {
                continue;
            }
            BYTE &curPixel = img[r][c];
            int bitDir = ((zero < peak)? -1: 1);
            int curBit = -1;
            if (curPixel == peak)
                curBit = 0;
            else if ((zero < peak) && (curPixel == (peak - 1))){
                curBit = 1;
                curPixel++;
            }
            else if ((zero > peak) && (curPixel == (peak + 1))){
                curBit = 1;
                curPixel--;
            }
            if (curBit >= 0) {
                if (recordCnt > 0){
                    record.push_back(curBit);
                    recordCnt--;
                }
                else if (dataCnt > 0){
                    data.push_back(curBit);
                    dataCnt--;
                }
                else{
                    return true;
                }
            }
        }
    }
    return true;
}

// Recover Histogram
bool
hs_img_recover_shift(vector<vector<BYTE>> &img,
                     const vector<vector<BYTE>> &ref,
                     hs_threshold &thrs,
                     vector<BYTE> &record){
    string TAG = "hs_img_recover_shift";
    size_t row = img.size();
    size_t col = img[0].size();
    
    int peak = thrs.peakPoint.back();
    int zero = thrs.zeroPoint.back();
    int recordCnt = thrs.bookkeeping.back();
    bool recordFlag = (recordCnt != 0)? true:false;
    int shiftCnt = 0;
    if (record.size() != recordCnt) {
        printf("Error (%s): Bookkeeping Number mismatch. %lu,%d\n",
               TAG.c_str(),
               record.size(),
               recordCnt);
        exit(-1);
    }
    
    // Shifting
    for (size_t r = 0; r < row; r++) {
        for (size_t c = 0; c < col; c++) {
            if (ref[r][c] != 255) {
                continue;
            }
            BYTE &curPixel = img[r][c];
            if (peak > zero) {
                if (curPixel >= zero && curPixel < peak){
                    curPixel++;
                }
                if (recordFlag && curPixel == (zero + 1)) {
                    curPixel -= record[shiftCnt++];
                }
            }
            else if (peak < zero){
                if (curPixel <= zero && curPixel > peak) {
                    curPixel--;
                }
                if (recordFlag && curPixel == (zero - 1)) {
                    curPixel += record[shiftCnt++];
                }
            }
        }
    }
    if (shiftCnt != recordCnt) {
        printf("Error (%s): Record Not Empty.", TAG.c_str());
        exit(-1);
    }
    return true;
}