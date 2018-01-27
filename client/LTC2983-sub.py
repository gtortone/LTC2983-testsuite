#!/usr/bin/python -B

import sys
import zmq
import signal
import time

from ROOT import TCanvas, TH1F, TSlider, THttpServer, TFile, TNtuple

###
title = "LTC2983 - channel 2 - rejection 50 Hz - 5 cm cable - ground floor"
channel = "CH2"
###

timestr = time.strftime("%Y%m%d-%H%M%S")
filename = str("LTC2983-" + channel + "-" + timestr + ".root")

f = TFile(filename, "recreate")
ntuple = TNtuple("nt", "samples", 's')
#c1 = TCanvas("c1","LTC2983 canvas",200,10,700,500)

#ch2plot = TH1F(channel, title, 20, -5, 5)
ch2plot = TH1F(channel, title, 1600, -400, 400)
ch2plot.GetXaxis().SetTitle("uV")
ch2plot.SetOption("B")
ch2plot.SetFillColor(46)

def terminate(signum, frame):
    print("Bye !")
    f.Write()
    sys.exit(0)

if __name__ == '__main__':

    signal.signal(signal.SIGINT, terminate)
    signal.signal(signal.SIGTERM, terminate)
    signal.signal(signal.SIGHUP, terminate)

    serv = THttpServer("http:8080")
    serv.SetReadOnly(False)
    serv.SetJSROOT("https://root.cern.ch/js/latest/")

    context = zmq.Context()
    socket = context.socket(zmq.SUB)

    print("Collecting updates from LTC2983 publisher")
    socket.connect("tcp://usop01:5556")
    socket.setsockopt(zmq.SUBSCRIBE, '')

    print("Storing data on: " + filename)

    nsamples = 0

    while True:
        msg = socket.recv().split('\0')[0]
        fval = float(msg)
        ntuple.Fill(fval)
        ch2plot.Fill(fval)

        nsamples = nsamples + 1

        if (nsamples % 1000) == 0:
            f.Write()

        if (nsamples == 5000):
            print("Done !")
            f.Write()
            sys.exit(0)

        #ch2plot.Draw()
        #c1.Update()
