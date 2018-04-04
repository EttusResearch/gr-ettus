#!/usr/bin/env python2
import time
import datetime
import threading

class rfnoc_register_probe():
    def __init__(self, looptime=1.0, enable=True, csvout=None, console=False):
        self.looptime = looptime
        self.csvout   = csvout
        self.console  = console
        self.queries  = []
        self.running  = False
        self.halt     = False
        self.file     = None
        if enable:
            if self.console:
                print("\nInitialized RFNoC Probe...")

            # TODO: Figure out how to call start after adding queries?
            self.start()
        else:
            # not enabled
            pass

    def add_query(self, nocblock, registers):
        newquery = {"block": nocblock, "registers": registers}
        self.queries.append(newquery)

        # Overwrite the old CSV...
        # TODO: figure out how to add blocks without overwriting current CSV
        self.opencsv()

    def query(self):
        # Loop through the queries and get register values
        results = []
        for probe in self.queries:
            block = probe["block"]
            regs  = probe["registers"]
            for reg in regs:
                val = block.get_register(reg)
                results.append((block.get_block_id(), reg, val))
        return results

    def opencsv(self):
        # Re-open the CSV
        if self.file:
            self.file.close()
        self.file = None
        if self.csvout:
            self.file = open(self.csvout, "w")
            strnames = []
            for probe in self.queries:
                block = probe["block"]
                regs  = probe["registers"]
                for reg in regs:
                    strnames.append("%s/%s" % (block.get_block_id(), reg))
            csvheader = ",".join(["time"]+strnames)
            self.file.write("%s\n" % (csvheader))

    def start(self):
        # self.opencsv()
        self.halt = False
        self.running = True
        self.runthread = threading.Thread(target=self.runloop)
        self.runthread.daemon = True
        self.runthread.start()

    def runloop(self):
        print("\nStarting RFNoC probe loop")
        while not self.halt:
            try:
                # Sleep for desired looptime
                time.sleep(self.looptime)

                # Query RFNoC registers
                results = self.query()
                values = [v for (_, _, v) in results]

                # Debug output
                if self.console:
                    if results:
                        print("\nQuerying RFNoC blocks...")
                    for block, reg, val in results:
                        print("%s/%s: %s" % (block, str(reg), str(val)))

                # File output
                if self.file:
                    newline = ",".join([str(datetime.datetime.now())]+[str(val) for val in values])
                    self.file.write("%s\n" % (newline))
            except Exception as e:
                print("\nCaught error in rfnoc_register_logger.runloop(): %s" % (e))
                print(e)
                break
        self.running = False

    def shutdown(self):
        if self.running:
            print("Ending RFNoC probe loop")
        self.halt = True
        while self.running:
            if self.console:
                print("Waiting for RFNoC probe to shutdown...")
            time.sleep(self.looptime)
        if self.file:
            self.file.close()

    def __del__(self):
        self.shutdown()
