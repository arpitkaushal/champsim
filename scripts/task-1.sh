cd ..

mkdir -p logs

echo  'TASK 1\nStart Time: ' >> ./logs/task-1.txt
echo  'TASK 1 ERRORS/WARNINGS\n' >> ./logs/task-1-err.txt

exec 1>> ./logs/task-1.txt 
exec 2>> ./logs/task-1-err.txt 

./build_champsim.sh bimodal no no no no lru 1    # single core build
echo '\n\nBuild Completed. Running the 1 core build.\n\n' 
./run_champsim.sh bimodal-no-no-no-no-lru-1core 1 10 400.perlbench-41B.champsimtrace.xz # single core run
echo '\n\nTask 1 Part 1 completed!\n\n'


./build_champsim.sh bimodal no no no no lru 4    # four core build
echo '\n\nBuild Completed. Running the 4 core build.\n\n' 
./run_4core.sh bimodal-no-no-no-no-lru-4core 1 10 0 400.perlbench-41B.champsimtrace.xz 454.calculix-104B.champsimtrace.xz 603.bwaves_s-5359B.champsimtrace.xz 649.fotonik3d_s-1B.champsimtrace.xz # 4 traces run
echo '\n\nTask 1 Part 2 completed!\n\n'

echo '\n\nTask 1 completed! :)\n\n'

exit