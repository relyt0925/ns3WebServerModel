from __future__ import division
import subprocess

gridLength = 1000
nodeDensity = [2, 4, 16, 256]
txPower = [10, 100, 500]
protocols = [0, 1]
trafficIntensity = [0.1, 0.25, 0.5, 0.9]

csv = 'p3Loc.csv'
logFile = 'p3.log'

f = open(logFile, 'w')

# RED experiments.
for t in txPower:
    for p in protocols:
        for i in trafficIntensity:
            for n in nodeDensity:
                cmd = "./waf --run 'scratch/p3Loc --gridLength={gridLength} --numNodes={n} --txPower={t} --isAODV={p} --trafficIntensity={i} --csv={csv} --verbose=0'".format(gridLength=gridLength, n=n, t=t, p=p, i=i, csv=csv)
                print cmd
                retVal = subprocess.call(cmd, shell=True)
                if retVal != 0:
                    f.write('{}\n'.format(cmd))

f.close()
