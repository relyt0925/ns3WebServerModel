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

SAVEFIGS = 0
GRAPHLOC = 0
LEGEND=0

colors = ['r', 'g', 'b', 'k', 'm', 'y', 'c']
markers = ['o:', 'x--', '*-', '^-.'] 

def getLineStyle():
    # Apparently function attributes is how you do static variables.
    i = getLineStyle.counter
    getLineStyle.counter += 1
    return colors[i%len(colors)] + markers[i%len(markers)]
# Initialize function static variable.
getLineStyle.counter = 0

plt.close('all')

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
    '''
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
    '''

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
