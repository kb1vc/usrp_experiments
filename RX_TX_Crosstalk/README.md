# RX_TX_Crosstalk test

This test emits a constant TX envelope (as in the manner of
uhd/examples/tx_waveforms --wave-type CONST) and, optionally,
activates the RX front end by tuning it to a frequency specified
on the command line.

In the case of the B210 (mine is serial # EFR04Z9BT with
firmware version 8.0 and FPGA version 11.0 -- revision 4,
and product 2) tuning the RX front end to a frequency near
the transmit frequency will produce spurious components in the
transmit output that are less than 25dB below the carrier.

For instance, try this:
~~~~~
cd build
cmake ../
make
./RX_TX_Crosstalk --args 'type=b200' --freq 5760.01e6 --ampl 0.3 --gain 70 --rate 625000 --rx-freq 5759.90e6
~~~~~

Then look at the output from the B200 on a spectrum analyzer.  You should
see a carrier near 5760.01 MHz and a spurious product at 5760.440 MHz
(or thereabouts) that is a little over 20dB below the carrier.
There will likely be other spurs (near 5760.110 and 5760.220) that are
about -40dBc.

Then try this:
~~~~~~
./RX_TX_Crosstalk --args 'type=b200' --freq 5760.01e6 --ampl 0.3 --gain 70 --rate 625000 --rx-freq 5749.90e6
~~~~~~

Presto!  The spurs are gone.

