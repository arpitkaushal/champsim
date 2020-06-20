>## Setting up ChampSim on your System

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
```

Hopefully, after all the above commands your ChampSim Build will work fine. 
So, run the default test command now.  
**Make sure that your are in the root directory of `ChampSim`.** 

``` bash
./build_champsim.sh bimodal no no no no lru 1
./run_champsim.sh bimodal-no-no-no-no-lru-1core 1 10 454.calculix-104B.champsimtrace.xz
```