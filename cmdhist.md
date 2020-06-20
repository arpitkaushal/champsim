>Setting up ChampSim on your System

        sudo apt-get install build-essential
        sudo apt-get update
        sudo apt-get install make
        sudo apt-get update
        whatis gcc
        gcc-7 --version
        sudo add-apt-repository ppa:ubuntu-toolchain-r/test
        sudo apt update
        sudo apt install g++-9 -y
        ./build_champsim.sh bimodal no no no no lru 1
        gcc-9 --version
        g++ --version
        sudo apt install g++


Hopefully, after all the above commands your ChampSim Build will work fine. 
So, run the default test command now.  

    ./build_champsim.sh bimodal no no no no lru 1
