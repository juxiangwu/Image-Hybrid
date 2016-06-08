//
//  PE.cpp
//  HybridCpp
//
//  Created by Wang on 16/5/29.
//  Copyright © 2016年 Wang. All rights reserved.
//

#include "PE.hpp"
// Overload << Operator
ostream& operator<<(ostream& out, const pe_val_bit &obj){
    out << obj.val << "|" << obj.bit;
    return out;
}

// Embed Main
bool
pe_img_embed_main(vector<vector<BYTE>> &img,
                  const vector<vector<BYTE>>& ref,
                  const vector<BYTE> &data_emb,
                  pe_threshold &thres){
    string TAG = "pe_img_embed_main";
    if (!thres.isReadyEmbed()){
        printf("Error (%s): pe_threshold lack of T,TF,TR,TL\n",
               TAG.c_str());
        exit(-1);
    }
    vector<vector<int>> I = byte2Int(img);
    vector<BYTE> data_re;   // Record Bookkeeping Data
    pe_img_embed_choosepoint(I, ref, thres); // Choose shrink value
    pe_img_embed_shift(I, ref, thres, data_re);    // Shrink Histogram
    
    if (!pe_img_embed_round(I, ref, data_emb, data_re, thres)){
        printf("Error (%s): Cannot Embed All Data.\n", TAG.c_str());
        return false;
    }
    thres.print(TAG);
    printf("%s: hist(min)=%d,hist(max)=%d, \n\t len(Record)=%lu, len(Data)=%lu\n",
           TAG.c_str(),
           minValue(I), maxValue(I),
           data_re.size(), data_emb.size());
    if (maxValue(I) > 255 || minValue(I) < 0) {
        printf("Error (%s): Underflow or Overflow!!!\n", TAG.c_str());
        return false;
    }
    img = int2Byte(I);
    return true;
}

// Embed Shrink Histogram
bool
pe_img_embed_shift(vector<vector<int>> &img,
                   const vector<vector<BYTE>>& ref,
                   pe_threshold & thres,
                   vector<BYTE> &record){
    string TAG = "pe_img_embed_shift";
    size_t row = img.size();
    size_t col = img[0].size();
    // Left Shift
    for (auto iter: thres.bookkeeping_left) {
        int val = iter.val;
        for (size_t r = 1; r < row - 1; r++) {
            for (size_t c = 1; c < col - 1; c++) {
                if (ref[r][c] != 0) {
                    continue;
                }
                int &curPixel = img[r][c];
                if (curPixel == (val + 1)) {
                    record.push_back(0x0);
                }
                else if (curPixel == val) {
                    curPixel++;
                    record.push_back(0x1);
                }
                else if (curPixel < val) {
                    curPixel++;
                }
            }
        }
    }
    // Right Shift
    for (auto iter: thres.bookkeeping_right) {
        int val = iter.val;
        for (size_t r = 1; r < row - 1; r++) {
            for (size_t c = 1; c < col - 1; c++) {
                if (ref[r][c] != 0) {
                    continue;
                }
                int &curPixel = img[r][c];
                if (curPixel == (val - 1)) {
                    record.push_back(0x0);
                }
                else if (curPixel == val) {
                    curPixel--;
                    record.push_back(0x1);
                }
                else if (curPixel > val) {
                    curPixel--;
                }
            }
        }
    }
    return true;
}


