#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Signal Replay
# Generated: Wed Jun  7 16:44:38 2017
##################################################

from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import osmosdr
import time


class signal_replay(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "Signal Replay")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 12e6
        usage = "usage: %prog [options] min_freq max_freq"
        parser = OptionParser(option_class=eng_option, usage=usage)
        (options, args) = parser.parse_args()
        ##################################################
        # Blocks
        ##################################################
        self.osmosdr_sink_0 = osmosdr.sink( args="numchan=" + str(1) + " " + "" )
        self.osmosdr_sink_0.set_sample_rate(samp_rate)
        self.osmosdr_sink_0.set_center_freq(921e6, 0)
        self.osmosdr_sink_0.set_freq_corr(0, 0)
        self.osmosdr_sink_0.set_gain(20, 0)
        self.osmosdr_sink_0.set_if_gain(20, 0)
        self.osmosdr_sink_0.set_bb_gain(20, 0)
        self.osmosdr_sink_0.set_antenna("", 0)
        self.osmosdr_sink_0.set_bandwidth(0, 0)
          
        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_gr_complex*1, args[0], False)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.blocks_file_source_0, 0), (self.osmosdr_sink_0, 0))    

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.osmosdr_sink_0.set_sample_rate(self.samp_rate)


def main(top_block_cls=signal_replay, options=None):

    tb = top_block_cls()
    tb.start()
    #try:
    #    raw_input('Press Enter to quit: ')
    #except EOFError:
    #    pass
    #tb.stop()
    tb.wait()


if __name__ == '__main__':
    main()
