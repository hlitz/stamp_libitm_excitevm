#!/bin/bash

echo "[b]ayes, [g]enome, [i]ntruder, [k]means, [l]abyrinthm [s]sca, [v]acation, [y]ada"
read -p "enter your choice :> " c

if [ "$c" == "b" ]; then
    echo "old then new"
    ITM_DEFAULT_METHOD=ml_wt ../TRANSACT_ORIG_OBJ/bayes/bayes -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t1
    ITM_DEFAULT_METHOD=ml_wt ./obj/bayes/bayes -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t1
fi

if [ "$c" == "g" ]; then
    echo "old then new"
    ITM_DEFAULT_METHOD=ml_wt ../TRANSACT_ORIG_OBJ/genome/genome -s64 -g16384 -n16777216 -t4
    ITM_DEFAULT_METHOD=ml_wt ./obj/genome/genome -s64 -g16384 -n16777216 -t4
fi

if [ "$c" == "i" ]; then
    echo "old then new"
    ITM_DEFAULT_METHOD=ml_wt ../TRANSACT_ORIG_OBJ/intruder/intruder -a10 -l128 -n262144 -s1 -t4
    ITM_DEFAULT_METHOD=ml_wt ./obj/intruder/intruder -a10 -l128 -n262144 -s1 -t4
fi

if [ "$c" == "k" ]; then
    echo "old then new"
    ITM_DEFAULT_METHOD=ml_wt ../TRANSACT_ORIG_OBJ/kmeans/kmeans -m40 -n40 -T0.00001 -i ../inputs/kmeans/random-n65536-d32-c16.txt -t1
    ITM_DEFAULT_METHOD=ml_wt ./obj/kmeans/kmeans -m40 -n40 -T0.00001 -i ../inputs/kmeans/random-n65536-d32-c16.txt -t1
fi

if [ "$c" == "l" ]; then
    echo "old then new"
    ITM_DEFAULT_METHOD=ml_wt ../TRANSACT_ORIG_OBJ/labyrinth/labyrinth -i ../inputs/labyrinth/random-x512-y512-z7-n512.txt -t4
    ITM_DEFAULT_METHOD=ml_wt ./obj/labyrinth/labyrinth -i ../inputs/labyrinth/random-x512-y512-z7-n512.txt -t4
fi

if [ "$c" == "s" ]; then
    echo "old then new"
    ITM_DEFAULT_METHOD=ml_wt ../TRANSACT_ORIG_OBJ/ssca2/ssca2 -s20 -i1.0 -u1.0 -l3 -p3 -t4
    ITM_DEFAULT_METHOD=ml_wt ./obj/ssca2/ssca2 -s20 -i1.0 -u1.0 -l3 -p3 -t4
fi

if [ "$c" == "v" ]; then
    echo "old then new"
    ITM_DEFAULT_METHOD=ml_wt ../TRANSACT_ORIG_OBJ/vacation/vacation -n2 -q90 -u98 -r1048576 -T4194304 -t4
    ITM_DEFAULT_METHOD=ml_wt ./obj/vacation/vacation -n2 -q90 -u98 -r1048576 -T4194304 -t4
fi

if [ "$c" == "y" ]; then
    echo "old then new"
    ITM_DEFAULT_METHOD=ml_wt ../TRANSACT_ORIG_OBJ/yada/yada -a 15 -i ../inputs/yada/ttimeu1000000.2 -t4
    ITM_DEFAULT_METHOD=ml_wt ./obj/yada/yada -a 15 -i ../inputs/yada/ttimeu1000000.2 -t4
fi