// Embed Choose Shrink Grey Value
bool
pe_img_embed_choosepoint(const vector<vector<int>> &img,
                         const vector<vector<BYTE>> &ref,
                         pe_threshold &thres){
    string TAG = "pe_img_embed_choosepoint";
    int tl = thres.TL;
    int tr = thres.TR;
    vector<int> hist = calHistogram(img, ref);
    int valmin = static_cast<int>(find_if(hist.begin(), hist.end(),
                                          [](int i){return i != 0;}) - hist.begin());
    int valmax = static_cast<int>(
                                  hist.rend() - find_if(hist.rbegin(), hist.rend(), [](int i){return i != 0;}) - 1); // Generic Algorithm & Lambda Expression
    // Log
    if (logFlag)
        printf("%s : min=%d, max=%d\n", TAG.c_str(), valmin, valmax);
    int k1 = 5, k2 = 1; // Weight Coefficient
    
    // Left Histogram Shrink
    for (int i = 0; i < tl; i++) {
        vector<pe_val_bit> cost;
        for (int j = valmin; j < valmax; j++) {
            int valc = k1 * (hist[j] + hist[j + 1]) +
            k2 * accumulate(hist.begin(), hist.begin() + j + 1, 0);
            cost.push_back(pe_val_bit(j, valc));
        }
        // Choose Minimal Value
        vector<pe_val_bit>::iterator iter
        = min_element(cost.begin(), cost.end());
        int v = iter->val;
        // Record Choose Value in Bookkeeping
        thres.bookkeeping_left.push_back(
                                         pe_val_bit(iter->val, hist[iter->val] + hist[iter->val + 1]));
        // Histogram Shift Simulate
        for (int j = v + 1; j > valmin; j--) {
            hist[j] += hist[j - 1];
            hist[j - 1] = 0;
        }
        valmin++;
    }
    
    // Right Histogram Shrink
    for (int i = 0; i < tr; i++) {
        vector<pe_val_bit> cost;
        for (int j = valmax; j > valmin; j--) {
            int valc = k1 * (hist[j] + hist[j - 1]) +
            k2 * accumulate(hist.begin() + j, hist.end(), 0);
            cost.push_back(pe_val_bit(j, valc));
        }
        // Choose Minimal Value
        vector<pe_val_bit>::iterator iter
        = min_element(cost.begin(), cost.end());
        int v = iter->val;
        // Record Choose Value in Bookkeeping
        thres.bookkeeping_right.push_back(
                                          pe_val_bit(iter->val, hist[iter->val] + hist[iter->val - 1]));
        // Histogram Shift Simulate
        for (int j = v - 1; j < valmax; j++) {
            hist[j] += hist[j + 1];
            hist[j + 1] = 0;
        }
        valmax--;
    }
    return true;
}

// Embed One Round
bool
pe_img_embed_round(vector<vector<int>> &img,
                   const vector<vector<BYTE>>& ref,
                   const vector<BYTE> &data,
                   const vector<BYTE> &record,
                   pe_threshold &thrs){
    string TAG = "pe_img_embed_round";
    vector<BYTE> alldata; // alldata = data + record
    alldata.insert(alldata.end(), data.begin(), data.end());
    alldata.insert(alldata.end(), record.begin(), record.end());
    for (int curCircle = 0; curCircle < 2; curCircle++) {
        int tp, tn;
        if (thrs.T >= 0) {
            tp = thrs.T;
            tn = -1 * tp;
        }
        else{
            tn = thrs.T;
            tp = -tn-1;
        }
        for (; tp >= 0; tp--, tn++) {
            // Log
            if (logFlag) {
                printf("%s: tn=%d, tp=%d, len=%lu, S=%d, P=%d, min=%d, max=%d\n",
                       TAG.c_str(), tn, tp,
                       alldata.size(), thrs.S, thrs.P,
                       minValue(img), maxValue(img));
            }
            
            pe_threshold thrs_temp(
                                   thrs.totalBit, thrs.T, thrs.TF, tn, tp);
            bool endFlag = pe_img_embed_mixEmb(img, ref, thrs_temp, alldata);
            
            if (endFlag) {
                // Transmit [S, P, circle]
                thrs.S = thrs_temp.S;
                thrs.P = thrs_temp.P;
                thrs.circle = curCircle;
                return true;
            }
        }
    }
    // Not embed all data
    return false;
}

