# Acknowledgement
The hardware and software of the SwiftMoteEC was developed by CMC Microsystems in collaboration with Dr. Phillippe Dauphin Ducharme and his team at Université de Sherbrooke’s Department of Chemistry and Abaxial Medical,  as part of the Acuité Québec Consortium.

# SwiftMoteECp_HW Repository
This repo is for development of the prototype hardware.  

Directories:  
> ./		: Project files including the schematic and printed circuit board  
>> /documentation	: A brief overview of power and programming  
>> /graphics	: A block diagram and photos of SwiftMote-Electrochemical  
>> /libraries     	: KiCad project related libraries including part symbols and footprints  
>> /licenses	: All hardware-related licenses  
>> /manufacturing	: All necessary files to manufacture the board.  Subdirectories include:  
>>>    /Bill_of_Materials : Bill of Materials (BoM) used to manufacture the first version of the board  
>>>    /Drill_Files	: Mechanical drill files  
>>>    /Instructions	: Special instructions to be provided to the board manufacturer  
>>>    /Plots	: Gerber files  
>>>    /Stackup	: Board stackup  
>>>    /XY_Position_Files : Orientation of the parts on the board  

Attention: 
1. Expansion header J2 was incorrectly specified (only 9-pins loaded, need all 10).  Change MPN FTSH-105-01-L-DV-007-P to FTSH-105-01-L-DV-P (Digikey PN# SAM8910-ND). You need to make this modification to the Bill of Materials before sending to manufacturing.
2. Capacitor C16 was changed to 10uF, MPN C0402C106M9PACTU , update the BoM.
