#!/usr/bin/python
import os,sys,argparse

parser = argparse.ArgumentParser(description='Turn a snoopy log into a binconfig and polyco for DiFX.')
parser.add_argument('snoopylog', metavar='S', help='The snoopy log file')
parser.add_argument("-f", "--freq", type=float, default=1152.0, help="The reference freq at which snoopy DM was calculated")
parser.add_argument("--timediff", type=float, default=0, help="The time difference between the VCRAFT and snoopy log arrival times for the pulse, including geometric delay, in ms")
args = parser.parse_args()
inttime = 10 # seconds

with open(args.snoopylog) as snoopyin:
    snoopylines = snoopyin.readlines()
nocommentlines = []
for line in snoopylines:
    print line
    if len(line) > 1 and not line[0] =="#":
        nocommentlines.append(line)
        print "Snoopy info", nocommentlines
if len(nocommentlines) != 1:
    print "ERROR: No information found"
    sys.exit()
splitline = nocommentlines[0].split()
pulsewidth = float(splitline[3])*1.7 # filterbank width = 1.7ms
dm = float(splitline[5])
mjd = float(splitline[7])
    
startmjd = mjd - inttime*pulsewidth/(2*86400000.0)
nearestmjdstartsecond = int((startmjd - int(startmjd))*86400)
fractionalsecond = (mjd - int(mjd))*86400 - nearestmjdstartsecond
startphase = (fractionalsecond - pulsewidth/2000.0)/inttime # pulse width is in samples, which are 1.7ms
endphase = (fractionalsecond + pulsewidth/2000.0)/inttime # pulse width is in samples, which are 1.7ms

print nearestmjdstartsecond, fractionalsecond
hh = nearestmjdstartsecond/3600
mm = (nearestmjdstartsecond - hh*3600)/60
ss = nearestmjdstartsecond - (hh*3600 + mm*60)
polycostartmjd = int(mjd) + float(nearestmjdstartsecond)/86400.0

phaseoffset = args.timediff/10000. # ms --> phase units given the 10s "period"

newstartphase = startphase+phaseoffset
newendphase = endphase+phaseoffset

# Will use this if/when we figure out the 50-70ms offset...
with open("craftfrb.binconfig", "w") as binconfout:
    binconfout.write("NUM POLYCO FILES:   1\n")
    binconfout.write("POLYCO FILE 0:      craftfrb.polyco\n")
    binconfout.write("NUM PULSAR BINS:    2\n")
    binconfout.write("SCRUNCH OUTPUT:     TRUE\n")
    binconfout.write("BIN PHASE END 0:    %.9f\n" % newstartphase)
    binconfout.write("BIN WEIGHT 0:       0.0\n")
    binconfout.write("BIN PHASE END 1:    %.9f\n" % newendphase)
    binconfout.write("BIN WEIGHT 1:       1.0\n")
    binconfout.close()

# Instead cover a 150ms range with bins of size 5ms
#with open("craftfrb.binconfig", "w") as binconfout:
#    binconfout.write("NUM POLYCO FILES:   1\n")
#    binconfout.write("POLYCO FILE 0:      craftfrb.polyco\n")
#    binconfout.write("NUM PULSAR BINS:    30\n")
#    binconfout.write("SCRUNCH OUTPUT:     FALSE\n")
#    for i in range(30):
#        phasestr = ("BIN PHASE END %d:" % i).ljust(20)
#        weightstr = ("BIN WEIGHT %d:" % i).ljust(20)
#        binconfout.write("%s%.9f\n" % (phasestr, startphase + (i-14)*0.0005))
#        binconfout.write("%s1.0\n" % (weightstr))
#    binconfout.close()



with open("craftfrb.polyco", "w") as polycoout:
    polycoout.write("fake+fake DD-MMM-12 %02d%02d%05.2f %.15f %.4f 0.0 0.0\n" % (hh,mm,ss,polycostartmjd, dm))
    polycoout.write("0.0 %.3f 0 100 3 %.3f\n" % (1.0/float(inttime), args.freq))
    polycoout.write("0.00000000000000000E-99 0.00000000000000000E-99 0.00000000000000000E-99\n")
    polycoout.close()