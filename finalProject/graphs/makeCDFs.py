# Import all libraries needed for the tutorial
from __future__ import division

# General syntax to import specific functions in a library: 
##from (library) import (specific library function)
from pandas import DataFrame, read_csv

# General syntax to import a library but no functions: 
##import (library) as (give the library a nickname/alias)
import matplotlib.pyplot as plt
import pandas as pd #this is how I usually import pandas
import sys #only needed to determine Python version number
import matplotlib #only needed to determine Matplotlib version number
import pdb
import numpy as np

SAVEFIGS = 1

colors = ['r', 'g', 'b', 'k', 'm', 'y', 'c']
markers = ['o:', 'x--', '*-', '^-.'] 

def getLineStyle():
    # Apparently function attributes is how you do static variables.
    i = getLineStyle.counter
    getLineStyle.counter += 1
    return colors[i%len(colors)] + markers[i%len(markers)]
# Initialize function static variable.
getLineStyle.counter = 0

def graphCDF(data, axs, label):
    ser = data
    ser = ser.order()
    ser[len(ser)] = ser.iloc[-1]
    cum_dist = np.linspace(0., 100., len(ser))
    ser_cdf = pd.Series(cum_dist, index=ser)
    #ser_cdf.plot(drawstyle='steps', label=label)
    axs.plot(ser_cdf.index[:], ser_cdf.iloc[:], label=label)
    return ser_cdf

def plotREDSettings(settings, title, filename):
    for s in settings:
        l = s[0]
        wq = s[1]
        maxp = s[2]
        minTh = s[3]
        maxTh = s[4]
        q = s[5]
        csv = 'p4/RED_load{}_queue{}_minTh{}_maxTh{}_wq{}_maxp{}.csv'.format(l, q, minTh, maxTh, wq, maxp)
        df = read_csv(csv, names=names)
        ser_cdf = graphCDF(df.responseTime, axs, label='RED - wq=1/{},maxp=1/{},th=({},{}), qlen={}'.format(wq, maxp, minTh, maxTh, q))
    axs.set_title(title)
    axs.set_xlabel('Response Times (ms)')
    axs.set_ylabel('Cumulative Probability (%)')
    axs.legend(loc='lower right')
    axs.set_xlim([0, 1000])
    if SAVEFIGS:
        plt.savefig(filename)

plt.close('all')

names = ['bt', 'responseTime']

# Figure 9a 9b 9c 9d.
# qlen 30, 60, 120, 190, 240
# load 80, 90, 98, 110
loads = [80, 90, 98, 110]
qlens = [30, 60, 120, 190, 240]
titles = list('abcd')
titles2 = list('abcd')
for l in loads:
    fig, axs = plt.subplots(1)
    for q in qlens:
        csv = 'p4/FIFO_load{}_queue{}.csv'.format(l, q)
        df = read_csv(csv, names=names)
        ser_cdf = graphCDF(df.responseTime, axs, label='qLen={}'.format(q))
        #ser_cdf = graphCDF(pd.Series(np.random.normal(size=100)), axs, label=csv)
    axs.set_title('Figure 9{}. FIFO Performance at {}% Load'.format(titles.pop(0), l))
    axs.set_xlabel('Response Times (ms)')
    axs.set_ylabel('Cumulative Probability (%)')
    #box = axs.get_position()
    #axs.set_position([box.x0, box.y0, box.width*0.8, box.height])
    #axs.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    axs.legend(loc='lower right')
    axs.set_xlim([0, 1000])
    if SAVEFIGS:
        plt.savefig('Figure_9{}.png'.format(titles2.pop(0)))

# Figure 10.
loads = [50, 70, 80, 90, 98, 110]
fig, axs = plt.subplots(1)
q = 120
for l in loads:
    csv = 'p4/FIFO_load{}_queue{}.csv'.format(l, q)
    df = read_csv(csv, names=names)
    ser_cdf = graphCDF(df.responseTime, axs, label='load={}%'.format(l))
