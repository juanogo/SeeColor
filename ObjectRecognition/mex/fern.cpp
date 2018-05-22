

#include "stdio.h"
#include "math.h"
#include <vector>
#include <map>
#include <set>
#include "tld.h"
//#ifdef _CHAR16T
//#define CHAR16_T
//#endif
#include "mex.h" 
using namespace std;

static double thrN;
static int nBBOX;
static int mBBOX;
static int nTREES;
static int nFEAT;
static int nSCALE;
static int iHEIGHT;
static int iWIDTH;
static int *BBOX = NULL;
static int *OFF  = NULL;
static double *IIMG = 0;
static double *IIMG2 = 0;
static vector<vector <double> > WEIGHT;
static vector<vector <int> > nP;
static vector<vector <int> > nN;
static int BBOX_STEP = 7;
static int nBIT = 1; // number of bits per feature

#define sub2idx(row,col,height) ((int) (floor((row)+0.5) + floor((col)+0.5)*(height)))


void update(double *x, int C, int N) {
	for (int i = 0; i < nTREES; i++) {

		int idx = (int) x[i];

		(C==1) ? nP[i][idx] += N : nN[i][idx] += N;

		if (nP[i][idx]==0) {
			WEIGHT[i][idx] = 0;
		} else {
			WEIGHT[i][idx] = ((double) (nP[i][idx])) / (nP[i][idx] + nN[i][idx]);
		}
	}
}


double measure_forest(double *idx) {
	double votes = 0;
	for (int i = 0; i < nTREES; i++) { 
		votes += WEIGHT[i][idx[i]];
	}
	return votes;
}


int measure_tree_offset(unsigned char *img, int idx_bbox, int idx_tree) {

	int index = 0;
	int *bbox = BBOX + idx_bbox*BBOX_STEP;
	int *off = OFF + bbox[5] + idx_tree*2*nFEAT;
	for (int i=0; i<nFEAT; i++) {
		index<<=1; 
		int fp0 = img[off[0]+bbox[0]];
		int fp1 = img[off[1]+bbox[0]];
		if (fp0>fp1) { index |= 1;}
		off += 2;
	}
	return index;	
}


double measure_bbox_offset(unsigned char *blur, int idx_bbox, double minVar, double *tPatt) {

	double conf = 0.0;
	double bboxvar = bbox_var_offset(IIMG,IIMG2,BBOX+idx_bbox*BBOX_STEP);
	if (bboxvar < minVar) {	return conf; }

	for (int i = 0; i < nTREES; i++) { 
		int idx = measure_tree_offset(blur,idx_bbox,i);
		tPatt[i] = idx;
		conf += WEIGHT[i][idx];
	}
	return conf;
}

int* create_offsets(double *scale0, double *x0) {

	int *offsets = (int*) malloc(nSCALE*nTREES*nFEAT*2*sizeof(int));
	int *off = offsets;

	for (int k = 0; k < nSCALE; k++){
		double *scale = scale0+2*k;
		for (int i = 0; i < nTREES; i++) {
			for (int j = 0; j < nFEAT; j++) {
				double *x  = x0 +4*j + (4*nFEAT)*i;
				*off++ = sub2idx((scale[0]-1)*x[1],(scale[1]-1)*x[0],iHEIGHT);
				*off++ = sub2idx((scale[0]-1)*x[3],(scale[1]-1)*x[2],iHEIGHT);
			}
		}
	}
	return offsets;
}

int* create_offsets_bbox(double *bb0) {

	int *offsets = (int*) malloc(BBOX_STEP*nBBOX*sizeof(int));
	int *off = offsets;

	for (int i = 0; i < nBBOX; i++) {
		double *bb = bb0+mBBOX*i;
		*off++ = sub2idx(bb[1]-1,bb[0]-1,iHEIGHT);
		*off++ = sub2idx(bb[3]-1,bb[0]-1,iHEIGHT);
		*off++ = sub2idx(bb[1]-1,bb[2]-1,iHEIGHT);
		*off++ = sub2idx(bb[3]-1,bb[2]-1,iHEIGHT);
		*off++ = (int) ((bb[2]-bb[0])*(bb[3]-bb[1]));
		*off++ = (int) (bb[4]-1)*2*nFEAT*nTREES; // pointer to features for this scale
		*off++ = bb[5]; // number of left-right bboxes, will be used for searching neighbours
		

	}
	return offsets;
}


