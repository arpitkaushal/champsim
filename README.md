# Branch Predictors on ChampSim
This is a website for the project we're undertaking as part of the course **ES215** `Computer Organization and Architecture` at IIT Gandhinagar. 

[*Git and Terminal Basic CheatSheet*](/bashandgit)  - To get you started and for quick reference. 


>## Setting up ChampSim on your System

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