For only one soma in one data. Logistic selected highest value.

input : filelist
output : filelist.soma, filelist.bmp filelist.attr

cmd: soma_detection_max.exe list.txt

filelist.soma contain soma coordinate report by the method.
filelist.bmp project data in xy-view for confirm the result.
filelist.attr show all candidates and features.
The order is 4 features (intensity, deviation, density, equilibrium) and
x y z intensity logistic_value bf_value. 
