# -*- coding: iso-8859-1 -*-
#
# LogFile.py --
#
# Authors: Nicolas Roussel
#
# Copyright � Inria

import sys
import math
import traceback

import numpy
from matplotlib import pyplot

from LogLine import LogLine, ANSI
from LogBlock import LogBlock
from LogStats import LogStats

class LogFile(object):

    def __init__(self, path, start=None, stop=None):
        self.path = path
        print()
        self.config, self.lines = self.__parseLines(path)
        print("%s: %d lines"%(path, len(self.lines)))
        # ----------------------------------------------------------------
        self.refpoint = self.blocks = self.stats = None
        if start==None:
            startstop = [
                ("piezo", "photodiode"),
                ("libpointing", "photodiode"),
                ("system", "photodiode"),
                ("libpointing", "screen"),
                ("libpointing", "repaint"),
                ("system", "screen"),
                ("system", "repaint")
            ]
        else:
            startstop = [
                (start, "photodiode"),
                (start, "screen"),
                (start, "repaint"),
            ]
        maxgoodlines = maxgoodblocks = 0
        for start, stop in startstop:
            try:
                obspoints, blocks, nbbadlines = self.__parseBlocks(self.lines, start, stop)
                nbgoodlines = len(self.lines)-nbbadlines
                nbgoodblocks = len(blocks)
                stats = self.__computeStats(blocks,obspoints+["leonardo-e2e"])
                #if blocks and (self.stats==None or nbgoodlines>maxgoodlines):
                if blocks and (self.stats==None or nbgoodblocks>maxgoodblocks):
                    print(ANSI.BLACK, end="")
                    self.refpoint = start
                    self.blocks = blocks
                    self.stats = stats
                    #maxgoodlines = nbgoodlines
                    maxgoodblocks = nbgoodblocks
                else:
                    print(ANSI.LIGHTGREY, end="")
                print("  %s-%s: %d lines ok (%.1f%%); %d blocks, %d ok for stats"%(start, stop, nbgoodlines, 100.0*nbgoodlines/len(self.lines), len(blocks), len(stats.blocks)), end="")
                print(" (%.1f%%)"%(100.0*len(stats.blocks)/len(blocks)) if blocks else "", end="")
                print(ANSI.ENDC)
            except:
                print("%s  %s-%s%s: parsing failed"%(ANSI.LIGHTGREY, start, stop, ANSI.ENDC))
                traceback.print_exc()
                pass

    # -----------------------------------------------------------------------------------------

    def __parseLines(self, path):
        config, lines = {}, []
        LOST, PARSING_CONFIG, PARSING_LINES  = range(3)
        state = LOST
        xtra = ""
        with open(path) as fd:
            linenumber = 0
            for line in fd:
                tryline = False
                linenumber += 1
                line = (xtra+" "+line).strip()
                if not line: continue
                if state==LOST:
                    if line.startswith("# -- begin config"):
                        state = PARSING_CONFIG
                    else:
                        tryline = True
                elif state==PARSING_CONFIG:
                    if line.startswith("# -- end config"):
                        state = PARSING_LINES
                    else:
                        key, value = line.split(":",1)
                        config[key.strip()] = value.strip()
                elif state==PARSING_LINES:
                    tryline = True
                if tryline:
                    try:
                        logline = LogLine(linenumber, line)
                        xtra = " ".join(logline.xtra)
                        lines.append(logline)
                        state = PARSING_LINES
                    except:
                        print(sys.exc_info()[1])
                        xtra = ""
        return config, lines

    def __parseBlocks(self, lines, start, stop):
        # Adjust the list of obspoints to those actually present
        obspoints = set([l.obspoint for l in lines])
        obspoints = [o for o in LogLine.obspoints if o in obspoints]
        # Ready to parse
        blocks, lost, nbbadlines = [], True, 0
        for line in lines:
            if lost:
                if line.obspoint==start:
                    blocklines = [line]
                    lost = False
                else:
                    nbbadlines += 1
            else:
                blocklines.append(line)
                if line.obspoint==stop:
                    block = LogBlock(blocklines, start, obspoints)
                    try:
                        ms = numpy.nan
                        if "piezo" in obspoints and "photodiode" in block.obspoints:
                            tp = block.findFirst("piezo").event_timestamp
                            td = block.findFirst("photodiode").event_timestamp
                            ms = (td-tp)*1e-6
                    except:
                        pass
                    finally:
                        block.lag["leonardo-e2e"] = ms
                    blocks.append(block)
                    lost = True
        return obspoints, blocks, nbbadlines

    def __computeStats(self, blocks, measures):
        stats = LogStats(blocks, measures)
        okblocks = []
        for block in blocks:
            outlier = False
            for measure in stats.measures:
                # Exclude blocks with at least one lag measure 2.5*mad away from the mean
                # See http://dx.doi.org/10.1016/j.jesp.2013.03.013
                if math.fabs(block.lag[measure]-stats.median[measure])>2.5*stats.mad[measure]:
                    outlier = True
                    break
            if not outlier: okblocks.append(block)
        return LogStats(okblocks, stats.measures)

    # -----------------------------------------------------------------------------------------

    def dump(self):
        print()
        print(self.config)
        print()
        lastblock = None
        for line in self.lines:
            if line.block!=lastblock:
                print()
                try:
                    p = lastblock.findFirst("piezo")
                    d = lastblock.findFirst("photodiode")
                    dpc = (d.timestamp-p.timestamp)*1e-9
                    dleo = (d.event_timestamp-p.event_timestamp)*1e-9
                    print((dpc-dleo)*1e3, dpc, dleo)
                except:
                    pass
            lastblock = line.block
            print(("|" if line.block else " "), line)

    def plot(self, title=None, minlag=None, maxlag=None):
        if minlag==None: minlag = math.floor(self.stats.minvalue)
        if maxlag==None: maxlag = math.ceil(self.stats.maxvalue)
        bins = numpy.linspace(minlag, maxlag, 2*(maxlag-minlag))

        if len(self.stats.measures)>1:
            fig, axarr = pyplot.subplots(len(self.stats.measures), sharex=True)
            if title:
                fig.suptitle(title, x=0.5, y=0.97, fontsize=10, color="grey")
            for i, measure in enumerate(self.stats.measures):
                lagvalues = [v for v in self.stats.values[measure] if v is not numpy.nan]
                weight = "bold" if measure==self.refpoint else "normal"
                color = "orange" if measure=="leonardo-e2e" else "darkblue"
                axarr[i].set_title("%s: %s"%(measure, self.stats.getTitle(measure)),
                                   x=1, fontsize=8, fontweight=weight, horizontalalignment="right")
                axarr[i].hist(lagvalues, bins, color=color)
            pyplot.tight_layout()
            fig.subplots_adjust(top=0.85)
        else:
            fig = pyplot.figure()
            if title: pyplot.title(self.stats.getTitle(measure))
            lagvalues = [v for v in self.stats.values[measure] if v is not numpy.nan]
            pyplot.hist(lagvalues, bins)

        return fig
