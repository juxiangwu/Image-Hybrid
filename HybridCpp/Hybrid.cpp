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
            const block_threshold thres){
    
    int row = (int)image.size();
    int col = (int)image[0].size();
//    int rstep = ceil(row * 1.0 / thres.blockNum);
//    int cstep = ceil(col * 1.0 / thres.blockNum);
    int rstep = thres.blockNum;
    int cstep = thres.blockNum;
    int cnt = 0;
    vector<vector<BYTE>> res(row, vector<BYTE>(col, 127));
    for(int r = 0; r < row; r += rstep){
        for (int c = 0; c < col; c += cstep) {
            cnt++;
            for (int i = r; i < row && i < r + rstep; i++) {
                for (int j = c; j < col && j < c + cstep; j++) {
                    if (seq[cnt] == 1) {
                        res[i][j] = 255;
                    }else{
                        res[i][j] = 0;
                    }
                }
            }
        }
    }
    return res;
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
    vector<BYTE> Do = readFile(DataPath, bits);
    vector<vector<BYTE>> Io(IMG);
    // Block
    vector<int> seq = blockSequence(IMG, b_thres);
    cout << "Seq Size = " << seq.size() << endl;
    vector<vector<BYTE>> Iref = refer_image(IMG, seq, b_thres);
    writeBMP(Iref, SrcDir + "block.bmp");
    int hs_bits = hs_img_embed_bits(calBlockHistogram(IMG, Iref), emb_val);
    hs_bits = (hs_bits > bits? bits: hs_bits);
    vector<BYTE> D_hs(Do.begin(), Do.begin() + hs_bits);
    vector<BYTE> D_pe(Do.begin() + hs_bits, Do.end());
    // HS Embed
    vector<vector<BYTE>> I_hs_o = IMG;
    hs_threshold hs_thres =
    hs_img_embed_main(IMG, Iref, D_hs, hs_bits, emb_val);
    // PE Embed
    vector<vector<BYTE>> I_pe_o = IMG;
    pe_thres.totalBit = (int)D_pe.size();
    
    if (pe_thres.totalBit != 0) {
        if (!pe_img_embed_main(IMG, Iref, D_pe, pe_thres))
            return false;
    }
    // PSNR
    double psnr1 = calPSNR(IMG, Io);
    printf("PSNR[Io,Im] = %2.2f (dB)\n", psnr1);
    
    // PE Recover
    vector<BYTE> Dr_pe, Dr_hs, Dr;
    if (pe_thres.totalBit != 0) {
        pe_img_recover_main(IMG, Iref, Dr_pe, pe_thres);
    }
    vector<vector<BYTE>> I_pe_r = IMG;
    
    // HS Recover
    hs_img_recover_main(IMG, Iref, Dr_hs, hs_thres);
    
    vector<vector<BYTE>> I_hs_r = IMG;
    Dr.assign(Dr_hs.begin(), Dr_hs.end());
    Dr.insert(Dr.end(), Dr_pe.begin(), Dr_pe.end());
    
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
    const double c_bpp = 0.;
    const int c_step = 10;
    vector<vector<BYTE>> img = readBMP(SrcDir + GreyImg[Idx]);
    cout << GreyImg[Idx] << endl;
    block_threshold b_thres(1, 0, 0, 8);    // Block Threshold
    vector<pe_threshold> pe_thres;          // PE Threshold
    for (int i = 0; i < c_step; i++) {
        pe_thres.push_back(pe_threshold(0,  // pe embed bit
                                        0,  // T
                                        linspace(1, 11)[i],// TF
                                        0,  // TL
                                        0   // TR
                                        ));
    }
    vector<pair<int, int>> emb_val;         // HS Threshold
    emb_val.push_back(make_pair(0, 2));
    int bestIdx = -1.0;
    double bestPSNR = -1;
    for (int i = 0; i < c_step; i++) {
        double psnr = hybrid_main(c_bpp, img, b_thres, pe_thres[i], emb_val);
        if (psnr > 0 && psnr > bestPSNR) {
            bestIdx = i;
            bestPSNR = psnr;
        }
        printf("\n");
    }
    printf("Best is %d. PSNR = %2.2f\n", bestIdx, bestPSNR);
    
}