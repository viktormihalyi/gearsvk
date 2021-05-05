set terminal svg size 700,600 enhanced fname 'arial'

set xlabel 'Index'
set ylabel 'Random value'

set title "Random numbers - GearsVk"
set output 'values.svg'
plot "values.txt" notitle lc rgb 'red'




set ylabel 'counts per bin'
set xlabel 'bins'

set yrange [0:500]
set xrange [-0.5:19.49]

set style fill solid 1.0 border lt -1

set title "Histogram - GearsVk"
set output 'histogram.svg'
plot "histogram.txt" notitle with boxes  fs solid 0.3 linecolor rgb "#aaaaaa"
