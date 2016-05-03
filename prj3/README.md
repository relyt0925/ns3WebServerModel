# ece6110

Here is the command to run with all the parameters
./waf --run "scratch/p3 --gridLength=100000 --numNodes=2 --txPower=100 --isAODV=True --trafficIntensity=0.1 --verbose=False --csv=Heyy.csv"

TxPower in milliwats
Traffic intensity as % of time sending (0-1.0)
isAODV as True/False
Grid Length in meters
NOTE: to run you need to replace your 
{ns-3.24.1}/src/applications/model/onoff-application.{h,cc} with the ones in this directory

The only thing I added was a getter method to the model to get the bytes sent from an OnOffApplication