axs.set_title('Figure 10. FIFO Performance at Different Loads (qLen=120)')
axs.set_xlabel('Response Times (ms)')
axs.set_ylabel('Cumulative Probability (%)')
axs.legend(loc='lower right')
axs.set_xlim([0, 1000])
if SAVEFIGS:
    plt.savefig('Figure_10.png')

# Figure 11.
loads = [50, 70, 80, 90, 98, 110]
fig, axs = plt.subplots(1)
wq=512
maxp=10
minTh=30
maxTh=90
q = 480
for l in loads:
    csv = 'p4/RED_load{}_queue{}_minTh{}_maxTh{}_wq{}_maxp{}.csv'.format(l, q, minTh, maxTh, wq, maxp)
    df = read_csv(csv, names=names)
    ser_cdf = graphCDF(df.responseTime, axs, label='load={}%'.format(l))
axs.set_title('Figure 11. RED Performance at Different Loads')
axs.set_xlabel('Response Times (ms)')
axs.set_ylabel('Cumulative Probability (%)')
#box = axs.get_position()
#axs.set_position([box.x0, box.y0, box.width*0.8, box.height])
#axs.legend(loc='center left', bbox_to_anchor=(1, 0.5))
axs.legend(loc='lower right')
axs.set_xlim([0, 1000])
if SAVEFIGS:
    plt.savefig('Figure_11.png')

# Figure 12a, 12b.
loads = [90, 98]
#loads=[90]
wq=512
maxp=10
ths = [(5, 15), (15, 45), (30, 90), (60, 180), (120, 360)]
q = 480
titles=list('ab')
titles2=list('ab')
for l in loads:
    fig, axs = plt.subplots(1)
    for minTh, maxTh in ths:
        csv = 'p4/RED_load{}_queue{}_minTh{}_maxTh{}_wq{}_maxp{}.csv'.format(l, q, minTh, maxTh, wq, maxp)
        df = read_csv(csv, names=names)
        ser_cdf = graphCDF(df.responseTime, axs, label='minTh={},maxTh={}'.format(minTh, maxTh))
    axs.set_title('Figure 12{}. RED Performance at {}% Load'.format(titles.pop(0), l))
    axs.set_xlabel('Response Times (ms)')
    axs.set_ylabel('Cumulative Probability (%)')
    axs.legend(loc='lower right')
    axs.set_xlim([0, 1000])
    if SAVEFIGS:
        plt.savefig('Figure_12{}.png'.format(titles2.pop(0)))

# Figure 13.
l=90
maxTh=90
wq=512
maxp=10
qlen=480
minThs = [5, 15, 30, 45, 60]
fig, axs = plt.subplots(1)
for minTh in minThs:
    csv = 'p4/RED_load{}_queue{}_minTh{}_maxTh{}_wq{}_maxp{}.csv'.format(l, q, minTh, maxTh, wq, maxp)
    df = read_csv(csv, names=names)
    ser_cdf = graphCDF(df.responseTime, axs, label='minTh={}'.format(minTh))
axs.set_title('Figure 13. RED Performance with Changing minTh')
axs.set_xlabel('Response Times (ms)')
axs.set_ylabel('Cumulative Probability (%)')
axs.legend(loc='lower right')
axs.set_xlim([0, 1000])
if SAVEFIGS:
    plt.savefig('Figure_13.png')

# Figure 14.
l=90
qlen=480
minTh=30
maxTh=90
wqs=[512, 256, 128]
maxps=[20, 10, 4]
fig, axs = plt.subplots(1)
for wq in wqs:
    for maxp in maxps:
        csv = 'p4/RED_load{}_queue{}_minTh{}_maxTh{}_wq{}_maxp{}.csv'.format(l, q, minTh, maxTh, wq, maxp)
        df = read_csv(csv, names=names)
        ser_cdf = graphCDF(df.responseTime, axs, label='wq=1/{},maxp=1/{}'.format(wq, maxp))
axs.set_title('Figure 14. RED Performance with different wq and maxp')
axs.set_xlabel('Response Times (ms)')
axs.set_ylabel('Cumulative Probability (%)')
axs.legend(loc='lower right')
axs.set_xlim([0, 1000])
if SAVEFIGS:
    plt.savefig('Figure_14.png')

