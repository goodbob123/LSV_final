## The zip file should include

### A. readme file (here)
which describes what are included in the zip file and provides
necessary information to reproduce your results (e.g., how to compile and run
your program, etc.).

### B. report (./report/report.pdf)
Your report in the pdf format. The suggested length is 4 pages for a
two-column form or 8 pages in a single-column form of conference papers.
Please write it in a precise way to show your methods and experimental
results. Note that in case of survey-type projects, be careful about the
copyright issue. You should summarize in your own words unless you have
exact quotes.
### C. source codes (./src)
Your source codes for implementation. (Note that your source code is
necessary for the course evaluation purpose. By submitting the source code,
you retain your copyright. No one will use it in other purposes without your
permission.)

### D. presentation slides (./report/slide.pdf)
Your presentation slides with update.

### E. Referenced materials (./ref_materials)
Referenced materials. (Benchmarks for experiment; referenced papers, slides,
documents, etc.)

### F. Other supplemental materials
Other supplemental materials

##  Reproduce  results

### Compile abc
1. cd src/abc
2. make

### WINDOWING REWRITE PART
make sure that there is ./abc executable file in working directory. 

``` 
cd src/WINDOW
make
./runAll.sh
```
#### Some command in ./runAll.sh
+ AIG2aag : convert .aig file to .aag (ASCII aig)
+ CIRRead : read circuit
+ CIRwindow : Do window rewriting
+ CIRWrite : Write a output .aag file

### SIMULATED ANNEALING
make sure that there is ./abc executable file in working directory. 

``` 
cd src/SA
./final.sh all
```
#### Some command in SA part
+ operation.sh : execute simulated annealing based on given command
+ final.sh : execute simulated annealing on files in benchmarks based on given command

### Machine Learning

