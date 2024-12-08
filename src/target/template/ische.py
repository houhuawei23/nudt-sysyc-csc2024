
import yaml

def loadInstScheduleInfo(file_path: str):
    with open(file_path, "r") as stream:
        try:
            isa_data = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            print(exc)
    ische_info = isa_data.get("ScheduleModel", None)
    if ische_info is None:
        print("No ScheduleModel found in the ISA file")
        return None
    
    models = dict()
    for modelName, model in ische_info.items():
        models[modelName] = dict()
        models[modelName]["classes"] = model.get("Classes", None)

    return models