# Release 1.0.0

## Hardware - Known Issues

1. Expansion header J2 was incorrectly specified (only 9-pins loaded, need all 10). Change MPN FTSH-105-01-L-DV-007-P to FTSH-105-01-L-DV-P (Digikey PN# SAM8910-ND). You need to make this modification to the Bill of Materials before sending to manufacturing.
2. Capacitor C16 was changed to 10uF, MPN C0402C106M9PACTU , update the BoM before sending to manufacturing.
##
## Software - Known Issues
1. Duplicated first data packet issue: after the initial connection is established, the client always receives the very first data packet twice. Further investigation showed that the SwiftMoteEC failed to receive the knowledgement sent by the client on the very first data packet. A temporary solution is provided with this release by avoiding saving the duplicated data packet in the .csv data file. 
##
