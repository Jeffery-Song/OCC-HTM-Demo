import os
def plot(fname, xname, yname, logx = False, logy = False, lines=2):
    f = open("dat/plots/"+fname+".plot", "w")
    print("set term svg size 500,300", file=f)
    print("set output \"dat/svgs/{}.svg\"".format(fname), file=f)
    # print("set multiplot layout {},{}".format(self.gen_layout()[0], self.gen_layout()[1]), file=f)
    # print("set title \"{}\"".format(self.img_title), file=f)
    # print("show title", file=f)
    # print("set xtics 50", file=f)
    # print("set ylabel \"\"", file=f)
    # print("set xtics rotate by -45", file=f)
    if logx is not False:
        print("set logscale x {}".format(logx), file=f)
    if logy is not False :
        print("set logscale y {}".format(logy), file=f)

    print("", file=f)
    print("set ylabel \"{}\"".format(yname), file=f)
    print("set xlabel \"{}\"".format(xname), file=f)
    if lines == 2:
        print("plot \"dat/{}.dat\" using 1:2  w lp pt 4 title columnheader(2),\\".format(fname), file=f)
        print("     \"dat/{}.dat\" using 1:3  w lp pt 8 title columnheader(3)".format(fname), file=f)
    elif lines == 3:
        print("plot \"dat/{}.dat\" using 1:2  w lp pt 4  title columnheader(2),\\".format(fname), file=f)
        print("     \"dat/{}.dat\" using 1:3  w lp pt 8  title columnheader(3),\\".format(fname), file=f)
        print("     \"dat/{}.dat\" using 1:4  w lp pt 10 title columnheader(4)".format(fname), file=f)
    # todo: key
    f.close()
    os.system("gnuplot dat/plots/{}.plot".format(fname))

if __name__ == "__main__":
    # plot("dat/hot_tp_client=16", "hotspot size", "Throughput")
    plot("hot20000_tp_client", "Client", "Throughput")
    plot("hot20000_occ_abort_client", "Client", "Abort Rate(%)")
    plot("hot20000_htm_abort_lock_client", "Client", "Rate(%)", logy=10)

    plot("hot2000_tp_client", "Client", "Throughput")
    plot("hot2000_occ_abort_client", "Client", "Abort Rate(%)")
    plot("hot2000_htm_abort_lock_client", "Client", "Rate(%)", logy=10)
    
    plot("nohot_tp_client", "Client", "Throughput", lines=3)
    plot("nohot_occ_abort_client", "Client", "Abort Rate(%)", lines=3)
    plot("nohot_htm_abort_lock_client", "Client", "Rate(%)", logy=10)

    plot("client_40_occ_abort_hotsize", "Hot Size", "Abort Rate(%)", logx=10)
    plot("client_40_tp_hotsize", "Hot Size", "Throughput", logx = 10)
    plot("client_40_htm_abort_lock_hotsize", "Hot Size", "Rate(%)", logx=10, logy=10)