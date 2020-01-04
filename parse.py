import os

exec_fname = {"rtm" : "./rtmbuild/bench", "normal" : "./build/bench"}

def get_log_prefix(sys_type, clients, use_hotspot, hotspot_size):
    if use_hotspot == True:
        return "logs/{}_clients={}_hot_size={}".format(sys_type, clients, hotspot_size)
    else:
        return "logs/{}_clients={}_nohot".format(sys_type, clients)

def get_tp_log_fname(sys_type, clients, use_hotspot, hotspot_size):
    return get_log_prefix(sys_type, clients, use_hotspot, hotspot_size)+"_tp.log"
def get_histo_log_fname(sys_type, clients, use_hotspot, hotspot_size):
    return get_log_prefix(sys_type, clients, use_hotspot, hotspot_size)+"_histo.log"

def parse_tp(fname):
    f = open(fname)
    lines = f.readlines()[2:5]
    def extract_from_line(line):
        return float(line.split(" ")[4])
    return (extract_from_line(lines[0]) + extract_from_line(lines[1]) + extract_from_line(lines[2])) / 3

def parse_total(fname):
    f = open(fname)
    line = f.readlines()[-1]
    return int(line.split(" ")[1]) + int(line.split(" ")[4])

def gen_total_histo_of(infname, outfname):
    # f = open(fname)
    # lines = f.readlines()[2:5]
    # for line
    os.system("tail -10001 {} > {}".format(infname, outfname))

def parse_rate_from_tp(fname):
    f = open(fname)
    line = f.readlines()[-1]
    total_txns = int(line.split(" ")[1])
    occ_aborts = int(line.split(" ")[4])
    htm_aborts = int(line.split(" ")[7])
    htm_use_lock = int(line.split(" ")[10])
    return float(occ_aborts*100)/(total_txns + occ_aborts), \
           float(htm_aborts*100)/(2.4*(total_txns + occ_aborts) + htm_aborts + htm_use_lock), \
           float(htm_use_lock*100)/(2.4*(total_txns + occ_aborts) + htm_aborts + htm_use_lock)

def parse_rate_from_histo(fname):
    os.system("tail -10001 {} > tmp.log".format(fname))
    f = open("tmp.log")
    total_txns = 0
    occ_aborts = 0
    htm_aborts = 0
    htm_use_lock = 0
    while True:
        line = f.readline()
        if not line:
            break
        total_txns += int(line.split(",")[1])
        occ_aborts += int(line.split(",")[2])
        htm_aborts += int(line.split(",")[3])
        htm_use_lock += int(line.split(",")[4])
    return float(occ_aborts*100)/(total_txns + occ_aborts), \
           float(htm_aborts*100)/(2.4*(total_txns + occ_aborts) + htm_aborts + htm_use_lock), \
           float(htm_use_lock*100)/(2.4*(total_txns + occ_aborts) + htm_aborts + htm_use_lock)

def gen_dat_total_x_client(client_range, hot, hot_size):
    if hot is True:
        f = open("dat/hot{}_total_client.dat".format(hot_size), "w")
    else:
        f = open("dat/nohot_total_client.dat", "w")
    print("clients\tNormal\tHTM\tHTMStore", file=f)
    for clients in client_range:
        normal_tp = parse_total(get_tp_log_fname("normal", clients, hot, hot_size))
        rtm_tp = parse_total(get_tp_log_fname("rtm", clients, hot, hot_size))
        rtmstore_tp = parse_total(get_tp_log_fname("rtmstore", clients, hot, hot_size))
        print("{}\t{}\t{}\t{}".format(clients, normal_tp, rtm_tp, rtmstore_tp), file=f)
    f.close()

def gen_dat_tp_x_client(client_range, hot, hot_size):
    if hot is True:
        f = open("dat/hot{}_tp_client.dat".format(hot_size), "w")
    else:
        f = open("dat/nohot_tp_client.dat", "w")
    print("clients\tNormal\tHTM\tHTMStore", file=f)
    for clients in client_range:
        normal_tp = parse_tp(get_tp_log_fname("normal", clients, hot, hot_size))
        rtm_tp = parse_tp(get_tp_log_fname("rtm", clients, hot, hot_size))
        rtmstore_tp = parse_tp(get_tp_log_fname("rtmstore", clients, hot, hot_size))
        print("{}\t{}\t{}\t{}".format(clients, normal_tp, rtm_tp, rtmstore_tp), file=f)
    f.close()

def gen_dat_tp_x_hot_size(client, hot_size_range):
    f = open("dat/client_{}_tp_hotsize.dat".format(client), "w")
    print("\"Hotspot Size\"\tNormal\tHTM", file=f)
    for hs in hot_size_range:
        normal_tp = parse_tp(get_tp_log_fname("normal", client, True, hs))
        rtm_tp = parse_tp(get_tp_log_fname("rtm", client, True, hs))
        print("{}\t{}\t{}".format(hs, normal_tp, rtm_tp), file=f)
    f.close()

