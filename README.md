# k_plus_triangle

Author: Adam Anrzejczak
Date: 2015-01-15

 Description:  Implementation of a k+ neighbours algorithm with a triangle inequality optimization
               for both sparse and dense data.

 References:

   "A Neighborhood-Based Clustering by Means of the Triangle Inequality."
   Marzena Kryszkiewicz and Piotr Lasek.
   Institute of Computer Science, Warsaw University of Technology.

   Abstract:
   Grouping data into meaningful clusters is an important task of both artificial
   intelligence and data mining. An important group of clustering algorithms are
   density based ones that require calculation of a neighborhood of a given data
   record. The bottleneck for such algorithms are high dimensional data. In this
   paper, we propose a new TI-k-Neighborhood-Index algorithm that calculates
   k-neighborhoods for all records in a given data set by means the triangle
   inequality. We prove experimentally that the NBC (Neighborhood Based
   Clustering) clustering algorithm supported by our index outperforms NBC
   supported by such known spatial indices as VA-file and R-tree both in the case
   of low and high dimensional data.



   Usage example:
    -k 6 -p -D dense.csv -O object.csv -n 2 -o -m 2 -f -c

   Parameters:
    -k <number of neighbours>
    -D <path to the database file>
    -O <path to the file with objects to classify>
    -d <path to the file with classes names>
    -n <column that stores class name; counted from 1 upwards>
    -m <metric; 1=manhattan, 2=euclidean>

   Switches:
    -p using k+ modification; off by default
    -s indication, that data records are of the sparse format
    -o suppresses terminal output (except for setup and log messages)
    -c treat dense data as sparse or sparse data as dense - not recommended
    -b brute force, namely no triangle optimization (~1000x slower)
    -t more detailed output
    -f forward heuristic, backward by default

   Output file is called output.txt and is overwritten with every run.