# Figure 16a.
settings = [(90, 512, 10, 5, 15, 480),
        (90, 256, 4, 5, 120, 480),
        (90, 512, 10, 120, 150, 480)]
fig, axs = plt.subplots(1)
plotREDSettings(settings, 'Figure 16a. "Bad" RED Parameters Settings at 90% Load', 'Figure_16a.png')

# Figure 16b.
settings = [(98, 512, 10, 5, 15, 480),
        (98, 512, 4, 5, 90, 480),
        (98, 512, 10, 120, 360, 480)]
fig, axs = plt.subplots(1)
plotREDSettings(settings, 'Figure 16b. "Bad" RED Parameters Settings at 98% Load', 'Figure_16b.png')

# Figure 22a.
l = 90
qlens = [120, 190]
fig, axs = plt.subplots(1)
# Uncongested
csv = 'p4/FIFO_load{}_queue{}.csv'.format(10, 120)
df = read_csv(csv, names=names)
ser_cdf = graphCDF(df.responseTime, axs, label='uncongested')
for q in qlens:
    csv = 'p4/FIFO_load{}_queue{}.csv'.format(l, q)
    df = read_csv(csv, names=names)
    ser_cdf = graphCDF(df.responseTime, axs, label='FIFO-qLen={}'.format(q))
settings = [(90, 512, 10, 30, 90, 120),
        (90, 512, 10, 60, 180, 480)]
plotREDSettings(settings, 'title', 'murgfilename')
axs.set_title('FIFO and RED at 90% Load')
axs.set_xlabel('Response Times (ms)')
axs.set_ylabel('Cumulative Probability (%)')
axs.legend(loc='lower right')
axs.set_xlim([0, 1000])
if SAVEFIGS:
    plt.savefig('Figure_22a.png')

# Figure 22b.
l = 98
qlens = [120, 190]
fig, axs = plt.subplots(1)
# Uncongested
csv = 'p4/FIFO_load{}_queue{}.csv'.format(10, 120)
df = read_csv(csv, names=names)
ser_cdf = graphCDF(df.responseTime, axs, label='uncongested')
for q in qlens:
    csv = 'p4/FIFO_load{}_queue{}.csv'.format(l, q)
    df = read_csv(csv, names=names)
    ser_cdf = graphCDF(df.responseTime, axs, label='FIFO-qLen={}'.format(q))
settings = [(98, 512, 10, 30, 90, 120),
        (98, 128, 20, 5, 90, 480)]
plotREDSettings(settings, 'title', 'murgfilename')
axs.set_title('FIFO and RED at 98% Load')
axs.set_xlabel('Response Times (ms)')
axs.set_ylabel('Cumulative Probability (%)')
axs.legend(loc='lower right')
axs.set_xlim([0, 1000])
if SAVEFIGS:
    plt.savefig('Figure_22b.png')

# Figure 22c.
l = 110
qlens = [120, 190]
fig, axs = plt.subplots(1)
# Uncongested
csv = 'p4/FIFO_load{}_queue{}.csv'.format(10, 120)
df = read_csv(csv, names=names)
ser_cdf = graphCDF(df.responseTime, axs, label='uncongested')
for q in qlens:
    csv = 'p4/FIFO_load{}_queue{}.csv'.format(l, q)
    df = read_csv(csv, names=names)
    ser_cdf = graphCDF(df.responseTime, axs, label='FIFO-qLen={}'.format(q))
settings = [(110, 512, 10, 30, 90, 120),
        (110, 256, 20, 30, 90, 480)]
plotREDSettings(settings, 'title', 'murgfilename')
axs.set_title('FIFO and RED at 110% Load')
axs.set_xlabel('Response Times (ms)')
axs.set_ylabel('Cumulative Probability (%)')
axs.legend(loc='lower right')
axs.set_xlim([0, 1000])
if SAVEFIGS:
    plt.savefig('Figure_22c.png')


if not SAVEFIGS:
    plt.show() 


