# Release 1.0.0

## Hardware - Known Issues

1. Expansion header J2 was incorrectly specified (only 9-pins loaded, need all 10). Change MPN FTSH-105-01-L-DV-007-P to FTSH-105-01-L-DV-P (Digikey PN# SAM8910-ND). You need to make this modification to the Bill of Materials before sending to manufacturing.
2. Capacitor C16 was changed to 10uF, MPN C0402C106M9PACTU , update the BoM before sending to manufacturing.
##
## Software - Known Issues
1. In the demo, at the initial transmission (or after a Bluetooth connection is re-established), the server (SwiftMoteEC) sends the first data packet twice. A temporary solution is provided with this release to minimize the amount of duplicated data sent and avoid saving the duplicated data packet in the .csv data file. 
##
