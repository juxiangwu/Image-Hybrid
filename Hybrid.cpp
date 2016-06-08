//
//  Hybrid.cpp
//  HybridCpp
//
//  Created by Wang on 16/5/29.
//  Copyright © 2016年 Wang. All rights reserved.
//

#include "Hybrid.hpp"

static vector<vector<BYTE>>
refer_image(const vector<vector<BYTE>>& image,
            const vector<int> seq,
            const int blockSize){
    
    const string TAG = "refer_image";
    int row = (int)image.size();
    int col = (int)image[0].size();
    int rstep = blockSize;
    int cstep = blockSize;
    int cnt = 0;
    vector<vector<BYTE>> res(row, vector<BYTE>(col, 127));
    for(int r = 0; r < row; r += rstep){
        for (int c = 0; c < col; c += cstep) {
            for (int i = r; i < row && i < r + rstep; i++) {
                for (int j = c; j < col && j < c + cstep; j++) {
                    if (seq[cnt] == 1) {
                        res[i][j] = 255;    // HS region
                    }
                    else if(seq[cnt] == 0){
                        res[i][j] = 0;      // PE region
                    }
                    else{
                        printf("Error %s: get I ref error. %d\n", TAG.c_str(), seq[cnt]);
                    }
                }
            }
            cnt++;
        }
    }
    return res;
}

static void
equal_vector(vector<int> vec1, vector<int> vec2){
    if (vec1.size() != vec2.size()) {
        printf("vector NOT EQUAL: vec1.size() = %lu, vec2.size() = %lu\n", vec1.size(), vec2.size());
    }
    bool res = true;
    for (int i = 0; i < vec1.size(); i++) {
        if (vec1[i] != vec2[i]) {
            printf("vec1[%d] = %d, vec2[%d] = %d\n", i, vec1[i], i, vec2[i]);
            res = false;
        }
    }
    printf("vector %s\n", res?"EQUAL":"NOT EQUAL");
}

static bool
hybrid_embed(const int bits,
             vector<vector<BYTE>> &IMG,
             const vector<BYTE> &Do,
             const vector<pair<int, int>> emb_val,
             block_threshold &b_thres,
             pe_threshold &pe_thres,
             hs_threshold &hs_thres){
    // Block
    vector<int> seq = blockSequence(IMG, b_thres);
    const vector<vector<BYTE>> Iref = refer_image(IMG, seq, b_thres.blockSize);
    int hs_bits = hs_img_embed_bits(calBlockHistogram(IMG, Iref), emb_val);
    hs_bits = (hs_bits > bits? bits: hs_bits);
    vector<BYTE> D_hs(Do.begin(), Do.begin() + hs_bits);
    vector<BYTE> D_pe(Do.begin() + hs_bits, Do.end());
    // HS Embed
    vector<vector<BYTE>> I_hs_o = IMG;
    hs_thres = hs_img_embed_main(IMG, Iref, D_hs, hs_bits, emb_val);
    // PE Embed
    vector<vector<BYTE>> I_pe_o = IMG;
    pe_thres.totalBit = (int)D_pe.size();
    if (pe_thres.totalBit != 0) {
        if (!pe_img_embed_main(IMG, Iref, D_pe, pe_thres))
        return false;
    }
    // New Block Threshold
    b_thres = getBlockThreshold(IMG, seq, b_thres);
    return true;
}

static bool
hybrid_recover(vector<vector<BYTE>> &IMG,
               vector<BYTE> &Dr,
               block_threshold b_thres,
               pe_threshold pe_thres,
               hs_threshold hs_thres,
               vector<int> seq,
               vector<vector<BYTE>> Iref){
    const string TAG = "hybrid_recover";
    // Recover Seq_r & Iref_r
    vector<int> seq_r = blockSequence(IMG, b_thres);
    vector<vector<BYTE>> Iref_r = refer_image(IMG, seq_r, b_thres.blockSize);
    // Debug
    equal_vector(seq, seq_r);
    printf("%s: PSNR[Iref, Iref_r] = %2.2f\n", TAG.c_str(), calPSNR(Iref, Iref_r));
    printf("%s: BIT PE = %d, HS = %d\n",
           TAG.c_str(), pe_thres.totalBit, hs_thres.totalBit);
    // PE Recover
    vector<BYTE> Dr_pe, Dr_hs;
    if (pe_thres.totalBit != 0) {
        pe_img_recover_main(IMG, Iref_r, Dr_pe, pe_thres);
    }
    vector<vector<BYTE>> I_pe_r = IMG;
    
    // HS Recover
    hs_img_recover_main(IMG, Iref_r, Dr_hs, hs_thres);
    
    vector<vector<BYTE>> I_hs_r = IMG;
    Dr.assign(Dr_hs.begin(), Dr_hs.end());
    Dr.insert(Dr.end(), Dr_pe.begin(), Dr_pe.end());
    return true;
}