'''
# Create some test data
dx = .10
X  = np.arange(-2,2,dx)
Y  = np.exp(-X**2)

# Normalize the data to a proper PDF
Y /= (dx*Y).sum()

# Compute the CDF
CY = np.cumsum(Y*dx)

plt.plot(X,Y)
plt.plot(X,CY,'ro--')

if GRAPHLOC == 0:
    names=['gridLength', 'numNodes', 'txPower', 'isAODV', 'trafficIntensity', 'totalEfficiency']
    df = read_csv('p3.csv', names=names)

    ################################################################################
    # Node Density Plots
    ################################################################################
    # Plot total efficiency vs number of nodes.
    fig, axs = plt.subplots(1, sharex=True)
    x = df.numNodes.unique()
    for i, a in enumerate(df.txPower.unique()):
        for j, b in enumerate(df.isAODV.unique()):
            for k, c in enumerate(df.trafficIntensity.unique()):
                y = df[(df.txPower == a) &
                            (df.isAODV == b) &
                            (df.trafficIntensity == c)].totalEfficiency
                label='TX{}-P{}-I{}'.format(a, b, c)
                lineStyle = getLineStyle()
                axs.plot(x, y, lineStyle, label=label)
    axs.set_title('Total Efficiency vs Number of Nodes')
    axs.set_xlabel('Number of Nodes')
    axs.set_xscale('log', basex=2)
    axs.set_ylabel('Total Efficiency (%)')
    box = axs.get_position()
    axs.set_position([box.x0, box.y0, box.width*0.8, box.height])
    axs.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    if SAVEFIGS:
        fig.savefig('nodeDensity_all.png')

    ################################################################################
    # TXPower Plots
    ################################################################################
    # Plot total efficiency vs tx power.
    fig, axs = plt.subplots(1, sharex=True)
    x = df.txPower.unique()
    for i, a in enumerate(df.numNodes.unique()):
        for j, b in enumerate(df.isAODV.unique()):
            for k, c in enumerate(df.trafficIntensity.unique()):
                y = df[(df.numNodes == a) &
                            (df.isAODV == b) &
                            (df.trafficIntensity == c)].totalEfficiency
                label='N{}-P{}-I{}'.format(a, b, c)
                lineStyle = getLineStyle()
                axs.plot(x, y, lineStyle, label=label)
    axs.set_title('Total Efficiency vs TX Power')
    axs.set_xlabel('TX Power (mW)')
    axs.set_ylabel('Total Efficiency (%)')
    box = axs.get_position()
    axs.set_position([box.x0, box.y0, box.width*0.8, box.height])
    axs.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    if SAVEFIGS:
        fig.savefig('txPower_all.png')

    ################################################################################
    # Routing Protocol Plots
    ################################################################################
    # Plot total efficiency vs routing protocols.
    AODV = df[df.isAODV==1].reset_index()
    OLSR = df[df.isAODV==0].reset_index()
    fig, axs = plt.subplots(1, sharex=True)
    axs.plot(AODV.totalEfficiency - OLSR.totalEfficiency, 'go-')
    axs.set_title('Total Efficiency Difference')
    axs.set_xlabel('Simulation Case Number')
    axs.set_ylabel('Total Efficiency Difference (%)')
    box = axs.get_position()
    axs.set_position([box.x0, box.y0, box.width*0.8, box.height])
    axs.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    if SAVEFIGS:
        fig.savefig('comparison_all.png')

    # Fix numNodes = 2, 4, 16, 64
    fig, axs = plt.subplots(1, sharex=True)
    AODV2 = AODV[AODV.numNodes==2].reset_index()
    OLSR2 = OLSR[OLSR.numNodes==2].reset_index()
    AODV4 = AODV[AODV.numNodes==4].reset_index()
    OLSR4 = OLSR[OLSR.numNodes==4].reset_index()
    AODV16 = AODV[AODV.numNodes==16].reset_index()
    OLSR16 = OLSR[OLSR.numNodes==16].reset_index()
    AODV256 = AODV[AODV.numNodes==256].reset_index()
    OLSR256 = OLSR[OLSR.numNodes==256].reset_index()
    axs.plot(AODV2.totalEfficiency - OLSR2.totalEfficiency, getLineStyle(), label='2 Nodes')
    axs.plot(AODV4.totalEfficiency - OLSR4.totalEfficiency, getLineStyle(), label='4 Nodes')
    axs.plot(AODV16.totalEfficiency - OLSR16.totalEfficiency, getLineStyle(), label='16 Nodes')
    axs.plot(AODV256.totalEfficiency - OLSR256.totalEfficiency, getLineStyle(), label='256 Nodes')
    axs.set_title('Total Efficiency Difference (AODV-OLSR)')
    axs.set_xlabel('Simulation Case Number')
    axs.set_ylabel('Total Efficiency Difference (%)')
    box = axs.get_position()
    axs.set_position([box.x0, box.y0, box.width*0.8, box.height])
    axs.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    if SAVEFIGS:
        fig.savefig('comparison_all.png')


    ################################################################################
    # Traffic Intensity Plots
    ################################################################################
    # Plot total efficiency vs traffic intensity.
    fig, axs = plt.subplots(1, sharex=True)
    x = df.trafficIntensity.unique()
    for i, a in enumerate(df.numNodes.unique()):
        for j, b in enumerate(df.isAODV.unique()):
            for k, c in enumerate(df.txPower.unique()):
                y = df[(df.numNodes == a) &
                            (df.isAODV == b) &
                            (df.txPower == c)].totalEfficiency
                label='N{}-P{}-TX{}'.format(a, b, c)
                lineStyle = getLineStyle()
                axs.plot(x, y, lineStyle, label=label)
    axs.set_title('Total Efficiency vs Traffic Intensity')
    axs.set_xlabel('Traffic Intensity')
    axs.set_ylabel('Total Efficiency (%)')
    box = axs.get_position()
    axs.set_position([box.x0, box.y0, box.width*0.8, box.height])
    axs.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    if SAVEFIGS:
        fig.savefig('trafficIntensity_all.png')

    if SAVEFIGS == 0:
        plt.show()

################################################################################
# Simulation Topology Plots
################################################################################
# Change XXXX:YYYY:ZZZZ string format dumped by ns3 into [x, y, z]
def parseXYZ(s):
    xyz = map(float, s.split(':'))
    return xyz

names=['gridLength', 'numNodes', 'txPower', 'isAODV', 'trafficIntensity', 'totalEfficiency', 'flowNum', 'flowEfficiency', 'src', 'dst']
df = read_csv('p3Loc.csv', names=names)

df.src = [parseXYZ(c) for c in df.src]
df.dst = [parseXYZ(c) for c in df.dst]

# Case 1. 4 Nodes, low traffic intensity.
# Create a 1000x1000 graph.
fig, axs = plt.subplots(1)
df16 = df[(df.isAODV==1) &
        (df.numNodes==4) &
        (df.txPower==100) &
        (df.trafficIntensity==0.25)].reset_index()
for i in xrange(len(df16)):
    [x1, y1, z1] = df16.src[i]
    [x2, y2, z2] = df16.dst[i]

    axs.plot(x1, y1, 'ko')
    axs.plot(x2, y2, 'ko')
    green = df16.flowEfficiency[i]/100
    # Green is good efficiency 
    # Red is bad efficiency
    # Black is rgb=[0, 0, 0]
    axs.plot([x1, x2], [y1, y2], color=[1-green, green, 0], label='effic={:.1f}%'.format(df16.flowEfficiency[i]))

axs.set_title('Link Efficiency [numNodes=4, txPower=100mW, AODV, trafficIntensity=.25]')
axs.set_xlabel('X Location')
axs.set_ylabel('Y Location')
if LEGEND:
    box = axs.get_position()
    axs.set_position([box.x0, box.y0, box.width*0.8, box.height])
    axs.legend(loc='center left', bbox_to_anchor=(1, 0.5))
axs.set_xlim([0, 1000])
axs.set_ylim([0, 1000])
if SAVEFIGS:
    fig.savefig('linkEfficiency_4nodes_lowTraffic_nolegend.png')

# Case 2. 4 Nodes, high traffic intensity.
# Create a 1000x1000 graph.
fig, axs = plt.subplots(1)
df16 = df[(df.isAODV==1) &
        (df.numNodes==4) &
        (df.txPower==100) &
        (df.trafficIntensity==0.9)].reset_index()
for i in xrange(len(df16)):
    [x1, y1, z1] = df16.src[i]
    [x2, y2, z2] = df16.dst[i]

    axs.plot(x1, y1, 'ko')
    axs.plot(x2, y2, 'ko')
    green = df16.flowEfficiency[i]/100
    # Green is good efficiency 
    # Red is bad efficiency
    # Black is rgb=[0, 0, 0]
    axs.plot([x1, x2], [y1, y2], color=[1-green, green, 0], label='effic={:.1f}%'.format(df16.flowEfficiency[i]))

axs.set_title('Link Efficiency [numNodes=4, txPower=100mW, AODV, trafficIntensity=.9]')
axs.set_xlabel('X Location')
axs.set_ylabel('Y Location')
if LEGEND:
    box = axs.get_position()
    axs.set_position([box.x0, box.y0, box.width*0.8, box.height])
    axs.legend(loc='center left', bbox_to_anchor=(1, 0.5))
axs.set_xlim([0, 1000])
axs.set_ylim([0, 1000])
if SAVEFIGS:
    fig.savefig('linkEfficiency_4nodes_highTraffic_nolegend.png')

# Case 3. 16 Nodes, traffic intensity scaled by full range.
# Create a 1000x1000 graph.
fig, axs = plt.subplots(1)
df16 = df[(df.isAODV==1) &
        (df.numNodes==16) &
        (df.txPower==100) &
        (df.trafficIntensity==0.1)].reset_index()
for i in xrange(len(df16)):
    [x1, y1, z1] = df16.src[i]
    [x2, y2, z2] = df16.dst[i]
    axs.plot(x1, y1, 'ko')
    axs.plot(x2, y2, 'ko')
    green = df16.flowEfficiency[i] / max(df16.flowEfficiency)
    # Green is good efficiency 
    # Red is bad efficiency
    # Black is rgb=[0, 0, 0]
    axs.plot([x1, x2], [y1, y2], color=[1-green, green, 0], label='effic={:.1f}%'.format(df16.flowEfficiency[i]))
axs.set_title('Link Efficiency [numNodes=16, txPower=100mW, AODV, trafficIntensity=.25]')
axs.set_xlabel('X Location')
axs.set_ylabel('Y Location')
if LEGEND:
    box = axs.get_position()
    axs.set_position([box.x0, box.y0, box.width*0.8, box.height])
    axs.legend(loc='center left', bbox_to_anchor=(1, 0.5))
axs.set_xlim([0, 1000])
axs.set_ylim([0, 1000])
if SAVEFIGS:
    fig.savefig('linkEfficiency_16nodes_nolegend.png')

# Case 4. 256 Nodes, traffic intensity scaled by full range.
# Create a 1000x1000 graph.
fig, axs = plt.subplots(1)
df16 = df[(df.isAODV==1) &
        (df.numNodes==256) &
        (df.txPower==100) &
        (df.trafficIntensity==0.1)].reset_index()
for i in xrange(len(df16)):
    [x1, y1, z1] = df16.src[i]
    [x2, y2, z2] = df16.dst[i]
    axs.plot(x1, y1, 'ko')
    axs.plot(x2, y2, 'ko')
    green = df16.flowEfficiency[i] / max(df16.flowEfficiency)
    # Green is good efficiency 
    # Red is bad efficiency
    # Black is rgb=[0, 0, 0]
    axs.plot([x1, x2], [y1, y2], color=[1-green, green, 0], label='effic={:.1f}%'.format(df16.flowEfficiency[i]))

axs.set_title('Link Efficiency [numNodes=256, txPower=100mW, AODV, trafficIntensity=.0.1]')
axs.set_xlabel('X Location')
axs.set_ylabel('Y Location')
axs.set_xlim([0, 1000])
axs.set_ylim([0, 1000])
if SAVEFIGS:
    fig.savefig('linkEfficiency_256nodes_nolegend.png')

if not SAVEFIGS:
    plt.show()
'''