// Embed Mix Embed
bool
pe_img_embed_mixEmb(vector<vector<int>> &img,
                    const vector<vector<BYTE>>& ref,
                    pe_threshold &thres,
                    vector<BYTE> &data){
    string TAG = "pe_img_embed_mixEmb";
    thres.P = 1;
    int row = int(img.size());
    int col = int(img[0].size());
    int tf = thres.TF;
    int tn = thres.TL;
    int tp = thres.TR;
    int pos = 0;
    vector<int> B = {1, 2, 1, 2, 0, 2, 1, 2, 1};
    int C = accumulate(B.begin(), B.end(), 0);
    for (int r = 1; r < row - 1; r++) {
        for (int c = 1; c < col - 1; c++) {
            pos++;
            if (ref[r][c] != 0) {
                continue;
            }
            int &curPixel = img[r][c];
            int curFluc = 0;
            int curAvg = 0;
            int num = 0;
            for (int i = r - 1; i <= r + 1; i++)
                for (int j = c - 1; j <= c + 1; j++)
                    curAvg += img[i][j] * B[num++];
            curAvg /= C;
            int curPe = curPixel - curAvg;
            num = 0;
            for (int i = r - 1; i <= r + 1; i++)
                for (int j = c - 1; j <= c + 1; j++)
                    curFluc += pow(double(img[i][j] - curAvg), 2.0) * B[num++];
            curFluc /= 3;
            if (curFluc < tf) {
                if ((tn < 0) && (curPe == tn)) {
                    curPe -= data.back();
                    data.pop_back();
                    if (data.empty()) {
                        curPixel = curPe + curAvg;
                        thres.S = tn;
                        thres.P = pos;
                        return true;
                    }
                }
                else if ((tn < 0) && (curPe < tn)){
                    curPe--;
                }
                else if ((tp >= 0) && (curPe == tp)){
                    curPe += data.back();
                    data.pop_back();
                    if (data.empty()) {
                        curPixel = curPe + curAvg;
                        thres.S = tp;
                        thres.P = pos;
                        return true;
                    }
                }
                else if ((tp >= 0) && (curPe > tp)){
                    curPe++;
                }
            } // curFluc < Tf
            curPixel = curPe + curAvg;
        }// for loop col
    }// for loop row
    thres.S = -4000;
    return false;
}

// Recover Main
bool
pe_img_recover_main(vector<vector<BYTE>> &img,
                    const vector<vector<BYTE>>& ref,
                    vector<BYTE> &data_re,
                    pe_threshold thres){
    
    string TAG = "pe_img_recover_main";
    int dlen = thres.totalBit;
    vector<vector<int>> I = byte2Int(img);
    vector<BYTE> alldata;
    // Recover Data
    pe_img_recover_round(I, ref, alldata, thres);
    // Assign Data
    if (alldata.size() < dlen) {
        return false;
    }
    data_re.assign(alldata.begin(), alldata.begin() + dlen);
    // Assign Record
    vector<BYTE> record(alldata.begin() + dlen, alldata.end());
    // Recover Shift
    pe_img_recover_shift(I, ref, thres, record);
    img = int2Byte(I);
    return true;
}

// Recover One Round
bool
pe_img_recover_round(vector<vector<int>> &img,
                     const vector<vector<BYTE>>& ref,
                     vector<BYTE> &data,
                     pe_threshold &thrs){
    string TAG = "pe_img_recover_round";
    int t = thrs.T;
    int tf = thrs.TF;
    int tr = thrs.TR;
    int tl = thrs.TL;
    int stopT = thrs.S;
    int endpos = thrs.P;
    int startTp;
    int startTn;
    if (t >= 0) {
        startTp = t;
        startTn = -1 * t;
    }
    else{
        startTn = t;
        startTp = -1 - t;
    }
    int curT;
    int circle = thrs.circle;
    for (; circle >= 0; circle--) {
        int tp, tn;
        if (stopT != -777) {
            if (t >= 0) {
                tp = abs(stopT);
                tn = -1 * tp;
            }
            else{
                if (stopT >= 0) {
                    tp = stopT;
                    tn = -1 - tp;
                }
                else{
                    tn = stopT;
                    tp = -1 - tn;
                }
            }
            stopT = -777;
        }
        else if (stopT == -777){
            if (t >= 0) {
                tp = 0;
                tn = 0;
            }
            else{
                tp = 0;
                tn = -1;
            }
        }
        for (; tp <= startTp && tn >= startTn; tp++, tn--) {
            if (logFlag) {
                printf("%s: tf=%d, tn=%d, tp=%d, stratTp=%d,startTn=%d, endpos=%d, ",
                       TAG.c_str(), tf, tp, tn, startTp, startTn, endpos);
            }
            // my_thres to transmit tp and tn
            pe_threshold my_thres(thrs.totalBit, t, tf, tn, tp, -888, endpos);
            pe_img_recover_mixRec(img, ref, data, my_thres);
            if (logFlag) {
                printf("len(data)=%lu\n", data.size());
            }
            
            endpos = -777;
        }
    }
    
    
    return true;
}

