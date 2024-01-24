from __future__ import print_function

import gymnasium as gym
from gymnasium import spaces
from gymnasium.utils import seeding

import numpy as np
import math
from PIL import Image, ImageDraw, ImageFont

import itertools
import logging
from six import StringIO
import sys
import os

class IllegalMove(Exception):
    pass
class CircuitRL(gym.Env):
    metadata = {
        "render_modes": ['ansi', 'human', 'rgb_array'],
        "render_fps": 2,
    }

    def __init__(self, config):
        assert(config["train_path"])
        self.train_file = []
        for file in os.listdir(config["train_path"]):
            if file[-3:] == "txt": self.train_file.append(config["train_path"] + "/" + file)
        self.train_set = self.read_train_file(self.train_file)

        assert(config["obs_size"])
        assert(len(config["obs_size"]) == 3)
        self.nPI, self.nPO, self.cone_size = config["obs_size"]

        # self.goal = np.array([[1,0,1,0,1,1,1,1,0,1,0,1,0,0,0,0],
        #                       [0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0]])
        # # self.node = np.array([[0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1],
        # #                       [1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0],
        # #                       [0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0],
        # #                       [0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0],
        # #                       [0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0],
        # #                       [1,1,1,1,1,1,1,1,0,1,0,1,0,0,0,0],
        # #                       [1,0,1,0,1,1,1,1,0,1,0,1,0,0,0,0],
        # #                       [0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0],
        # #                       [0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0]])
        # self.node = np.array([[1,0,1,0,1,1,1,1,0,1,0,1,0,0,0,0],
        #                       [0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0]])
        self.tolerance = config["tolerance"]
        self.is_assisted = config["assisted"]
        self.seed()
        
        self.action_space = spaces.MultiDiscrete([self.cone_size,2,self.cone_size,2,self.cone_size],dtype = int)
        self.observation_space = spaces.Dict({'cone':spaces.Box(low=0, high=1, shape=(self.cone_size, 2**self.nPI), dtype=bool),
                                              'goal':spaces.Box(low=0, high=1, shape=(self.nPO, 2**self.nPI), dtype=bool)})
        # # Reset ready for a game
        self.set_illegal_move_reward(-1.)
        self.eval = False
        self.reset()
        # goal, node, bias, num_step, step
    def read_train_file(self, train_files):
        train_set = []
        for path in train_files:
            with open(path, 'r') as f:
                done_read_node = False
                train_data = {"node" : [], "goal" : []}
                for line in f.readlines():
                    if line[0:2] == "po": 
                        done_read_node = True
                        continue
                    elif line[0] == "0" or line[0] == "1":
                        tt_aig_str = line.split(",")
                        tt_aig = []
                        for i in tt_aig_str:
                            if i.isnumeric(): tt_aig.append(int(i))
                            assert(0 <= tt_aig[-1] <= 1)
                        if done_read_node:
                            train_data["goal"].append(tt_aig)
                        else:
                            train_data["node"].append(tt_aig)
                train_data["goal"] = np.array(train_data["goal"], dtype=bool)
                train_data["node"] = np.array(train_data["node"], dtype=bool)
                train_set.append(train_data)
        return train_set
    def set_illegal_move_reward(self, reward):
        """Define the reward/penalty for performing an illegal move. Also need
            to update the reward range for this."""
        self.illegal_move_reward = reward
        self.reward_range = (self.illegal_move_reward, self.nPO + 1)

    def get_obs(self):
        obs = dict()
        obs['cone'] = self.Cone
        obs['goal'] = self.goal
        return obs

    def seed(self, seed=None):
        self.np_random, seed = seeding.np_random(seed)
        return [seed]

    # Implement gym interface
    def step(self, action):
        logging.debug("Action {}".format(action))
        id_0 = action[0]
        inv_0 = action[1]
        id_1 = action[2]
        inv_1 = action[3]
        id_dump = action[4]
        score = 0
        done = None
        info = {
            'find_PO': False,
            'is_inv': False,
            'illegal_move': False,
            'score': 0,
        }
        try:
            # print(action)
            fi_0 = self.Cone[id_0] if inv_0 == 0 else ~self.Cone[id_0]

            fi_1 = self.Cone[id_1] if inv_1 == 0 else ~self.Cone[id_1]

            fo = fi_0 & fi_1

            if (np.all(fo) or not np.any(fo)):
                logging.debug("compute-const")
                raise IllegalMove
            if (np.any(np.all(self.Cone ^ ~fo, axis = 1) | np.all(self.Cone ^ fo, axis = 1))):
                logging.debug("re-compute")
                # print(self.Cone)
                # print(fo)
                raise IllegalMove
            if (self.nAig < self.cone_size - self.nPI):
                # print(self.Cone[id_dump])
                if (np.any(self.Cone[id_dump])):
                    logging.debug("waste-mem")
                    raise IllegalMove

            self.Cone[id_dump] = fo



            is_node = np.all(self.node ^ ~fo ,axis = 1) | np.all(self.node ^ fo,axis = 1)
            node_id = np.argwhere(is_node == True)
            for id in node_id:
                if (self.is_node[id[0]] == False):
                    score += (len(self.goal) * self.num_step)**(-1)
                    self.is_node[id[0]] = True
                else:
                    logging.debug("re-compute")
                    raise IllegalMove

            is_goal = np.all(self.goal ^ ~fo,axis = 1) | np.all(self.goal ^ fo,axis = 1)
            goal_id = np.argwhere(is_goal == True)
            for id in goal_id:
                if (self.is_goal[id[0]] == False):
                    info["find_PO"] = True
                    if (np.any(np.all(self.goal ^ ~fo,axis = 1))):
                        info["is_inv"] = False
                    else:
                        info["is_inv"] = True
                    score += (2.0 - self.nAig/self.num_step)
                    self.is_goal[id[0]] = True
                    self.goal[id[0]] = np.full((2**self.nPI,),False)
                else:
                    logging.debug("re-compute")
                    raise IllegalMove
            self.score += score
            self.nAig += 1
            done = (self.nAig >= self.num_step) or np.all(self.is_goal)
            reward = float(score)
            # if (done):
            #     print("done")
            #     print(self.num_step)
            #     print(self.nAig)
            #     print(self.is_goal)
            #     print(score)
            #     print(reward)
        except IllegalMove:
            logging.debug("Illegal move")
            info['illegal_move'] = True
            reward = self.illegal_move_reward
            # TODO: Modify this part for the agent to have a chance to explore other actions (optional)
            done = True
        truncate = False
        info['score']   = self.score
        # obs = np.concatenate((self.Cone,self.goal),axis = 0)
        
        # Return observation (board state), reward, done, truncate and info dict
        return self.get_obs(), reward, done, truncate, info

    def reset(self, seed=None, options=None):
        self.seed(seed=seed)
        # goal, node, bias, num_step, step
        data_id = np.random.randint(0,len(self.train_set))
        self.node = self.train_set[data_id]["node"]
        self.goal = self.train_set[data_id]["goal"]
        # print(self.goal)
        if ("." in self.tolerance):
            self.num_step = int(float(self.tolerance) * (len(self.goal) + len(self.node)))
        else:
            self.num_step = len(self.goal) + len(self.node) + int(self.tolerance)
        # print(self.num_step)
        # print(len(self.goal))
        # print(len(self.node))

        self.nAig = 0
        self.score = 0
        self.is_goal = np.full((len(self.goal),),False)
        self.is_node = np.full((len(self.node),),False)
        self.Cone = np.full((self.cone_size, 2**self.nPI), False)
        for mt in range(2**self.nPI):
            mt_ = mt
            j = 0
            while (mt_ != 0):
                if (mt_%2): self.Cone[self.nPI - j - 1][mt] = True
                # else: self.x[mt][j] = 0.0
                j = j+1
                mt_ = mt_//2
        self.score = 0
        self.nAig = 0
        # obs = np.concatenate((self.Cone,self.goal),axis = 0)
        return self.get_obs(),{}

    def render(self, mode='human'):
        outfile = StringIO() if mode == 'ansi' else sys.stdout
        outfile.write("")
        s = 'Score: {}\n'.format(self.score)
        npa = np.array(self.Cone)
        s += "{}\n".format(npa)
        outfile.write(s)
        return outfile

    def get(self, x, y):
        """Return the value of one square."""
        return self.Cone[x, y]

    def set(self, x, y, val):
        """Set the value of one square."""
        self.Cone[x, y] = val

    def get_board(self):
        """Retrieve the whole board, useful for testing."""
        return self.Cone

    def set_board(self, new_board):
        """Retrieve the whole board, useful for testing."""
        self.Cone = new_board

# if __name__ == '__main__':
#     M = CircuitRL()
#     M.reset()
#     action = np.array([[9,1,7,0,3]])
#     print(action)
#     M.step(action)