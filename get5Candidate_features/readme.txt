The ground truth of soma in soma_ground_truth.txt contain soma coordinates.
cmd: get5Candidate_features.exe soma_5coordinates.txt

input : soma coordinate for obtaining features
output : filename.attr, contained features for each coordinate.

the order in attr is 4 features (intensity, deviation, density, equilibrium) and
x y z intensity target(1 or 0)

soma_ground_truth.txt 
filename
x y z
filename
x y z
...

the number of soma coordinate is fix 5
soma_5coordinates.txt
filename
x1 y1 z1 
x2 y2 z2
...
...
...
filename
x1 y1 z1
...
...