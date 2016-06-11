//
//  Hybrid.hpp
//  HybridCpp
//
//  Created by Wang on 16/5/29.
//  Copyright © 2016年 Wang. All rights reserved.
//

#ifndef Hybrid_hpp
#define Hybrid_hpp

#include <stdio.h>
#include "Util.hpp"
#include "HS.hpp"
#include "PE.hpp"

double
hybrid_main(double bpp,
            vector<vector<BYTE>> IMG,
                  block_threshold b_thres,
                  pe_threshold pe_thres,
                  vector<pair<int, int>>);
void
hybrid_exp_main();

void
hybrid_exp_chest();
#endif /* Hybrid_hpp */
