# Implementation of: Assembly of chromosome-scale contigs by efficiently resolving repetitive sequences with long reads

## Install
To install just run the `install.sh` script and it will prepare the necessary files. It fetches the `minimap2` 
submodule and compiles this project. The resulting binary will show up in the project root folder.

```bash
./install.sh
```

## Use
To run HERA, simply run the `run.sh` script with a path to your dataset folder.

```bash
./run.sh <dataset-dir>
```

The dataset must be of the following structure (including the filenames):
```
<dataset-dir>
       L  reads.fasta
       L  contigs.fasta
       L  rr_overlaps.paf
       L  cr_overlaps.paf
```

### Acknowledgements
Original paper:
> Assembly of chromosome-scale contigs by efficiently resolving repetitive sequences with long reads\
Huilong Du, Chengzhi Liang\
bioRxiv: 345983\
doi: [https://doi.org/10.1101/345983](https://doi.org/10.1101/345983)

Created as a project for the [Bioinformatics course](https://www.fer.unizg.hr/en/course/bio) at [FER](https://www.fer.unizg.hr/en), [University of Zagreb](http://www.unizg.hr/homepage/).

### Support
Project is under the supervision of:
* Domagoj Latečki
* Juraj Fulir
* Rudolf Lovrenčić