double
hybrid_main(double bpp,
            vector<vector<BYTE>> IMG,
            block_threshold b_thres,
            pe_threshold pe_thres,
            vector<pair<int, int>> emb_val){
    const string TAG = "hybrid_main";
    // Read Image & Data
    int heigth = (int)IMG.size(), width = (int)IMG[0].size();
    int bits = bpp * heigth * width;
    printf("%s: Bits = %d\n", TAG.c_str(), bits);
    vector<BYTE> Do = readFile(DataPath, bits), Dr;
    vector<vector<BYTE>> Io(IMG);
    hs_threshold hs_thres;
    // Debug
    vector<int> seq = blockSequence(IMG, b_thres);
    vector<vector<BYTE>> Iref = refer_image(IMG, seq, b_thres.blockSize);
    // Embed
    hybrid_embed(bits, IMG, Do, emb_val, b_thres, pe_thres, hs_thres);
    // PSNR
    double psnr1 = calPSNR(IMG, Io);
    printf("PSNR[Io,Im] = %2.2f (dB)\n", psnr1);
    // Recover
    hybrid_recover(IMG, Dr, b_thres, pe_thres, hs_thres, seq, Iref);
    // Check Reversible
    bool check = true;
    double psnr2 = calPSNR(IMG, Io);
    printf("PSNR[Ir,Io] = %2.2f (dB), ", psnr2);
    if (psnr2 != INFINITY) {
        check = false;
    }
    if (equal(Do.begin(), Do.end(), Dr.begin())) {
        printf("Data: True\n");
    }else{
        printf("Data: False\n");
        check = false;
    }
    
    if (check) {
        printf("CORRECT\n");
        return psnr1;
    }else{
        printf("ERROR\n");
        return -1;
    }
}

void
hybrid_exp_main(){
    const int Idx = 2;                      // image index
    const double c_bpp = 0.01;
    const int c_step = 10;
    vector<vector<BYTE>> img = readBMP(SrcDir + GreyImg[Idx]);
    cout << GreyImg[Idx] << endl;
    block_threshold b_thres(1, 16);    // Block Threshold
    vector<pe_threshold> pe_thres;          // PE Threshold
    for (int i = 0; i < c_step; i++) {
        pe_thres.push_back(pe_threshold(0,  // pe embed bit
                                        -3,  // T
                                        linspace(240, 250)[i],// TF
                                        3,  // TL
                                        1   // TR
                                        ));
    }
    vector<pair<int, int>> emb_val;         // HS Threshold
    emb_val.push_back(make_pair(0, 2));
    int bestIdx = -1.0;
    double bestPSNR = -1;
    for (int i = 0; i < c_step; i++) {
        printf("No. %d\n", i);
        double psnr = hybrid_main(c_bpp, img, b_thres, pe_thres[i], emb_val);
        if (psnr > 0 && psnr > bestPSNR) {
            bestIdx = i;
            bestPSNR = psnr;
        }
        printf("\n");
    }
    printf("Best is %d. PSNR = %2.2f\n", bestIdx, bestPSNR);
    
}

void
hybrid_exp_chest(){
    const int Idx = 2;
    const double bpp[] = {0.01, 0.02, 0.03, 0.04, 0.05,
        0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7};
    block_threshold b_thres(1, 8);      // Block Threshold
    vector<pe_threshold> pe_thres(6);   // PE Threshold
    pe_thres.push_back(pe_threshold(0, 0, 2, 0, 0));
    pe_thres.push_back(pe_threshold(0, 0, 57, 0, 0));
    pe_thres.push_back(pe_threshold(0, 1, 245, 1, 0));
    pe_thres.push_back(pe_threshold(0, 2, 82, 2, 0));
    pe_thres.push_back(pe_threshold(0, 2, 730, 2, 1));
    pe_thres.push_back(pe_threshold(0, -3, 245, 3, 1));
    vector<pair<int, int>> emb_val;         // HS Threshold
    emb_val.push_back(make_pair(0, 2));
    for (int i = 0; i < pe_thres.size(); i++) {
        printf("Name: %s, Payload: %1.2f (bpp)\n", GreyImg[Idx].c_str(), bpp[i]);
        vector<vector<BYTE>> img = readBMP(SrcDir + GreyImg[Idx]);
        hybrid_main(bpp[i], img, b_thres, pe_thres[i], emb_val);
        printf("\n");
    }
    
}