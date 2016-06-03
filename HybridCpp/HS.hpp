//
//  HS.hpp
//  HybridCpp
//
//  Created by Wang on 16/5/29.
//  Copyright © 2016年 Wang. All rights reserved.
//

#ifndef HS_hpp
#define HS_hpp

#include <stdio.h>
#include "Util.hpp"


using namespace std;

// Histogram Shifting Method
bool
hs_img_main(vector<vector<BYTE>> img,
            vector<BYTE> data,
            int bitNum,
            const vector<pair<int, int>> emb_val);

// Embed Data into Grey Image
hs_threshold
hs_img_embed_main(vector<vector<BYTE>> &img,
                  const vector<vector<BYTE>> &ref,
                  const vector<BYTE> &data_emb,
                  int bitNum,
                  const vector<pair<int, int>> emb_val);

// Extract Data from Grey Image
bool
hs_img_recover_main(vector<vector<BYTE>> &img,
                    const vector<vector<BYTE>> &ref,
                    vector<BYTE> &data_re,
                    hs_threshold thres);

// Is Enough Embed
int
hs_img_embed_bits(vector<int> hist,
                  const vector<pair<int, int>> emb_val);

// Embed Shift Histogram
bool
hs_img_embed_shift(vector<vector<BYTE>> &img,
                   const vector<vector<BYTE>> &ref,
                   hs_threshold & thrs,
                   vector<BYTE> &record,
                   const pair<int, int> emb_val);

// Recover Histogram
bool
hs_img_recover_shift(vector<vector<BYTE>> &img,
                     const vector<vector<BYTE>> &ref,
                     hs_threshold &thrs,
                     vector<BYTE> &record);

// One Round Embed
bool
hs_img_embed_round(vector<vector<BYTE>> &img,
                   const vector<vector<BYTE>> &ref,
                   const vector<BYTE> &data,
                   const vector<BYTE> &record,
                   hs_threshold &thrs);

// One Round Extract
bool
hs_img_recover_round(vector<vector<BYTE>> &img,
                     const vector<vector<BYTE>> &ref,
                     vector<BYTE> &data,
                     vector<BYTE> &record,
                     hs_threshold &thrs);

#endif /* HS_hpp */