// Recover
bool
pe_img_recover_mixRec(vector<vector<int>> &img,
                      const vector<vector<BYTE>>& ref,
                      vector<BYTE> &data,
                      const pe_threshold &thrs){
    string TAG = "pe_img_recover_mixRec";
    int tp = thrs.TR;
    int tn = thrs.TL;
    int tf = thrs.TF;
    int row = static_cast<int>(img.size());
    int col = static_cast<int>(img[0].size());
    int endpos = thrs.P;
    if (endpos > 0) {
        endpos = (row - 2) * (col - 2) - endpos;
    }
    else {
        endpos = 0;
    }
    vector<int> B = {1, 2, 1, 2, 0, 2, 1, 2, 1};
    double C = accumulate(B.begin(), B.end(), 0);
    for (int r = row - 2; r > 0; r--) {
        for (int c = col - 2; c > 0; c--) {
            if (endpos > 0) {
                endpos--;
                continue;
            }
            if (ref[r][c] != 0) {
                continue;
            }
            int &curPixel = img[r][c];
            int curFluc = 0;
            int curAvg = 0;
            int num = 0;
            for (int i = r - 1; i <= r + 1; i++)
                for (int j = c - 1; j <= c + 1; j++)
                    curAvg += img[i][j] * B[num++];
            curAvg /= C; // x bar
            int curPe = curPixel - curAvg; // Prediction Error
            num = 0;
            for (int i = r - 1; i <= r + 1; i++)
                for (int j = c - 1; j <= c + 1; j++)
                    curFluc += pow(double(img[i][j] - curAvg), 2.0) * B[num++];
            curFluc /= 3; // Fluctuation
            
            if (curFluc < tf) {
                if(tp >= 0 && curPe == tp){
                    data.push_back(0);
                }
                else if(tp >= 0 && curPe == (tp + 1)){
                    data.push_back(1);
                    curPixel--;
                }
                else if(tp >= 0 && curPe > (tp + 1)){
                    curPixel--;
                }
                else if(tn < 0 && curPe == tn){
                    data.push_back(0);
                }
                else if(tn < 0 && curPe == (tn - 1)){
                    data.push_back(1);
                    curPixel++;
                }
                else if(tn < 0 && curPe < (tn - 1)){
                    curPixel++;
                }
            }
        }
    }
    return true;
}


// Recover Shift Histogram
bool
pe_img_recover_shift(vector<vector<int>> &img,
                     const vector<vector<BYTE>>& ref,
                     pe_threshold &thrs,
                     vector<BYTE> &record){
    string TAG = "pe_img_recover_shift";
    vector<pe_val_bit> bkp_l(thrs.bookkeeping_left);
    vector<pe_val_bit> bkp_r(thrs.bookkeeping_right);
    int row = (int)img.size();
    int col = (int)img[0].size();
    // Recover Shift Right
    while (!bkp_r.empty()) {
        int val = bkp_r.back().val;
        bkp_r.pop_back();
        int cnt = 0;
        for (int r = row - 2; r >= 1; r--) {
            for (int c = col - 2; c >= 1; c--) {
                if (ref[r][c] != 0) {
                    continue;
                }
                int &curPixel = img[r][c];
                if (curPixel == (val - 1)) {
                    cnt++;
                    if (record.empty()) {
                        printf("Error (%s): Record Mismatch val=%d,[%d,%d].\n",
                               TAG.c_str(), val, r, c);
                        exit(-1);
                    }
                    if (record.back() == 0x1){
                        curPixel += 1;
                    }
                    record.pop_back();
                }
                else if(curPixel >= val){
                    curPixel += 1;
                }
            }
        }
    }
    
    // Recover Shift Left
    while (!bkp_l.empty()) {
        int val = bkp_l.back().val;
        bkp_l.pop_back();
        int cnt = 0;
        for (int r = row - 2; r >= 1; r--) {
            for (int c = col - 2; c >= 1; c--) {
                if (ref[r][c] != 0) {
                    continue;
                }
                int &curPixel = img[r][c];
                if (curPixel == (val + 1)) {
                    cnt++;
                    if (record.empty()) {
                        printf("Error (%s): Record Mismatch val=%d,[%d,%d].\n",
                               TAG.c_str(), val, r, c);
                        exit(-1);
                    }
                    if (record.back() == 0x1){
                        curPixel--;
                    }
                    record.pop_back();
                }
                else if (curPixel <= val){
                    curPixel--;
                }
            }
        }
    }
    return true;
}