#!/bin/bash

./bin/analyze analysis files/gg_events.root files/gg_ntuples_withimage.root ntupler BackgroundGG
./bin/analyze analysis files/qq_events.root files/qq_ntuples_withimage.root ntupler BackgroundQQ
./bin/analyze analysis files/wplus_events.root files/wp_ntuples_withimage.root ntupler SignalWplus
./bin/analyze analysis files/wminus_events.root files/wm_ntuples_withimage.root ntupler SignalWminus