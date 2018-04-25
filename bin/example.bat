rem dag-merger.exe -m "../PLY/bunny1k.ply" -s 2 -l 1 -e 1000.0 -c "./debug" -p 1 -i 3
rem dag-viewer.exe -m "../PLY/bunny1k.ply" -c "./debug/4/bunny1k/2_1/1000_near3/"
dag-upsample.exe -m "../PLY/bunny5k.ply" -n "../PLY/bunny1k.ply" -s 2 -l 1 -e 1000.0 -c "./debug" -b "./debug"
dag-viewer.exe -m "../PLY/bunny5k.ply" -c "./debug/4/bunny5k/2_1/1000_near3/"

pause

