># Table of Contents
1. [Setting up ChampSim on your System](#setting-up-champsim-on-your-system)
2. [Task 1](#task-1)
3. [Task 2](#task-2)

## Setting up ChampSim on your System
I used WSL Ubuntu, and had to run the following commands to have finally been able to run ChampSim. See if this helps you!

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
2. Download `task-1.sh` from [here](scripts/task-1.sh) and paste it in the `scripts` folder.     
3. Now, make sure that you're in the `scripts` directory, and then, run, 
   `time sh ./task-1.sh`

## Task 2
1. Go to the root directory of ChampSim, and `cd scripts`
2. Download `task-2.sh` from [here](scripts/task-2.sh) and paste it in the `scripts` folder.     
3. Now, make sure that you're in the `scripts` directory, and then, run, 
   `time sh ./task-2.sh`
 