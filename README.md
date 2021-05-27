# root2gnuplot

A small utility to convert objects in ROOT files into the gnuplot text-based data format.
Currently the following object types are supported:
- TH1, TH2 (all TH1 subclasses) - TH3 and THNSparse not implemented yet   
- TGraph, TGraphErrors, TGraphAsymErrors   

```bash
Usage:
    root2gnuplot.C <--help> input.root<:name> output.dat <format>, if <:name> omitted, convert all
    --help print more information

help:
format specifiers (case sensitive):
    x, y, z: coordinate from first, second, third axis for TH1, TH2, TH3 histograms

    <axis>low: value on <axis> at lower error bar, example: xlow, ylow, zlow

    <axis>high: value on <axis> at upper error bar, example: xhigh, yhigh, zhigh

    d<axis>: value on <axis> at upper error bar minus value on <axis>, example: dx, dy, dz


default formats:
    TH1: x y xlow xhigh ylow yhigh

examples:
    convert a single object and output a single file:
    ``root2gnuplot.C ACO_STARLight_AuAu.root:Acc0_idphi TEST.dat``
    - converts histogram named "Acc0_idphi" in ROOT file "ACO_STARLight_AuAu.root" and outputs into a file named "TEST.dat"

    convert all supported objects in a file and output one data file for each:
    ``root2gnuplot.C ACO_STARLight_AuAu.root data/SL``
    - This will export all supported objects (using default formats) in the file using the output "data/SL" as a prefix. 
    - Full output filename would be "data/SL_<name>.dat" where "<name>" is the ROOT object\'s name
    - TODO: support overriding formats per type

example output:
#converted from TH1 name: Acc0_idphi, file: ACO_STARLight_AuAu.root, date: 27-05-2021 10-50-05
#x              y              xlow           xhigh          ylow           yhigh          
 0.078525       360820         0              0.15705        360219         361421       
...

```