double randdouble() 
{ 
	return rand()/(double(RAND_MAX)+1); 
} 


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	//mexPrintf("nrhs: %d \n",nrhs);
	if (nrhs == 0) {
		mexPrintf("CLEANUP: function(0);\n");
		mexPrintf("INIT: function(1, img, bb, features, scales)\n");
		mexPrintf("UPDATE: Conf = function(2,X,Y,Margin,Bootstrap,Idx)\n");
		mexPrintf("EVALUATE: Conf = function(3,X)\n");
		mexPrintf("DETECT: function(4,img,maxBBox,minVar,Conf,X)\n");
		mexPrintf("GET PATTERNS: patterns = fern(5,img,idx,minVar)\n");
		return;
	}



	//mexPrintf("%d - "(int) *mxGetPr(prhs[0]));

	switch ((int) *mxGetPr(prhs[0])) {

		// CLEANUP: function(0);
		// =============================================================================
	case 0:  {
		srand(0); // fix state of random generator

		thrN = 0; nBBOX = 0; mBBOX = 0; nTREES = 0; nFEAT = 0; nSCALE = 0; iHEIGHT = 0; iWIDTH = 0;

		free(BBOX); BBOX = 0;
		free(OFF); OFF = 0;
		free(IIMG); IIMG = 0;
		free(IIMG2); IIMG2 = 0;
		WEIGHT.clear();
		nP.clear();
		nN.clear();
		return;
			 }

			 // INIT: function(1, img, bb, features, scales)
			 //                0  1    2   3         4
			 // =============================================================================
	case 1:  {
		
		if (nrhs!=5) { mexPrintf("fern: wrong input.\n"); return; }
		if (BBOX!=0) { mexPrintf("fern: already initialized.\n"); return; }

		iHEIGHT    = mxGetM(prhs[1]);
		iWIDTH     = mxGetN(prhs[1]);
		nTREES     = mxGetN(mxGetField(prhs[3],0,"x"));
		nFEAT      = mxGetM(mxGetField(prhs[3],0,"x")) / 4; // feature has 2 points: x1,y1,x2,y2
		thrN       = 0.5 * nTREES;
		nSCALE     = mxGetN(prhs[4]);

		IIMG       = (double*) malloc(iHEIGHT*iWIDTH*sizeof(double));
		IIMG2      = (double*) malloc(iHEIGHT*iWIDTH*sizeof(double));

		// BBOX
		mBBOX      = mxGetM(prhs[2]); 
		nBBOX      = mxGetN(prhs[2]);
		BBOX	   = create_offsets_bbox(mxGetPr(prhs[2]));
		//mexPrintf("(1)  (2)  %d %d \n",BBOX[0],BBOX[1]  );
		double *x  = mxGetPr(mxGetField(prhs[3],0,"x"));
		double *s  = mxGetPr(prhs[4]);
		OFF		   = create_offsets(s,x);

		for (int i = 0; i<nTREES; i++) {
			WEIGHT.push_back(vector<double>(pow(2.0,nBIT*nFEAT), 0));
			nP.push_back(vector<int>(pow(2.0,nBIT*nFEAT), 0));
			nN.push_back(vector<int>(pow(2.0,nBIT*nFEAT), 0));
		}

		for (int i = 0; i<nTREES; i++) {
			for (int j = 0; j < WEIGHT[i].size(); j++) {
				WEIGHT[i].at(j) = 0;
				nP[i].at(j) = 0;
				nN[i].at(j) = 0;
			}
		}

		return;
			 }

	// UPDATE
	// =============================================================================
	case 2: {

		if (nrhs!=5 && nrhs!=6) { mexPrintf("Conf = function(2,X,Y,Margin,Bootstrap,Idx)\n"); return; }
		//                                                   0 1 2 3      4         5

		double *X     = mxGetPr(prhs[1]);
		int numX      = mxGetN(prhs[1]);
		double *Y     = mxGetPr(prhs[2]);
		double thrP   = *mxGetPr(prhs[3]) * nTREES;
		int bootstrap = (int) *mxGetPr(prhs[4]);


		int step = numX / 10;

		if (nrhs == 5) {
			for (int j = 0; j < bootstrap; j++) {
				for (int i = 0; i < step; i++) {
					for (int k = 0; k < 10; k++) {
					
						int I = k*step + i;
						double *x = X+nTREES*I;
						if (Y[I] == 1) {
							if (measure_forest(x) <= thrP)
								update(x,1,1);
						} else {
							if (measure_forest(x) >= thrN)
								update(x,0,1);
						}
					}
				}
			}
		}
		if (nrhs == 6) {
			double *idx   = mxGetPr(prhs[5]);
			int nIdx      = mxGetN(prhs[5])*mxGetM(prhs[5]);


			for (int j = 0; j < bootstrap; j++) {
				for (int i = 0; i < nIdx; i++) {
					int I = idx[i]-1;
					double *x = X+nTREES*I;
					if (Y[I] == 1) {
						if (measure_forest(x) <= thrP)
							update(x,1,1);
					} else {
						if (measure_forest(x) >= thrN)
							update(x,0,1);
					}
				}
			}
		}



		if (nlhs==1) {
			plhs[0] = mxCreateDoubleMatrix(1, numX, mxREAL); 
			double *resp0 = mxGetPr(plhs[0]);

			for (int i = 0; i < numX; i++) {
				*resp0++ = measure_forest(X+nTREES*i);
			}
		}

		return;
			}

	// EVALUATE PATTERNS
	// =============================================================================
	case 3: {

		if (nrhs!=2) { mexPrintf("Conf = function(2,X)\n"); return; }
		//                                        0 1  

		double *X     = mxGetPr(prhs[1]);
		int numX      = mxGetN(prhs[1]);

		plhs[0] = mxCreateDoubleMatrix(1, numX, mxREAL); 
		double *resp0 = mxGetPr(plhs[0]);

		for (int i = 0; i < numX; i++) {
			*resp0++ = measure_forest(X+nTREES*i);
		}

		return;
			}

			// DETECT: TOTAL RECALL
			// =============================================================================
	case 4: {

		if (nrhs != 6) { 
			mexPrintf("function(4,img,maxBBox,minVar,conf,patt)\n"); 
			//                  0 1   2       3      4    5   
			return; 
		}

		// Pointer to preallocated output matrixes
		//mexPrintf("nBBOX %d \n",nBBOX);
		double *conf = mxGetPr(prhs[4]); if ( mxGetN(prhs[4]) != nBBOX) { mexPrintf("Wrong input.\n"); return; }
		double *patt = mxGetPr(prhs[5]); if ( mxGetN(prhs[5]) != nBBOX) { mexPrintf("Wrong input.\n"); return; }
		for (int i = 0; i < nBBOX; i++) { conf[i] = -1; }

		// Setup sampling of the BBox
		double probability = *mxGetPr(prhs[2]);

		double nTest  = nBBOX * probability; if (nTest <= 0) return;
		if (nTest > nBBOX) nTest = nBBOX;
		double pStep  = (double) nBBOX / nTest;
		double pState = randdouble() * pStep;

		// Input images
		unsigned char *input = (unsigned char*) mxGetPr(mxGetField(prhs[1],0,"input"));
		unsigned char *blur  = (unsigned char*) mxGetPr(mxGetField(prhs[1],0,"blur"));

		// Integral images
		iimg(input,IIMG,iHEIGHT,iWIDTH);
		iimg2(input,IIMG2,iHEIGHT,iWIDTH);

		// log: 0 - not visited, 1 - visited
		int *log = (int*) calloc(nBBOX,sizeof(int));

		// variance
		double minVar = *mxGetPr(prhs[3]);

		// totalrecall
		//double totalrecall = *mxGetPr(prhs[6]);
		int I = 0;
		int K = 2;

		while (1) 
		{
			// Get index of bbox
			I = (int) floor(pState);
			pState += pStep;
			if (pState >= nBBOX) { break; }

			// measure bbox
			log[I] = 1;
			double *tPatt = patt + nTREES*I;
			conf[I] = measure_bbox_offset(blur,I,minVar,tPatt);

			// total recall
			/*
			if (totalrecall == 1 && conf[i] > thrN) 
			{
				int I;
				int W = *(BBOX + i*BBOX_STEP + 6);

				for (int gH = -K; gH <= K; gH++ ) 
				{
					for (int gW = -K; gW <= K; gW++)
					{
						I = i+gW+(gH)*W;
						if (I >= 0 && I < nBBOX && log[I]==0)
						{
							log[I] = 1;
							tPatt = patt + nTREES*I;
							conf[I] = measure_bbox_offset(blur,I,minVar,tPatt);
						}
					}
				}
			}
			*/
		}

		free(log);
		return;
			}

			// GET PATTERNS
	case 5: {

		if (nrhs !=4) { mexPrintf("patterns = fern(5,img,idx,var)\n"); return; }
		//                                         0 1   2   3   

		// image
		unsigned char *input = (unsigned char*) mxGetPr(mxGetField(prhs[1],0,"input"));
		unsigned char *blur  = (unsigned char*) mxGetPr(mxGetField(prhs[1],0,"blur"));

		//if (mxGetM(prhs[1])!=iHEIGHT) { mexPrintf("fern: wrong input image.\n"); return; }

		// bbox indexes
		double *idx = mxGetPr(prhs[2]);
		int numIdx = mxGetM(prhs[2]) * mxGetN(prhs[2]);

		// minimal variance
		double minVar = *mxGetPr(prhs[3]);
		if (minVar > 0) {
			iimg(input,IIMG,iHEIGHT,iWIDTH);
			iimg2(input,IIMG2,iHEIGHT,iWIDTH);
		}

		// output patterns
		plhs[0] = mxCreateDoubleMatrix(nTREES,numIdx,mxREAL);
		double *patt = mxGetPr(plhs[0]);

		plhs[1] = mxCreateDoubleMatrix(1,numIdx,mxREAL);
		double *status = mxGetPr(plhs[1]);

		for (int j = 0; j < numIdx; j++) {

			if (minVar > 0) {
				double bboxvar = bbox_var_offset(IIMG,IIMG2,BBOX+j*BBOX_STEP);
				if (bboxvar < minVar) {	continue; }
			}
			status[j] = 1;
			double *tPatt = patt + j*nTREES;
			for (int i = 0; i < nTREES; i++) {
				tPatt[i] = (double) measure_tree_offset(blur, idx[j]-1, i);
			}
		}
		return;
			}

    case 6: {
		//outputs
		int l=pow(2.0,nBIT*nFEAT);
		plhs[0] = mxCreateDoubleMatrix(nTREES,l, mxREAL);//WEIGHT
	    plhs[1] = mxCreateDoubleMatrix(nTREES,l, mxREAL);//nP
		plhs[2] = mxCreateDoubleMatrix(nTREES,l, mxREAL);//nN
		plhs[3] = mxCreateDoubleMatrix(1,10, mxREAL);// static int parameters
							

	    double *out = mxGetPr(plhs[0]);	double *out1 = mxGetPr(plhs[1]);// you needpointers to 
		double *out2 = mxGetPr(plhs[2]);double *out3 = mxGetPr(plhs[3]);// proccess outputs
		
		
		for (int i=0; i<nTREES; i++) {// fill the outputs
			for (int j=0; j<l; j++){
			  out[nTREES*j+i]=WEIGHT[i].at(j);
			  out1[nTREES*j+i]=nP[i].at(j);
			  out2[nTREES*j+i]=nN[i].at(j);
			
			}

		}
				
	    out3[0]=thrN;out3[1]=nBBOX;out3[2]=nTREES;out3[3]=nFEAT;//fill more outputs
		out3[4]=nSCALE;out3[5]=iHEIGHT;out3[6]=iWIDTH;out3[7]=BBOX_STEP;
		out3[8]=nBIT;out3[9]=mBBOX;
		
		return;
			}
	
	   case 7: {
		    
		   // INIT: function(7, static_int_par, WEIGHT, nP, nN , tld.features,  tld.scales, tld.grid)
			 //              0  1               2       3   4    5              6           7
			 // =============================================================================
	     double *temp = mxGetPr(prhs[1]);

		 thrN=temp[0];nBBOX=temp[1];nTREES=temp[2];nFEAT=temp[3];
		 nSCALE=temp[4];iHEIGHT=temp[5];iWIDTH=temp[6];BBOX_STEP=temp[7];
		 nBIT=temp[8];mBBOX=temp[9];

		 for (int i = 0; i<nTREES; i++) {
			WEIGHT.push_back(vector<double>(pow(2.0,nBIT*nFEAT), 0));
			nP.push_back(vector<int>(pow(2.0,nBIT*nFEAT), 0));
			nN.push_back(vector<int>(pow(2.0,nBIT*nFEAT), 0));
		}

		 double *temp1 = mxGetPr(prhs[2]);
		 double *temp2 = mxGetPr(prhs[3]);
		 double *temp3 = mxGetPr(prhs[4]);

		 for (int i = 0; i<nTREES; i++) {
			 for (int j = 0; j < WEIGHT[i].size(); j++) {
				WEIGHT[i].at(j) = temp1[nTREES*j+i];
				nP[i].at(j) = temp2[nTREES*j+i];
				nN[i].at(j) = temp3[nTREES*j+i];
			 }
		 }

		 
		
		IIMG       = (double*) malloc(iHEIGHT*iWIDTH*sizeof(double));
		IIMG2      = (double*) malloc(iHEIGHT*iWIDTH*sizeof(double));

		BBOX	   = create_offsets_bbox(mxGetPr(prhs[7]));
		
		double *x  = mxGetPr(mxGetField(prhs[5],0,"x"));
		double *s  = mxGetPr(prhs[6]);
		OFF		   = create_offsets(s,x);
		
		return;
			}
	}

} 

