# -*- coding: iso-8859-1 -*-
#
# LogFile.py --
#
# Authors: Nicolas Roussel
#
# Copyright © Inria

import numpy

class LogBlock(object):

    def __init__(self, lines, refpoint, obspoints):
        for line in lines: line.block = self        
        self.lines = lines
        self.refpoint = refpoint
        self.obspoints = obspoints
        self.lag = {}
        epoch = self.findFirst(refpoint).timestamp
        for obspoint in obspoints:
            try:
                ms = (self.findFirst(obspoint).timestamp-epoch)*1e-6
            except StopIteration:
                ms = numpy.nan
            self.lag[obspoint] = ms
            
    def findFirst(self, obspoint):
        return next(l for l in self.lines if l.obspoint==obspoint)

    def findLast(self, obspoint):
        return next(l for l in reversed(self.lines) if l.obspoint==obspoint)
