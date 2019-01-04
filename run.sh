#!/usr/bin/env bash

# Dataset structure
#   <dataset-dir>
#          L  reads.fasta
#          L  contigs.fasta
#          L  rr_overlaps.paf
#          L  cr_overlaps.paf

if [ $# -ne 1 ]
then
    echo "Please provide the dataset directory path!"
    echo "Usage: ./run.sh <dataset-dir>"
    echo "Dataset structure must match the following"
    echo "<dataset-dir>"
    echo "       L  reads.fasta"
    echo "       L  contigs.fasta"
    echo "       L  rr_overlaps.paf"
    echo "       L  cr_overlaps.paf"
    exit 1
fi

# Source directory.
dir=$1

# Standardized input file names.
reads_file=$dir/"reads.fast?"
contigs_file=$dir/"contigs.fast?"

# Overlap file names.
rr_file=$dir/"rr_overlaps.paf"
cr_file=$dir/"cr_overlaps.paf"

# Construct overlaps if needed.
if [ ! -f $rr_file -a ! -f $cr_file ]
then
    mkdir "./res" 2> /dev/null
    ./minimap2/minimap2 --dual=yes -x ava-pb $reads_file $reads_file  > $rr_file
    ./minimap2/minimap2 --dual=yes -x ava-pb $contigs_file $reads_file> $cr_file
else
    echo "Overlap files already exist!"
fi

# TODO Run HERA.
