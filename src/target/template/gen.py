import os
import sys
import yaml
import jinja2
import copy
from jinja2 import Environment, FileSystemLoader

# import typing
from typing import List, Dict, Tuple

from mytarget import Target
from iinfo import load_inst_info, get_mirgen_insts
from isel import load_isel_info, parse_isel_item


def gen_file_jinja2(template_file, output_dir, params):
    env = jinja2.Environment(loader=FileSystemLoader(os.path.dirname(__file__)))
    template = env.get_template(template_file)
    output_file = os.path.join(
        output_dir, os.path.basename(template_file).replace(".jinja2", "")
    )
    with open(output_file, "w") as f:
        res = template.render(params)
        f.write(res)
    os.system("clang-format -i {}".format(output_file))


"""
InstInfoDecl.hpp.jinja2/InstInfoImpl.hpp.jinja2: 
    target_name, inst_name_list, inst_info_dict
ISelInfoDecl.hpp.jinja2/ISelInfoImpl.hpp.jinja2: 
    target_name, isel_dict, match_insts_list, match_insts_dict
"""

if __name__ == "__main__":
    gen_data_yml = sys.argv[1]  # "generic.yml"
    isa_data_yml = sys.argv[2]  # "riscv.yml"
    output_dir = sys.argv[3]

    tar = Target(gen_data_yml, isa_data_yml, output_dir)
    params = {
        "target_name": tar.target_name,
        "isel_dict": tar.isel_dict,
        "inst_name_list": tar.inst_name_list,
        "inst_info_dict": tar.inst_info_dict,
        "match_insts_list": tar.generic_insts_list,
        "match_insts_dict": tar.generic_insts_dict,
        "branch_list": tar.branch_list,
        "models": tar.models,
    }
    # print(tar.branch_list)
    gen_file_jinja2("InstInfoDecl.hpp.jinja2", output_dir, params)
    gen_file_jinja2("InstInfoImpl.hpp.jinja2", output_dir, params)

    if tar.target_name.lower() == "generic":
        exit(0)
    gen_file_jinja2("ISelInfoDecl.hpp.jinja2", output_dir, params)
    gen_file_jinja2("ISelInfoImpl.hpp.jinja2", output_dir, params)

    gen_file_jinja2("ScheduleModelDecl.hpp.jinja2", output_dir, params)
    gen_file_jinja2("ScheduleModelImpl.hpp.jinja2", output_dir, params)

"""
解析 pattern, replace, 得到 match_list, select_list
input:
isel_info, mirgen_insts_dict
output:
isel_dict: dict(inst_name: str -> isel_item_list: list(isel_item))
    isel_item:
        # match
        match_id: int
        match_list: list(match_item)
            match_item:
                type: match_inst
                root: id
                inst_name: InstXXX
                capture_list: [0, 1]
                lookup_list: [0] # lookup def

                type: predicate
                code:
                new_ops:
        # select 
        select_list: list(select_item)
            select_item:


pattern:
    match_id

    match_list: []
        type: match_inst
        root: id
        inst_name: InstXXX
        capture_list: [0, 1]
        lookup_list: [0] # lookup def

        type: predicate
        code:
        new_ops:

    select_list: []
        type: select_inst
        inst_name: InstXXX
        inst_ref_name: InstXXX
        operands:
        idx:
        used_as_operand: true/false

        type: custom
        code: 
        idx:

"""
