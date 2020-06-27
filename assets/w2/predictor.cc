#include "predictor.h"

#define WT_SIZE 10 // number of entries in weight tables
#define GHL 128	// length of global history for SGHR, GHR, HA and HTrain
#define WL_1 20 // history length for the separate 1024-entry tables
#define WL_2 16 // history length for the single 1024-entry table
#define WL_3 29 // history length for the single 512-entry table
#define THH 107 // Initial threshold of trainning

/* --- weight tables --- */
int8_t     weightT[1<<(WT_SIZE)][WL_1]; // Taken weight table for the most recent 20 branches. 7 * 1024 * 20 = 143,360 bits
int8_t	   weightNT[1<<(WT_SIZE)][WL_1]; // Not-Taken weight table for the most recent 20 branches. 7 * 1024 * 20 = 143,360 bits
int8_t	   weight1[1<<WT_SIZE][WL_2]; // Single weight table for the next 16 branches. 7 * 1024 * 16 = 114,688 bits
int8_t	   weight2[1<<WT_SIZE][WL_3]; // Single weight table for the last 29 branches. 7 * 512 * 29 = 103,936 bits
// Cost of weight tables: 519,680 bits (61.7KB)
// Total Cost including misc info: 523,032 bits (62.1KB)

/* --- Shift registers --- */
bool *SGHR; //speculative history register used for updates
bool *HTrain; // A register indicate if training is needed 
bool *GHR;	//perfect history used for prediction and updates
uint32_t *HA;	// Path address register

int8_t specul;  // Speculation counter. Counting how many branches are fetched but not retired.
int32_t threshold; // dynamic threshold value as in O-GEHL
int8_t TC; //threshold counter as in O-GEHL

// count number of runs
uint32_t runs;

void PredictorInit() {
    runs = 0;
    SGHR = new bool[GHL];	
    HTrain = new bool[GHL];
    GHR = new bool[GHL];
    HA = new uint32_t[GHL];	
    assert(SGHR);
    assert(HTrain);
    assert(GHR);
    assert(HA);
}

/*----- Hash function that calculating index of weight tables -----*/
uint32_t gen_widx(uint32_t cur_pc, uint32_t path_pc, uint32_t wt_size) 
{
    cur_pc = (cur_pc ) ^ (cur_pc / (1<<wt_size));
    path_pc = (path_pc) ^ (path_pc / (1<<wt_size));
    uint32_t widx = cur_pc ^ (path_pc);
    widx = widx % (1<<wt_size);
    return widx;
}

void PredictorReset() {
    // this function is called before EVERY run
    // it is used to reset predictors and change configurations

    for (int i = 0; i < (1<<WT_SIZE); i++) {
        for(int j = 0; j < WL_1; j++)
        {
            weightT[i][j] = 0;
            weightNT[i][j] = 0;
	}
        for(int j = 0; j < WL_2; j++)
        {
            weight1[i][j] = 0;
	}
    }

    for (int i = 0; i < (1<<(WT_SIZE-1)); i++) {
        for(int j = 0; j < WL_3; j++)
        {
            weight2[i][j] = 0;
	}
    }
    specul = 0;
//    for(int i=0; i<(1<<WT_SIZE); i++) {
    threshold = THH;
    TC = 0;
//  }
}

