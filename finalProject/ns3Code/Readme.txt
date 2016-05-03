All the variables are the same for as Project 2 except with a couple slight changes listed below

nFlows- number of flows PER NODE (ie number of browsers on one node (one node represents MANY PEOPLE)
nNodes- number of nodes (nFlows per node on the client side, 1 server per node on the server side)
NOTE: Multiple clients connect to one server as done in Jeffay paper.
Everything else is the same as project 2 (all tcp stuff can be made constant in your code)

Also note the experiments should be based off the paper.
It looks like we can determine load based on Figure 4 in the paper.
WE CANNOT BASE IT OFF TABLE 2 as our system samples real world data to determine the number of requests. However, the real world data produces the results in Figure 4.

Ie. For 90% load find point on graph that corresponds to 9 Mbps and set topology to 7 nodes and do math to find number of browsers (nFlows) per node. In this case it looks like it would be 3200 total browsers or 458 browsers per node 

Example Command
./waf --run "scratch/p4 --minTh=262144 --maxTh=786432 --weightFactor=.002 --maxDropProb=50 --queueSize=1048576 --verbose=False --nFlows=1 --isDropTail=False --bottleneckRate=50 --receiverWindowSize=30000 --linkDelays=.005 --nNodes=7 --csv=MY.csv"

NOTE: maxDropProb is in % form (DO NOT PASS IN DECIMAL VALUE) (IE. 50=50%)
Everything else is explained pretty well in the cmd calls
Most of the sizes are in bytes (minTh,maxTh,queueSize,receiverWindowSize)
linkDelays is in seconds (hence .005= 5 ms)
bottleneckRate in Mbps