def gen_dat_tp_x_hot_size(client, hot_size_range):
    f = open("dat/client_{}_total_hotsize.dat".format(client), "w")
    print("\"Hotspot Size\"\tNormal\tHTM", file=f)
    for hs in hot_size_range:
        normal_tp = parse_total(get_tp_log_fname("normal", client, True, hs))
        rtm_tp = parse_total(get_tp_log_fname("rtm", client, True, hs))
        print("{}\t{}\t{}".format(hs, normal_tp, rtm_tp), file=f)
    f.close()

def gen_dat_occ_abort_rate_x_client(client_range, hot, hot_size):
    if hot is True:
        f = open("dat/hot{}_occ_abort_client.dat".format(hot_size), "w")
    else:
        f = open("dat/nohot_occ_abort_client.dat", "w")
    print("clients\tNormal\tHTM\tHTMStore", file=f)
    for clients in client_range:
        normal_abort, _, _ = parse_rate_from_tp(get_tp_log_fname("normal", clients, hot, hot_size))
        rtm_abort, _, _ = parse_rate_from_tp(get_tp_log_fname("rtm", clients, hot, hot_size))
        rtmstore_abort, _, _ = parse_rate_from_tp(get_tp_log_fname("rtmstore", clients, hot, hot_size))
        print("{}\t{}\t{}\t{}".format(clients, normal_abort, rtm_abort, rtmstore_abort), file=f)
    f.close()

def gen_dat_occ_abort_rate_x_hot_size(client, hot_size_range):
    f = open("dat/client_{}_occ_abort_hotsize.dat".format(client), "w")
    print("\"Hotspot Size\"\tNormal\tHTM", file=f)
    for hs in hot_size_range:
        normal_abort, _, _ = parse_rate_from_tp(get_tp_log_fname("normal", client, True, hs))
        rtm_abort, _, _ = parse_rate_from_tp(get_tp_log_fname("rtm", client, True, hs))
        print("{}\t{}\t{}".format(hs, normal_abort, rtm_abort), file=f)
    f.close()

def gen_dat_htm_abort_lock_rate_x_client(client_range, hot, hot_size):
    if hot is True:
        f = open("dat/hot{}_htm_abort_lock_client.dat".format(hot_size), "w")
    else:
        f = open("dat/nohot_htm_abort_lock_client.dat", "w")
    print("Clients\t\"HTM Abort\"\t\"HTM Lock\"", file=f)
    for clients in client_range:
        _, htm_abort, htm_lock = parse_rate_from_tp(get_tp_log_fname("rtm", clients, hot, hot_size))
        # _, htm_abort, htm_lock = parse_rate_from_histo(get_histo_log_fname("rtm", clients, hot, hot_size))
        print("{}\t{}\t{}".format(clients, htm_abort, htm_lock), file=f)
    f.close()

def gen_dat_htm_abort_lock_rate_x_hot_size(client, hot_size_range):
    f = open("dat/client_{}_htm_abort_lock_hotsize.dat".format(client), "w")
    print("\"Hotspot Size\"\t\"HTM Abort\"\t\"HTM Lock\"", file=f)
    for hs in hot_size_range:
        _, htm_abort, htm_lock = parse_rate_from_tp(get_tp_log_fname("rtm", client, True, hs))
        # _, htm_abort, htm_lock = parse_rate_from_histo(get_histo_log_fname("rtm", client, True, hs))
        print("{}\t{}\t{}".format(hs, htm_abort, htm_lock), file=f)
    f.close()

if __name__ == "__main__":
    client_range = [1] + [c for c in range(2, 8, 2)] + [c for c in range(8, 49, 4)]
    hotsize_range = [200, 400, 600, 800, 2000, 4000, 6000, 8000, 12000, 16000, 20000]

    gen_dat_tp_x_client(client_range, False, 0)
    gen_dat_total_x_client(client_range, False, 0)
    gen_dat_occ_abort_rate_x_client(client_range, False, 0)
    gen_dat_htm_abort_lock_rate_x_client(client_range, False, 0)

    # gen_dat_tp_x_client(client_range, True, 2000)
    # gen_dat_total_x_client(client_range, True, 2000)
    # gen_dat_occ_abort_rate_x_client(client_range, True, 2000)
    # gen_dat_htm_abort_lock_rate_x_client(client_range, True, 2000)

    # gen_dat_tp_x_client(client_range, True, 20000)
    # gen_dat_total_x_client(client_range, True, 20000)
    # gen_dat_occ_abort_rate_x_client(client_range, True, 20000)
    # gen_dat_htm_abort_lock_rate_x_client(client_range, True, 20000)

    # gen_dat_tp_x_hot_size(40, hotsize_range)
    # gen_dat_occ_abort_rate_x_hot_size(40, hotsize_range)
    # gen_dat_htm_abort_lock_rate_x_hot_size(40, hotsize_range)