void PredictorRunACycle() {
    const cbp3_cycle_activity_t *cycle_info = get_cycle_info();
    int32_t accum = 0;
    // make prediction at fetch stage
    for (int i = 0; i < cycle_info->num_fetch; i++) { 	
        uint32_t fe_ptr = cycle_info->fetch_q[i];	
        const cbp3_uop_dynamic_t *uop = &fetch_entry(fe_ptr)->uop;
	
        if (uop->type & IS_BR_CONDITIONAL) {
	  /*----- Algorithm of Prediction -----*/	
		accum = 0;
		/*----- First WL_1 branches: use 1024-entry separate T/NT weight tables -----*/
	    	for(int j=0; j <WL_1; j++) {
		  uint32_t widx = gen_widx(uop->pc, HA[j], WT_SIZE); // compute index to access either weight table
		  
	    	  if( GHR[j] == 1)		// If history is Taken
		    accum += weightT[widx][j];  // Then add Taken weight
	    	  else 				// If history is Not-Taken
		    accum += weightNT[widx][j]; // Then add Not-Taken weight
		}
		/*----- Next WL_2 branches: use 1024-enrty single weight tables -----*/
		for(int j=0; j<WL_2; j++)
		{
		  uint32_t widx = gen_widx(uop->pc, HA[WL_1+j], WT_SIZE); // compute index to access either weight table
	    	  if( GHR[WL_1+j] == 1)	
		    accum += weight1[widx][j];  
	    	  else 				
		    accum -= weight1[widx][j];
		}
		/*----- Last WL_3 branches: use 512-entry single weight tables -----*/
		for(int j=0; j<WL_3; j++)
		{
		  uint32_t widx = gen_widx(uop->pc, HA[WL_1+WL_2+j], WT_SIZE-1); // compute index to access weight table
	    	  if( GHR[WL_1+WL_2+j] == 1)	
		    accum += weight2[widx][j];  
	    	  else 				
		    accum -= weight2[widx][j]; 
		}
		bool pred = (accum >= 0); // Predict Taken if sum >= 0; Predict Not-Taken if sum <= 0;

		// Report prediction:
	        assert(report_pred(fe_ptr, false, pred));
		// Update history shift registers (to be used in Update phase)
	        for(int j=GHL-1; j>0; j--)
	        {
	        	SGHR[j] = SGHR[j-1];
			GHR[j] = GHR[j-1];
	        	HTrain[j] = HTrain[j-1];
			HA[j] = HA[j-1];
		}
		SGHR[0] = pred;
		GHR[0] = uop->br_taken?1:0;
		if(accum>-threshold && accum<threshold)
		  HTrain[0]=1; // 1 means trainning needed
		else
		  HTrain[0]=0;
		HA[0]=uop->pc; // HA records the path address
		specul++;	// Increment the speculation counter because a branch is fetched into the pipeline      
        } 
    }
    
    for (int i = 0; i < cycle_info->num_retire; i++) {
        uint32_t rob_ptr = cycle_info->retire_q[i];
        const cbp3_uop_dynamic_t *uop = &rob_entry(rob_ptr)->uop;
        if ( uop->type & IS_BR_CONDITIONAL) {
	    	bool t = uop->br_taken;
		if( (t != SGHR[specul-1]) || (HTrain[specul-1] == 1) ) 	//Training needed if threshold not exceeded or predict wrong
		{
		  /*----- Algorithm for Update -----*/
			
		  /*----- First WL_1 branches: update T/NT weight tables separately -----*/
			  for(int j = 0; j < WL_1; j++)	{
			      uint32_t widx = gen_widx(uop->pc, HA[j+specul], WT_SIZE); // compute the index to access either weight table;  
			      if(t==1 && GHR[specul+j]==1)
			      { if(weightT[widx][j]<63) weightT[widx][j]++;}
			      else if(t==0 && GHR[specul+j]==1)
			      { if(weightT[widx][j]>-64) weightT[widx][j]--;}
			      else if(t==1 && GHR[specul+j]==0)
			      { if(weightNT[widx][j]<63) weightNT[widx][j]++;}
			      else if(t==0 && GHR[specul+j]==0)
			      { if(weightNT[widx][j]>-64) weightNT[widx][j]--;}		
			  }    
		  /*----- Next WL_2 branches: update regular weight tables -----*/
			  for(int j = 0; j < WL_2; j++)	{
			    uint32_t widx = gen_widx(uop->pc, HA[j+WL_1+specul], WT_SIZE); // compute the index to access either weight table;  
			      if(t==GHR[specul+j+WL_1])
			      { if(weight1[widx][j]<63) weight1[widx][j]++;}
			      else 
			      { if(weight1[widx][j]>-64) weight1[widx][j]--;}	
			  } 
   		/*----- Last WL_3 branches: update regular weight tables -----*/
			  for(int j = 0; j < WL_3; j++)	{
			    uint32_t widx = gen_widx(uop->pc, HA[j+WL_1+WL_2+specul], WT_SIZE-1); // compute the index to access either weight table;  
			      if(t==GHR[specul+j+WL_1+WL_2])
			      { if(weight2[widx][j]<63) weight2[widx][j]++;}
			      else 
			      { if(weight2[widx][j]>-64) weight2[widx][j]--;}	
			  }    		
		}
		/*------Update the threshold -------*/
		if(t != SGHR[specul-1]) {
		    TC++;
		    if(TC==63) {
		   	TC = 0;
			threshold++;
		    }
		}
		else if(t==SGHR[specul-1] && HTrain[specul-1] == 1) {
		    TC--;
		    if(TC==-63) {
			TC = 0;
			threshold--;
		    }		
		}

		specul--; // Decrement speculation counter because a branch is retired.
        }
    }
}

void PredictorRunEnd() {
  runs ++;
  if (runs < 1) // set rewind_marked to indicate that we want more runs
      rewind_marked = true;
}

void PredictorExit() {
  ;
}
