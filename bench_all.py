import os

exec_fname = {"rtm" : "./rtmbuild/bench", 
              "normal" : "./build/bench",
              "rtmstore" : "./rtm_txn_build/bench",
              "rtmstore" : "./rtm_store_build/bench"}

log_fname = {"rtm" : "rtm", 
              "normal" : "normal",
              "rtmtxn" : "rtm",
              "rtmstore" : "normal"}

def gen_log_prefix(sys_type, clients, use_hotspot, hotspot_size):
    if use_hotspot == True:
        return "logs/{}_clients={}_hot_size={}".format(sys_type, clients, hotspot_size)
    else:
        return "logs/{}_clients={}_nohot".format(sys_type, clients)


def gen_cmd(sys_type, clients, use_hotspot, hotspot_size):
    log_prefix = gen_log_prefix(sys_type, clients, use_hotspot, hotspot_size)
    if use_hotspot == False:
        return "{0} -c {1} 2>{2}_tp.log; mv {3}.log {2}_histo.log".format(exec_fname[sys_type], clients, log_prefix, log_fname[sys_type])
    else:
        return "{0} -c {1} -h -s {4} 2>{2}_tp.log; mv {3}.log {2}_histo.log".format(exec_fname[sys_type], clients, log_prefix, log_fname[sys_type], hotspot_size)
    

def run_one_config(clients, use_hotspot, hotspot_size):
    # print(gen_cmd("rtm", clients, use_hotspot, hotspot_size))
    # os.system(gen_cmd("rtm", clients, use_hotspot, hotspot_size))
    # print(gen_cmd("normal", clients, use_hotspot, hotspot_size))
    # os.system(gen_cmd("normal", clients, use_hotspot, hotspot_size))
    print(gen_cmd("rtmstore", clients, use_hotspot, hotspot_size))
    os.system(gen_cmd("rtmstore", clients, use_hotspot, hotspot_size))
    
if __name__ == "__main__":
    # for clients in [1] + [c for c in range(2, 8, 2)] + [c for c in range(8, 49, 4)]:
    #     # run_one_config(clients, False, 200)
    #     run_one_config(clients, True, 2000)
    #     run_one_config(clients, True, 20000)
    # for hs in [200, 400, 600, 800, 2000, 4000, 6000, 8000, 20000]:
    # for hs in [12000, 16000]:
    #     run_one_config(40, True, hs)
    for clients in [1] + [c for c in range(2, 8, 2)] + [c for c in range(8, 49, 4)]:
        run_one_config(clients, False, 200)
