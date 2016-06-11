#Hybrid Reversible Data Hiding#
--------------  
## Abstract ##
In medical images, the region with complex texture is called regions of interest (ROI) and the background area called regions of noninterest (RONI). In this approach, histogram shifting (HS) method is used in RONI and prediction error (PE) method is used in ROI. The hybrid reversible data hiding (RDH) method is superior than HS method and PE method.

------------------------  
## Data Embedding Algorithm ##

1. Blocking
The medical image with size H\*W devided into blocks with size n\*n. Blocks are tarversed in raster order and devided into ROI and RONI by comparing each block's standard deviation with block threshold (T<sub>stdev</sub>). Block sequence is recorded.
2. Embedding
RONI are embedded by HS method.  
ROI are embedded by PE method.
3. Block information
After data embedding, the block sequence may be affected by the difference between marked image and original image. We would record the difference block's index in this sequence as side information.  

------------------------  
## Data Extracting Algorithm ##
1. Blocking  
We can recover block information by block threshold and block bookkeeping data.
2. Extracting  
In ROI, data can be extracted and image can be recovered reveribly by PE threshold.  
In RONI, data can be extracted and image can be recovered reveribly by HS threshold. 
