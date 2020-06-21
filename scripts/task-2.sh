# !/bin/bash
cd ..

mkdir -p logs

echo  'TASK 2\nStart Time: ' >> ./logs/task-2.txt
echo  'TASK 2 ERRORS/WARNINGS\n' >> ./logs/task-2-err.txt

exec 1>> ./logs/task-2.txt 
exec 2>> ./logs/task-2-err.txt 

SECONDS=0 # reset BASH time counter
echo `Script started at $SECONDS`

# NOTE: You only need to build once for a configuration. 
# BUILDING 
./build_champsim.sh bimodal no no no no lru 1           
echo '\n\nBuild bimodal completed. \n\n'                
./build_champsim.sh gshare no no no no lru 1            
echo '\n\nBuild gshare completed. \n\n'                 
./build_champsim.sh hashed_perceptron no no no no lru 1 
echo '\n\nBuild  hashed_perceptron completed. \n\n'     
./build_champsim.sh perceptron no no no no lru 1        
echo '\n\nBuild perceptron completed. \n\n'             

echo `The BUILDS happend at $SECONDS`

# RUNNING BUILDS

# BIMODAL
    ./run_champsim.sh bimodal-no-no-no-no-lru-1core 1 10 454.calculix-104B.champsimtrace.xz      
    echo 'Bimodal 454 done! :)'  
    ./run_champsim.sh bimodal-no-no-no-no-lru-1core 1 10 603.bwaves_s-5359B.champsimtrace.xz      
    echo 'Bimodal 603 done! :)'  
    ./run_champsim.sh bimodal-no-no-no-no-lru-1core 1 10 649.fotonik3d_s-1B.champsimtrace.xz     
    echo 'Bimodal 649 done! :)'  
    ./run_champsim.sh bimodal-no-no-no-no-lru-1core 1 10 654.roms_s-1021B.champsimtrace.xz        
    echo 'Bimodal 654 done! :)'  

    echo `\n\nTask 2 Part bimodal completed! Time Elapsed: $SECONDS\n\n`

# GSHARE
    ./run_champsim.sh gshare-no-no-no-no-lru-1core 1 10 454.calculix-104B.champsimtrace.xz      
    echo 'gshare 454 done! :)'  
    ./run_champsim.sh gshare-no-no-no-no-lru-1core 1 10 603.bwaves_s-5359B.champsimtrace.xz      
    echo 'gshare 603 done! :)'  
    ./run_champsim.sh gshare-no-no-no-no-lru-1core 1 10 649.fotonik3d_s-1B.champsimtrace.xz     
    echo 'gshare 649 done! :)'  
    ./run_champsim.sh gshare-no-no-no-no-lru-1core 1 10 654.roms_s-1021B.champsimtrace.xz        
    echo 'gshare 654 done! :)'  

    echo `\n\nTask 2 Part gshare completed! Time Elapsed: $SECONDS \n\n`


# HASHED_PERCEPTRON
    ./run_champsim.sh hashed_perceptron-no-no-no-no-lru-1core 1 10 454.calculix-104B.champsimtrace.xz      
    echo 'hashed_perceptron 454 done! :)'  
    ./run_champsim.sh hashed_perceptron-no-no-no-no-lru-1core 1 10 603.bwaves_s-5359B.champsimtrace.xz      
    echo 'hashed_perceptron 603 done! :)'  
    ./run_champsim.sh hashed_perceptron-no-no-no-no-lru-1core 1 10 649.fotonik3d_s-1B.champsimtrace.xz     
    echo 'hashed_perceptron 649 done! :)'  
    ./run_champsim.sh hashed_perceptron-no-no-no-no-lru-1core 1 10 654.roms_s-1021B.champsimtrace.xz        
    echo 'hashed_perceptron 654 done! :)'  

    echo `\n\nTask 2 Part hashed_perceptron completed! Time Elapsed: $SECONDS \n\n`


# PERCEPTRON
    ./run_champsim.sh perceptron-no-no-no-no-lru-1core 1 10 454.calculix-104B.champsimtrace.xz      
    echo 'perceptron 454 done! :)'  
    ./run_champsim.sh perceptron-no-no-no-no-lru-1core 1 10 603.bwaves_s-5359B.champsimtrace.xz      
    echo 'perceptron 603 done! :)'  
    ./run_champsim.sh perceptron-no-no-no-no-lru-1core 1 10 649.fotonik3d_s-1B.champsimtrace.xz     
    echo 'perceptron 649 done! :)'  
    ./run_champsim.sh perceptron-no-no-no-no-lru-1core 1 10 654.roms_s-1021B.champsimtrace.xz        
    echo 'perceptron 654 done! :)'  

    echo `\n\nTask 2 Part perceptron completed! Time Elapsed: $SECONDS \n\n`


echo `TASK 2 COMPLETED! :)\n Time Elapsed: $SECONDS \n\n\n\n\n`

exit

# run without build took 7m 41s

# Traces
# 400.perlbench-41B.champsimtrace.xz
# 454.calculix-104B.champsimtrace.xz
# 603.bwaves_s-5359B.champsimtrace.xz
# 649.fotonik3d_s-1B.champsimtrace.xz
# 654.roms_s-1021B.champsimtrace.xz