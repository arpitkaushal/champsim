># Table of Contents
1. [Setting up ChampSim on your System](#setting-up-champsim-on-your-system)
2. [Task 1](#task-1)

## Setting up ChampSim on your System
``` bash
sudo apt-get install build-essential
sudo apt-get install make
sudo apt-get update
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-9 -y
./build_champsim.sh bimodal no no no no lru 1
gcc-9 --version                                     # confirm install
sudo apt install g++
g++ --version                                       # confirm install
mkdir dp3_traces                                    # add traces in this folder
```

Hopefully, after all the above commands your ChampSim Build will work fine. 
So, run the default test command now.  
**Make sure that your are in the root directory of `ChampSim`.** 

``` bash
./build_champsim.sh bimodal no no no no lru 1
./run_champsim.sh bimodal-no-no-no-no-lru-1core 1 10 454.calculix-104B.champsimtrace.xz
```

## Task 1
1. Go to the root directory of ChampSim, and `cd scripts`
2. Make a new file `task-1.sh` by running the command - `touch task-1.sh`.
3. Open the file, and copy-paste the below code snippet. Save the file. 
   [`Alternative`] Download `task-1.sh` from [here](scripts/task-1.sh) and paste it the `scripts` folder.     

``` bash
cd ..

./build_champsim.sh bimodal no no no no lru 1   2>/dev/null # single core build
echo '\n\nBuild Completed. Running the 1 core build.\n\n' 
./run_champsim.sh bimodal-no-no-no-no-lru-1core 1 10 400.perlbench-41B.champsimtrace.xz # single core run
echo '\n\nTask 1 Part 1 completed!\n\n'


./build_champsim.sh bimodal no no no no lru 4   2>/dev/null # four core build
echo '\n\nBuild Completed. Running the 4 core build.\n\n' 
./run_4core.sh bimodal-no-no-no-no-lru-4core 1 10 0 400.perlbench-41B.champsimtrace.xz 454.calculix-104B.champsimtrace.xz 603.bwaves_s-5359B.champsimtrace.xz 649.fotonik3d_s-1B.champsimtrace.xz # 4 traces run
echo '\n\nTask 1 Part 2 completed!\n\n'

echo '\n\nTask 1 completed! :)\n\n'

exit
```

4. Now, make sure that you're in the `scripts` directory, and then, run, 
   `sh ./task-1.sh`
 