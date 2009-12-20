set term dumb 125 45
set xlabel 'Uhrzeit'
set ylabel 'Anzahl'
set xdata time
set timefmt "%s"
set format x "%H:%M:%S"
#plot [] [0:11] 'waffel.input' u ($2+3600):($1 > 0 ? $1 : 1/0) w points ps 5 pt 3 t '14.12.09' , 'waffel.input-091204' u ($2+3600+10*24*3600):($1 > 0 ? $1+.25 : 1/0) w points ps 5 pt 2 t '4.12.09'
plot [] [0:11] 'waffel.input' u ($2+3600):($1 > 0 ? $1 : 1/0) w points ps 5 pt 3 t '15.12.09'

