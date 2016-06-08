//
//  PE.hpp
//  HybridCpp
//
//  Created by Wang on 16/5/29.
//  Copyright © 2016年 Wang. All rights reserved.
//

#ifndef PE_hpp
#define PE_hpp

#include <stdio.h>
#include "Util.hpp"

using namespace std;
static bool logFlag = false;

// Overload << Operator
ostream&
operator<<(ostream& out,
           const pe_val_bit &obj);

// Embed Data into Image
bool
pe_img_embed_main(vector<vector<BYTE>> &img,
                  const vector<vector<BYTE>>& ref,
                  const vector<BYTE> &data_emb,
                  pe_threshold &thres);

// Extract Data from Grey Image
bool
pe_img_recover_main(vector<vector<BYTE>> &img,
                    const vector<vector<BYTE>>& ref,
                    vector<BYTE> &data_re,
                    pe_threshold thres);

// Embed Shrink Histogram
bool
pe_img_embed_shift(vector<vector<int>> &img,
                   const vector<vector<BYTE>>& ref,
                   pe_threshold & thrs,
                   vector<BYTE> &record);

// Embed Choose Shrink Value
bool
pe_img_embed_choosepoint(const vector<vector<int>> &img,
                         const vector<vector<BYTE>> &ref,
                         pe_threshold &thres);

// One Round Embed
bool
pe_img_embed_round(vector<vector<int>> &img,
                   const vector<vector<BYTE>>& ref,
                   const vector<BYTE> &data,
                   const vector<BYTE> &record,
                   pe_threshold &thrs);

// Embed Mix Embed
bool
pe_img_embed_mixEmb(vector<vector<int>> &img,
                    const vector<vector<BYTE>>& ref,
                    pe_threshold &thres,
                    vector<BYTE> &data);

// Recover One Round
bool
pe_img_recover_round(vector<vector<int>> &img,
                     const vector<vector<BYTE>>& ref,
                     vector<BYTE> &data,
                     pe_threshold &thrs);

// Recover
bool
pe_img_recover_mixRec(vector<vector<int>> &img,
                      const vector<vector<BYTE>>& ref,
                      vector<BYTE> &data,
                      const pe_threshold &thrs);

// Recover Shift Histogram
bool
pe_img_recover_shift(vector<vector<int>> &img,
                     const vector<vector<BYTE>>& ref,
                     pe_threshold &thrs,
                     vector<BYTE> &record);


#endif /* PE_hpp */
