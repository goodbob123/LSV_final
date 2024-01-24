import sys

import warnings
import gymnasium as gym
from gymnasium.envs.registration import register

import wandb
from wandb.integration.sb3 import WandbCallback

from stable_baselines3.common.monitor import Monitor
from stable_baselines3.common.vec_env import DummyVecEnv, VecVideoRecorder
from stable_baselines3 import A2C, DDPG, DQN, PPO, SAC, TD3

import numpy as np

warnings.filterwarnings("ignore")
register(
    id='cktRL',   # id, used by make
    entry_point='envs:CircuitRL', # correspond to ./envs/ with class My2048Env
)

# Set hyper params (configurations) for training
my_config = {
    "run_id": "example",

    "algorithm": PPO,
    "policy_network": "MultiInputPolicy",

    "obs_size": None, # should be (#PI, #PO, k-cut)
    "train_path": None,
    "tolerance": "1.0",
    "assisted": True,
    "save_path": "models/sample_model",

    "load_model": None,
    "epoch_num": -1,
    "timesteps_per_epoch": 100,
    "eval_episode_num": 100,

    "makeckt":False,
}

# node = np.array([[False,False,False,False,False,False,True,True],[False,False,False,False ,True,False ,True,False],[False,False,False,False ,True,False,False,False],[True ,True, True, True,False, True,False,False]])
# goal = np.array([[False,False,False,False, True,False, True, True]])
def make_env():
    env = gym.make('cktRL', config = my_config) #, n = 3, goal = goal, node = node
    return env

def train(env, model, config):

    current_best = 0
    epoch = 0
    while(True):
        if (epoch == config["epoch_num"]): break
        ### Train agent using SB3
        # Uncomment to enable wandb logging
        model.learn(
            total_timesteps=config["timesteps_per_epoch"],
            reset_num_timesteps=False,
            # callback=WandbCallback(
            #     gradient_save_freq=100,
            #     verbose=2,
            # ),
        )

        ### Evaluation
        # print(config["run_id"])
        avg_score = 0
        for seed in range(config["eval_episode_num"]):
            done = False

            # Set seed using old Gym API
            env.seed(seed)
            obs = env.reset()

            # Interact with env using old Gym API
            while not done:
                action, _state = model.predict(obs, deterministic=True)
                obs, reward, done, info = env.step(action)
                # print(obs['cone'])
                # print(done)
            
            avg_score += info[0]['score']/config["eval_episode_num"]
        env.eval = False
        with open ("./log.txt", "a") as f:
            f.write(f"Epoch: {epoch} / Avg_score: {avg_score}\n")
            if current_best < avg_score:
                f.write("Saving Model\n")
                f.write(">>>>\n")
        # wandb.log(
        #     {
        #      "avg_score": avg_score}
        # )
        

        ### Save best model
        if current_best < avg_score:
            current_best = avg_score
            save_path = config["save_path"]
            model.save(f"{save_path}/model")

        epoch += 1

def makeckt(env, model, config):
    obs = env.reset()
    nPI, nPO, cone_size = config["obs_size"]
    match_node = 0
    record_node = np.zeros((cone_size,))
    record_aig = []
    record_PO = []
    for i in range(nPI):
        record_node[i] = i + 1
    i = nPI
    done = False
    while not done:
        action, _state = model.predict(obs, deterministic=True)
        id_0 = action[0][0]
        inv_0 = action[0][1]
        id_1 = action[0][2]
        inv_1 = action[0][3]
        id_dump = action[0][4]

        f1 = record_node[id_0]
        f2 = record_node[id_1]
        assert(f1 != 0 and f2 != 0)
        record_aig.append([2 * i, 2 * id_0 + inv_0, 2 * id_1 + inv_1])
        record_node[id_dump] = i

        obs, reward, done, info = env.step(action)
        print(obs)
        print(action)
        if (reward != 0): match_node+=1
        if (info[0]["find_PO"]):
            if (info[0]["is_inv"]): record_PO.append(2 * i + 1)
            else: record_PO.append(2 * i)
        i += 1
    i-=1
    print("result score is:", info[0]['score'])
    if (len(record_PO) == 0): 
        print("the circuit has no output is currect")
        print("#matchnode", match_node)
    else: 
        print("there is some correct output")
        print("#matchnode", match_node)

    with open(config["train_path"] + "/result.aag", 'w') as f:
        f.write(f"aag {i - 1} {nPI} 0 {len(record_PO)} {i - 1 - nPI - len(record_PO)}\n") # M = i - 1, I = nPI, L = 0, O = nPO, A = M - I - O
        for pi in range(nPI):
            f.write(f"{2 * pi + 2}\n")
        for po in record_PO:
            f.write(f"{po}\n")
        for aig in record_aig:
            f.write(f"{aig[0]} {aig[1]} {aig[2]}\n")
    
if __name__ == "__main__":

    # Create wandb session (Uncomment to enable wandb logging)
    # run = wandb.init(
    #     project="assignment_3",
    #     config=my_config,
    #     sync_tensorboard=True,  # auto-upload sb3's tensorboard metrics
    #     id=my_config["run_id"]
    # )

    now_arg = ""
    # print(sys.argv)
    for i, arg in enumerate(sys.argv):
        if i == 0: pass
        elif i == 1: 
            my_config["train_path"] = arg
        else:
            if (arg == "-a"): now_arg = "a"
            elif (arg == "-t"): 
                now_arg = "t"
            elif (arg == "-l"): now_arg = "l"
            elif (arg == "-m"): my_config["makeckt"] = True
            else:
                if (arg == "T" and now_arg == "a"): my_config["assisted"] = True
                if (arg == "F" and now_arg == "a"): my_config["assisted"] = False
                if (arg.replace('.','',1).isdigit() and now_arg == "t"): 
                    my_config["tolerance"] = arg
                if (now_arg == "l"): my_config["load_model"] = arg
                now_arg = ""

    with open (my_config["train_path"] + "/setting", 'r') as f:
        setting = f.readline()
        setting = setting.split(" ")
        assert(len(setting) == 3)
        my_config["obs_size"] = (int(setting[0]),int(setting[1]),int(setting[2]))
    # print(my_config)
    env = DummyVecEnv([make_env])

    # Create model from loaded config and train
    # Note: Set verbose to 0 if you don't want info messages
    model = my_config["algorithm"](
        my_config["policy_network"], 
        env, 
        verbose=0,
        # tensorboard_log=my_config["run_id"]
    )
    save_path = my_config["save_path"]
    if (my_config["load_model"] and my_config["load_model"][-4:] == ".zip"):
        # print("in")
        my_config["load_model"] = my_config["load_model"][:-4]
        model = PPO.load(my_config["load_model"])
        model.set_env(env)
    
    if (my_config["makeckt"]):
        assert(my_config["load_model"])
        makeckt(env, model, my_config)
    else:
        train(env, model, my_config)