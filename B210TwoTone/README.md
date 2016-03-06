# B210 Two Tone generator


For instance, try this:
~~~~~
cd build
cmake ../
make
./B210TwoTone  --freq 5760.01e6 --sep .1e6 --ampl 0.3 --gain 80 
~~~~~

Then look at the output from the B200 on a spectrum analyzer.  You should
see two carriers near 5760.01 MHz and a spurious product at 5760.11 MHz

The max separation seems to be about 15 MHz or so...

