Presentation

1. Overview/Motivation/Abstract
   - 2001 Perceptron introduced (Block Diagram)
   - Perceptron has limitations (Cannot separate Linearly Inseparable Branches)
   - Intuitive Example For Loops Example (Positive/Negative Correlations Possible but Mixed not possible )
   - Separate Weight Tables (so correlations are filtered ) but only few of them. Abbreviate SWP. (Block Diagram)
   - SWP and PSWP. Because recent branches have stronger correlation. 
     At negligible cost in performance, we cut down hardware costs by half (and reduce latecy as well? Source: Simulation Times).      
2. Code Overview
3. Results -
   1. During Hardware Analysis found out that we're comparing two incompatible (in terms of computational power) branch predictors. 
   2. Partial SWP paper implementation used more weight tables. Had two choices, scale Perceptron up OR scale Partial SWP down. 
   3. Chose to scale down because less hardware is good, and other comparisons with CSim versions (assuming they have equivalent hardware requirements). But you can find data for both simulations on the sheets.    
   4. Perceptron vs Partial SWP
   5. SWP vs PSWP
   6. Partial SWP 10 vs 20 vs 30 
      - Not much difference Performance Wise.
      - But the Hardware scales. So, less is more, but 20 was chosen by the implementors.
   7. General Trends
   8. Average Metrics
   9. Intel Pin For Loop Trace
4. Hardware Analysis
   1. Perceptron vs. Partial SWP vs. SWP  (Weight Table Analysis)  
   2. Scale Down vs. Scale Up Config   AND Link to Scale Up vs Scale Down Plots
   3. Simulation Time Insight? Computations Involved? 