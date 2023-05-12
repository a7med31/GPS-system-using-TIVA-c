# GPS-system-using-TIVA-c
This is a GPS tracking system program that uses a TIVA c microcontroller to calculate the total distance from a starting point to any destination specified.

Components used:

    -TM4C123G LaunchPad.

    -NEO 6M GPS module.

How it works:

    -The GPS subsystem stores the coordinates of the start point. 

    -The destination point is entered into the program as coordinates.

    -After reaching the destination point, the GPS subsystem stores the coordinates of the end point and calculates the total distance that was taken by the user. 
    
The output will be as following:

    -The built-in LED will be turned on(red) when the target destination is far away by distance > 5 meters. 
    -The built-in LED will be turned on(yellow) when the target destination is about to be reached < 5 meters.

    -The built-in LED will be turned on(green) when the target destination is reached.

    -The coordinates(lattitude and longitude) and the total distance will be printed on the monitor screen as the program is running.


 

