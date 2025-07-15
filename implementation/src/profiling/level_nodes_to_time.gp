set terminal svg size 800, 600 enhanced font 'Arial,12'
set output 'time_plot.svg'
set dgrid3d 50,50
set hidden3d 
set contour base 
set cntrparam levels 10 
set view 60,40
set xlabel "num of Levels" 
set ylabel "num of Nodes" 
set zlabel "Time"

set title "time elapsed process computation for a graph with x levels, and y nodes"

splot 'time.dat' using 1:2:3 with lines palette

unset output
