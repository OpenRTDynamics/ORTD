#/bin/sh

#`cat ../scilabdir.conf`/bin/scilab -nw -nogui -f ld_toolbox/builder.sce
`cat ../scilabdir.conf`/bin/scilab -nw -nogui -e "exec('ld_toolbox/builder.sce'); quit"
