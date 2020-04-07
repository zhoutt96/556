./sendfile -r agate.clear.rice.edu:18004 -f data/30MB.bin

/usr/bin/netsim --drop=95 --delay=95 --reorder=95 --duplicate=95 --mangle=95
/usr/bin/netsim --duplicate=95