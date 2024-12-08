import os
import sys
import yaml
import jinja2
import copy
from jinja2 import Environment, FileSystemLoader

from typing import List, Dict, Tuple

from iinfo import load_inst_info, get_mirgen_insts
from isel import load_isel_info, parse_isel_item
from ische import loadInstScheduleInfo

class Target:
    # gen_data_yml: str
    # isa_data_yml: str
    # output_dir: str
    target_name = ""
    # inst info
    inst_name_list = list()
    inst_info_dict = dict()
    # isel info
    isel_item_list = list()
    isel_dict = dict()

    # MIR Generic Inst Info
    generic_insts_dict = dict()
    generic_insts_list = list()

    # branch list
    branch_list = list()

    def __init__(self, gen_data_yml=str, isa_data_yml=str, output_dir=str):
        self.gen_data_yml = gen_data_yml
        self.isa_data_yml = isa_data_yml
        self.output_dir = output_dir

        # 1. Load isa_data_yml, gen isa target inst info decl/impl files
        self.target_name, self.inst_info_dict, self.inst_name_list, self.branch_list = load_inst_info(
            self.isa_data_yml
        )
        # 2.1 Load MIR Generic Inst Info, for isel
        self.generic_insts_dict, self.generic_insts_list = get_mirgen_insts(
            self.gen_data_yml
        )
        if self.target_name.lower() == "generic":
            self.isel_item_list = []
            self.isel_dict = {}
            self.models = {}
            return 
        # 2.2 Load ISA Inst Selection Info and Parse ISel Items
        self.isel_item_list = load_isel_info(isa_data_yml)

        for isel_item in self.isel_item_list:
            if isel_item["pattern"]["name"] == "InstAdd":
                print("InstAdd")
            res = parse_isel_item(isel_item, self.generic_insts_dict, self.inst_info_dict)
            if not res:
                continue
            if isel_item["match_inst_name"] not in self.isel_dict:
                self.isel_dict[isel_item["match_inst_name"]] = []
            self.isel_dict[isel_item["match_inst_name"]].append(isel_item)

        # 3. Load ISA Schedule Info
        self.models = loadInstScheduleInfo(isa_data_yml)
    # for []
    def __getitem__(self, key):
        return getattr(self, key, None)

    # for dict()
    def __iter__(self):
        self._keys = list(self.__dict__.keys())
        self._index = 0
        return self

    def __next__(self):
        if self._index < len(self._keys):
            key = self._keys[self._index]
            self._index += 1
            return key, getattr(self, key)
        else:
            raise StopIteration
